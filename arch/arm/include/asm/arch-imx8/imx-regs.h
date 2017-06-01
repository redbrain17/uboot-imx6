/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 */

#ifndef __ASM_ARCH_IMX_REGS_H__
#define __ASM_ARCH_IMX_REGS_H__

#define MU_BASE_ADDR(id)	((0x5D1B0000UL + (id*0x10000)))

#define LPUART_BASE			0x5A060000

#define GPT1_BASE_ADDR		0x5D140000
#define SCU_LPUART_BASE		0x33220000
#define GPIO1_BASE_ADDR		0x5D080000
#define GPIO2_BASE_ADDR		0x5D090000
#define GPIO3_BASE_ADDR		0x5D0A0000
#define GPIO4_BASE_ADDR		0x5D0B0000
#define GPIO5_BASE_ADDR		0x5D0C0000
#define GPIO6_BASE_ADDR		0x5D0D0000
#define GPIO7_BASE_ADDR		0x5D0E0000
#define GPIO8_BASE_ADDR		0x5D0F0000
#define LPI2C1_BASE_ADDR	0x5A800000
#define LPI2C2_BASE_ADDR	0x5A810000
#define LPI2C3_BASE_ADDR	0x5A820000
#define LPI2C4_BASE_ADDR	0x5A830000
#define LPI2C5_BASE_ADDR	0x5A840000

#ifdef CONFIG_LPUART
#define LPUART_BASE		SCU_LPUART_BASE
#endif

#define ROM_SW_INFO_ADDR	0x00000890

#define OTG_BASE_ADDR		0x5b0d0000
#define USB_PHY0_BASE_ADDR	0x5b100000

/*
 * If ROM fail back to USB recover mode, USBPH0_PWD will be clear to use USB
 * If boot from the other mode, USB0_PWD will keep reset value
 */
#define	is_boot_from_usb(void) (!(readl(USB_PHY0_BASE_ADDR) & (1<<20)))
#define	disconnect_from_pc(void) writel(0x0, OTG_BASE_ADDR + 0x140)

#endif /* __ASM_ARCH_IMX_REGS_H__ */
