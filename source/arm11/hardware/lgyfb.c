#include <stdatomic.h>
#include "types.h"
#include "arm11/hardware/lgyfb.h"
#include "arm11/hardware/interrupt.h"
#include "hardware/corelink_dma-330.h"
#include "hardware/gfx.h"
#include "lgyfb_dma330.h"


#define REG_LGYFB_TOP_CNT    *((vu32*)(0x10111000))
#define REG_LGYFB_TOP_SIZE   *((vu32*)(0x10111004))
#define REG_LGYFB_TOP_STAT   *((vu32*)(0x10111008))
#define REG_LGYFB_TOP_IRQ    *((vu32*)(0x1011100C))
#define REG_LGYFB_TOP_ALPHA  *((vu32*)(0x10111020))
#define REG_LGYFB_TOP_UNK_F0 *((vu32*)(0x101110F0))
// TODO: Add the missing regs.
#define LGYFB_TOP_FIFO       *((vu32*)(0x10311000))

#define REG_LGYFB_BOT_CNT    *((vu32*)(0x10110000))
#define REG_LGYFB_BOT_SIZE   *((vu32*)(0x10110004))
#define REG_LGYFB_BOT_STAT   *((vu32*)(0x10110008))
#define REG_LGYFB_BOT_IRQ    *((vu32*)(0x1011000C))
#define REG_LGYFB_BOT_ALPHA  *((vu32*)(0x10110020)) // 8 bit alpha for all pixels.
#define REG_LGYFB_BOT_UNK_F0 *((vu32*)(0x101100F0))
// TODO: Add the missing regs.
#define LGYFB_BOT_FIFO       *((vu32*)(0x10310000))


static bool flag = false;



static void lgyFbDmaIrqHandler(UNUSED u32 intSource)
{
	DMA330_ackIrq(0);
	DMA330_run(0, program);
	atomic_store_explicit(&flag, true, memory_order_relaxed);
}

void LGYFB_init(void)
{
	if(DMA330_run(0, program)) return;

	REG_LGYFB_TOP_SIZE  = LGYFB_SIZE(160u, 240u);
	REG_LGYFB_TOP_STAT  = LGYFB_IRQ_MASK;
	REG_LGYFB_TOP_IRQ   = 0;
	REG_LGYFB_TOP_ALPHA = 0xFF;
	REG_LGYFB_TOP_CNT   = LGYFB_DMA_E /*| LGYFB_OUT_SWIZZLE*/ | LGYFB_OUT_FMT_5551 | LGYFB_ENABLE;

	IRQ_registerHandler(IRQ_CDMA_EVENT0, 13, 0, true, lgyFbDmaIrqHandler);
}

