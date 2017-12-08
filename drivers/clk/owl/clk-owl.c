/*
 * Actions OWL family clocks
 *
 * Copyright (C) 2017 Wowotech
 *
 * Common clock code for OWL clocks ("CKEN" type clocks + DT)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 */

#include <linux/clk-provider.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

/* now, used just for spinklock */
struct owl_clk_gate {
	struct clk_gate gate;

	spinlock_t lock;
};
struct owl_clk_div {
	struct clk_divider div;

	spinlock_t lock;
};
struct owl_clk_mux {
	struct clk_mux mux;

	spinlock_t lock;
};
struct owl_clk_mul {
	struct clk_multiplier mul;

	spinlock_t lock;
};

static __init struct owl_clk_gate *owl_of_gate_setup(struct device_node *node)
{
	u32 reg;

	struct owl_clk_gate *gate = NULL;

	/* check if there is a gate */
	if (of_property_read_u32(node, "actions,gate-reg", &reg) == 0) {
		gate = kzalloc(sizeof(struct owl_clk_gate), GFP_KERNEL);
		if (gate) {
			gate->gate.reg = ioremap(reg, 4);

			if (IS_ERR(gate->gate.reg) ||
			    of_property_read_u8(node, "actions,gate-bit-idx",
						&gate->gate.bit_idx)) {
				kfree(gate);
				gate = NULL;
			}

			spin_lock_init(&gate->lock);
			gate->gate.lock = &gate->lock;
		}
	}

	return gate;
}

static __init struct owl_clk_div *owl_of_div_setup(struct device_node *node)
{
	u32 reg;

	struct owl_clk_div *div = NULL;

	/* check if there is a divider */
	if (of_property_read_u32(node, "actions,div-reg", &reg) == 0) {
		div = kzalloc(sizeof(struct owl_clk_div), GFP_KERNEL);
		if (div) {
			div->div.reg = ioremap(reg, 4);

			if (IS_ERR(div->div.reg) ||
			    of_property_read_u8(node, "actions,div-shift",
						&div->div.shift) ||
			    of_property_read_u8(node, "actions,div-width",
						&div->div.width)) {
				kfree(div);
				div = NULL;
			}

			spin_lock_init(&div->lock);
			div->div.lock = &div->lock;
		}
	}

	return div;
}


static __init struct owl_clk_mul *owl_of_mul_setup(struct device_node *node)
{
	u32 reg;

	struct owl_clk_mul *mul = NULL;

	/* check if there is a multiplier */
	if (of_property_read_u32(node, "actions,mul-reg", &reg) == 0) {
		mul = kzalloc(sizeof(struct owl_clk_mul), GFP_KERNEL);
		if (mul) {
			mul->mul.reg = ioremap(reg, 4);

			if (IS_ERR(mul->mul.reg) ||
			    of_property_read_u8(node, "actions,mul-shift",
						&mul->mul.shift) ||
			    of_property_read_u8(node, "actions,mul-width",
						&mul->mul.width)) {
				kfree(mul);
				mul = NULL;
			}

			spin_lock_init(&mul->lock);
			mul->mul.lock = &mul->lock;
		}
	}

	return mul;
}


static __init struct owl_clk_mux *owl_of_mux_setup(struct device_node *node)
{
	u32 reg;

	struct owl_clk_mux *mux = NULL;

	/* check if there is a mux */
	if (of_property_read_u32(node, "actions,mux-reg", &reg) == 0) {
		mux = kzalloc(sizeof(struct owl_clk_mux), GFP_KERNEL);
		if (mux) {
			mux->mux.reg = ioremap(reg, 4);

			if (IS_ERR(mux->mux.reg) ||
			    of_property_read_u8(node, "actions,mux-shift",
						&mux->mux.shift) ||
			    of_property_read_u32(node, "actions,mux-mask",
						 &mux->mux.mask)) {
				kfree(mux);
				mux = NULL;
			}

			spin_lock_init(&mux->lock);
			mux->mux.lock = &mux->lock;
		}
	}

	return mux;
}

static __init const char ** owl_of_parents_setup(struct device_node *node,
						 int *parent_cnt)
{
	int cnt;
	const char **parents = NULL;

	cnt = of_clk_get_parent_count(node);
	if (cnt > 0) {
		parents = kzalloc((sizeof(char *) * cnt), GFP_KERNEL);
		if (!parents) {
			pr_info("parents is NULL\n");
			*parent_cnt = 0;
		} else {
			of_clk_parent_fill(node, parents, cnt);
			*parent_cnt = cnt;
		}
	}

	return parents;
}

static void __init owl_com_clock_setup(struct device_node *node)
{
	struct owl_clk_gate *gate = owl_of_gate_setup(node);
	struct owl_clk_div *div = owl_of_div_setup(node);
	struct owl_clk_mul *mul = owl_of_mul_setup(node);
	struct owl_clk_mux *mux = owl_of_mux_setup(node);

	int parent_cnt = 0;
	const char **parents;

	struct clk *clk;

	parents = owl_of_parents_setup(node, &parent_cnt);

	pr_info("%s: %s\n", __func__, node->name);

	pr_debug("gate %p, div %p, mul %p, mux %p\n", gate, div, mul, mux);
	pr_debug("parent %p, parent_cnt %d\n", parents, parent_cnt);

	if (gate == NULL && div == NULL && mul == NULL && div == NULL) {
		pr_err("no clocks!!\n");
		goto err_out;
	}

	if (mul != NULL && div != NULL) {
		pr_err("com clock can not support both mul and div\n");
		goto err_out;
	}

	if (mux != NULL && (parents == NULL || parent_cnt < 2)) {
		pr_err("mux clock with invalid parents(%p/%d)\n",
		       parents, parent_cnt);
		goto err_out;
	}

	clk = clk_register_composite(NULL, node->name,
			parents, parent_cnt,
			(mux == NULL ? NULL : &mux->mux.hw),
			(mux == NULL ? NULL : &clk_mux_ops),
			(mul == NULL ? &div->div.hw : &mux->mux.hw),
			(mul == NULL ? &clk_divider_ops : &clk_multiplier_ops),
			(gate == NULL ? NULL : &gate->gate.hw),
			(gate == NULL ? NULL : &clk_gate_ops),
			0);

	if (IS_ERR(clk))
		goto err_out;

	of_clk_add_provider(node, of_clk_src_simple_get, clk);

	return;

err_out:
	if (parents)
		kfree(parents);

	if (gate)
		kfree(gate);
	if (mux)
		kfree(mux);
	if (mul)
		kfree(mul);
	if (div)
		kfree(div);
}
CLK_OF_DECLARE(owl_com, "actions,com-clock", owl_com_clock_setup);
