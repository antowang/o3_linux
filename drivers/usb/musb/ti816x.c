/*
 * Texas Instruments AM3517 "glue layer"
 *
 * Copyright (c) 2008, MontaVista Software, Inc. <source@mvista.com>
 *
 * Based on the DaVinci "glue layer" code.
 * Copyright (C) 2005-2006 by Texas Instruments
 *
 * This file is part of the Inventra Controller Driver for Linux.
 *
 * The Inventra Controller Driver for Linux is free software; you
 * can redistribute it and/or modify it under the terms of the GNU
 * General Public License version 2 as published by the Free Software
 * Foundation.
 *
 * The Inventra Controller Driver for Linux is distributed in
 * the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
 * License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with The Inventra Controller Driver for Linux ; if not,
 * write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <linux/init.h>
#include <linux/clk.h>
#include <linux/io.h>
#include <linux/usb/otg.h>

/*#include <plat/control.h>*/
#include "cppi41.h"
#include "am_netra.h"

#include "musb_core.h"
#include "cppi41_dma.h"

struct musb *g_musb;

#undef USB_NETRA_DEBUG

#ifdef USB_NETRA_DEBUG
#define	dprintk(x, ...) printk(x, ## __VA_ARGS__)
#else
#define dprintk(x, ...)
#endif

/*
 * AM3517 specific definitions
 */

/* CPPI 4.1 queue manager registers */
#define QMGR_PEND0_REG		0x4090
#define QMGR_PEND1_REG		0x4094
#define QMGR_PEND2_REG		0x4098

/* USB 2.0 PHY Control */
#define CONF2_PHY_GPIOMODE     (1 << 23)
#define CONF2_OTGMODE          (3 << 14)
#define CONF2_SESENDEN         (1 << 13)       /* Vsess_end comparator */
#define CONF2_VBDTCTEN         (1 << 12)       /* Vbus comparator */
#define CONF2_REFFREQ_24MHZ    (2 << 8)
#define CONF2_REFFREQ_26MHZ    (7 << 8)
#define CONF2_REFFREQ_13MHZ    (6 << 8)
#define CONF2_REFFREQ          (0xf << 8)
#define CONF2_PHYCLKGD         (1 << 7)
#define CONF2_VBUSSENSE        (1 << 6)
#define CONF2_PHY_PLLON        (1 << 5)        /* override PLL suspend */
#define CONF2_RESET            (1 << 4)
#define CONF2_PHYPWRDN         (1 << 3)
#define CONF2_OTGPWRDN         (1 << 2)
#define CONF2_DATPOL           (1 << 1)


#define USB_TX_EP_MASK	0xffff		/* EP0 + 15 Tx EPs */
#define USB_RX_EP_MASK	0xfffe		/* 15 Rx EPs */

#define USB_TX_INTR_MASK	(USB_TX_EP_MASK << USB_INTR_TX_SHIFT)
#define USB_RX_INTR_MASK	(USB_RX_EP_MASK << USB_INTR_RX_SHIFT)

#define A_WAIT_BCON_TIMEOUT	1100		/* in ms */

static irqreturn_t netra_interrupt(int irq, void *hci);
static irqreturn_t cppi41dma_Interrupt(int irq, void *hci);

struct usbotg_ss {
	/* usbss access lock */
	spinlock_t	lock;
	void		*base;
	void		*intc_base;
	int		init_done;
};
static struct usbotg_ss usbss;

u32 usbss_read(u32 offs)
{
	unsigned long flags;
	u32 val = 0;

	if (!usbss.init_done)
		return val;

	spin_lock_irqsave(&usbss.lock, flags);
	val = *(volatile u32 *)(u32)((u32)usbss.base + offs);
	spin_unlock_irqrestore(&usbss.lock, flags);

	return val;
}

void usbss_write(u32 offs, u32 val)
{
	unsigned long flags;

	if (!usbss.init_done)
		return;

	spin_lock_irqsave(&usbss.lock, flags);
	*(volatile u32 *)((u32)usbss.base + offs) = val;
	spin_unlock_irqrestore(&usbss.lock, flags);
}

int usbotg_ss_init(struct musb *musb)
{
	int status = 0;

	if (!usbss.init_done) {
		usbss.base = ioremap(NETRA_IPSS_USBSS_BASE,
				NETRA_IPSS_USBSS_LEN);
		usbss.intc_base = ioremap(0x50000000, 0x400);
		spin_lock_init(&usbss.lock);
		usbss.init_done = 1;
		/* eoi to usbss */
		usbss_write(USBSS_IRQ_EOI, 0);
		/* clear any USBSS interrupts */
		usbss_write(USBSS_IRQ_STATUS, usbss_read(USBSS_IRQ_STATUS));
	}
	return status;
}

void set_threshold(u8 ctrl_id, u8 epn, u8 count, u8 is_tx)
{
	u32 ctrl_base;
	u32 reg_offset;
	u32 val;

	if (ctrl_id == 0) {
		if (is_tx)
			ctrl_base = USBSS_IRQ_DMA_THRESHOLD_TX0;
		else
			ctrl_base = USBSS_IRQ_DMA_THRESHOLD_RX0;
	} else {
		if (is_tx)
			ctrl_base = USBSS_IRQ_DMA_THRESHOLD_TX1;
		else
			ctrl_base = USBSS_IRQ_DMA_THRESHOLD_RX1;
	}

	reg_offset = ctrl_base + (4 * (epn/4));
	val = usbss_read(reg_offset);
	val |= count << ((epn % 4) * 8);

	dprintk("threshold write (usb%d-%s): offset=0x%x, val=0x%x\n",
		ctrl_id, is_tx ? "Tx" : "Rx", reg_offset, val);

	usbss_write(reg_offset, val);
}

