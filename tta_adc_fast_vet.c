/*
 * tta_adc_fast_vet.c -- linear self-walking DMAC tape with tagged sensor reads
 *
 * Runtime transport:
 *
 *     for (;;) {
 *         active_request = requested_request; // { token, mux command }
 *         send(active_request.command);
 *         hide_leds_and_wait();
 *         send(*tape->payload);               // two SPI words
 *         wait_and_read_adc();
 *         expose_leds();
 *         tape = tape->next;
 *     }
 *
 * RX tags the ADC result with the exact request token that produced it:
 *
 *     discard sensor response
 *     latched_token = active_request.token
 *     discard LED responses
 *     adc_rdr = SPI_RDR
 *     completed_token = latched_token          // publication/commit marker
 *
 * The CPU publishes a request by writing command first and token last.  DMA
 * copies token first and command second.  Therefore the only torn request DMA
 * can observe is { old token, new command }, which is harmless: that sample is
 * still labelled old and a synchronous reader ignores it.  DMA cannot label an
 * old command with a new token.
 *
 * The stock reader publishes a request and waits for its exact completed token
 * before consuming the ADC mailbox.
 *
 * The LED nested loops still exist only as immutable repeated tape data.  No
 * descriptor count scales with columns, dither phases, or repeat count.
 */

#include "tta_adc_fast_vet.h"
#include "dmac.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/* SPI0 chip-select CSR indices (NPCS0/1/2). */
#ifndef CS_LEDS
#define CS_LEDS   0
#endif
#ifndef CS_SENSOR
#define CS_SENSOR 1
#endif
#ifndef CS_ADC
#define CS_ADC    2
#endif

/* LED output-enable pin (PC5 / Arduino pin 37). */
#ifndef PIN_LED_OE
#define PIN_LED_OE PIO_PC5
#endif

/* The stock Arduino linker places these DMA-visible buffers in ordinary SRAM. */
#define NFC_SRAM __attribute__((aligned(4)))
#define DMA_TX NFC_SRAM
#define DMA_RX NFC_SRAM

_Static_assert(MAXROWS == 8u, "TTA LED encoder assumes eight rows");

/* -------------------------------------------------------------------------- */
/* Configuration                                                              */
/* -------------------------------------------------------------------------- */

#define TTA_COLS_200                    26u
#define TTA_COLS_128                    17u
#define TTA_LED_PHASES                  4u
#define TTA_LED_WORDS                   2u

/* Tape-length / LED-pacing knob: how many identical tape records each LED
 * phase occupies.  No descriptor count scales with it. */
#ifndef TTA_LED_REPEATS_200
#define TTA_LED_REPEATS_200             2u
#endif
#ifndef TTA_LED_REPEATS_128
#define TTA_LED_REPEATS_128             3u
#endif
#ifndef TTA_LED_REPEATS_MAX
#define TTA_LED_REPEATS_MAX             3u   /* LS-128 uses 3, LS-200 uses 2; sizes g_tape[] */
#endif

#ifndef TTA_LED_PRE_SHIFT_BEATS
#define TTA_LED_PRE_SHIFT_BEATS         8u
#endif
#ifndef TTA_LED_POST_SHIFT_BEATS
#define TTA_LED_POST_SHIFT_BEATS        8u
#endif
#ifndef TTA_LED_SCBR
#define TTA_LED_SCBR                    4u
#endif
/* Global chip-select dead zone (MCK cycles).  Fast-Z needs this small;
 * 8 matches the fast transport experiments instead of burning ~3us per switch. */
#ifndef TTA_DLYBCS
#define TTA_DLYBCS                      0u
#endif

/* Sensor request token space.  The token only tags an ADC result with the
 * request that produced it; correctness comes from the synchronous
 * single-flight protocol, not from the token width (any space >= 3 is safe
 * because the live token set spans at most two consecutive values).  Roll it
 * fast on purpose: an 8-bit mask wraps every 255 requests, i.e. every few ms
 * at scan rates, so the wrap/reuse path is exercised constantly instead of
 * being dead code that would only wrap after ~2^32 requests (~a day of
 * continuous play, and never across boots since the counter resets on start). */
#ifndef TTA_SENSOR_TOKEN_MASK
#define TTA_SENSOR_TOKEN_MASK           0xffu
#endif

/* Tape record capacity. Sized for the largest runtime model: LS-200 needs
 * 26 x 4 x 2 = 208 records (TTA_LED_REPEATS_200 = 2), LS-128 needs 17 x 4 x 3 = 204
 * (TTA_LED_REPEATS_128 = 3). */
#define TTA_TAPE_ENTRY_MAX 208u

_Static_assert(TTA_COLS_200 * TTA_LED_PHASES * TTA_LED_REPEATS_200 <= TTA_TAPE_ENTRY_MAX,
               "TTA_TAPE_ENTRY_MAX too small for LS-200");
