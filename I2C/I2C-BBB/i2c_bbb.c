#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include "i2c_bbb.h"

struct i2c_bbb {
	struct i2c_adapter adap;
	struct device *dev;
	struct wait_queue_head wait_queue;
	atomic_t condition;
	void __iomem *addr;
	int irq;
	unsigned int flags;
	unsigned char *data;
};

enum speed {
	STANDARD_MODE = 0,
	FAST_MODE,
};

static void __i2c_bbb_speed(struct i2c_bbb *bbb, int clock, enum speed flag)
{
	int scll = 0, sclh = 0;
	int speed = 0;

	if(flag == STANDARD_MODE)
		speed = 100000;
	else
		speed = 400000;

	scll = (((clock / (speed * 2)) - 7) & 0xFF);
	sclh = (((clock / (speed * 2)) - 5) & 0xFF);

	writel(scll, bbb->addr + OMAP_I2C_SCLL);
	writel(sclh, bbb->addr + OMAP_I2C_SCLH);
}

static int i2c_bbb_speed(struct i2c_bbb *bbb, int prescale, enum speed flag)
{
	int clock_speed = SCLK_CLOCK / prescale;

	if((!bbb) || (flag != STANDARD_MODE && flag != FAST_MODE))
		return -EINVAL;

	if(prescale < 1 || prescale > 256)
		return -EINVAL;

	writel((prescale - 1), bbb->addr + OMAP_I2C_PSC);

	__i2c_bbb_speed(bbb, clock_speed, flag);

	return 0;
}

static int i2c_bbb_select_irq(struct i2c_bbb *bbb, int flags)
{
	if(!bbb)
		return -EINVAL;

	writel(0x6FFF, bbb->addr + OMAP_I2C_IRQENABLE_CLR);
	flags &= 0x6FFF;
	writel(flags, bbb->addr + OMAP_I2C_IRQENABLE_SET);

	return 0;
}

static int i2c_bbb_enable(struct i2c_bbb *bbb)
{
	int val = 0;

	if(!bbb)
                return -EINVAL;

	val = readl(bbb->addr + OMAP_I2C_CON);
	val |= OMAP_I2C_CON_EN;
	val &= ~OMAP_I2C_CON_OPMODE(3) & ~OMAP_I2C_CON_STB;
	
	writel(val, bbb->addr + OMAP_I2C_CON);

	return 0;
}

static void i2c_bbb_disable(struct i2c_bbb *bbb)
{
	int val;
	
	val = readl(bbb->addr + OMAP_I2C_CON);
	val &= ~OMAP_I2C_CON_EN;

	writel(val, bbb->addr + OMAP_I2C_CON);
}

static int i2c_bbb_set_threshold(struct i2c_bbb *bbb, int sh_val)
{
	int val;

	if(!bbb)
                return -EINVAL;
	val = readl(bbb->addr + OMAP_I2C_BUF);

	val |= OMAP_I2C_BUF_RXTRSH((sh_val - 1)) | OMAP_I2C_BUF_TXTRSH((sh_val - 1));
	writel(val, bbb->addr + OMAP_I2C_BUF);

	return 0;
}

static irqreturn_t i2c_bbb_irq_handler(int irq, void *args)
{	
	struct i2c_bbb *bbb = (struct i2c_bbb *)args;
	int flags;

	if(!bbb || irq != bbb->irq)
		return IRQ_NONE;

	flags = readl(bbb->addr + OMAP_I2C_IRQSTATUS);
	if(!(flags & (OMAP_I2C_IRQ_XDR | OMAP_I2C_IRQ_RDR | OMAP_I2C_IRQ_XRDY | 
				OMAP_I2C_IRQ_RRDY | OMAP_I2C_IRQ_NACK)))
		return IRQ_NONE;
	
	return IRQ_WAKE_THREAD;
}

