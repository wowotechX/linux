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

#include <linux/pinctrl/pinctrl.h>
#include <linux/pinctrl/pinmux.h>

#include "core.h"

struct owl_pinctrl_priv {
	void __iomem *membase;
};

struct owl_pmx_reginfo {
	const unsigned *offset;
	const unsigned *mask;
	const unsigned *val;

	unsigned num_entries;
};

struct owl_pinctrl_group {
	const char *name;
	const unsigned *pins;
	unsigned num_pins;

	const struct owl_pmx_reginfo *reginfo;
};

struct owl_pmx_func {
	const char *name;
	const char * const *groups;
	unsigned num_groups;
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

/* uart5_0_grp, MFP_CTL1(0xE01B0044)
 * bit28:26 and bit25:23 equal 001(UART5_RX)
 * and 100(UART5_TX) */
static const unsigned s900_uart5_0_offset[] = {0x44, 0x44 };
static const unsigned s900_uart5_0_mask[] = {0x7 << 26, 0x7 << 23 };
static const unsigned s900_uart5_0_val[] = {0x1 << 26, 0x4 << 23 };
static const struct owl_pmx_reginfo s900_uart5_0_reginfo = {
	.offset	= s900_uart5_0_offset,
	.mask	= s900_uart5_0_mask,
	.val	= s900_uart5_0_val,
	.num_entries = 2,
};

/* uart5_1_grp, MFP_CTL2(0xE01B0048)
 * bit19:17 and bit16:14 equal 101(UART5_RX)
 * and 101(UART5_TX) */
static const unsigned s900_uart5_1_offset[] = {0x48, 0x48 };
static const unsigned s900_uart5_1_mask[] = {0x7 << 17, 0x7 << 14 };
static const unsigned s900_uart5_1_val[] = {0x5 << 17, 0x5 << 14 };
static const struct owl_pmx_reginfo s900_uart5_1_reginfo = {
	.offset	= s900_uart5_1_offset,
	.mask	= s900_uart5_1_mask,
	.val	= s900_uart5_1_val,
	.num_entries = 2,
};

/* uart5_2_grp, MFP_CTL2(0xE01B0048)
 * bit21 and bit20 equal 1(UART5_RX)
 * and 1(UART5_TX) */
static const unsigned s900_uart5_2_offset[] = {0x48, 0x48 };
static const unsigned s900_uart5_2_mask[] = {0x1 << 21, 0x1 << 20 };
static const unsigned s900_uart5_2_val[] = {0x1 << 21, 0x1 << 20 };
static const struct owl_pmx_reginfo s900_uart5_2_reginfo = {
	.offset	= s900_uart5_2_offset,
	.mask	= s900_uart5_2_mask,
	.val	= s900_uart5_2_val,
	.num_entries = 2,
};

static const struct owl_pinctrl_group s900_groups[] = {
	{
		.name = "uart5_0_grp",
		.pins = s900_uart5_0_pins,
		.num_pins = ARRAY_SIZE(s900_uart5_0_pins),
		.reginfo = &s900_uart5_0_reginfo,
	},
	{
		.name = "uart5_1_grp",
		.pins = s900_uart5_1_pins,
		.num_pins = ARRAY_SIZE(s900_uart5_1_pins),
		.reginfo = &s900_uart5_1_reginfo,
	},
	{
		.name = "uart5_2_grp",
		.pins = s900_uart5_2_pins,
		.num_pins = ARRAY_SIZE(s900_uart5_2_pins),
		.reginfo = &s900_uart5_2_reginfo,
	},
};

static int s900_get_groups_count(struct pinctrl_dev *pctldev)
{ 
	dev_dbg(pctldev->dev, "%s\n", __func__);

	return ARRAY_SIZE(s900_groups);
}
static const char *s900_get_group_name(struct pinctrl_dev *pctldev,
				       unsigned selector)
{
	dev_dbg(pctldev->dev, "%s, selector %d\n", __func__, selector);

	return s900_groups[selector].name;
}

static int s900_get_group_pins(struct pinctrl_dev *pctldev, unsigned selector,
			       const unsigned **pins, unsigned *num_pins)
{
	dev_dbg(pctldev->dev, "%s, selector %d\n", __func__, selector);

	*pins = s900_groups[selector].pins;
	*num_pins = s900_groups[selector].num_pins;

	return 0;
}

static int s900_dt_node_to_map(struct pinctrl_dev *pctldev,
			       struct device_node *np_config,
			       struct pinctrl_map **map,
			       unsigned *num_maps)
{
	int ret, child_cnt;

	const char *function;
	const char *group;

	struct device *dev = pctldev->dev;
	struct device_node *np;

	dev_dbg(dev, "%s\n", __func__);

	*map = NULL;
	*num_maps = 0;

	child_cnt = of_get_child_count(np_config);
	dev_dbg(dev, "child_cnt %d\n", child_cnt);

	if (child_cnt == 0)
		return 0;

	*map = kzalloc(sizeof(struct pinctrl_map) * child_cnt, GFP_KERNEL);
	if (*map == NULL) {
		dev_dbg(dev, "malloc failed\n");
		return -ENOMEM;
	}

	for_each_child_of_node(np_config, np) {
		ret = of_property_read_string(np, "function", &function);
		if (ret != 0)
			continue;

		ret = of_property_read_string(np, "group", &group);
		if (ret != 0)
			continue;

		dev_dbg(dev, "got a pinmux entry: %s-%s\n", function, group);

		(*map)[*num_maps].type = PIN_MAP_TYPE_MUX_GROUP;
		(*map)[*num_maps].data.mux.function = function;
		(*map)[*num_maps].data.mux.group = group;
		(*num_maps)++;
	}

	return 0;
}
static void s900_dt_free_map(struct pinctrl_dev *pctldev,
			     struct pinctrl_map *map, unsigned num_maps)
{
	dev_dbg(pctldev->dev, "%s\n", __func__);