/* AM3517 specific read/write functions */
u16 musb_readw(const void __iomem *addr, unsigned offset)
{
	u32 tmp;
	u16 val;

	if (addr == g_musb->mregs) {
		switch (offset) {
		case MUSB_INTRTXE:
			if (g_musb->read_mask & AM3517_READ_ISSUE_INTRTXE)
				return g_musb->intrtxe;
		case MUSB_INTRRXE:
			if (g_musb->read_mask & AM3517_READ_ISSUE_INTRRXE)
				return g_musb->intrrxe;
		default:
			break;
		}
	}
	tmp = __raw_readl(addr + (offset & ~3));

	switch (offset & 0x3) {
	case 0:
		val = (tmp & 0xffff);
		break;
	case 1:
		val = (tmp >> 8) & 0xffff;
		break;
	case 2:
	case 3:
	default:
		val = (tmp >> 16) & 0xffff;
		break;
	}
	return val;
}

void musb_writew(void __iomem *addr, unsigned offset, u16 data)
{
	if (addr == g_musb->mregs) {
		switch (offset) {
		case MUSB_INTRTXE:
			g_musb->read_mask |= AM3517_READ_ISSUE_INTRTXE;
			g_musb->intrtxe = data;
			break;
		case MUSB_INTRRXE:
			g_musb->read_mask |= AM3517_READ_ISSUE_INTRRXE;
			g_musb->intrrxe = data;
		default:
			break;
		}
	}
	__raw_writew(data, addr + offset);
}

u8 musb_readb(const void __iomem *addr, unsigned offset)
{
	u32 tmp;
	u8 val;

	if (addr == g_musb->mregs) {

		switch (offset) {
		case MUSB_FADDR:
			if (g_musb->read_mask & AM3517_READ_ISSUE_FADDR)
				return g_musb->faddr;
		case MUSB_POWER:
			if (g_musb->read_mask & AM3517_READ_ISSUE_POWER) {
				return g_musb->power;
			} else {
				tmp = __raw_readl(addr);
				val = (tmp >> 8);
				if (tmp & 0xffff0000) {
					DBG(2, "Missing Tx interrupt\
						event = 0x%x\n", (u16)\
						((tmp & 0xffff0000) >> 16));
				}
				g_musb->power = val;
				g_musb->read_mask |= AM3517_READ_ISSUE_POWER;
				return val;
			}
		case MUSB_INTRUSBE:
			if (g_musb->read_mask & AM3517_READ_ISSUE_INTRUSBE)
				return g_musb->intrusbe;
		default:
			break;
		}
	}

	tmp = __raw_readl(addr + (offset & ~3));

	switch (offset & 0x3) {
	case 0:
		val = tmp & 0xff;
		break;
	case 1:
		val = (tmp >> 8);
		break;
	case 2:
		val = (tmp >> 16);
		break;
	case 3:
	default:
		val = (tmp >> 24);
		break;
	}
	return val;
}
void musb_writeb(void __iomem *addr, unsigned offset, u8 data)
{
	if (addr == g_musb->mregs) {

		switch (offset) {
		case MUSB_FADDR:
			g_musb->read_mask |= AM3517_READ_ISSUE_FADDR;
			g_musb->faddr = data;
			break;
		case MUSB_POWER:
			g_musb->read_mask |= AM3517_READ_ISSUE_POWER;
			g_musb->power = data;
			break;
		case MUSB_INTRUSBE:
			g_musb->read_mask |= AM3517_READ_ISSUE_INTRUSBE;
			g_musb->intrusbe = data;
		default:
			break;
		}
	}

	__raw_writeb(data, addr + offset);
}



#ifdef CONFIG_USB_TI_CPPI41_DMA
/*
 * CPPI 4.1 resources used for USB OTG controller module:
 *
 tx/rx completion queues for usb0 */
static const u16 tx_comp_q[] = {93, 94, 95, 96, 97,
				98, 99, 100, 101, 102,
				103, 104, 105, 106, 107 };

static const u16 rx_comp_q[] = {109, 110, 111, 112, 113,
				114, 115, 116, 117, 118,
				119, 120, 121, 122, 123 };

/* tx/rx completion queues for usb1 */
static const u16 tx_comp_q1[] = {125, 126, 127, 128, 129,
				 130, 131, 132, 133, 134,
				 135, 136, 137, 138, 139 };

static const u16 rx_comp_q1[] = {141, 142, 143, 144, 145,
				 146, 147, 148, 149, 150,
				 151, 152, 153, 154, 155 };

const struct usb_cppi41_info usb_cppi41_info = {
	.dma_block	= 0,
	.ep_dma_ch	= {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 },
	.q_mgr		= 0,
	.num_tx_comp_q	= 15,
	.num_rx_comp_q	= 15,
	.tx_comp_q	= tx_comp_q,
	.rx_comp_q	= rx_comp_q
};