static irqreturn_t i2c_bbb_irq_thread_handler(int irq, void *args)
{

	struct i2c_bbb *bbb = (struct i2c_bbb *)args;
        int flags, i = 0, val = 0;

        if(!bbb || irq != bbb->irq)
                return IRQ_NONE;

        flags = readl(bbb->addr + OMAP_I2C_IRQSTATUS);

	if(flags & OMAP_I2C_IRQ_XDR) {
                val = readl(bbb->addr + OMAP_I2C_BUFSTAT);
                val &= (0x3F << 0);

                for(i = 0; i < val; i++)
                        writel(*bbb->data++, bbb->addr + OMAP_I2C_DATA);

                writel(OMAP_I2C_IRQ_XDR, bbb->addr + OMAP_I2C_IRQSTATUS);

                atomic_set(&bbb->condition, 1);
                wake_up_interruptible(&bbb->wait_queue);
        } else if (flags & OMAP_I2C_IRQ_RDR) {
                val = readl(bbb->addr + OMAP_I2C_BUFSTAT);
                val &= (0x3F << 8);

                for(i = 0; i < val; i++)
                        *bbb->data++ = readl(bbb->addr + OMAP_I2C_DATA);

                writel(OMAP_I2C_IRQ_RDR, bbb->addr + OMAP_I2C_IRQSTATUS);
                
		atomic_set(&bbb->condition, 1);
                wake_up_interruptible(&bbb->wait_queue);
        } else if (flags & OMAP_I2C_IRQ_XRDY) {
                for(i = 0; i <= THRESHOULD_SIZE; i++)
                        writel(*bbb->data++, bbb->addr + OMAP_I2C_DATA);

                writel(OMAP_I2C_IRQ_XRDY, bbb->addr + OMAP_I2C_IRQSTATUS);

                val = readl(bbb->addr + OMAP_I2C_BUFSTAT);
                val &= (0x3F << 0);
                if(!val) {
                        atomic_set(&bbb->condition, 1);
                        wake_up_interruptible(&bbb->wait_queue);
                }
        } else if(flags & OMAP_I2C_IRQ_RRDY) {
                for(i = 0; i <= THRESHOULD_SIZE; i++)
                        *bbb->data++ = readl(bbb->addr + OMAP_I2C_DATA);

                writel(OMAP_I2C_IRQ_RRDY, bbb->addr + OMAP_I2C_IRQSTATUS);

                val = readl(bbb->addr + OMAP_I2C_BUFSTAT);
                val &= (0x3F << 8);
                if(!val) {
                        atomic_set(&bbb->condition, 1);
                        wake_up_interruptible(&bbb->wait_queue);
                }
        } else if(flags & OMAP_I2C_IRQ_NACK) {

		writel(OMAP_I2C_IRQ_NACK, bbb->addr + OMAP_I2C_IRQSTATUS);
		atomic_set(&bbb->condition, 1);
                wake_up_interruptible(&bbb->wait_queue);
	}

	return IRQ_HANDLED;
}
static int __i2c_bbb_master_xfer(struct i2c_bbb *bbb, struct i2c_msg *msg)
{
	int val = 0;

	if(!bbb || !msg)
		return -EINVAL;

	val = readl(bbb->addr + OMAP_I2C_CON);

	if(msg->flags & I2C_M_RD) {
		val &= ~OMAP_I2C_CON_TRX;
		/* Clear RX Buffer */
		writel(OMAP_I2C_BUF_RXFIFO_CLR, bbb->addr + OMAP_I2C_BUF);
	} else {
		val |= OMAP_I2C_CON_TRX;
		/* Clear TX Buffer */
		writel(OMAP_I2C_BUF_TXFIFO_CLR, bbb->addr + OMAP_I2C_BUF);
	}

	if(msg->flags & I2C_M_TEN)
		val |= OMAP_I2C_CON_XSA;
	else
		val &= ~OMAP_I2C_CON_XSA;

	/* Master Mode */
	val |= OMAP_I2C_CON_MST;
	
	writel(val, bbb->addr + OMAP_I2C_CON);

	/* Slave Address */
	writel(msg->addr, bbb->addr + OMAP_I2C_SA);

	/* Data lenght */
	writel(msg->len, bbb->addr + OMAP_I2C_CNT);

	bbb->data = msg->buf;

	/* Generate Start & Stop Condition */
	val = readl(bbb->addr + OMAP_I2C_CON);
	val |= OMAP_I2C_CON_STT | OMAP_I2C_CON_STP;
	writel(val, bbb->addr + OMAP_I2C_CON);
	
	atomic_set(&bbb->condition, 0);
	wait_event_interruptible(bbb->wait_queue, atomic_read(&bbb->condition));

	return 0;
}

static int i2c_bbb_master_xfer(struct i2c_adapter *adap, 
				struct i2c_msg *msgs, int num)
{
	struct i2c_bbb *bbb = i2c_get_adapdata(adap);
	int i = 0, count = 0, retval = 0;

	pm_runtime_get_sync(bbb->dev);

	/* Checking the i2c bus */
	if(readl(bbb->addr + OMAP_I2C_IRQSTATUS) & OMAP_I2C_IRQ_BB)
		return -EBUSY;

	for(i = 0; i < num; i++, msgs++) {
		retval = __i2c_bbb_master_xfer(bbb, msgs);
		if(!retval)
			count++;
	}
	pm_runtime_put_sync(bbb->dev);
	return count;
}

static int i2c_bbb_smbus_xfer(struct i2c_adapter *adap, u16 addr,
			   unsigned short flags, char read_write,
			   u8 command, int size, union i2c_smbus_data *data)
{
	pr_info("Huu Thinh:     %s\t%d\n", __func__, __LINE__);
	return 0;
}

static u32 i2c_bbb_functionality(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_PROTOCOL_MANGLING;
}

static struct i2c_algorithm i2c_bbb_algorithm = {
	.master_xfer	= i2c_bbb_master_xfer,
	.smbus_xfer	= i2c_bbb_smbus_xfer,
	.functionality	= i2c_bbb_functionality
};

static int i2c_bbb_get_scl(struct i2c_adapter *adap)
{
	pr_info("Huu Thinh:     %s\t%d\n", __func__, __LINE__);
	return 0;
}

static void i2c_bbb_set_scl(struct i2c_adapter *adap, int val)
{
	pr_info("Huu Thinh:     %s\t%d\n", __func__, __LINE__);
}

