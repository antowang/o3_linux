/*
 *
 * Copyright (C) 2009 Texas Instruments Inc.
 * Author: Yihe Hu  <yihehu@ti.com>
 *
 *
 * Some code and ideas taken from TI OMAP2 Platforms
 * by Tomi Valkeinen.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __TI816X_RAM_H__
#define __TI816X_RAM_H__

#include <asm/types.h>

extern int ti816x_vram_add_region(unsigned long paddr, size_t size);
extern int ti816x_vram_free(unsigned long paddr, void *vaddr, size_t size);
extern int ti816x_vram_alloc(int mtype, size_t size, unsigned long *paddr);
extern void ti816x_set_sdram_vram(u32 size, u32 start);
extern void ti816x_set_sram_vram(u32 size, u32 start);

#endif
