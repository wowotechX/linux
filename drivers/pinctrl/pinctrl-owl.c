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

/*============================================================================
 *			       platform driver
 *==========================================================================*/
static const struct of_device_id owl_pinctrl_of_match[] = {
	{
		.compatible = "actions,s900-pinctrl",
	},
	{
	},
};
MODULE_DEVICE_TABLE(of, owl_pinctrl_of_match);

static int __init owl_pinctrl_probe(struct platform_device *pdev)
{
	return 0;
}

static void __exit owl_pinctrl_remove(struct platform_device *pdev)
{
}

static struct platform_driver owl_pinctrl_platform_driver = {
	.probe	  = owl_pinctrl_probe,
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