void rotateFrame(void)
{
	GX_displayTransfer((u32*)0x18400000, 160u<<16 | 256u, (u32*)0x18200000, 160u<<16 | 256u, 3u<<12 | 3u<<8 | 1u<<1);
	GFX_waitForEvent(GFX_EVENT_PPF, false);

alignas(16) static const u8 firstList[1136] =
{
	0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x0F, 0x00, 0x00, 0x00, 0x06, 0x03,
	0x1C, 0x01, 0x2F, 0x80, 0x00, 0x00, 0x03, 0x03, 0xF0, 0xF0, 0x18, 0x01,
	0xF0, 0xF0, 0x18, 0x01, 0x6E, 0x00, 0x0F, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x16, 0x01, 0x0F, 0x00, 0x00, 0x00, 0x02, 0x00, 0x17, 0x01, 0x0F, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x1B, 0x01, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0x00,
	0x12, 0x01, 0x3F, 0x80, 0x0F, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x45, 0x00,
	0x41, 0x00, 0x3F, 0x80, 0x10, 0x11, 0x11, 0x38, 0x00, 0x90, 0x46, 0x00,
	0x14, 0xAE, 0x47, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x68, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x00, 0x2F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x29, 0x02, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x53, 0x02, 0x03, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x44, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xCB, 0x02, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x4E, 0xCC, 0x02, 0x7F, 0x00,
	0x01, 0xF0, 0x07, 0x4E, 0x02, 0x08, 0x02, 0x08, 0x03, 0x18, 0x02, 0x08,
	0x04, 0x28, 0x02, 0x08, 0x05, 0x38, 0x02, 0x08, 0x06, 0x10, 0x20, 0x4C,
	0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
	0xBF, 0x02, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xD5, 0x02, 0x0F, 0x00,
	0x6E, 0x03, 0x00, 0x00, 0xD6, 0x02, 0x6F, 0x00, 0xA1, 0x0A, 0x00, 0x00,
	0x68, 0xC3, 0x06, 0x00, 0x64, 0xC3, 0x06, 0x00, 0x62, 0xC3, 0x06, 0x00,
	0x61, 0xC3, 0x06, 0x00, 0x6F, 0x03, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x7F,
	0xBA, 0x02, 0x0F, 0x00, 0x03, 0x00, 0x00, 0x00, 0xBD, 0x02, 0x0F, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x4A, 0x02, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x51, 0x02, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00, 0x5E, 0x02, 0x01, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x4F, 0x00, 0x7F, 0x80, 0x00, 0x01, 0x02, 0x03,
	0x0C, 0x0D, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
	0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x64, 0x00, 0x0F, 0x00,
	0x01, 0x01, 0x00, 0x00, 0x6F, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x29, 0x02, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x52, 0x02, 0x0F, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x54, 0x02, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xA0,
	0x89, 0x02, 0x0F, 0x00, 0x7B, 0x00, 0x00, 0x00, 0x01, 0x02, 0x1F, 0x80,
	0x00, 0x00, 0xFC, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xA0,
	0xB9, 0x02, 0x0B, 0x00, 0x01, 0x00, 0x00, 0x00, 0x42, 0x02, 0x0F, 0x00,
	0x10, 0x00, 0x00, 0x00, 0xBB, 0x02, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x6D, 0x00, 0x0F, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x40, 0x00, 0x0F, 0x00, 0x00, 0x00, 0xBF, 0x00,
	0x4D, 0x00, 0x1F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x10, 0x00, 0x00, 0x00, 0x04, 0x01, 0x3F, 0x80, 0x10, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x10, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x26, 0x01, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x03, 0x01, 0x0F, 0x00, 0x00, 0x00, 0x76, 0x76, 0x01, 0x01, 0x0F, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x02, 0x01, 0x0F, 0x00, 0x00, 0x01, 0xE4, 0x00,
	0x00, 0x01, 0x07, 0x00, 0x00, 0x3C, 0x00, 0x80, 0x30, 0x01, 0x0F, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x62, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x18, 0x01, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x00, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x6A, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x81, 0x00, 0x4F, 0x80, 0x00, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x03, 0x02, 0x00, 0x00, 0x00,
	0x8E, 0x00, 0x0F, 0x00, 0x01, 0x10, 0x01, 0x00, 0x80, 0x00, 0x0B, 0x00,
	0x00, 0x00, 0x01, 0x00, 0x80, 0x00, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x8B, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x00, 0x07, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFD, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xE1, 0x00, 0x0F, 0x00, 0x03, 0x00, 0x03, 0x00, 0xC0, 0x00, 0x4F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0xC8, 0x00, 0x4F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0xD0, 0x00, 0x4F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0xD8, 0x00, 0x4F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0xF0, 0x00, 0x4F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x0F, 0x00, 0xF8, 0x00, 0x4F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	0x00, 0x00, 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00, 0xC0, 0x02, 0x3F, 0x80,
	0xBF, 0x00, 0x00, 0x3E, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0x02, 0x0F, 0x00,
	0x00, 0x00, 0x80, 0xBF, 0xC1, 0x02, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x88, 0x88, 0x08, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0A, 0xD7, 0xA3, 0xBB,
	0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0xFF, 0x7F, 0xB0, 0x02, 0x0F, 0x00, 0x00, 0x01, 0x00, 0x00,
	0x5E, 0x02, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x5F, 0x02, 0x0F, 0x00,
	0x00, 0x00, 0x00, 0x80, 0x27, 0x02, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x53, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x45, 0x02, 0x01, 0x00,
	0x0F, 0x00, 0x00, 0x00, 0x32, 0x02, 0x0F, 0x00, 0x3E, 0x00, 0x00, 0x00,
	0x33, 0x02, 0x2F, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x43, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80, 0x80, 0x3D, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x7C, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x33, 0x02, 0x2F, 0x80, 0x80, 0x3D, 0x00, 0x00, 0x00, 0xE0, 0x3E, 0x00,
	0x3E, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80, 0xE0, 0x46, 0x00, 0x00,
	0x00, 0x40, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80,
	0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00,
	0x33, 0x02, 0x2F, 0x80, 0xE0, 0x46, 0x00, 0x00, 0x00, 0x7C, 0x47, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80, 0x00, 0x3F, 0x00, 0x00,
	0x00, 0xE0, 0x3E, 0x00, 0x01, 0x00, 0x00, 0x00, 0x45, 0x02, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x53, 0x02, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x31, 0x02, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00, 0x11, 0x01, 0x0F, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x63, 0x00, 0x0F, 0x00, 0x78, 0x56, 0x34, 0x12, 0x10, 0x00, 0x0F, 0x00,
	0x78, 0x56, 0x34, 0x12, 0x10, 0x00, 0x0F, 0x00
};

alignas(16) static const u8 secondList[448] =
{
	0x01, 0x00, 0x00, 0x00, 0x10, 0x01, 0x0F, 0x00, 0x00, 0x00, 0x06, 0x03,
	0x1C, 0x01, 0x2F, 0x80, 0x00, 0x00, 0x03, 0x03, 0xF0, 0xF0, 0x18, 0x01,
	0xF0, 0xF0, 0x18, 0x01, 0x6E, 0x00, 0x0F, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x16, 0x01, 0x0F, 0x00, 0x00, 0x00, 0x02, 0x00, 0x17, 0x01, 0x0F, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x1B, 0x01, 0x0F, 0x00, 0x0F, 0x00, 0x00, 0x00,
	0x12, 0x01, 0x3F, 0x80, 0x0F, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00,
	0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0x45, 0x00,
	0x41, 0x00, 0x3F, 0x80, 0x10, 0x11, 0x11, 0x38, 0x00, 0x90, 0x46, 0x00,
	0x14, 0xAE, 0x47, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x68, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x65, 0x00, 0x2F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80,
	0xC0, 0x02, 0x0F, 0x00, 0x00, 0x00, 0x80, 0xBF, 0xC1, 0x02, 0xFF, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x88, 0x88, 0x08, 0x3C, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x0A, 0xD7, 0xA3, 0xBB, 0x00, 0x00, 0x80, 0xBF, 0x00, 0x00, 0x80, 0x3F,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x5E, 0x02, 0x02, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x5F, 0x02, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x80,
	0x27, 0x02, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00, 0x53, 0x02, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x45, 0x02, 0x01, 0x00, 0x0F, 0x00, 0x00, 0x00,
	0x32, 0x02, 0x0F, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x43, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x33, 0x02, 0x2F, 0x80, 0x80, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x3E, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x7C, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80,
	0x80, 0x3D, 0x00, 0x00, 0x00, 0xE0, 0x3E, 0x00, 0x3E, 0x00, 0x00, 0x00,
	0x33, 0x02, 0x2F, 0x80, 0xE0, 0x46, 0x00, 0x00, 0x00, 0x40, 0x43, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80, 0x00, 0x3F, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x33, 0x02, 0x2F, 0x80,
	0xE0, 0x46, 0x00, 0x00, 0x00, 0x7C, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x33, 0x02, 0x2F, 0x80, 0x00, 0x3F, 0x00, 0x00, 0x00, 0xE0, 0x3E, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x45, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x53, 0x02, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 0x31, 0x02, 0x0F, 0x00,
	0x01, 0x00, 0x00, 0x00, 0x11, 0x01, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00,
	0x10, 0x01, 0x0F, 0x00, 0x01, 0x00, 0x00, 0x00, 0x63, 0x00, 0x0F, 0x00,
	0x78, 0x56, 0x34, 0x12, 0x10, 0x00, 0x0F, 0x00, 0x78, 0x56, 0x34, 0x12,
	0x10, 0x00, 0x0F, 0x00
};

	static bool normalRender = false;
	u32 listSize;
	const u32 *list;
	if(normalRender == false)
	{
		normalRender = true;

		listSize = 1136;
		list = (u32*)firstList;
	}
	else
	{
		listSize = 448;
		list = (u32*)secondList;
	}
	GX_processCommandList(listSize, list);
	GFX_waitForEvent(GFX_EVENT_P3D, false);
	GX_displayTransfer((u32*)(0x18180000 + (16 * 240 * 2)), 368u<<16 | 240u,
	                   GFX_getFramebuffer(SCREEN_TOP) + (16 * 240 * 2), 368u<<16 | 240u, 3u<<12 | 3u<<8);
	GFX_waitForEvent(GFX_EVENT_PPF, false);
}

void LGYFB_processFrame(void)
{
	if(atomic_load_explicit(&flag, memory_order_relaxed))
	{
		atomic_store_explicit(&flag, false, memory_order_relaxed);

		// Rotate the 240x160 frame using the GPU.
		rotateFrame();
		GFX_swapFramebufs();
	}
}

void LGYFB_deinit(void)
{
	REG_LGYFB_TOP_CNT = 0;

	DMA330_kill(0);

	IRQ_unregisterHandler(IRQ_CDMA_EVENT0);
}
