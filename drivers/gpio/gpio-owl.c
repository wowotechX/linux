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
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/slab.h>

#include <asm/io.h>

#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>

#define GPIO_OUTEN		(0x0000)
#define GPIO_INEN		(0x0004)
#define GPIO_DAT		(0x0008)

struct owl_gpio_priv {
	void __iomem		*membase;

	struct gpio_chip	gpiochip;
};

static inline void _gpiochip_set_bit(struct gpio_chip *chip,
				     unsigned reg, unsigned bit)
{
	struct owl_gpio_priv *priv = gpiochip_get_data(chip);

	writel(readl(priv->membase + reg) | (1 << bit),
	       priv->membase + reg);
}

static inline void _gpiochip_clear_bit(struct gpio_chip *chip,
				       unsigned reg, unsigned bit)
{
	struct owl_gpio_priv *priv = gpiochip_get_data(chip);

	writel(readl(priv->membase + reg) & (~(1 << bit)),
	       priv->membase + reg);
}

static inline int _gpiochip_get_bit(struct gpio_chip *chip,
				    unsigned reg, unsigned bit)
{
	struct owl_gpio_priv *priv = gpiochip_get_data(chip);

	return ((readl(priv->membase + reg) >> bit) & 0x1);
}

static int owl_gpio_request(struct gpio_chip *chip, unsigned offset)
{
	//return pinctrl_request_gpio(offset);
	return 0;
}

static void owl_gpio_free(struct gpio_chip *chip, unsigned offset)
{
	_gpiochip_clear_bit(chip, GPIO_INEN, offset);
	_gpiochip_clear_bit(chip, GPIO_OUTEN, offset);

	//return pinctrl_free_gpio(offset);
	return 0;
}

static int owl_gpio_direction_input(struct gpio_chip *chip, unsigned offset)
{
	_gpiochip_clear_bit(chip, GPIO_OUTEN, offset);
	_gpiochip_set_bit(chip, GPIO_INEN, offset);

	return 0;
}

static void owl_gpio_set(struct gpio_chip *chip, unsigned offset, int value);
static int owl_gpio_direction_output(struct gpio_chip *chip, unsigned offset,
					int value)
{
	_gpiochip_clear_bit(chip, GPIO_INEN, offset);

	owl_gpio_set(chip, offset, value);
	_gpiochip_set_bit(chip, GPIO_OUTEN, offset);

	return 0;
}

static void owl_gpio_set(struct gpio_chip *chip, unsigned offset, int value)
{
	dev_dbg(chip->parent, "offset %d, value %d\n", offset, value);

	if (value == 0)
		_gpiochip_clear_bit(chip, GPIO_DAT, offset);
	else
		_gpiochip_set_bit(chip, GPIO_DAT, offset);
}

static int owl_gpio_get(struct gpio_chip *chip, unsigned offset)
{
	dev_dbg(chip->parent, "offset %d, value %d\n",
		offset, _gpiochip_get_bit(chip, GPIO_DAT, offset));

	return _gpiochip_get_bit(chip, GPIO_DAT, offset);
}

static const struct gpio_chip owl_gpio_chip = {
	.label			= "owl-gpio",
	.request		= owl_gpio_request,
	.free			= owl_gpio_free,
	.direction_input	= owl_gpio_direction_input,
	.direction_output	= owl_gpio_direction_output,
	.get			= owl_gpio_get,
	.set			= owl_gpio_set,
	.base			= 0,
	.ngpio			= 32,
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
	int ret;

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

	priv->membase = ioremap(resource->start, resource_size(resource));
	if (IS_ERR(priv->membase)) {
		dev_err(dev, "Failed to map memory resource\n");
		return PTR_ERR(priv->membase);
	}

	memcpy(&priv->gpiochip, &owl_gpio_chip, sizeof(struct gpio_chip));
	of_property_read_u32(dev->of_node, "base", &priv->gpiochip.base);
	priv->gpiochip.parent = dev;

	ret = devm_gpiochip_add_data(dev, &priv->gpiochip, priv);
	if (ret < 0) {
		dev_err(dev, "Failed to add gpiochip\n");
		return ret;
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