const struct usb_cppi41_info usb1_cppi41_info = {
	.dma_block      = 0,
	.ep_dma_ch      = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 },
	.q_mgr          = 0,
	.num_tx_comp_q  = 15,
	.num_rx_comp_q  = 15,
	.tx_comp_q      = tx_comp_q1,
	.rx_comp_q      = rx_comp_q1
};

/* Fair scheduling */
u32 dma_sched_table[] = {
	0x81018000, 0x83038202, 0x85058404, 0x87078606,
	0x89098808, 0x8b0b8a0a, 0x8d0d8c0c, 0x00008e0e
};

/* DMA block configuration */
static const struct cppi41_tx_ch tx_ch_info[] = {
	[0] = {
		.port_num	= 1,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 32} , {0, 33} }
	},
	[1] = {
		.port_num	= 2,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 34} , {0, 35} }
	},
	[2] = {
		.port_num	= 3,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 36} , {0, 37} }
	},
	[3] = {
		.port_num	= 4,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 38} , {0, 39} }
	},
	[4] = {
		.port_num	= 5,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 40} , {0, 41} }
	},
	[5] = {
		.port_num	= 6,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 42} , {0, 43} }
	},
	[6] = {
		.port_num	= 7,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 44} , {0, 45} }
	},
	[7] = {
		.port_num	= 8,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 46} , {0, 47} }
	},
	[8] = {
		.port_num	= 9,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 48} , {0, 49} }
	},
	[9] = {
		.port_num	= 10,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 50} , {0, 51} }
	},
	[10] = {
		.port_num	= 11,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 52} , {0, 53} }
	},
	[11] = {
		.port_num	= 12,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 54} , {0, 55} }
	},
	[12] = {
		.port_num	= 13,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 56} , {0, 57} }
	},
	[13] = {
		.port_num	= 14,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 58} , {0, 59} }
	},
	[14] = {
		.port_num	= 15,
		.num_tx_queue	= 2,
		.tx_queue	= { {0, 60} , {0, 61} }
	}
};

struct cppi41_dma_block cppi41_dma_block[CPPI41_NUM_DMA_BLOCK] = {
	[0] = {
		.num_tx_ch	= 15,
		.num_rx_ch	= 15,
		.tx_ch_info	= tx_ch_info
	}
};
EXPORT_SYMBOL(cppi41_dma_block);

/* Queues 0 to 66 are pre-assigned, others are spare */
static const u32 assigned_queues[] = {	0xffffffff, /* queue 0..31 */
					0xffffffff, /* queue 32..63 */
					0xffffffff, /* queue 64..95 */
					0xffffffff, /* queue 96..127 */
					0x0fffffff  /* queue 128..155 */
					};

/* Queue manager information */
struct cppi41_queue_mgr cppi41_queue_mgr[CPPI41_NUM_QUEUE_MGR] = {
	[0] = {
		.num_queue	= 159,
		.queue_types	= CPPI41_FREE_DESC_BUF_QUEUE |
					CPPI41_UNASSIGNED_QUEUE,
		.base_fdbq_num	= 0,
		.assigned	= assigned_queues
	}
};
EXPORT_SYMBOL(cppi41_queue_mgr);


static void *cppi41_dma_base;

int __init cppi41_init(struct musb *musb)
{
	u16 numch, blknum = usb_cppi41_info.dma_block, order;
	u32 offs = 0x2000;
	u32 nIrq = 10;

	cppi41_dma_base = ioremap(NETRA_USB_CPPIDMA_BASE,
					NETRA_USB_CPPIDMA_LEN);

	printk(KERN_INFO, "cppi41_dma_base = %p\n", cppi41_dma_base);

	/* init mappings */
	cppi41_queue_mgr[0].q_mgr_rgn_base	= (void *)((u32)cppi41_dma_base
							+ (0x4000 - offs));
	cppi41_queue_mgr[0].desc_mem_rgn_base	= (void *)((u32)cppi41_dma_base
							+ (0x5000 - offs));
	cppi41_queue_mgr[0].q_mgmt_rgn_base	= (void *)((u32)cppi41_dma_base
							+ (0x6000 - offs));
	cppi41_queue_mgr[0].q_stat_rgn_base	= (void *)((u32)cppi41_dma_base
							 + (0x7000 - offs));

	cppi41_dma_block[0].global_ctrl_base	= (void *)((u32)cppi41_dma_base
							 + (0x2000 - offs));
	cppi41_dma_block[0].ch_ctrl_stat_base	= (void *)((u32)cppi41_dma_base
							 + (0x2800 - offs));
	cppi41_dma_block[0].sched_ctrl_base	= (void *)((u32)cppi41_dma_base
							 + (0x3000 - offs));
	cppi41_dma_block[0].sched_table_base	= (void *)((u32)cppi41_dma_base
							 + (0x3800 - offs));

	/* Initialize for Linking RAM region 0 alone */
	cppi41_queue_mgr_init(usb_cppi41_info.q_mgr, 0, 0x3fff);

	printk(KERN_INFO, "cppi41_queue_mgr_init done\n");
	numch =  USB_CPPI41_NUM_CH * 2;
	order = get_count_order(numch);

	/* TODO: check two teardown desc per channel (5 or 7 ?)*/
	if (order < 5)
		order = 5;

	cppi41_dma_block_init(blknum, usb_cppi41_info.q_mgr, order,
			dma_sched_table, numch);

#ifdef CONFIG_USB_TI_CPPI41_DMA
	/* attach to the IRQ */
	if (request_irq(nIrq, cppi41dma_Interrupt, 0, "musb_dma", musb))
		printk(KERN_INFO, "request_irq %d failed!\n", nIrq);
	else
		printk(KERN_INFO, "registerd cppi-dma Intr @ IRQ %d\n", nIrq);
#endif

#define USBSS_INTR_RX_STARV	0x00000001
#define USBSS_INTR_PD_CMPL	0x00000004
	/* enable all the interrupts */
	usbss_write(USBSS_IRQ_EOI, 0);
	usbss_write(USBSS_IRQ_ENABLE_SET, USBSS_INTR_PD_CMPL);
	/*usbss_write(USBSS_IRQ_DMA_ENABLE_0, 0xFFFeFFFe);*/

	return 0;
}
#endif /* CONFIG_USB_TI_CPPI41_DMA */