	/* others, TODO */

	kfree(map);
}

static struct pinctrl_ops s900_pctrl_ops = {
	.get_groups_count = s900_get_groups_count,
	.get_group_name = s900_get_group_name,
	.get_group_pins = s900_get_group_pins,
	.dt_node_to_map = s900_dt_node_to_map,
	.dt_free_map = s900_dt_free_map,
};
/*
 * pin groups end
 */

/*
 * pin multiplex
 */
static const char * const uart5_groups[] = {
	"uart5_0_grp", "uart5_1_grp", "uart5_2_grp"
};

static const struct owl_pmx_func s900_functions[] = {
	{
		.name = "uart5",
		.groups = uart5_groups,
		.num_groups = ARRAY_SIZE(uart5_groups),
	},
};

static int s900_get_functions_count(struct pinctrl_dev *pctldev)
{
	dev_dbg(pctldev->dev, "%s\n", __func__);

	return ARRAY_SIZE(s900_functions);
}

static const char *s900_get_fname(struct pinctrl_dev *pctldev,
				  unsigned selector)
{
	dev_dbg(pctldev->dev, "%s, selector %d\n", __func__, selector);

	return s900_functions[selector].name;
}

static int s900_get_groups(struct pinctrl_dev *pctldev, unsigned selector,
			   const char * const **groups, unsigned *num_groups)
{
	dev_dbg(pctldev->dev, "%s, selector %d\n", __func__, selector);

	*groups = s900_functions[selector].groups;
	*num_groups = s900_functions[selector].num_groups;

	return 0;
}

static int s900_set_mux(struct pinctrl_dev *pctldev, unsigned selector,
			unsigned group)
{
	int i;
	uint32_t val;

	struct owl_pinctrl_priv *priv;

	const struct owl_pmx_reginfo *reginfo;

	dev_dbg(pctldev->dev, "%s, selector %d, group %d\n",
		__func__, selector, group);

	priv = (struct owl_pinctrl_priv *)pctldev->driver_data;
	reginfo = s900_groups[group].reginfo;

	for (i = 0; i < reginfo->num_entries; i++) {
		dev_dbg(pctldev->dev, " offset %x, mask %x, val %x\n",
			reginfo->offset[i], reginfo->mask[i], reginfo->val[i]);

		val = readl(priv->membase + reginfo->offset[i]);
		val &= ~reginfo->mask[i];
		val |= reginfo->val[i];
		writel(val, priv->membase + reginfo->offset[i]);
	}

	return 0;
}

static struct pinmux_ops s900_pmxops = {
	.get_functions_count = s900_get_functions_count,
	.get_function_name = s900_get_fname,
	.get_function_groups = s900_get_groups,
	.set_mux = s900_set_mux,
	.strict = true,
};
/*
 * pin multiplex end
 */

static const struct pinctrl_desc s900_desc = {
	.name = "s900",
	.pins = s900_pins,
	.npins = ARRAY_SIZE(s900_pins),

	.pctlops = &s900_pctrl_ops,
	.pmxops = &s900_pmxops,

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
	struct owl_pinctrl_priv *priv;

	struct resource *resource;

	const struct pinctrl_desc *desc;

	dev_info(dev, "%s\n", __func__);

	desc = of_device_get_match_data(dev);
	if (desc == NULL) {
		dev_err(dev, "device get match data failed\n");
		return -ENODEV;
	}

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

	pctl = pinctrl_register((struct pinctrl_desc *)desc, dev, NULL);
	if (!pctl) {
		dev_err(dev, "could not register pinctrl desc\n");
		return -EINVAL;
	}
	pctl->driver_data = (void *)priv;

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