_Static_assert(TTA_COLS_128 * TTA_LED_PHASES * TTA_LED_REPEATS_128 <= TTA_TAPE_ENTRY_MAX,
               "TTA_TAPE_ENTRY_MAX too small for LS-128");

/* One TX iteration:
 *   0 latch requested {token,command} -> active request
 *   1 sensor target SPI from active.command
 *   2 hide / pre-shift delay
 *   3 LED payload SPI[2]
 *   4 post-shift delay
 *   5 ADC SPI
 *   6 expose
 *   7 load next tape record
 *   8 patch next LED SADDR in pristine image
 *   9 patch next tape-loader SADDR in pristine image
 *   A/B repair descriptors appended by tta_dmac_ring_close(). */
#define TTA_TX_REQ_LATCH_I              0u
#define TTA_TX_SENSOR_I                 1u
#define TTA_TX_HIDE_I                   2u
#define TTA_TX_LED_I                    3u
#define TTA_TX_POST_I                   4u
#define TTA_TX_EXPOSE_I                 5u
#define TTA_TX_ADC_I                    6u
#define TTA_TX_WALK_LOAD_I              7u
#define TTA_TX_PATCH_LED_I              8u
#define TTA_TX_PATCH_WALK_I             9u
#define TTA_TX_WORK_DESCS               10u
#define TTA_TX_TOTAL_DESCS              (TTA_TX_WORK_DESCS + 2u)

/* RX consumes the four SPI results and publishes the request token only AFTER
 * the ADC mailbox has been written. */
#define TTA_RX_SENSOR_I                 0u
#define TTA_RX_LATCH_TOKEN_I            1u
#define TTA_RX_LED_I                    2u
#define TTA_RX_ADC_I                    3u
#define TTA_RX_PUBLISH_TOKEN_I          4u
#define TTA_RX_WORK_DESCS               5u
#define TTA_RX_TOTAL_DESCS              (TTA_RX_WORK_DESCS + 2u)

_Static_assert(TTA_LED_REPEATS_200 > 0u &&
               TTA_LED_REPEATS_200 <= TTA_LED_REPEATS_MAX,
               "TTA_LED_REPEATS_200 is out of range");
_Static_assert(TTA_LED_REPEATS_128 > 0u &&
               TTA_LED_REPEATS_128 <= TTA_LED_REPEATS_MAX,
               "TTA_LED_REPEATS_128 is out of range");

/* -------------------------------------------------------------------------- */
/* Public application storage                                                 */
/* -------------------------------------------------------------------------- */

uint8_t tta_led[TTA_PAD_COUNT] NFC_SRAM;

/* -------------------------------------------------------------------------- */
/* Model                                                                      */
/* -------------------------------------------------------------------------- */

static uint8_t g_cols;
static uint8_t g_led_repeats;

/* -------------------------------------------------------------------------- */
/* Hardware word encoding                                                     */
/* -------------------------------------------------------------------------- */

#define MUX_MSB_COLBOTSW    0x80u
#define MUX_MSB_COLTOPSW    0x40u
#define MUX_MSB_COLADR4INV  0x20u
#define MUX_LSB_ADCPULLUP   0x20u

static inline uint8_t
pad_col(uint16_t pad)
{
    return (uint8_t)(pad / MAXROWS);
}
static inline uint8_t
pad_row(uint16_t pad)
{
    return (uint8_t)(pad % MAXROWS);
}
static inline uint32_t
tdr(uint16_t data, uint8_t cs)
{
    return tta_dmac_spi_word(data, cs, true);
}

static uint16_t
mux_msb_bottom(uint8_t hw_col)
{
    uint16_t v = (uint16_t)((hw_col & 0x1fu) | MUX_MSB_COLBOTSW);
    if ((hw_col & 0x10u) == 0u) v |= MUX_MSB_COLADR4INV;
    return v;
}
static uint16_t
mux_msb_top(uint8_t hw_col)
{
    uint16_t v = (uint16_t)((hw_col & 0x1fu) | MUX_MSB_COLTOPSW);
    if ((hw_col & 0x10u) == 0u) v |= MUX_MSB_COLADR4INV;
    return v;
}

static uint32_t
mux_z(uint16_t pad)
{
    const uint8_t col = pad_col(pad), row = pad_row(pad);
    const uint16_t lsb = (uint16_t)((row & 7u) | MUX_LSB_ADCPULLUP);
    /* Sensor mux uses the LOGICAL column (stock selectSensorCell behaviour). */
    return tdr((uint16_t)((lsb << 8) | mux_msb_bottom(col)),
               CS_SENSOR);
}
static uint32_t
mux_x(uint16_t pad)
{
    const uint8_t col = pad_col(pad), row = pad_row(pad);
    const uint16_t lsb = (uint16_t)((row & 7u) | 0x50u);
    return tdr((uint16_t)((lsb << 8) | mux_msb_bottom(col)),
               CS_SENSOR);
}
static uint32_t
mux_y(uint16_t pad)
{
    const uint8_t col = pad_col(pad), row = pad_row(pad);
    const uint16_t lsb = (uint16_t)((row & 7u) | 0x18u);
    return tdr((uint16_t)((lsb << 8) | mux_msb_top(col)),
               CS_SENSOR);
}