#ifdef CONFIG_USB_TI_CPPI41_DMA
int cppi41_disable_sched_rx(void)
{
	u16 numch = 35, blknum = usb_cppi41_info.dma_block;

	dma_sched_table[0] = 0x02810100;
	dma_sched_table[1] = 0x04830382;
	dma_sched_table[2] = 0x06850584;
	dma_sched_table[3] = 0x08870786;
	dma_sched_table[4] = 0x0a890988;
	dma_sched_table[5] = 0x0c8b0b8a;
	dma_sched_table[6] = 0x0e8d0d8c;
	dma_sched_table[7] = 0x0000008e;

	cppi41_dma_sched_tbl_init(blknum, usb_cppi41_info.q_mgr,
		dma_sched_table, numch);
	return 0;
}

int cppi41_enable_sched_rx(void)
{
	u16 numch = 32, blknum = usb_cppi41_info.dma_block;

	dma_sched_table[0] = 0x81018000;
	dma_sched_table[1] = 0x83038202;
	dma_sched_table[2] = 0x85058404;
	dma_sched_table[3] = 0x87078606;
	dma_sched_table[4] = 0x89098808;
	dma_sched_table[5] = 0x8b0b8a0a;
	dma_sched_table[6] = 0x8d0d8c0c;
	dma_sched_table[7] = 0x00008e0e;

	cppi41_dma_sched_tbl_init(blknum, usb_cppi41_info.q_mgr,
		dma_sched_table, numch);
	return 0;
}
#endif

/*
 * REVISIT (PM): we should be able to keep the PHY in low power mode most
 * of the time (24 MHZ oscillator and PLL off, etc) by setting POWER.D0
 * and, when in host mode, autosuspending idle root ports... PHYPLLON
 * (overriding SUSPENDM?) then likely needs to stay off.
 */

static inline void phy_on(void)
{
#if 0
	u32 devconf2;
	/*
	 * Start the on-chip PHY and its PLL.
	 */
	devconf2 = omap_ctrl_readl(OMAP3517_CONTROL_DEVCONF2);

	devconf2 &= ~(CONF2_RESET | CONF2_PHYPWRDN | CONF2_OTGPWRDN |
			CONF2_OTGMODE | CONF2_REFFREQ | CONF2_PHY_GPIOMODE);
	devconf2 |= CONF2_SESENDEN | CONF2_VBDTCTEN | CONF2_PHY_PLLON |
		    CONF2_REFFREQ_13MHZ | CONF2_DATPOL;

	omap_ctrl_writel(devconf2, OMAP3517_CONTROL_DEVCONF2);

	pr_info("Waiting for PHY clock good...\n");
	while (!(omap_ctrl_readl(OMAP3517_CONTROL_DEVCONF2)
			& CONF2_PHYCLKGD))
		cpu_relax();
#endif
}

static inline void phy_off(void)
{
#if 0
	u32 devconf2;
	/*
	 * Power down the on-chip PHY.
	 */
	devconf2 = omap_ctrl_readl(OMAP3517_CONTROL_DEVCONF2);

	devconf2 &= ~CONF2_PHY_PLLON;
	devconf2 |=  CONF2_PHYPWRDN | CONF2_OTGPWRDN;
	omap_ctrl_writel(devconf2, OMAP3517_CONTROL_DEVCONF2);
#endif
}

/*
 * Because we don't set CTRL.UINT, it's "important" to:
 *	- not read/write INTRUSB/INTRUSBE (except during
 *	  initial setup, as a workaround);
 *	- use INTSET/INTCLR instead.
 */

/**
 * musb_platform_enable - enable interrupts
 */
void musb_platform_enable(struct musb *musb)
{
	void __iomem *reg_base = musb->ctrl_base;
	u32 epmask, coremask;

	/* Workaround: setup IRQs through both register sets. */
	epmask = ((musb->epmask & USB_TX_EP_MASK) << USB_INTR_TX_SHIFT) |
	       ((musb->epmask & USB_RX_EP_MASK) << USB_INTR_RX_SHIFT);
	coremask = (0x01ff << USB_INTR_USB_SHIFT);

	coremask &= ~0x8; /* disable the SOF */

	musb_writel(reg_base, USB_EP_INTR_SET_REG, epmask);
	musb_writel(reg_base, USB_CORE_INTR_SET_REG, coremask);

	/* Force the DRVVBUS IRQ so we can start polling for ID change. */
	if (is_otg_enabled(musb))
		musb_writel(reg_base, USB_CORE_INTR_SET_REG,
			    USB_INTR_DRVVBUS << USB_INTR_USB_SHIFT);
}

