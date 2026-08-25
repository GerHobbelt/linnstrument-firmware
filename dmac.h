#ifndef TTA_DMAC_H
#define TTA_DMAC_H

/*
 * dmac.h
 *
 * Small SAM3X DMAC vocabulary used by TTA.
 *
 * The only non-obvious primitive here is tta_dmac_ring_t.  SAM3X writes CTRLA
 * back to completed linked-list descriptors, consuming BTSIZE.  A ring therefore
 * keeps a pristine image plus a live image.  Its final two live descriptors are:
 *
 *      A: restore work descriptors + B, then execute B
 *      B: restore A, then execute work[0]
 *
 * TTA uses that for its small LED TX call table.  No CPU descriptor repair is
 * required.
 */

#include "tta_adc_fast_vet.h"
#include <sam.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef TTA_DMAC_TX_CH
#define TTA_DMAC_TX_CH 0u
#endif
#ifndef TTA_DMAC_RX_CH
#define TTA_DMAC_RX_CH 1u
#endif
#ifndef TTA_DMAC_SPI0_TX_INTERFACE
#define TTA_DMAC_SPI0_TX_INTERFACE 1u
#endif
#ifndef TTA_DMAC_SPI0_RX_INTERFACE
#define TTA_DMAC_SPI0_RX_INTERFACE 2u
#endif

#define TTA_DMAC_DESC_WORDS 5u
#define TTA_DMAC_MAX_BTSIZE 4095u

typedef struct __attribute__((aligned(4))) {
    volatile uint32_t SADDR;
    volatile uint32_t DADDR;
    volatile uint32_t CTRLA;
    volatile uint32_t CTRLB;
    volatile uint32_t DSCR;
} tta_dmac_desc_t;

_Static_assert(sizeof(tta_dmac_desc_t) == 20u,
               "SAM3X DMAC descriptor must be five words");
_Static_assert(_Alignof(tta_dmac_desc_t) == 4u,
               "SAM3X DMAC descriptor must be word aligned");

static inline uint32_t
tta_dmac_ctrla(uint32_t words)
{
    return DMAC_CTRLA_BTSIZE(words) |
           DMAC_CTRLA_SCSIZE_CHK_1 |
           DMAC_CTRLA_DCSIZE_CHK_1 |
           DMAC_CTRLA_SRC_WIDTH_WORD |
           DMAC_CTRLA_DST_WIDTH_WORD;
}

static inline uint32_t
tta_dmac_ctrlb_m2m(bool src_inc, bool dst_inc)
{
    return DMAC_CTRLB_FC_MEM2MEM_DMA_FC |
           (src_inc ? DMAC_CTRLB_SRC_INCR_INCREMENTING
                    : DMAC_CTRLB_SRC_INCR_FIXED) |
           (dst_inc ? DMAC_CTRLB_DST_INCR_INCREMENTING
                    : DMAC_CTRLB_DST_INCR_FIXED) |
           DMAC_CTRLB_SRC_DSCR_FETCH_FROM_MEM |
           DMAC_CTRLB_DST_DSCR_FETCH_FROM_MEM;
}

static inline uint32_t
tta_dmac_ctrlb_spi_tx(void)
{
    return DMAC_CTRLB_FC_MEM2PER_DMA_FC |
           DMAC_CTRLB_SRC_INCR_INCREMENTING |
           DMAC_CTRLB_DST_INCR_FIXED |
           DMAC_CTRLB_SRC_DSCR_FETCH_FROM_MEM |
           DMAC_CTRLB_DST_DSCR_FETCH_FROM_MEM;
}

static inline void
tta_dmac_desc_set(tta_dmac_desc_t *d,
                  const volatile void *src,
                  volatile void *dst,
                  uint32_t words,
                  uint32_t ctrlb,
                  tta_dmac_desc_t *next)
{
    d->SADDR = (uint32_t)(uintptr_t)src;
    d->DADDR = (uint32_t)(uintptr_t)dst;
    d->CTRLA = tta_dmac_ctrla(words);
    d->CTRLB = ctrlb;
    d->DSCR = (uint32_t)(uintptr_t)next;
}

static inline void
tta_dmac_desc_copy(tta_dmac_desc_t *d,
                   const volatile void *src,
                   volatile void *dst,
                   uint32_t words,
                   tta_dmac_desc_t *next)
{
    tta_dmac_desc_set(d, src, dst, words,
                      tta_dmac_ctrlb_m2m(true, true), next);
}

static inline void
tta_dmac_desc_write(tta_dmac_desc_t *d,
                    const volatile uint32_t *src,
                    volatile uint32_t *dst,
                    uint32_t beats,
                    tta_dmac_desc_t *next)
{
    tta_dmac_desc_set(d, src, dst, beats,
                      tta_dmac_ctrlb_m2m(false, false), next);
}

static inline void
tta_dmac_desc_spi_tx(tta_dmac_desc_t *d,
                     const uint32_t *src,
                     uint32_t words,
                     tta_dmac_desc_t *next)
{
    tta_dmac_desc_set(d, src, &SPI0->SPI_TDR, words,
                      tta_dmac_ctrlb_spi_tx(), next);
}

static inline void
tta_dmac_copy_desc_image(tta_dmac_desc_t *dst,
                         const tta_dmac_desc_t *src,
                         uint16_t count)
{
    volatile uint32_t *d = (volatile uint32_t *)dst;
    const uint32_t *s = (const uint32_t *)src;
    const uint32_t words = (uint32_t)count * TTA_DMAC_DESC_WORDS;
    for (uint32_t i = 0u; i < words; ++i)
        d[i] = s[i];
    __DMB();
}

