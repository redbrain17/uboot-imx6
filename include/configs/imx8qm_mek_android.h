/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef IMX8QM_MEK_ANDROID_H
#define IMX8QM_MEK_ANDROID_H

#define CONFIG_USBD_HS

#define CONFIG_BCB_SUPPORT
#define CONFIG_CMD_READ
#define CONFIG_USB_GADGET_VBUS_DRAW	2

#define CONFIG_ANDROID_AB_SUPPORT
#define CONFIG_AVB_SUPPORT
#define CONFIG_SUPPORT_EMMC_RPMB
#define CONFIG_SYSTEM_RAMDISK_SUPPORT
#define CONFIG_AVB_FUSE_BANK_SIZEW 0
#define CONFIG_AVB_FUSE_BANK_START 0
#define CONFIG_AVB_FUSE_BANK_END 0
#define CONFIG_FASTBOOT_LOCK
#define FSL_FASTBOOT_FB_DEV "mmc"

#define IMX_LOAD_HDMI_FIMRWARE
#define IMX_HDMI_FIRMWARE_LOAD_ADDR (CONFIG_SYS_SDRAM_BASE + SZ_256M)
#define IMX_HDMITX_FIRMWARE_SIZE 0x20000
#define IMX_HDMIRX_FIRMWARE_SIZE 0x20000

#ifdef CONFIG_SYS_MALLOC_LEN
#undef CONFIG_SYS_MALLOC_LEN
#define CONFIG_SYS_MALLOC_LEN           (96 * SZ_1M)
#endif

#define CONFIG_USB_FUNCTION_FASTBOOT
#define CONFIG_CMD_FASTBOOT

#define CONFIG_ANDROID_BOOT_IMAGE
#define CONFIG_FASTBOOT_FLASH

#define CONFIG_FSL_FASTBOOT
#define CONFIG_FASTBOOT_USB_DEV 1
#define CONFIG_ANDROID_RECOVERY

#if defined CONFIG_SYS_BOOT_SATA
#define CONFIG_FASTBOOT_STORAGE_SATA
#define CONFIG_FASTBOOT_SATA_NO 0
#else
#define CONFIG_FASTBOOT_STORAGE_MMC
#endif

#define CONFIG_CMD_BOOTA
#define CONFIG_SUPPORT_RAW_INITRD
#define CONFIG_SERIAL_TAG

#undef CONFIG_EXTRA_ENV_SETTINGS
#undef CONFIG_BOOTCOMMAND

#define CONFIG_EXTRA_ENV_SETTINGS					\
	"splashpos=m,m\0"	  \
	"fdt_high=0xffffffffffffffff\0"	  \
	"initrd_high=0xffffffffffffffff\0" \
	"panel=NULL\0" \

#define CONFIG_FASTBOOT_BUF_ADDR   0x98000000
#define CONFIG_FASTBOOT_BUF_SIZE   0x19000000

#if defined(CONFIG_SPL_BUILD) && defined(CONFIG_NOT_UUU_BUILD)
/* Change default spl/atf/u-boot address to support partition reboot. */
#ifdef  CONFIG_SPL_TEXT_BASE
#undef  CONFIG_SPL_TEXT_BASE
#define CONFIG_SPL_TEXT_BASE           0x80000000
#endif
#ifdef  CONFIG_SPL_STACK
#undef  CONFIG_SPL_STACK
#define CONFIG_SPL_STACK               0x8003E000
#endif
#ifdef  CONFIG_SPL_BSS_START_ADDR
#undef  CONFIG_SPL_BSS_START_ADDR
#define CONFIG_SPL_BSS_START_ADDR      0x80038000
#endif
#ifdef  CONFIG_SYS_SPL_MALLOC_START
#undef  CONFIG_SYS_SPL_MALLOC_START
#define CONFIG_SYS_SPL_MALLOC_START    0x80020000
#endif
#ifdef  CONFIG_MALLOC_F_ADDR
#undef  CONFIG_MALLOC_F_ADDR
#define CONFIG_MALLOC_F_ADDR           0x80020000
#endif
#ifdef  CONFIG_ATF_TEXT_BASE
#undef  CONFIG_ATF_TEXT_BASE
#define CONFIG_ATF_TEXT_BASE           0x80040000
#endif
#ifdef  CONFIG_SYS_ATF_START
#undef  CONFIG_SYS_ATF_START
#define CONFIG_SYS_ATF_START           0x80040000
#endif
#endif

#define AVB_AB_I_UNDERSTAND_LIBAVB_AB_IS_DEPRECATED

#endif /* IMX8QM_MEK_ANDROID_H */