/**
 * musb_platform_disable - disable HDRC and flush interrupts
 */
void musb_platform_disable(struct musb *musb)
{
	void __iomem *reg_base = musb->ctrl_base;

	musb_writel(reg_base, USB_CORE_INTR_CLEAR_REG, USB_INTR_USB_MASK);
	musb_writel(reg_base, USB_EP_INTR_CLEAR_REG,
			 USB_TX_INTR_MASK | USB_RX_INTR_MASK);
	musb_writeb(musb->mregs, MUSB_DEVCTL, 0);
	musb_writel(reg_base, USB_IRQ_EOI, 0);
}

/* REVISIT: it's not clear whether AM3517 can support full OTG.  */

static int vbus_state = -1;

#ifdef CONFIG_USB_MUSB_HDRC_HCD
#define portstate(stmt)	stmt
#else
#define portstate(stmt)
#endif

static void netra_source_power(struct musb *musb, int is_on, int immediate)
{
	if (is_on)
		is_on = 1;

	if (vbus_state == is_on)
		return;
	vbus_state = is_on;		/* 0/1 vs "-1 == unknown/init" */
}

static void netra_set_vbus(struct musb *musb, int is_on)
{
	WARN_ON(is_on && is_peripheral_active(musb));
	netra_source_power(musb, is_on, 0);
}

#define	POLL_SECONDS	2

static struct timer_list otg_workaround;

static void otg_timer(unsigned long _musb)
{
	struct musb		*musb = (void *)_musb;
	void __iomem		*mregs = musb->mregs;
	u8			devctl;
	unsigned long		flags;

	/* We poll because DaVinci's won't expose several OTG-critical
	* status change events (from the transceiver) otherwise.
	 */
	devctl = musb_readb(mregs, MUSB_DEVCTL);
	DBG(7, "Poll devctl %02x (%s)\n", devctl, otg_state_string(musb));

	spin_lock_irqsave(&musb->lock, flags);
	switch (musb->xceiv->state) {
	case OTG_STATE_A_WAIT_BCON:
		devctl &= ~MUSB_DEVCTL_SESSION;
		musb_writeb(musb->mregs, MUSB_DEVCTL, devctl);

		devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
		if (devctl & MUSB_DEVCTL_BDEVICE) {
			musb->xceiv->state = OTG_STATE_B_IDLE;
			MUSB_DEV_MODE(musb);
		} else {
			musb->xceiv->state = OTG_STATE_A_IDLE;
			MUSB_HST_MODE(musb);
		}
		break;
	case OTG_STATE_A_WAIT_VFALL:
		/*
		 * Wait till VBUS falls below SessionEnd (~0.2 V); the 1.3
		 * RTL seems to mis-handle session "start" otherwise (or in
		 * our case "recover"), in routine "VBUS was valid by the time
		 * VBUSERR got reported during enumeration" cases.
		 */
		if (devctl & MUSB_DEVCTL_VBUS) {
			mod_timer(&otg_workaround, jiffies + POLL_SECONDS * HZ);
			break;
		}
		musb->xceiv->state = OTG_STATE_A_WAIT_VRISE;
		musb_writel(musb->ctrl_base, USB_CORE_INTR_SET_REG,
			    MUSB_INTR_VBUSERROR << USB_INTR_USB_SHIFT);
		break;
	case OTG_STATE_B_IDLE:
		if (!is_peripheral_enabled(musb))
			break;

		/*
		 * There's no ID-changed IRQ, so we have no good way to tell
		 * when to switch to the A-Default state machine (by setting
		 * the DEVCTL.SESSION flag).
		 *
		 * Workaround:  whenever we're in B_IDLE, try setting the
		 * session flag every few seconds.  If it works, ID was
		 * grounded and we're now in the A-Default state machine.
		 *
		 * NOTE: setting the session flag is _supposed_ to trigger
		 * SRP but clearly it doesn't.
		 */
		devctl = musb_readb(mregs, MUSB_DEVCTL);
		if (devctl & MUSB_DEVCTL_BDEVICE)
			mod_timer(&otg_workaround, jiffies + POLL_SECONDS * HZ);
		else
			musb->xceiv->state = OTG_STATE_A_IDLE;
		break;
	default:
		break;
	}
	spin_unlock_irqrestore(&musb->lock, flags);
}

void musb_platform_try_idle_x(struct musb *musb, unsigned long timeout)
{
	static unsigned long last_timer;

	if (!is_otg_enabled(musb))
		return;

	if (timeout == 0)
		timeout = jiffies + msecs_to_jiffies(3);

	/* Never idle if active, or when VBUS timeout is not set as host */
	if (musb->is_active || (musb->a_wait_bcon == 0 &&
				musb->xceiv->state == OTG_STATE_A_WAIT_BCON)) {
		DBG(4, "%s active, deleting timer\n", otg_state_string(musb));
		del_timer(&otg_workaround);
		last_timer = jiffies;
		return;
	}

	if (time_after(last_timer, timeout) && timer_pending(&otg_workaround)) {
		DBG(4, "Longer idle timer already pending, ignoring...\n");
		return;
	}
	last_timer = timeout;

	DBG(4, "%s inactive, starting idle timer for %u ms\n",
	    otg_state_string(musb), jiffies_to_msecs(timeout - jiffies));
	mod_timer(&otg_workaround, timeout);
}

