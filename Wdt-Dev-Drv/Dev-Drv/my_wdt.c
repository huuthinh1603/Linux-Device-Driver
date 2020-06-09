#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/watchdog.h>
#include <linux/slab.h>
#include <asm/io.h>
#include <linux/pm_runtime.h>
#include <linux/mod_devicetable.h>
#include <linux/platform_device.h>
#include "my_wdt.h"

#define AUTHOR	"Huu Thinh <huuthinh1603@gmail.com>"
#define DESC	"This module is a wdt driver"

#define MIN_TIMEOUT	10
#define MAX_TIMEOUT	600
#define TTGR_VAL	0x1234

struct omap_wdt_dev {
	struct watchdog_device wdev;
	struct device *dev;
	void __iomem *base_addr;
	uint32_t ttgr_pattern;	
};

static void omap_wdt_enable(struct omap_wdt_dev *omap_wdev)
{
        writel_relaxed(0xBBBB, omap_wdev->base_addr + OMAP_WDT_WSPR);
        while((readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WWPS) & OMAP_W_PEND_WSPR))
                cpu_relax();

        writel_relaxed(0x4444, omap_wdev->base_addr + OMAP_WDT_WSPR);
        while((readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WWPS) & OMAP_W_PEND_WSPR))
                cpu_relax();
}

static void omap_wdt_disable(struct omap_wdt_dev *omap_wdev)
{
	writel_relaxed(0xAAAA, omap_wdev->base_addr + OMAP_WDT_WSPR);
	while((readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WWPS) & OMAP_W_PEND_WSPR))
                cpu_relax();

	writel_relaxed(0x5555, omap_wdev->base_addr + OMAP_WDT_WSPR);
	while((readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WWPS) & OMAP_W_PEND_WSPR))
                cpu_relax();
}

static void omap_wdt_clock(struct omap_wdt_dev *omap_wdev)
{
	uint32_t val;

	/* Watchdog timer have clock equal 1024 */
	val = readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WCLR);
	val |= (1 << 5) | (5 << 2);
	writel_relaxed(val, omap_wdev->base_addr + OMAP_WDT_WCLR);
	while((readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WWPS) & OMAP_W_PEND_WCLR))
                cpu_relax();
}

static void omap_wdt_reload(struct omap_wdt_dev *omap_wdev)
{
	omap_wdev->ttgr_pattern = ~omap_wdev->ttgr_pattern;
	writel_relaxed(omap_wdev->ttgr_pattern, omap_wdev->base_addr + OMAP_WDT_WTGR);
	while((readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WWPS) & OMAP_W_PEND_WTGR))
                cpu_relax();
}

static void omap_wdt_set_counter(struct omap_wdt_dev *omap_wdev, uint32_t timeout)
{

	writel_relaxed(GET_WLDR_VAL(timeout), omap_wdev->base_addr + OMAP_WDT_WLDR);
	while((readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WWPS) & OMAP_W_PEND_WLDR))
                cpu_relax();
}

static uint32_t omap_wdt_get_couter(struct omap_wdt_dev *omap_wdev)
{
	uint32_t val = 0;

	val = readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WCRR);
	return val;
}

static int omap_wdt_start(struct watchdog_device *wdev)
{
	struct omap_wdt_dev *omap_wdev = watchdog_get_drvdata(wdev);

	pm_runtime_get_sync(omap_wdev->dev);

	omap_wdt_clock(omap_wdev);
	omap_wdt_set_counter(omap_wdev, wdev->timeout);
	omap_wdt_enable(omap_wdev);
	omap_wdt_reload(omap_wdev);

	pr_info("Huu Thinh: %s  %d\n", __func__, __LINE__);
	return 0;
}
static int omap_wdt_stop(struct watchdog_device *wdev)
{
	struct omap_wdt_dev *omap_wdev = watchdog_get_drvdata(wdev);

	omap_wdt_disable(omap_wdev);
	pm_runtime_put_sync(omap_wdev->dev);

	pr_info("Huu Thinh: %s  %d\n", __func__, __LINE__);
	return 0;
}

static int omap_wdt_ping(struct watchdog_device *wdev)
{
	struct omap_wdt_dev *omap_wdev = watchdog_get_drvdata(wdev);

	omap_wdt_reload(omap_wdev);

	pr_info("Huu Thinh: %s  %d\n", __func__, __LINE__);
	return 0;
}

