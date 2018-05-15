/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef IMX8MQ_EVK_ANDROID_THINGS_H
#define IMX8MQ_EVK_ANDROID_THINGS_H

#define CONFIG_BCB_SUPPORT
#define CONFIG_CMD_READ

#define CONFIG_ANDROID_AB_SUPPORT
#define CONFIG_AVB_SUPPORT
#define CONFIG_SUPPORT_EMMC_RPMB
#define CONFIG_SYSTEM_RAMDISK_SUPPORT
#define CONFIG_AVB_FUSE_BANK_SIZEW 0
#define CONFIG_AVB_FUSE_BANK_START 0
#define CONFIG_AVB_FUSE_BANK_END 0
#define CONFIG_FASTBOOT_LOCK
#define FSL_FASTBOOT_FB_DEV "mmc"

#define CONFIG_ENABLE_LOCKSTATUS_SUPPORT

#ifdef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN           (64 * SZ_1M)
#endif

#define CONFIG_USB_FUNCTION_FASTBOOT
#define CONFIG_FASTBOOT_FLASH
#define CONFIG_FASTBOOT_STORAGE_MMC

#define CONFIG_USB_GADGET
#define CONFIG_CMD_FASTBOOT
#define CONFIG_FSL_FASTBOOT
#define CONFIG_ANDROID_RECOVERY

#define CONFIG_FASTBOOT_BUF_ADDR   CONFIG_SYS_LOAD_ADDR
#define CONFIG_FASTBOOT_BUF_SIZE   0x19000000

#define CONFIG_CMD_BOOTA
#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_SERIAL_TAG

#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOOTCOMMAND

#define CONFIG_EXTRA_ENV_SETTINGS		\
	"splashpos=m,m\0"			\
	"fdt_high=0xffffffffffffffff\0"		\
	"initrd_high=0xffffffffffffffff\0"	\

#define AVB_AB_I_UNDERSTAND_LIBAVB_AB_IS_DEPRECATED

#endif /* IMX8MQ_EVK_ANDROID_THINGS_H */