static inline uint16_t
adc_raw(uint32_t rdr)
{
    return (uint16_t)((rdr >> 2u) & 0x0fffu);
}
/* -------------------------------------------------------------------------- */
/* CPU-owned tagged sensor pivot                                              */
/* -------------------------------------------------------------------------- */

/*
 * Keep token first and command second.
 *
 * CPU publication order is the reverse: command, DMB, token.  DMA then copies
 * token followed by command.  If DMA sees the new token it is guaranteed to
 * see the command that belongs to it.  Seeing old-token/new-command is safe
 * because the consumer is waiting for the new token.
 */
typedef struct __attribute__((aligned(4))) {
    volatile uint32_t token;
    volatile uint32_t command;
} tta_sensor_request_t;

_Static_assert(sizeof(tta_sensor_request_t) == 2u * sizeof(uint32_t),
               "sensor request must remain two DMA words");

/* CPU writes g_sensor_requested.  TX snapshots it once per tape iteration. */
static volatile tta_sensor_request_t g_sensor_requested DMA_TX;
static volatile tta_sensor_request_t g_sensor_active DMA_TX;
static uint32_t g_sensor_next_token;

static uint32_t g_adc_cmd DMA_TX;

/* RX mailbox.  g_sensor_completed_token is the commit marker: RX writes the ADC
 * value first and only then publishes the token that generated it. */
static volatile uint32_t g_adc_rdr DMA_RX;
static volatile uint32_t g_rx_sink DMA_RX;
static volatile uint32_t g_rx_token_latch DMA_RX;
static volatile uint32_t g_sensor_completed_token DMA_RX;

/* -------------------------------------------------------------------------- */
/* LED framebuffer                                                            */
/* -------------------------------------------------------------------------- */

static volatile uint32_t
    g_led_words[MAXCOLS][TTA_LED_PHASES][TTA_LED_WORDS] NFC_SRAM;

/* Cap total RGB drive without changing hue.  This makes multi-channel colours
 * comparable in perceived brightness to the primary colours while retaining
 * the 5 x 5 x 5 palette. */
#ifndef TTA_LED_NORMALIZE_CAP
#define TTA_LED_NORMALIZE_CAP             6u
#endif

static uint8_t g_led_normalize_map[125];
static uint8_t g_led_low_power_map[125];
static bool g_led_low_power;

static void
build_led_normalize_map(void)
{
    for (uint16_t color = 0u; color < 125u; ++color) {
        const uint8_t r = (uint8_t)(color / 25u);
        const uint8_t g = (uint8_t)((color / 5u) % 5u);
        const uint8_t b = (uint8_t)(color % 5u);
        const uint8_t total = (uint8_t)(r + g + b);
        uint8_t nr = r, ng = g, nb = b;

        if (total > TTA_LED_NORMALIZE_CAP) {
            nr = (uint8_t)((r * TTA_LED_NORMALIZE_CAP) / total);
            ng = (uint8_t)((g * TTA_LED_NORMALIZE_CAP) / total);
            nb = (uint8_t)((b * TTA_LED_NORMALIZE_CAP) / total);
        }

        g_led_normalize_map[color] = (uint8_t)(nr * 25u + ng * 5u + nb);
        g_led_low_power_map[color] =
            (uint8_t)(((nr + 1u) / 2u) * 25u +
                      ((ng + 1u) / 2u) * 5u +
                      ((nb + 1u) / 2u));
    }
}

/* colour -> four-phase dither mask:
 *   bits 0..3  R phases, 4..7 G phases, 8..11 B phases. */
