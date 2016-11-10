/*
 * Author: wowo<wowo@wowotech.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/serial_core.h>
#include <linux/console.h>
#include <asm/io.h>

#define UART5_BASE		(0xE012a000)

#define	UART_STAT		(0xc)
#define	UART_TXDAT		(0x8)

#define	UART_STAT_TFFU		(0x1 <<	6)	/* TX FIFO full	Status */

/*============================================================================
 *			old earlycon, will be removed later
 *==========================================================================*/

/* TODO */
static void __iomem		*uart5_base;

/* TODO, FIXME!!! */
static void owl_serial_putc(struct uart_port *port, int ch)
{
	/* wait for TX FIFO untill it is not full */
	while (readl(uart5_base + UART_STAT) & UART_STAT_TFFU)
		;

	writel(ch, uart5_base + UART_TXDAT);
}

static void earlycon_owl_write(struct console *con, const char *s, unsigned n)
{
	/* TODO */
	uart_console_write(NULL, s, n, owl_serial_putc);
}

int __init earlycon_owl_setup(struct earlycon_device *device, const char *opt)
{
	/* TODO */
	uart5_base = early_ioremap(UART5_BASE, 4);

	device->con->write = earlycon_owl_write;
	return 0;
}
EARLYCON_DECLARE(owl_serial, earlycon_owl_setup);


/*============================================================================
 *				new serial driver
 *==========================================================================*/
static const struct of_device_id owl_serial_of_match[] = {
	{
		.compatible = "actions,s900-serial",
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, owl_serial_of_match);

static int owl_serial_probe(struct platform_device *pdev)
{
	int ret = 0;

	const struct of_device_id *match;

	dev_info(&pdev->dev, "%s\n", __func__);

	match = of_match_device(owl_serial_of_match, &pdev->dev);
	if (!match) {
		dev_err(&pdev->dev, "Error: No device match found\n");
		return -ENODEV;
	}

	return ret;
}

static int owl_serial_remove(struct platform_device *pdev)
{
	dev_info(&pdev->dev, "%s\n", __func__);

	return 0;
}

static struct platform_driver owl_serial_platform_driver = {
	.probe		= owl_serial_probe,
	.remove		= owl_serial_remove,
	.driver		= {
		.name	= "owl_serial",
		.of_match_table = owl_serial_of_match,
	},
};

static int __init owl_serial_init(void)
{
	int ret;

	pr_info("%s\n", __func__);

	ret = platform_driver_register(&owl_serial_platform_driver);
	if (ret < 0) {
		pr_err("owl_serial_platform_driver register failed, ret = %d\n", ret);
		return ret;
	}
	return 0;
}

static void __exit owl_serial_exit(void)
{
	pr_info("%s\n", __func__);

	platform_driver_unregister(&owl_serial_platform_driver);
}

module_init(owl_serial_init);
module_exit(owl_serial_exit);

MODULE_ALIAS("platform driver: owl_serial");
MODULE_DESCRIPTION("serial driver for S900 soc");
MODULE_AUTHOR("wowo<wowo@wowotech.net>");
MODULE_LICENSE("GPL v2");
