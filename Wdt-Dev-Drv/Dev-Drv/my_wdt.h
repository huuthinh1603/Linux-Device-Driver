#ifndef __MY_WDT_H__
#define __MY_WDT_H__

#define OMAP_WDT_WDSC		0x10
#define OMAP_WDT_WDST		0x14
#define OMAP_WDT_WCLR		0x24
#define OMAP_WDT_WCRR		0x28
#define OMAP_WDT_WLDR		0x2C
#define OMAP_WDT_WTGR		0x30
#define OMAP_WDT_WWPS		0x34
#define OMAP_WDT_WSPR		0x48

#define OMAP_RESETDONE		(1 << 0)

#define OMAP_W_PEND_WSPR	(1 << 4)
#define OMAP_W_PEND_WTGR	(1 << 3)
#define OMAP_W_PEND_WLDR	(1 << 2)
#define OMAP_W_PEND_WCLR	(1 << 0)

#define SECOND_TO_VAL(sec)	((32768 / (1 << 5)) * sec)
#define GET_WLDR_VAL(sec)	(0xFFFFFFFF - SECOND_TO_VAL(sec) + 1)
#define VAL_TO_SECOND(val)	((0xFFFFFFFF - val) / ((32768 / (1 << 5))))

#endif /* __MY_WDT_H__ */