static const uint16_t g_led_dither_lut[256] = {
    0x000u, 0x100u, 0x500u, 0x700u, 0xf00u, 0x010u, 0x110u, 0x510u,
    0x710u, 0xf10u, 0x050u, 0x150u, 0x550u, 0x750u, 0xf50u, 0x070u,
    0x170u, 0x570u, 0x770u, 0xf70u, 0x0f0u, 0x1f0u, 0x5f0u, 0x7f0u,
    0xff0u, 0x001u, 0x101u, 0x501u, 0x701u, 0xf01u, 0x011u, 0x111u,
    0x511u, 0x711u, 0xf11u, 0x051u, 0x151u, 0x551u, 0x751u, 0xf51u,
    0x071u, 0x171u, 0x571u, 0x771u, 0xf71u, 0x0f1u, 0x1f1u, 0x5f1u,
    0x7f1u, 0xff1u, 0x005u, 0x105u, 0x505u, 0x705u, 0xf05u, 0x015u,
    0x115u, 0x515u, 0x715u, 0xf15u, 0x055u, 0x155u, 0x555u, 0x755u,
    0xf55u, 0x075u, 0x175u, 0x575u, 0x775u, 0xf75u, 0x0f5u, 0x1f5u,
    0x5f5u, 0x7f5u, 0xff5u, 0x007u, 0x107u, 0x507u, 0x707u, 0xf07u,
    0x017u, 0x117u, 0x517u, 0x717u, 0xf17u, 0x057u, 0x157u, 0x557u,
    0x757u, 0xf57u, 0x077u, 0x177u, 0x577u, 0x777u, 0xf77u, 0x0f7u,
    0x1f7u, 0x5f7u, 0x7f7u, 0xff7u, 0x00fu, 0x10fu, 0x50fu, 0x70fu,
    0xf0fu, 0x01fu, 0x11fu, 0x51fu, 0x71fu, 0xf1fu, 0x05fu, 0x15fu,
    0x55fu, 0x75fu, 0xf5fu, 0x07fu, 0x17fu, 0x57fu, 0x77fu, 0xf7fu,
    0x0ffu, 0x1ffu, 0x5ffu, 0x7ffu, 0xfffu, 0x000u, 0x100u, 0x500u,
    0x700u, 0xf00u, 0x010u, 0x110u, 0x510u, 0x710u, 0xf10u, 0x050u,
    0x150u, 0x550u, 0x750u, 0xf50u, 0x070u, 0x170u, 0x570u, 0x770u,
    0xf70u, 0x0f0u, 0x1f0u, 0x5f0u, 0x7f0u, 0xff0u, 0x001u, 0x101u,
    0x501u, 0x701u, 0xf01u, 0x011u, 0x111u, 0x511u, 0x711u, 0xf11u,
    0x051u, 0x151u, 0x551u, 0x751u, 0xf51u, 0x071u, 0x171u, 0x571u,
    0x771u, 0xf71u, 0x0f1u, 0x1f1u, 0x5f1u, 0x7f1u, 0xff1u, 0x005u,
    0x105u, 0x505u, 0x705u, 0xf05u, 0x015u, 0x115u, 0x515u, 0x715u,
    0xf15u, 0x055u, 0x155u, 0x555u, 0x755u, 0xf55u, 0x075u, 0x175u,
    0x575u, 0x775u, 0xf75u, 0x0f5u, 0x1f5u, 0x5f5u, 0x7f5u, 0xff5u,
    0x007u, 0x107u, 0x507u, 0x707u, 0xf07u, 0x017u, 0x117u, 0x517u,
    0x717u, 0xf17u, 0x057u, 0x157u, 0x557u, 0x757u, 0xf57u, 0x077u,
    0x177u, 0x577u, 0x777u, 0xf77u, 0x0f7u, 0x1f7u, 0x5f7u, 0x7f7u,
    0xff7u, 0x00fu, 0x10fu, 0x50fu, 0x70fu, 0xf0fu, 0x01fu, 0x11fu,
    0x51fu, 0x71fu, 0xf1fu, 0x05fu, 0x15fu, 0x55fu, 0x75fu, 0xf5fu,
    0x07fu, 0x17fu, 0x57fu, 0x77fu, 0xf7fu, 0x0ffu, 0x1ffu, 0x5ffu,
    0x7ffu, 0xfffu, 0x000u, 0x100u, 0x500u, 0x700u, 0xf00u, 0x010u,
};

/* Spread four phase bits to four bytes of 0/1. */
static const uint32_t g_led_phase_spread[16] = {
    0x00000000u, 0x00000001u, 0x00000100u, 0x00000101u,
    0x00010000u, 0x00010001u, 0x00010100u, 0x00010101u,
    0x01000000u, 0x01000001u, 0x01000100u, 0x01000101u,
    0x01010000u, 0x01010001u, 0x01010100u, 0x01010101u,
};

