/*
 * Copyright (C) 2012 ARM Ltd.
 * Author: Marc Zyngier <marc.zyngier@arm.com>
 *
 * Adapted for ARM and earlycon:
 * Copyright (C) 2014 Linaro Ltd.
 * Author: Rob Herring <robh@kernel.org>
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
#include <linux/console.h>
#include <linux/init.h>
#include <linux/serial_core.h>

#include <asm/io.h>

#define UART5_BASE		(0xE012a000)

#define	UART_STAT		(0xc)
#define	UART_TXDAT		(0x8)

#define	UART_STAT_TFFU		(0x1 <<	6)	/* TX FIFO full	Status */

/* TODO */
static void __iomem		*uart5_base;

/* TODO, FIXME!!! */
static void owl_serial_putc(struct uart_port *port, int ch)
{
	if (readl(uart5_base + UART_STAT) & UART_STAT_TFFU)
		return;

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
