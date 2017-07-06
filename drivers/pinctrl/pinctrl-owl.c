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
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

#include <linux/pinctrl/pinctrl.h>

struct owl_pinctrl_group {
	const char *name;
	const unsigned *pins;
	unsigned num_pins;
};

/*============================================================================
 *			      pinctrl_desc for s900 soc
 *==========================================================================*/

/*
 * pins
 */
static const struct pinctrl_pin_desc s900_pins[] = {
	PINCTRL_PIN(6, "A7"),
	PINCTRL_PIN(15, "B6"),
	PINCTRL_PIN(24, "C5"),
	PINCTRL_PIN(37, "D8"),
	PINCTRL_PIN(60, "G1"),
	PINCTRL_PIN(61, "G2"),
};
/*
 * pins end
 */

/*
 * pin groups
 */

static const unsigned s900_uart5_0_pins[] = { 6, 37 };
static const unsigned s900_uart5_1_pins[] = { 24, 15 };
static const unsigned s900_uart5_2_pins[] = { 61, 60 };

static const struct owl_pinctrl_group s900_groups[] = {
	{
		.name = "uart5_0_grp",
		.pins = s900_uart5_0_pins,
		.num_pins = ARRAY_SIZE(s900_uart5_0_pins),
	},
	{
		.name = "uart5_1_grp",
		.pins = s900_uart5_1_pins,
		.num_pins = ARRAY_SIZE(s900_uart5_1_pins),
	},
	{
		.name = "uart5_2_grp",
		.pins = s900_uart5_2_pins,
		.num_pins = ARRAY_SIZE(s900_uart5_2_pins),
	},
};

static int s900_get_groups_count(struct pinctrl_dev *pctldev)
{ 
	return ARRAY_SIZE(s900_groups);
}
static const char *s900_get_group_name(struct pinctrl_dev *pctldev,
				       unsigned selector)
{
	return s900_groups[selector].name;
}

static int s900_get_group_pins(struct pinctrl_dev *pctldev, unsigned selector,
			       const unsigned **pins, unsigned *num_pins)
{
	*pins = s900_groups[selector].pins;
	*num_pins = s900_groups[selector].num_pins;

	return 0;
}

static struct pinctrl_ops s900_pctrl_ops = {
	.get_groups_count = s900_get_groups_count,
	.get_group_name = s900_get_group_name,
	.get_group_pins = s900_get_group_pins,
};
/*
 * pin groups end
 */

static const struct pinctrl_desc s900_desc = {
	.name = "s900",
	.pins = s900_pins,
	.npins = ARRAY_SIZE(s900_pins),

	.pctlops = &s900_pctrl_ops,

	.owner = THIS_MODULE,
};

/*============================================================================
 *			       platform driver
 *==========================================================================*/
static const struct of_device_id owl_pinctrl_of_match[] = {
	{
		.compatible = "actions,s900-pinctrl",
		.data = &s900_desc,
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, owl_pinctrl_of_match);

static int owl_pinctrl_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pinctrl_dev *pctl;
	const struct pinctrl_desc *desc;

	dev_info(dev, "%s\n", __func__);

	desc = of_device_get_match_data(dev);
	if (desc == NULL) {
		dev_err(dev, "device get match data failed\n");
		return -ENODEV;
	}

	pctl = pinctrl_register((struct pinctrl_desc *)desc, dev, NULL);
	if (!pctl) {
		dev_err(dev, "could not register pinctrl desc\n");
		return -EINVAL;
	}

	return 0;
}

static int owl_pinctrl_remove(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver owl_pinctrl_platform_driver = {
	.probe  = owl_pinctrl_probe,
	.remove	 = owl_pinctrl_remove,
	.driver	 = {
		.name   = "owl_pinctrl",
		.of_match_table = owl_pinctrl_of_match,
	},
};
module_platform_driver(owl_pinctrl_platform_driver);

MODULE_ALIAS("platform driver: owl_pinctrl");
MODULE_DESCRIPTION("pinctrl driver for owl seria soc(s900, etc.)");
MODULE_AUTHOR("wowo<wowo@wowotech.net>");
MODULE_LICENSE("GPL v2");