static void
encode_led_column(uint8_t logical_col,
                  uint32_t out[TTA_LED_PHASES][TTA_LED_WORDS])
{
    /* The LED chain is addressed by the TACTILE column number directly (stock
     * refreshLedColumn: ledColShifted = actualCol << 2).  COL_INDEX is only the
     * refresh ORDER (non-sequential column lighting), not an address remap. */
    uint8_t shifted = (uint8_t)(logical_col << 2u);
    if ((logical_col & 16u) == 0u) shifted |= 0x80u;
    const uint8_t column = (uint8_t)~shifted;

    uint32_t rphase = 0u;
    uint32_t gphase = 0u;
    uint32_t bphase = 0u;

    const uint8_t *const colours = &tta_led[(uint16_t)logical_col * MAXROWS];

    for (uint8_t row = 0u; row < MAXROWS; ++row) {
        uint8_t color = colours[row];
        if (color > 124u) color = 124u;
        const uint8_t mapped = g_led_low_power
            ? g_led_low_power_map[color]
            : g_led_normalize_map[color];
        const uint16_t d = g_led_dither_lut[mapped];
        const uint32_t bit = 1u << row;

        rphase |= g_led_phase_spread[(d >> 0u) & 0x0fu] * bit;
        gphase |= g_led_phase_spread[(d >> 4u) & 0x0fu] * bit;
        bphase |= g_led_phase_spread[(d >> 8u) & 0x0fu] * bit;
    }

    for (uint8_t phase = 0u; phase < TTA_LED_PHASES; ++phase) {
        const uint32_t shift = (uint32_t)phase * 8u;
        const uint8_t rbits = (uint8_t)(rphase >> shift);
        const uint8_t gbits = (uint8_t)(gphase >> shift);
        const uint8_t bbits = (uint8_t)(bphase >> shift);

        const uint32_t word =
            ((uint32_t)column << 24u) |
            ((uint32_t)bbits << 16u) |
            ((uint32_t)gbits << 8u) |
            rbits;

        out[phase][0] =
            tta_dmac_spi_word((uint16_t)(word >> 16u), CS_LEDS, false);
        out[phase][1] =
            tta_dmac_spi_word((uint16_t)word, CS_LEDS, true);
    }
}

/* -------------------------------------------------------------------------- */
/* The actual LED tape                                                        */
/* -------------------------------------------------------------------------- */

typedef struct __attribute__((aligned(4))) {
    uint32_t payload; /* -> uint32_t[2] SPI_TDR words */
    uint32_t next;    /* -> next tta_tape_entry_t */
} tta_tape_entry_t;

_Static_assert(sizeof(tta_tape_entry_t) == 2u * sizeof(uint32_t),
               "tape entry must remain two DMA words");

static tta_tape_entry_t g_tape[TTA_TAPE_ENTRY_MAX] DMA_TX;
static uint16_t g_tape_count;

/* DMA stages exactly one NEXT record here before patching the pristine ring. */
static volatile tta_tape_entry_t g_walk DMA_TX;

static void
build_led_words(void)
{
    for (uint8_t col = 0u; col < g_cols; ++col) {
        uint32_t words[TTA_LED_PHASES][TTA_LED_WORDS];
        encode_led_column(col, words);

        for (uint8_t phase = 0u; phase < TTA_LED_PHASES; ++phase) {
            g_led_words[col][phase][0] = words[phase][0];
            g_led_words[col][phase][1] = words[phase][1];
        }
    }
    __DMB();
}

static bool
build_tape(void)
{
    uint16_t n = 0u;

    for (uint8_t col = 0u; col < g_cols; ++col) {
        for (uint8_t phase = 0u; phase < TTA_LED_PHASES; ++phase) {
            for (uint8_t repeat = 0u; repeat < g_led_repeats; ++repeat) {
                if (n >= TTA_TAPE_ENTRY_MAX)
                    return false;
                g_tape[n].payload =
                    (uint32_t)(uintptr_t)&g_led_words[col][phase][0];
                ++n;
            }
        }
    }

    if (n == 0u)
        return false;

    g_tape_count = n;

    for (uint16_t i = 0u; i < g_tape_count; ++i) {
        const uint16_t next =
            (uint16_t)((i + 1u < g_tape_count) ? (i + 1u) : 0u);
        g_tape[i].next = (uint32_t)(uintptr_t)&g_tape[next];
    }

    g_walk = g_tape[(g_tape_count > 1u) ? 1u : 0u];
    __DMB();
    return true;
}

/* -------------------------------------------------------------------------- */
/* Tiny self-walking TX ring and RX ring                                      */
/* -------------------------------------------------------------------------- */

static tta_dmac_desc_t g_tx_clean[TTA_TX_TOTAL_DESCS] DMA_TX;
static tta_dmac_desc_t g_tx_live[TTA_TX_TOTAL_DESCS] DMA_TX;
static tta_dmac_ring_t g_tx_ring;

static tta_dmac_desc_t g_rx_clean[TTA_RX_TOTAL_DESCS] DMA_RX;
static tta_dmac_desc_t g_rx_live[TTA_RX_TOTAL_DESCS] DMA_RX;
static tta_dmac_ring_t g_rx_ring;

static uint32_t g_spin_word DMA_TX = 0x5aa5a55au;
static volatile uint32_t g_spin_sink DMA_TX;
static uint32_t g_oe_mask DMA_TX;
static uint32_t g_oe_expose_mask DMA_TX;

/* Sensor waits use this state to return cleanly before the tape starts. */
static bool g_initialized;
static volatile bool g_running;