static irqreturn_t cppi41dma_Interrupt(int irq, void *hci)
{
	struct musb  *musb, *usb0_musb = hci;
	u32 intr_status;
	irqreturn_t ret = IRQ_NONE;
	u32 q_cmpl_status_0 = 0, q_cmpl_status_1 = 0;
	u32 usb0_tx_intr, usb0_rx_intr;
	void *q_mgr_base = cppi41_queue_mgr[0].q_mgr_rgn_base;
	unsigned long flags;

	musb = hci;
	spin_lock_irqsave(&musb->lock, flags);

	/*
	 * CPPI 4.1 interrupts share the same IRQ and the EOI register but
	 * don't get reflected in the interrupt source/mask registers.
	 */
	if (is_cppi41_enabled()) {
		/*
		 * Check for the interrupts from Tx/Rx completion queues; they
		 * are level-triggered and will stay asserted until the queues
		 * are emptied.  We're using the queue pending register 0 as a
		 * substitute for the interrupt status register and reading it
		 * directly for speed.
		 */
		intr_status = usbss_read(USBSS_IRQ_STATUS);

		if (intr_status)
			usbss_write(USBSS_IRQ_STATUS, intr_status);
		else
			printk(KERN_DEBUG, "spurious usbss intr\n");

		dprintk("amIsr: intr_status = %x\n", intr_status);
		if (intr_status & (0xf04|USBSS_INTR_RX_STARV)) {
			q_cmpl_status_0 = musb_readl(q_mgr_base, 0x98);
			q_cmpl_status_1 = musb_readl(q_mgr_base, 0x9c);
		}

		/* USB0 tx/rx completion */
		if (intr_status & (0xf04|USBSS_INTR_RX_STARV)) {
			/* usb0 tx completion interrupt for ep1..15 */
			usb0_tx_intr = (q_cmpl_status_0 >> 29) |
					((q_cmpl_status_1 & 0xFFF) << 3);
			usb0_rx_intr = (q_cmpl_status_1 >> 13);

			dprintk("[%x][%x]tx=%x,rx=%x\n", q_cmpl_status_0,
				q_cmpl_status_1, usb0_tx_intr, usb0_rx_intr);

			/* get proper musb handle based usb0/usb1 ctrl-id */
			usb0_musb = hci;

			DBG(4, "CPPI 4.1 IRQ: Tx %x, Rx %x\n", usb0_tx_intr,
						usb0_rx_intr);
			cppi41_completion(usb0_musb, usb0_rx_intr,
						usb0_tx_intr);
			ret = IRQ_HANDLED;
		}
#if 0
		/* USB1 tx/rx completion */
		if (intr_status & 0xc00) {
			u32 usb1_tx_intr, usb1_rx_intr, q_cmpl_status_2 = 0;
			struct musb *usb1_musb;

			q_cmpl_status_2 = musb_readl(q_mgr_base, 0xa0);
			usb1_tx_intr	= (q_cmpl_status_1 >> 29) |
					((q_cmpl_status_2 & 0xFFF) << 3);
			usb1_rx_intr    = (q_cmpl_status_2 >> 13) & 0x7FFF;

			/* get proper handle musb based usb0/usb1 ctrl-id */
			usb1_musb = hci;

			DBG(4, "CPPI 4.1 IRQ: Tx %x, Rx %x\n", usb1_tx_intr,
				usb1_rx_intr);
			cppi41_completion(usb1_musb, usb1_rx_intr,
				usb1_tx_intr);
			ret = IRQ_HANDLED;
		}
#endif
		usbss_write(USBSS_IRQ_EOI, 0);
	}
	spin_unlock_irqrestore(&musb->lock, flags);
	return ret;
}