static int i2c_bbb_get_sda(struct i2c_adapter *adap)
{
	pr_info("Huu Thinh:     %s\t%d\n", __func__, __LINE__);
	return 0;
}

static struct i2c_bus_recovery_info i2c_bbb_recovery_info = {
	.recover_bus	= i2c_generic_scl_recovery,
	.get_scl	= i2c_bbb_get_scl,
	.set_scl	= i2c_bbb_set_scl,
	.get_sda	= i2c_bbb_get_sda,
};

static struct i2c_adapter_quirks i2c_bbb_quirks = {
	.flags	= 0,
	.max_num_msgs	= MAX_NUM_MESSAGE_TRANFER,
	.max_write_len	= MAX_NUM_LENGTH_MESSAGE,
	.max_read_len	= MAX_NUM_LENGTH_MESSAGE
};

static int i2c_bbb_plf_probe(struct platform_device *pdev) 
{
	struct i2c_bbb *bbb;
	struct i2c_adapter *adap;
	struct resource *res;
	void __iomem *addr;
	int retval, irq;

	bbb = devm_kzalloc(&pdev->dev, sizeof(*bbb), GFP_KERNEL);
	if(!bbb) {
		pr_err("Memory allocation has failed\n");
		return -ENOMEM;
	}

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	addr = devm_ioremap_resource(&pdev->dev, res);
	if(IS_ERR(addr)) {
		retval = PTR_ERR(addr);
		goto ioremap_fail;
	}

	irq = platform_get_irq(pdev, 0);
	retval = devm_request_threaded_irq(&pdev->dev, irq, i2c_bbb_irq_handler, 
			i2c_bbb_irq_thread_handler, IRQF_ONESHOT, "i2c_bbbb", bbb);
	if(retval) {
		pr_err("Registerring the IRQ handler failed\n");
		goto irq_fail;
	}

	bbb->addr = addr;
	bbb->irq = irq;
	bbb->dev = &pdev->dev;

	adap = &bbb->adap;
	adap->owner = THIS_MODULE;
	adap->class = I2C_CLASS_DEPRECATED;
	adap->timeout = TIMEOUT;
	adap->retries = RETRIES_NUM;
	strncpy(adap->name, "I2C BeagleBone Black", sizeof(adap->name));
	adap->dev.parent = &pdev->dev;
	adap->dev.of_node = pdev->dev.of_node;
	adap->algo = &i2c_bbb_algorithm;
	adap->bus_recovery_info = &i2c_bbb_recovery_info;
	adap->quirks = &i2c_bbb_quirks;

	i2c_set_adapdata(adap, (void *)bbb);
	platform_set_drvdata(pdev, (void*)bbb);

	/* Initialize the wait queue */
	init_waitqueue_head(&bbb->wait_queue);

	retval = i2c_add_adapter(adap);
	if(retval) {
		pr_err("The i2c adapter registation has failed\n");
		goto add_adap_fail;
	}
	
	pm_runtime_enable(bbb->dev);
	pm_runtime_get_sync(bbb->dev);

	writel_relaxed(0, bbb->addr + OMAP_I2C_CON);

	i2c_bbb_speed(bbb, 4, FAST_MODE);

	i2c_bbb_set_threshold(bbb, THRESHOULD_SIZE);

	i2c_bbb_enable(bbb);

	i2c_bbb_select_irq(bbb, OMAP_I2C_IRQ_XDR | OMAP_I2C_IRQ_RDR | OMAP_I2C_IRQ_XRDY |
			OMAP_I2C_IRQ_RRDY | OMAP_I2C_IRQ_NACK);

	pm_runtime_put_sync(bbb->dev);

	pr_info("Huu Thinh:	%s\t%d\n", __func__, __LINE__);
	return 0;

add_adap_fail:
irq_fail:
	devm_iounmap(&pdev->dev, addr);
ioremap_fail:
	devm_kfree(&pdev->dev, bbb);
	return retval;
}

static int i2c_bbb_plf_remove(struct platform_device *pdev)
{
	struct i2c_bbb *bbb;

	bbb = (struct i2c_bbb *)platform_get_drvdata(pdev);

	pm_runtime_get_sync(bbb->dev);

	i2c_bbb_disable(bbb);
	i2c_del_adapter(&bbb->adap);

	pm_runtime_put_sync(bbb->dev);
	pm_runtime_disable(bbb->dev);

	pr_info("Huu Thinh:     %s\t%d\n", __func__, __LINE__);
	return 0;
}

static struct of_device_id i2c_bbb_plf_of_id[] = {
	{.compatible = "i2c_bbb"},
	{ }
};
MODULE_DEVICE_TABLE(of, i2c_bbb_plf_of_id);


static struct platform_driver i2c_bbb_plf_driver = {
	.probe		= i2c_bbb_plf_probe,
	.remove		= i2c_bbb_plf_remove,
	.driver		= {
		.name	= "i2c_bbb",
		.of_match_table = i2c_bbb_plf_of_id,
	},
};

module_platform_driver(i2c_bbb_plf_driver);

MODULE_LICENSE("GPL");