static bool
build_tx_ring(void)
{
    tta_dmac_ring_init(&g_tx_ring, g_tx_live, g_tx_clean,
                       TTA_TX_WORK_DESCS);

    /* 0: atomically-enough snapshot the CPU publication for this iteration.
     * Source word order is token then command; see publication ordering above. */
    tta_dmac_desc_copy(&g_tx_clean[TTA_TX_REQ_LATCH_I],
                       (const volatile void *)&g_sensor_requested,
                       (volatile void *)&g_sensor_active,
                       2u, &g_tx_live[TTA_TX_SENSOR_I]);

    /* 1: use the command from the SNAPSHOT, never the live CPU request. */
    tta_dmac_desc_spi_tx(&g_tx_clean[TTA_TX_SENSOR_I],
                         (const uint32_t *)&g_sensor_active.command,
                         1u, &g_tx_live[TTA_TX_HIDE_I]);

    /* 2: hide immediately, burn deterministic AHB beats before shifting. */
    tta_dmac_desc_write(&g_tx_clean[TTA_TX_HIDE_I],
                        &g_oe_mask, &PIOC->PIO_SODR,
                        TTA_LED_PRE_SHIFT_BEATS,
                        &g_tx_live[TTA_TX_LED_I]);

    /* 3: current tape element.  This SADDR is patched in the pristine image
     * near the end of every iteration. */
    tta_dmac_desc_spi_tx(&g_tx_clean[TTA_TX_LED_I],
                         (const uint32_t *)(uintptr_t)g_tape[0].payload,
                         TTA_LED_WORDS,
                         &g_tx_live[TTA_TX_POST_I]);

    /* 4: keep outputs blank while the sensor mux settles after the LED shift. */
    tta_dmac_desc_write(&g_tx_clean[TTA_TX_POST_I],
                         &g_spin_word, &g_spin_sink,
                         TTA_LED_POST_SHIFT_BEATS,
                         &g_tx_live[TTA_TX_ADC_I]);

    /* 5: sample with LEDs blanked, avoiding LED-drive noise in the ADC read. */
    tta_dmac_desc_spi_tx(&g_tx_clean[TTA_TX_ADC_I],
                         &g_adc_cmd, 1u,
                          &g_tx_live[TTA_TX_EXPOSE_I]);

    /* 6: expose the newly shifted LED state after the ADC transfer. */
    tta_dmac_desc_write(&g_tx_clean[TTA_TX_EXPOSE_I],
                         &g_oe_expose_mask, &PIOC->PIO_CODR,
                         1u, &g_tx_live[TTA_TX_WALK_LOAD_I]);

    /* 7: prefetch NEXT LED tape record. */
    const uint16_t first_next = (g_tape_count > 1u) ? 1u : 0u;
    tta_dmac_desc_copy(&g_tx_clean[TTA_TX_WALK_LOAD_I],
                       &g_tape[first_next], (void *)&g_walk,
                       2u, &g_tx_live[TTA_TX_PATCH_LED_I]);

    /* 8: next.payload -> pristine LED descriptor SADDR. */
    tta_dmac_desc_write(&g_tx_clean[TTA_TX_PATCH_LED_I],
                        (const uint32_t *)&g_walk.payload,
                        &g_tx_clean[TTA_TX_LED_I].SADDR,
                        1u, &g_tx_live[TTA_TX_PATCH_WALK_I]);

    /* 9: next.next -> pristine tape-loader SADDR. */
    tta_dmac_desc_write(&g_tx_clean[TTA_TX_PATCH_WALK_I],
                        (const uint32_t *)&g_walk.next,
                        &g_tx_clean[TTA_TX_WALK_LOAD_I].SADDR,
                        1u, tta_dmac_ring_repair_entry(&g_tx_ring));

    return tta_dmac_ring_close(&g_tx_ring);
}

static uint32_t
ctrlb_spi_rx_fixed_dst(void)
{
    return DMAC_CTRLB_FC_PER2MEM_DMA_FC |
           DMAC_CTRLB_SRC_INCR_FIXED |
           DMAC_CTRLB_DST_INCR_FIXED |
           DMAC_CTRLB_SRC_DSCR_FETCH_FROM_MEM |
           DMAC_CTRLB_DST_DSCR_FETCH_FROM_MEM;
}