/* Self-repairing outer ring.  live/clean need room for work_count + 2 entries.
 * Layout is [work ...][B][A], which lets A restore work+B contiguously. */
typedef struct {
    tta_dmac_desc_t *live;
    tta_dmac_desc_t *clean;
    uint16_t work_count;
} tta_dmac_ring_t;

static inline void
tta_dmac_ring_init(tta_dmac_ring_t *r,
                   tta_dmac_desc_t *live,
                   tta_dmac_desc_t *clean,
                   uint16_t work_count)
{
    r->live = live;
    r->clean = clean;
    r->work_count = work_count;
}

static inline tta_dmac_desc_t *
tta_dmac_ring_entry(const tta_dmac_ring_t *r)
{
    return &r->live[0];
}

static inline tta_dmac_desc_t *
tta_dmac_ring_repair_entry(const tta_dmac_ring_t *r)
{
    return &r->live[r->work_count + 1u]; /* A */
}

static inline bool
tta_dmac_ring_close(tta_dmac_ring_t *r)
{
    if (r == NULL || r->live == NULL || r->clean == NULL ||
        r->work_count == 0u)
        return false;

    const uint16_t b = r->work_count;
    const uint16_t a = (uint16_t)(r->work_count + 1u);
    const uint32_t restore_words =
        (uint32_t)(r->work_count + 1u) * TTA_DMAC_DESC_WORDS;

    if (restore_words > TTA_DMAC_MAX_BTSIZE)
        return false;

    /* A restores every work descriptor plus B, then runs freshly-restored B. */
    tta_dmac_desc_copy(&r->clean[a],
                       &r->clean[0],
                       &r->live[0],
                       restore_words,
                       &r->live[b]);

    /* B restores A, then starts the next lap. */
    tta_dmac_desc_copy(&r->clean[b],
                       &r->clean[a],
                       &r->live[a],
                       TTA_DMAC_DESC_WORDS,
                       &r->live[0]);

    tta_dmac_copy_desc_image(r->live, r->clean,
                             (uint16_t)(r->work_count + 2u));
    return true;
}

static inline uint32_t
tta_dmac_spi_word(uint16_t data, uint8_t npcs, bool last)
{
    return (uint32_t)data |
           SPI_TDR_PCS((~(1u << npcs)) & 0x0fu) |
           (last ? SPI_TDR_LASTXFER : 0u);
}

static inline void
tta_dmac_init(void)
{
    PMC->PMC_PCER1 = PMC_PCER1_PID39;
    DMAC->DMAC_EN = DMAC_EN_ENABLE;

    DMAC->DMAC_EBCIDR = 0xffffffffu;
    NVIC_DisableIRQ(DMAC_IRQn);
    NVIC_ClearPendingIRQ(DMAC_IRQn);
    (void)DMAC->DMAC_EBCISR;

#ifdef DMAC_GCFG_ARB_CFG_ROUND_ROBIN
    DMAC->DMAC_GCFG = DMAC_GCFG_ARB_CFG_ROUND_ROBIN;
#endif

    DMAC->DMAC_CH_NUM[TTA_DMAC_TX_CH].DMAC_CFG =
        DMAC_CFG_DST_PER(TTA_DMAC_SPI0_TX_INTERFACE) |
        DMAC_CFG_DST_H2SEL_HW |
        DMAC_CFG_SOD_DISABLE |
        DMAC_CFG_FIFOCFG_ASAP_CFG;

    DMAC->DMAC_CH_NUM[TTA_DMAC_RX_CH].DMAC_CFG =
        DMAC_CFG_SRC_PER(TTA_DMAC_SPI0_RX_INTERFACE) |
        DMAC_CFG_SRC_H2SEL_HW |
        DMAC_CFG_SOD_DISABLE |
        DMAC_CFG_FIFOCFG_ASAP_CFG;
}

static inline void
tta_dmac_start(tta_dmac_desc_t *tx_entry,
               tta_dmac_desc_t *rx_entry)
{
    DmacCh_num *tx = &DMAC->DMAC_CH_NUM[TTA_DMAC_TX_CH];
    DmacCh_num *rx = &DMAC->DMAC_CH_NUM[TTA_DMAC_RX_CH];

    while ((SPI0->SPI_SR & SPI_SR_TXEMPTY) == 0u) {
    }
    while ((SPI0->SPI_SR & SPI_SR_RDRF) != 0u)
        (void)SPI0->SPI_RDR;
    (void)DMAC->DMAC_EBCISR;

    tx->DMAC_CTRLB = tta_dmac_ctrlb_m2m(true, true);
    tx->DMAC_DSCR = (uint32_t)(uintptr_t)tx_entry;
    if (rx_entry != NULL) {
        rx->DMAC_CTRLB = tta_dmac_ctrlb_m2m(true, true);
        rx->DMAC_DSCR = (uint32_t)(uintptr_t)rx_entry;
    }
    __DMB();

    /* RX waits first; WDRBT prevents TX from overrunning RDR. */
    if (rx_entry != NULL)
        DMAC->DMAC_CHER = (1u << TTA_DMAC_RX_CH);
    DMAC->DMAC_CHER = (1u << TTA_DMAC_TX_CH);
}

#endif /* TTA_DMAC_H */
