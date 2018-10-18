/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef IMX8QM_MEK_ANDROID_AUTO_XEN_H
#define IMX8QM_MEK_ANDROID_AUTO_XEN_H

#undef CONFIG_SYS_SDRAM_BASE
#undef CONFIG_NR_DRAM_BANKS
#undef PHYS_SDRAM_1
#undef PHYS_SDRAM_2
#undef PHYS_SDRAM_1_SIZE
#undef PHYS_SDRAM_2_SIZE

#define CONFIG_SYS_SDRAM_BASE		0x80000000
#define CONFIG_NR_DRAM_BANKS		2
#define PHYS_SDRAM_1			0x80000000
#define PHYS_SDRAM_2			0x200000000
#define PHYS_SDRAM_1_SIZE		0x40000000	/* 2 GB */
#define PHYS_SDRAM_2_SIZE		0x60000000	/* 1536 MB */

#undef CONFIG_LOADADDR
#define CONFIG_LOADADDR			0x80280000
#undef CONFIG_SYS_INIT_SP_ADDR
#define CONFIG_SYS_INIT_SP_ADDR		0x80200000

#undef CONFIG_REQUIRE_SERIAL_CONSOLE
#undef CONFIG_IMX_SMMU

#undef CONFIG_FASTBOOT_USB_DEV
#define CONFIG_FASTBOOT_USB_DEV 0 /* Use OTG port, not typec port */

/* This needs to be stay same in iomem in domu.cfg */
#ifdef SC_IPC_CH
#undef SC_IPC_CH
#endif
#define SC_IPC_CH			0x5d1d0000

#define AVB_AB_I_UNDERSTAND_LIBAVB_AB_IS_DEPRECATED

#endif /* IMX8QM_MEK_ANDROID_AUTO_XEN_H */