static bool
build_rx_ring(void)
{
    tta_dmac_ring_init(&g_rx_ring, g_rx_live, g_rx_clean,
                       TTA_RX_WORK_DESCS);

    /* 0: consume the sensor-shift-register response. */
    tta_dmac_desc_set(&g_rx_clean[TTA_RX_SENSOR_I],
                      &SPI0->SPI_RDR, (void *)&g_rx_sink,
                      1u, ctrlb_spi_rx_fixed_dst(),
                      &g_rx_live[TTA_RX_LATCH_TOKEN_I]);

    /* 1: latch the token immediately after the sensor command transaction.
     * This decouples RX attribution from the next iteration overwriting
     * g_sensor_active. */
    tta_dmac_desc_write(&g_rx_clean[TTA_RX_LATCH_TOKEN_I],
                        (const uint32_t *)&g_sensor_active.token,
                        &g_rx_token_latch,
                        1u, &g_rx_live[TTA_RX_LED_I]);

    /* 2: consume LED hi + LED lo. */
    tta_dmac_desc_set(&g_rx_clean[TTA_RX_LED_I],
                      &SPI0->SPI_RDR, (void *)&g_rx_sink,
                      TTA_LED_WORDS, ctrlb_spi_rx_fixed_dst(),
                      &g_rx_live[TTA_RX_ADC_I]);

    /* 3: capture ADC result FIRST. */
    tta_dmac_desc_set(&g_rx_clean[TTA_RX_ADC_I],
                      &SPI0->SPI_RDR, (void *)&g_adc_rdr,
                      1u, ctrlb_spi_rx_fixed_dst(),
                      &g_rx_live[TTA_RX_PUBLISH_TOKEN_I]);

    /* 4: publish the exact request token LAST.  Observing this token on the CPU
     * side means the ADC mailbox already contains a result for that request. */
    tta_dmac_desc_write(&g_rx_clean[TTA_RX_PUBLISH_TOKEN_I],
                        (const uint32_t *)&g_rx_token_latch,
                        &g_sensor_completed_token,
                        1u, tta_dmac_ring_repair_entry(&g_rx_ring));

    return tta_dmac_ring_close(&g_rx_ring);
}

/* -------------------------------------------------------------------------- */
/* Hardware                                                                   */
/* -------------------------------------------------------------------------- */

static void
apply_spi(void)
{
    SPI0->SPI_MR = (SPI0->SPI_MR & ~SPI_MR_DLYBCS_Msk) |
                   SPI_MR_DLYBCS(TTA_DLYBCS) |
                   SPI_MR_WDRBT;

    SPI0->SPI_CSR[CS_LEDS] =
        SPI_CSR_NCPHA | SPI_CSR_BITS_16_BIT | SPI_CSR_CSAAT |
        SPI_CSR_SCBR(TTA_LED_SCBR) | SPI_CSR_DLYBCT(0u);

    SPI0->SPI_CSR[CS_SENSOR] =
        SPI_CSR_NCPHA | SPI_CSR_BITS_16_BIT |
        SPI_CSR_SCBR(4u) | SPI_CSR_DLYBCT(0u);

    SPI0->SPI_CSR[CS_ADC] =
        SPI_CSR_NCPHA | SPI_CSR_CSNAAT | SPI_CSR_BITS_16_BIT |
        SPI_CSR_SCBR(4u) | SPI_CSR_DLYBCT(0u);
    __DMB();
}

/* -------------------------------------------------------------------------- */
/* CPU-facing sensor API                                                      */
/* -------------------------------------------------------------------------- */

static bool
sensor_pad_valid(uint16_t pad)
{
    return pad < (uint16_t)g_cols * MAXROWS;
}

static uint32_t
next_sensor_token(void)
{
    /* Deliberately masked to TTA_SENSOR_TOKEN_MASK so the counter rolls fast:
     * the wrap path is normal operation and is exercised on every session. */
    uint32_t token = (g_sensor_next_token + 1u) & TTA_SENSOR_TOKEN_MASK;
    if (token == 0u)
        token = 1u; /* reserve zero for invalid/no-request */
    g_sensor_next_token = token;
    return token;
}

static uint32_t
publish_sensor_request(uint16_t pad, uint32_t command)
{
    if (!sensor_pad_valid(pad))
        return 0u;

    const uint32_t token = next_sensor_token();

    /* Publish payload first, commit token last.  TX copies token first. */
    g_sensor_requested.command = command;
    __DMB();
    g_sensor_requested.token = token;
    __DMB();

    return token;
}

uint32_t
tta_sensor_request_z(uint16_t pad)
{
    if (!sensor_pad_valid(pad)) return 0u;
    return publish_sensor_request(pad, mux_z(pad));
}

uint32_t
tta_sensor_request_x(uint16_t pad)
{
    if (!sensor_pad_valid(pad)) return 0u;
    return publish_sensor_request(pad, mux_x(pad));
}

uint32_t
tta_sensor_request_y(uint16_t pad)
{
    if (!sensor_pad_valid(pad)) return 0u;
    return publish_sensor_request(pad, mux_y(pad));
}

bool
tta_sensor_wait_acquired_token(uint32_t token)
{
    if (token == 0u)
        return false;

    while (g_running) {
        if (g_rx_token_latch == token) {
            __DMB();
            return true;
        }
    }

    return false;
}

bool
tta_sensor_wait_token(uint32_t token)
{
    if (token == 0u)
        return false;

    /* Exact-token wait is intentional. The request register is latest-wins,
     * so callers must not supersede a token they intend to wait for. */
    while (g_running) {
        if (g_sensor_completed_token == token) {
            __DMB();
            return true;
        }
    }

    return false;
}

static uint32_t
tta_sensor_latest_rdr(void)
{
    __DMB();
    return g_adc_rdr;
}

