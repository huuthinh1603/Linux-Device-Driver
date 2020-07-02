#ifndef __I2C_BBB_H__
#define __I2C_BBB_H__

#define	OMAP_I2C_IRQSTATUS		0x28
#define OMAP_I2C_IRQENABLE_SET		0x2C
#define OMAP_I2C_IRQENABLE_CLR		0x30
#define OMAP_I2C_SYSS			0x90
#define OMAP_I2C_BUF			0x94
#define OMAP_I2C_CNT			0x98
#define OMAP_I2C_DATA			0x9C
#define OMAP_I2C_CON			0xA4
#define OMAP_I2C_SA			0xAC
#define OMAP_I2C_PSC			0xB0
#define OMAP_I2C_SCLL			0xB4
#define OMAP_I2C_SCLH			0xB8
#define OMAP_I2C_SYSTEST		0xBC
#define OMAP_I2C_BUFSTAT		0xC0

/* I2C Interrupt Register */
#define OMAP_I2C_IRQ_XDR		(1 << 14)
#define OMAP_I2C_IRQ_RDR		(1 << 13)
#define OMAP_I2C_IRQ_BB			(1 << 12)
#define OMAP_I2C_IRQ_XRDY		(1 << 4)
#define OMAP_I2C_IRQ_RRDY		(1 << 3)
#define OMAP_I2C_IRQ_ARDY		(1 << 2)
#define OMAP_I2C_IRQ_NACK		(1 << 1)

/* System Status Register */
#define OMAP_I2C_SYSS_RDONE		(1 << 0)

/* Buffer Configuration Register */
#define OMAP_I2C_BUF_RXFIFO_CLR		(1 << 14)
#define OMAP_I2C_BUF_RXTRSH(val)	(val << 8)
#define OMAP_I2C_BUF_TXFIFO_CLR		(1 << 6)
#define OMAP_I2C_BUF_TXTRSH(val)	(val << 0)

/* I2C Configuration Register */
#define OMAP_I2C_CON_EN			(1 << 15)
#define OMAP_I2C_CON_OPMODE(val)	(val << 12)
#define OMAP_I2C_CON_STB		(1 << 11)
#define OMAP_I2C_CON_MST		(1 << 10)
#define OMAP_I2C_CON_TRX		(1 << 9)
#define OMAP_I2C_CON_XSA		(1 << 8)
#define OMAP_I2C_CON_STP		(1 << 1)
#define OMAP_I2C_CON_STT		(1 << 0)

#define SCLK_CLOCK			48000000
#define THRESHOULD_SIZE			32
#define MAX_NUM_MESSAGE_TRANFER         128
#define MAX_NUM_LENGTH_MESSAGE          512
#define TIMEOUT                         (0 * HZ)
#define RETRIES_NUM			0

#endif /* __I2C_BBB_H__ */