static irqreturn_t netra_interrupt(int irq, void *hci)
{
	struct musb  *musb = hci;
	void __iomem *reg_base = musb->ctrl_base;
	unsigned long flags;
	irqreturn_t ret = IRQ_NONE;
	u32 pend1 = 0, pend2 = 0;
	u32 epintr, usbintr;

	spin_lock_irqsave(&musb->lock, flags);

	/*
	 * NOTE: AM3517 shadows the Mentor IRQs.  Don't manage them through
	 * the Mentor registers (except for setup), use the TI ones and EOI.
	 */
	/* Acknowledge and handle non-CPPI interrupts */
	/* Get endpoint interrupts */
	musb->int_rx = musb->int_tx = musb->int_usb = 0;
	epintr = musb_readl(reg_base, USB_EP_INTR_STATUS_REG);
	if (epintr) {
		musb_writel(reg_base, USB_EP_INTR_STATUS_REG, epintr);

		musb->int_rx =
			(epintr & USB_RX_INTR_MASK) >> USB_INTR_RX_SHIFT;
		musb->int_tx =
			(epintr & USB_TX_INTR_MASK) >> USB_INTR_TX_SHIFT;
	}

	/* Get usb core interrupts */
	usbintr = musb_readl(reg_base, USB_CORE_INTR_STATUS_REG);
	if (!usbintr && !epintr) {
		printk(KERN_DEBUG, "c[%x]ep[%x]\n", usbintr, epintr);
		goto eoi;
	}

	if (usbintr) {
		musb_writel(reg_base, USB_CORE_INTR_STATUS_REG, usbintr);
		musb->int_usb =
			(usbintr & USB_INTR_USB_MASK) >> USB_INTR_USB_SHIFT;
	}

	/*
	 * DRVVBUS IRQs are the only proxy we have (a very poor one!) for
	 * AM3517's missing ID change IRQ.  We need an ID change IRQ to
	 * switch appropriately between halves of the OTG state machine.
	 * Managing DEVCTL.SESSION per Mentor docs requires that we know its
	 * value but DEVCTL.BDEVICE is invalid without DEVCTL.SESSION set.
	 * Also, DRVVBUS pulses for SRP (but not at 5V) ...
	 */
	if (usbintr & (USB_INTR_DRVVBUS << USB_INTR_USB_SHIFT)) {
		int drvvbus = musb_readl(reg_base, USB_STAT_REG);
		void __iomem *mregs = musb->mregs;
		u8 devctl = musb_readb(mregs, MUSB_DEVCTL);
		int err;

		err = is_host_enabled(musb) && (musb->int_usb &
						MUSB_INTR_VBUSERROR);
		if (err) {
			/*
			 * The Mentor core doesn't debounce VBUS as needed
			 * to cope with device connect current spikes. This
			 * means it's not uncommon for bus-powered devices
			 * to get VBUS errors during enumeration.
			 *
			 * This is a workaround, but newer RTL from Mentor
			 * seems to allow a better one: "re"-starting sessions
			 * without waiting for VBUS to stop registering in
			 * devctl.
			 */
			musb->int_usb &= ~MUSB_INTR_VBUSERROR;
			musb->xceiv->state = OTG_STATE_A_WAIT_VFALL;
			mod_timer(&otg_workaround, jiffies + POLL_SECONDS * HZ);
			WARNING("VBUS error workaround (delay coming)\n");
		} else if (is_host_enabled(musb) && drvvbus) {
			musb->is_active = 1;
			MUSB_HST_MODE(musb);
			musb->xceiv->default_a = 1;
			musb->xceiv->state = OTG_STATE_A_WAIT_VRISE;
			portstate(musb->port1_status |= USB_PORT_STAT_POWER);
			del_timer(&otg_workaround);
		} else {
			musb->is_active = 0;
			MUSB_DEV_MODE(musb);
			musb->xceiv->default_a = 0;
			musb->xceiv->state = OTG_STATE_B_IDLE;
			portstate(musb->port1_status &= ~USB_PORT_STAT_POWER);
		}

		/* NOTE: this must complete power-on within 100 ms. */
		netra_source_power(musb, drvvbus, 0);
		DBG(2, "VBUS %s (%s)%s, devctl %02x\n",
				drvvbus ? "on" : "off",
				otg_state_string(musb),
				err ? " ERROR" : "",
				devctl);
		ret = IRQ_HANDLED;
	}

	if (musb->int_tx || musb->int_rx || musb->int_usb) {
		irqreturn_t mret;
		mret = musb_interrupt(musb);
		if (mret == IRQ_HANDLED)
			ret = IRQ_HANDLED;
	}

 eoi:
	/* EOI needs to be written for the IRQ to be re-asserted. */
	if (ret == IRQ_HANDLED || epintr || usbintr) {
		/* write EOI */
		musb_writel(reg_base, USB_IRQ_EOI, 0);
	}

	ret = IRQ_HANDLED;

	/* Poll for ID change */
	if (is_otg_enabled(musb) && musb->xceiv->state == OTG_STATE_B_IDLE)
		mod_timer(&otg_workaround, jiffies + POLL_SECONDS * HZ);

	spin_unlock_irqrestore(&musb->lock, flags);

	if (ret != IRQ_HANDLED) {
		if (epintr || usbintr)
			/*
			 * We sometimes get unhandled IRQs in the peripheral
			 * mode from EP0 and SOF...
			 */
			DBG(2, "Unhandled USB IRQ %08x-%08x\n",
					 epintr, usbintr);
		else if (printk_ratelimit())
			/*
			 * We've seen series of spurious interrupts in the
			 * peripheral mode after USB reset and then after some
			 * time a real interrupt storm starting...
			 */
			DBG(2, "Spurious IRQ, CPPI 4.1 status %08x, %08x\n",
					 pend1, pend2);
	}
	return ret;
}
int musb_platform_set_mode(struct musb *musb, u8 musb_mode)
{
	void __iomem *reg_base = musb->ctrl_base;

	/* TODO: implement this using CONF0 */
	if (musb_mode == MUSB_HOST) {
		musb_writel(reg_base, USB_MODE_REG, 0);
		dprintk("%s: value of mode reg=%x\n\n", __func__,
					musb_readl(reg_base, USB_MODE_REG));
	} else
	if (musb_mode == MUSB_PERIPHERAL) {
		/* TODO commmented writing 8 to USB_MODE_REG device
			mode is not working */
		/* musb_writel(reg_base, USB_MODE_REG, 8); */
	}
	return -EIO;
}