uint16_t
tta_sensor_latest_raw(void)
{
    return adc_raw(tta_sensor_latest_rdr());
}

/* -------------------------------------------------------------------------- */
/* Lifecycle                                                                  */
/* -------------------------------------------------------------------------- */

bool
tta_init(void)
{
    if (g_running) return false;

    g_cols = (LINNMODEL == 128) ? TTA_COLS_128 : TTA_COLS_200;
    g_led_repeats = (LINNMODEL == 128)
                        ? (uint8_t)TTA_LED_REPEATS_128
                        : (uint8_t)TTA_LED_REPEATS_200;
    if (NUMCOLS != g_cols) return false;

    memset(tta_led, 0, sizeof(tta_led));
    memset((void *)g_led_words, 0, sizeof(g_led_words));
    memset(g_tape, 0, sizeof(g_tape));
    memset((void *)&g_walk, 0, sizeof(g_walk));

    g_oe_mask = PIN_LED_OE;
    g_oe_expose_mask = PIN_LED_OE;
    g_adc_cmd = tdr(0u, CS_ADC);
    g_adc_rdr = 0u;
    g_rx_sink = 0u;
    g_rx_token_latch = 0u;
    g_sensor_completed_token = 0u;

    g_sensor_next_token = 0u;
    g_sensor_requested.command = mux_z(0u);
    g_sensor_requested.token = 0u;
    g_sensor_active.command = g_sensor_requested.command;
    g_sensor_active.token = 0u;
    g_led_low_power = false;

    build_led_normalize_map();
    build_led_words();
    if (!build_tape() || !build_tx_ring() || !build_rx_ring())
        return false;

    PIOC->PIO_SODR = g_oe_mask;
    __DMB();

    g_initialized = true;
    return true;
}

static void
reset_dma_cursor(void)
{
    const uint16_t first_next = (g_tape_count > 1u) ? 1u : 0u;

    g_tx_clean[TTA_TX_LED_I].SADDR = g_tape[0].payload;
    g_tx_clean[TTA_TX_WALK_LOAD_I].SADDR =
        (uint32_t)(uintptr_t)&g_tape[first_next];
    g_walk = g_tape[first_next];

    tta_dmac_copy_desc_image(g_tx_ring.live, g_tx_ring.clean,
                             (uint16_t)(g_tx_ring.work_count + 2u));
    tta_dmac_copy_desc_image(g_rx_ring.live, g_rx_ring.clean,
                             (uint16_t)(g_rx_ring.work_count + 2u));
    __DMB();
}

bool
tta_start(void)
{
    if (g_running) return false;
    if (!g_initialized && !tta_init()) return false;

    reset_dma_cursor();
    g_adc_rdr = 0u;
    g_rx_token_latch = 0u;
    g_sensor_completed_token = 0u;
    g_sensor_active.token = 0u;

    /* Re-publish the idle request as token 0; the first real request is 1. */
    g_sensor_requested.command = mux_z(0u);
    __DMB();
    g_sensor_requested.token = 0u;
    g_sensor_next_token = 0u;
    __DMB();

    apply_spi();
    tta_dmac_init();

    g_running = true;
    __DMB();

    tta_dmac_start(tta_dmac_ring_entry(&g_tx_ring),
                   tta_dmac_ring_entry(&g_rx_ring));
    return true;
}

/* -------------------------------------------------------------------------- */
/* LED publication                                                            */
/* -------------------------------------------------------------------------- */

void
tta_led_commit(uint32_t dirty_columns)
{
    const uint32_t valid =
        (g_cols == 0u) ? 0u : ((1u << g_cols) - 1u);
    uint32_t pending = dirty_columns & valid;

    while (pending != 0u) {
        const uint8_t col = (uint8_t)__builtin_ctz(pending);
        pending &= pending - 1u;

        uint32_t words[TTA_LED_PHASES][TTA_LED_WORDS];
        encode_led_column(col, words);

        /* Fixed payload addresses keep the tape immutable.  Each 32-bit
         * store is atomic on this core/AHB path.  Publish low then high makes
         * the most likely mixed case benign on the repeated exposure. */
        for (uint8_t phase = 0u; phase < TTA_LED_PHASES; ++phase) {
            g_led_words[col][phase][1] = words[phase][1];
            g_led_words[col][phase][0] = words[phase][0];
        }
    }

    __DMB();
}

void
tta_led_set_low_power(bool enabled)
{
    if (g_led_low_power == enabled)
        return;

    g_led_low_power = enabled;
    build_led_words();
}

void
tta_led_set_enabled(bool enabled)
{
    g_oe_expose_mask = enabled ? g_oe_mask : 0u;
    __DMB();

    if (!enabled)
        PIOC->PIO_SODR = g_oe_mask;
    else if (!g_running)
        PIOC->PIO_CODR = g_oe_mask;
}
