/*
 * arch/arm/mach-omap2/clockdomains816x.h
 *
 * TI816X Clock Domain data.
 *
 * Copyright (C) 2010 Texas Instruments, Inc. - http://www.ti.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ARCH_ARM_MACH_OMAP2_CLOCKDOMAINS816X_H
#define __ARCH_ARM_MACH_OMAP2_CLOCKDOMAINS816X_H

#include <plat/clockdomain.h>

#include "cm.h"
#include "cm-regbits-816x.h"

#if defined(CONFIG_ARCH_TI816X)

/*
 * TODO:
 * - Add other domains as required
 * - Fill up associated powerdomans
 * - Consider dependencies across domains
 */

static struct clockdomain alwon_l3_slow_816x_clkdm = {
	.name		  = "alwon_l3_slow_clkdm",
	.pwrdm		  = { .name = NULL },
	.clkstctrl_reg	  = TI816X_CM_ALWON_L3_SLOW_CLKSTCTRL,
	.clktrctrl_mask	  = TI816X_CLKTRCTRL_MASK,
	.flags		  = CLKDM_CAN_HWSUP_SWSUP,
	.omap_chip	  = OMAP_CHIP_INIT(CHIP_IS_TI816X),
};

#endif

#endif