static int omap_wdt_set_timeout(struct watchdog_device *wdev, unsigned int timeout)
{
	struct omap_wdt_dev *omap_wdev = watchdog_get_drvdata(wdev);
	int err;

	err = watchdog_init_timeout(wdev, timeout, omap_wdev->dev);
        if(err)
                return err;
	
	omap_wdt_set_counter(omap_wdev, wdev->timeout);	
	pr_info("Huu Thinh: %s  %d\n", __func__, __LINE__);

	return 0;
}

static unsigned int omap_wdt_get_timeleft(struct watchdog_device *wdev)
{
	struct omap_wdt_dev *omap_wdev = watchdog_get_drvdata(wdev);
	uint32_t val = omap_wdt_get_couter(omap_wdev);

	pr_info("Huu Thinh: %s  %d\n", __func__, __LINE__);
	
	return VAL_TO_SECOND(val);
}	

static struct watchdog_ops omap_wdt_ops	= {
	.owner	= THIS_MODULE,
	.start	= omap_wdt_start,
	.stop	= omap_wdt_stop,
	.ping	= omap_wdt_ping,
	.set_timeout	= omap_wdt_set_timeout,
	.get_timeleft	= omap_wdt_get_timeleft,	
};

static struct watchdog_info omap_wdt_info = {
	.options	= WDIOF_SETTIMEOUT | WDIOF_MAGICCLOSE | WDIOF_KEEPALIVEPING,
	.identity	= "Omap Watchdog",
};

static int omap_wdt_plf_probe(struct platform_device *pdev)
{
	struct omap_wdt_dev *omap_wdev;
	struct watchdog_device *wdev;
	struct resource *res;
	void __iomem *base_addr;
	int err;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res)
		return -EINVAL;

	omap_wdev = kzalloc(sizeof(*omap_wdev), GFP_KERNEL);
	if(!omap_wdev) {
		pr_err("Wdt module didn't allocated memory\n ");
		return -ENOMEM;
	}

	base_addr = devm_ioremap_resource(&pdev->dev, res);
	if(IS_ERR(base_addr))
		return PTR_ERR(base_addr);

	omap_wdev->base_addr = base_addr;
	omap_wdev->ttgr_pattern = TTGR_VAL;
	omap_wdev->dev = &pdev->dev;

	wdev = &omap_wdev->wdev;
	wdev->parent = &pdev->dev;
	wdev->info = &omap_wdt_info;
	wdev->ops = &omap_wdt_ops;
	wdev->min_timeout = MIN_TIMEOUT;
	wdev->max_timeout = MAX_TIMEOUT;

	watchdog_set_drvdata(wdev, (void *)omap_wdev);
	dev_set_drvdata(&pdev->dev, omap_wdev);
	
	err = watchdog_init_timeout(wdev, 0, omap_wdev->dev);
	if(err)
		goto out;

	err = watchdog_register_device(wdev);
	if(err) {
		pr_err("Register the wdt device wasn't susscess\n");
		goto out;
	}
		
	pm_runtime_enable(omap_wdev->dev);
	pm_runtime_get_sync(omap_wdev->dev);
	
	while(!(readl_relaxed(omap_wdev->base_addr + OMAP_WDT_WDST) & OMAP_RESETDONE))
                cpu_relax();

	omap_wdt_enable(omap_wdev);
	omap_wdt_disable(omap_wdev);

	pm_runtime_put_sync(omap_wdev->dev);
	pr_info("Huu Thinh: %s	%d\n", __func__, __LINE__);
	return 0;
out:
	kfree(omap_wdev);
	return err;

}

static int omap_wdt_plf_remove(struct platform_device *pdev)
{
	struct omap_wdt_dev *omap_wdev;
        struct watchdog_device *wdev;

	omap_wdev = (struct omap_wdt_dev *)dev_get_drvdata(&pdev->dev);
	wdev = &omap_wdev->wdev;
	
	watchdog_unregister_device(wdev);
	pm_runtime_disable(omap_wdev->dev);
	kfree(omap_wdev);
	
	pr_info("Huu Thinh: %s  %d\n", __func__, __LINE__);
	return 0;
}

static struct of_device_id of_omap_wdt_plf_id[] = {
	{ .compatible = "my_wdt" },
	{ }
};

static struct platform_driver omap_wdt_plf_drv = {
	.probe	= omap_wdt_plf_probe,
	.remove	= omap_wdt_plf_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "my_wdt",
		.of_match_table = of_omap_wdt_plf_id,
	},
};

module_platform_driver(omap_wdt_plf_drv);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESC);
