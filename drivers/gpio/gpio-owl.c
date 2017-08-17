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
#define DEBUG

#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>
#include <linux/slab.h>

#include <asm/io.h>

struct owl_gpio_priv {
	void __iomem *membase;
};

/*============================================================================
 *			       platform driver
 *==========================================================================*/
static const struct of_device_id owl_gpio_of_match[] = {
	{
		.compatible = "actions,s900-gpio",
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, owl_gpio_of_match);

static int owl_gpio_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *resource;

	struct owl_gpio_priv *priv;

	dev_info(dev, "%s\n", __func__);

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL) {
		dev_err(dev, "Failed to allocate memory for priv\n");
		return -ENOMEM;
	}

	resource = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!resource) {
		dev_err(dev, "No IO memory resource\n");
		return -ENODEV;
	}

	priv->membase = devm_ioremap_resource(dev, resource);
	if (IS_ERR(priv->membase)) {
		dev_err(dev, "Failed to map memory resource\n");
		return PTR_ERR(priv->membase);
	}

	return 0;
}

static int owl_gpio_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver owl_gpio_platform_driver = {
	.probe  = owl_gpio_probe,
	.remove	 = owl_gpio_remove,
	.driver	 = {
		.name   = "owl_gpio",
		.of_match_table = owl_gpio_of_match,
	},
};
module_platform_driver(owl_gpio_platform_driver);

MODULE_ALIAS("platform driver: owl_gpio");
MODULE_DESCRIPTION("gpio driver for owl seria soc(s900, etc.)");
MODULE_AUTHOR("wowo<wowo@wowotech.net>");
MODULE_LICENSE("GPL v2");
