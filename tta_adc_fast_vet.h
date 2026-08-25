#ifndef TTA_H
#define TTA_H

#include <stdbool.h>
#include <stdint.h>

/* The Arduino sketch defines these too; keep the C tape backend independent
 * of the sketch's generated translation unit. */
#ifndef MAXCOLS
#define MAXCOLS 26
#endif
#ifndef MAXROWS
#define MAXROWS 8
#endif

extern uint8_t LINNMODEL;
extern uint8_t NUMCOLS;

#ifdef __cplusplus
extern "C" {
#endif

#define TTA_PAD_COUNT ((uint16_t)(MAXCOLS * MAXROWS))

/* Application state: one flat LED buffer. */
extern uint8_t tta_led[TTA_PAD_COUNT];

/*
 * Linear self-walking DMAC tape (adapted from nextgen-linnstrument):
 *
 *   for (;;) {
 *       send(cpu_selected_sensor);   // g_sensor_cmd, one aligned word
 *       hide_leds_and_wait();
 *       send(tape->payload);         // two SPI words
 *       wait_and_expose_leds();
 *       read_adc();                  // latest value -> mailbox
 *       tape = tape->next;           // self-walk via g_walk + patches
 *   }
 *
 * The active TX program is ten work descriptors plus the A/B repair pair;
 * RX has five work descriptors plus its repair pair. LED repetition is
 * literal repeated data pointers (the tape array is built once as column x
 * dither_phase x repeat), never repeated descriptors.
 */

bool tta_init(void);
bool tta_start(void);

uint16_t tta_sensor_latest_raw(void);   /* 12-bit, non-inverted */

void tta_led_commit(uint32_t dirty_columns);
void tta_led_set_enabled(bool enabled);
void tta_led_set_low_power(bool enabled);

/* Tagged sensor request API. Token 0 means invalid/no request. Tokens live in
 * a small rolling space (see TTA_SENSOR_TOKEN_MASK in tta_adc_fast_vet.c) so
 * the wrap/reuse path is exercised constantly; callers must keep the
 * synchronous single-flight discipline: publish, then wait for that token,
 * before publishing again. */
uint32_t tta_sensor_request_z(uint16_t pad);
uint32_t tta_sensor_request_x(uint16_t pad);
uint32_t tta_sensor_request_y(uint16_t pad);

/* requested -> acquired (MUX command sent) -> completed (ADC mailbox written) */
bool tta_sensor_wait_acquired_token(uint32_t token);
bool tta_sensor_wait_token(uint32_t token);

#ifdef __cplusplus
}
#endif

#endif /* TTA_H */