int musb_platform_init(struct musb *musb)
{
	void __iomem *reg_base = musb->ctrl_base;
	struct clk              *otg_fck;
	u32 rev;

	g_musb = musb;
	g_musb->read_mask = 0;
	g_musb->faddr = 0;
	g_musb->power = 0;
	g_musb->intrtxe = 0;
	g_musb->intrrxe = 0;
	g_musb->intrusbe = 0;

	/* usb subsystem init */
	usbotg_ss_init(musb);

	usb_nop_xceiv_register();

	musb->xceiv = otg_get_transceiver();
	if (!musb->xceiv)
		return -ENODEV;

	/* mentor is at offset of 0x400 in am3517 */
	musb->mregs += USB_MENTOR_CORE_OFFSET;

	/* not required as clock is set in usb-musb.c file in arch */
	/* musb->clock = clk_get(NULL, "usbotg_ck"); */
	if (IS_ERR(musb->clock))
		return PTR_ERR(musb->clock);

	if (musb->set_clock)
		musb->set_clock(musb->clock, 1);
	else
		clk_enable(musb->clock);

	DBG(2, "usbotg_ck=%lud\n", clk_get_rate(musb->clock));
	otg_fck = clk_get(NULL, "usbotg_phy_ck");
	clk_enable(otg_fck);

	DBG(2, "usbotg_phy_ck=%lud\n", clk_get_rate(otg_fck));
	/* Returns zero if e.g. not clocked */
	rev = musb_readl(reg_base, USB_REVISION_REG);
	if (!rev)
		return -ENODEV;

	if (is_host_enabled(musb))
		setup_timer(&otg_workaround, otg_timer, (unsigned long) musb);

	musb->board_set_vbus = netra_set_vbus;
	netra_source_power(musb, 0, 1);

	/* follow recommended reset procedure */
	/* Reset the controller */
	musb_writel(reg_base, USB_CTRL_REG, USB_SOFT_RESET_MASK);

	/* wait till reset bit clears */
	while ((musb_readl(reg_base, USB_CTRL_REG) & 0x1))
		cpu_relax();

	/* clock disable */
	clk_disable(musb->clock);

	/* Start the on-chip PHY and its PLL. */
	phy_on();

	msleep(5);

	/* clock enable */
	clk_enable(musb->clock);

#ifdef CONFIG_USB_TI_CPPI41_DMA
	cppi41_init(musb);
#endif

	musb->a_wait_bcon = A_WAIT_BCON_TIMEOUT;
	musb->isr = netra_interrupt;

	/* set musb controller to host mode */
	if (is_host_enabled(musb))
		musb_platform_set_mode(musb, MUSB_HOST);
	else
		musb_platform_set_mode(musb, MUSB_PERIPHERAL);

	musb_writel(reg_base, USB_IRQ_EOI, 0);
	usbss_write(USBSS_IRQ_EOI, 0);

	return 0;
}

int musb_platform_exit(struct musb *musb)
{
	if (is_host_enabled(musb))
		del_timer_sync(&otg_workaround);

	netra_source_power(musb, 0, 1);

	/* Delay to avoid problems with module reload... */
	if (is_host_enabled(musb) && musb->xceiv->default_a) {
		int maxdelay = 30;
		u8 devctl, warn = 0;

		/*
		 * If there's no peripheral connected, this can take a
		 * long time to fall...
		 */
		do {
			devctl = musb_readb(musb->mregs, MUSB_DEVCTL);
			if (!(devctl & MUSB_DEVCTL_VBUS))
				break;
			if ((devctl & MUSB_DEVCTL_VBUS) != warn) {
				warn = devctl & MUSB_DEVCTL_VBUS;
				DBG(1, "VBUS %d\n",
					warn >> MUSB_DEVCTL_VBUS_SHIFT);
			}
			msleep(1000);
			maxdelay--;
		} while (maxdelay > 0);

		/* In OTG mode, another host might be connected... */
		if (devctl & MUSB_DEVCTL_VBUS)
			DBG(1, "VBUS off timeout (devctl %02x)\n", devctl);
	}

	phy_off();

#ifdef CONFIG_USB_TI_CPPI41_DMA
	cppi41_exit();
#endif
	return 0;
}

#ifdef CONFIG_PM
void musb_platform_save_context(struct musb_context_registers
		*musb_context)
{
	/* Save CPPI41 DMA related registers */
}

void musb_platform_restore_context(struct musb_context_registers
		*musb_context)
{
	/* Restore CPPI41 DMA related registers */
}
#endif

/* AM35x supports only 32bit read operation */
void musb_read_fifo(struct musb_hw_ep *hw_ep, u16 len, u8 *dst)
{
	void __iomem *fifo = hw_ep->fifo;
	u32	val;
	int	i;

	/* Read for 32bit-aligned destination address */
	if ((likely((0x03 & (unsigned long) dst) == 0)) && len >= 4) {
		readsl(fifo, dst, len >> 2);
		dst += (len & ~0x03);
		len &= 0x03;
	}
	/* Now read the rest 1 to 3 bytes or complete length if
	 * unaligned address.
	 */
	if (len > 4) {
		for (i = 0; i < (len >> 2); i++) {
			val = musb_readl(fifo, 0);
			memcpy(dst, &val, 4);
			dst += 4;
		}
		len %= 4;
	}
	if (len > 0) {
		val = musb_readl(fifo, 0);
		memcpy(dst, &val, len);
	}
}
