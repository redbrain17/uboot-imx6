/*
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <linux/errno.h>
#include <asm/arch/clock.h>
#include <asm/imx-common/sci/sci.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/i2c.h>
#include <asm/arch/sys_proto.h>

DECLARE_GLOBAL_DATA_PTR;

u32 get_lpuart_clk(void)
{
	return mxc_get_clock(MXC_UART_CLK);
}

static u32 get_arm_main_clk(void)
{
	sc_err_t err;
	sc_pm_clock_rate_t clkrate;

	if (is_imx8qm())
		err = sc_pm_get_clock_rate((sc_ipc_t)gd->arch.ipc_channel_handle,
				SC_R_A53, SC_PM_CLK_CPU, &clkrate);
	else if (is_imx8qxp())
		err = sc_pm_get_clock_rate((sc_ipc_t)gd->arch.ipc_channel_handle,
				SC_R_A35, SC_PM_CLK_CPU, &clkrate);
	else
		err = SC_ERR_UNAVAILABLE;

	if (err != SC_ERR_NONE) {
		printf("sc get ARM clk failed! err=%d\n", err);
		return 0;
	}
	return clkrate;
}

unsigned int mxc_get_clock(enum mxc_clock clk)
{
	sc_err_t err;
	sc_pm_clock_rate_t clkrate;

	switch (clk) {
	case MXC_UART_CLK:
		err = sc_pm_get_clock_rate((sc_ipc_t)gd->arch.ipc_channel_handle,
				SC_R_UART_0, 2, &clkrate);
		if (err != SC_ERR_NONE) {
			printf("sc get UART clk failed! err=%d\n", err);
			return 0;
		}
		return clkrate;
	case MXC_ESDHC_CLK:
		err = sc_pm_get_clock_rate((sc_ipc_t)gd->arch.ipc_channel_handle,
				SC_R_SDHC_0, 2, &clkrate);
		if (err != SC_ERR_NONE) {
			printf("sc get uSDHC1 clk failed! err=%d\n", err);
			return 0;
		}
		return clkrate;
	case MXC_ESDHC2_CLK:
		err = sc_pm_get_clock_rate((sc_ipc_t)gd->arch.ipc_channel_handle,
				SC_R_SDHC_1, 2, &clkrate);
		if (err != SC_ERR_NONE) {
			printf("sc get uSDHC2 clk failed! err=%d\n", err);
			return 0;
		}
		return clkrate;
	case MXC_ESDHC3_CLK:
		err = sc_pm_get_clock_rate((sc_ipc_t)gd->arch.ipc_channel_handle,
				SC_R_SDHC_2, 2, &clkrate);
		if (err != SC_ERR_NONE) {
			printf("sc get uSDHC3 clk failed! err=%d\n", err);
			return 0;
		}
		return clkrate;
	case MXC_FEC_CLK:
		err = sc_pm_get_clock_rate((sc_ipc_t)gd->arch.ipc_channel_handle,
				SC_R_ENET_0, 2, &clkrate);
		if (err != SC_ERR_NONE) {
			printf("sc get ENET clk failed! err=%d\n", err);
			return 0;
		}
		return clkrate;
	case MXC_ARM_CLK:
		return get_arm_main_clk();
	default:
		printf("Unsupported mxc_clock %d\n", clk);
		break;
	}

	return 0;
}

u32 imx_get_fecclk(void)
{
	return mxc_get_clock(MXC_FEC_CLK);
}

int enable_i2c_clk(unsigned char enable, unsigned i2c_num)
{
	sc_ipc_t ipc;
	sc_err_t err;

	if (i2c_num >= ARRAY_SIZE(imx_i2c_desc))
		return -EINVAL;

	ipc = gd->arch.ipc_channel_handle;

	if (enable)
		err = sc_pm_clock_enable(ipc,
			imx_i2c_desc[i2c_num].rsrc, 2, true, false);
	else
		err = sc_pm_clock_enable(ipc,
			imx_i2c_desc[i2c_num].rsrc, 2, false, false);

	if (err != SC_ERR_NONE) {
		printf("i2c clock error %d\n", err);
		return -EPERM;
	}

	return 0;
}

u32 imx_get_i2cclk(unsigned i2c_num)
{
	sc_err_t err;
	sc_ipc_t ipc;
	u32 clock_rate;

	if (i2c_num >= ARRAY_SIZE(imx_i2c_desc))
		return 0;

	ipc = gd->arch.ipc_channel_handle;
	err = sc_pm_get_clock_rate(ipc, imx_i2c_desc[i2c_num].rsrc, 2,
		&clock_rate);
	if (err != SC_ERR_NONE)
		return 0;

	return clock_rate;
}

void init_clk_fspi(int index)
{
	sc_err_t sciErr = 0;
	sc_pm_clock_rate_t rate;
	sc_ipc_t ipcHndl = gd->arch.ipc_channel_handle;

	/* Set FSPI0 clock root to 29 MHz */
	rate = 29000000;
	sciErr = sc_pm_set_clock_rate(ipcHndl, SC_R_FSPI_0, SC_PM_CLK_PER, &rate);
	if (sciErr != SC_ERR_NONE) {
		puts("FSPI0 setrate failed\n");
		return;
	}

	/* Enable FSPI0 clock root */
	sciErr = sc_pm_clock_enable(ipcHndl, SC_R_FSPI_0, SC_PM_CLK_PER, true, false);
	if (sciErr != SC_ERR_NONE) {
		puts("FSPI0 enable clock failed\n");
		return;
	}

	return;
}

void init_clk_gpmi_nand(void)
{
	sc_err_t sciErr = 0;
	sc_pm_clock_rate_t rate;
	sc_ipc_t ipcHndl = gd->arch.ipc_channel_handle;

	/* Set NAND BCH clock root to 50 MHz */
	rate = 50000000;
	sciErr = sc_pm_set_clock_rate(ipcHndl, SC_R_NAND, SC_PM_CLK_PER, &rate);
	if (sciErr != SC_ERR_NONE) {
		puts("NAND BCH set rate failed\n");
		return;
	}

	/* Enable NAND BCH clock root */
	sciErr = sc_pm_clock_enable(ipcHndl, SC_R_NAND, SC_PM_CLK_PER, true, false);
	if (sciErr != SC_ERR_NONE) {
		puts("NAND BCH enable clock failed\n");
		return;
	}

	/* Set NAND GPMI clock root to 50 MHz */
	rate = 50000000;
	sciErr = sc_pm_set_clock_rate(ipcHndl, SC_R_NAND, SC_PM_CLK_MST_BUS, &rate);
	if (sciErr != SC_ERR_NONE) {
		puts("NAND GPMI set rate failed\n");
		return;
	 }

	/* Enable NAND GPMI clock root */
	sciErr = sc_pm_clock_enable(ipcHndl, SC_R_NAND, SC_PM_CLK_MST_BUS, true, false);
	if (sciErr != SC_ERR_NONE) {
		puts("NAND GPMI enable clock failed\n");
		return;
	}

	return;
}

void enable_usboh3_clk(unsigned char enable)
{
	return;
}

void init_clk_usb3(int index)
{
	sc_err_t err;
	sc_ipc_t ipc;

	ipc = gd->arch.ipc_channel_handle;

	err = sc_pm_clock_enable(ipc, SC_R_USB_2, SC_PM_CLK_MISC, true, false);
	if (err != SC_ERR_NONE)
		printf("USB3 set clock failed!, line=%d (error = %d)\n",
			__LINE__, err);

	err = sc_pm_clock_enable(ipc, SC_R_USB_2, SC_PM_CLK_MST_BUS, true, false);
	if (err != SC_ERR_NONE)
		printf("USB3 set clock failed!, line=%d (error = %d)\n",
			__LINE__, err);

	err = sc_pm_clock_enable(ipc, SC_R_USB_2, SC_PM_CLK_PER, true, false);
	if (err != SC_ERR_NONE)
		printf("USB3 set clock failed!, line=%d (error = %d)\n",
			__LINE__, err);

	return;
}


void init_clk_usdhc(u32 index)
{
#ifdef CONFIG_IMX8QM
	sc_rsrc_t usdhcs[] = {SC_R_SDHC_0, SC_R_SDHC_1, SC_R_SDHC_2};
	u32 instances = 3;
#else
	sc_rsrc_t usdhcs[] = {SC_R_SDHC_0, SC_R_SDHC_1};
	u32 instances = 2;
#endif

	sc_err_t err;
	sc_ipc_t ipc;
	sc_pm_clock_rate_t actual = 200000000;

	ipc = gd->arch.ipc_channel_handle;

	if (index >= instances)
		return;

	err = sc_pm_set_clock_rate(ipc, usdhcs[index], 2, &actual);
	if (err != SC_ERR_NONE) {
		printf("SDHC_%d set clock failed! (error = %d)\n", index, err);
		return;
	}

	if (actual != 200000000)
		debug("Actual rate for SDHC_%d is %d\n", index, actual);

	err = sc_pm_clock_enable(ipc, usdhcs[index], SC_PM_CLK_PER, true, false);
	if (err != SC_ERR_NONE) {
		printf("SDHC_%d per clk enable failed!\n", index);
		return;
	}
}

void init_clk_fec(int index)
{
	sc_err_t err;
	sc_ipc_t ipc;
	sc_pm_clock_rate_t rate = 24000000;
	sc_rsrc_t enet[2] = {SC_R_ENET_0, SC_R_ENET_1};

	if (index > 1)
		return;

	if (index == -1)
		index = 0;

	ipc = gd->arch.ipc_channel_handle;

	/* Disable SC_R_ENET_0 clock root */
	err = sc_pm_clock_enable(ipc, enet[index], 0, false, false);
	err |= sc_pm_clock_enable(ipc, enet[index], 2, false, false);
	err |= sc_pm_clock_enable(ipc, enet[index], 4, false, false);
	if (err != SC_ERR_NONE) {
		printf("\nSC_R_ENET_0 set clock disable failed! (error = %d)\n", err);
		return;
	}

	/* Set SC_R_ENET_0 clock root to 125 MHz */
	rate = 125000000;

	/* div = 8 clk_source = PLL_1 ss_slice #7 in verfication codes */
	err = sc_pm_set_clock_rate(ipc, enet[index], 2, &rate);
	if (err != SC_ERR_NONE) {
		printf("\nSC_R_ENET_0 set clock ref clock 125M failed! (error = %d)\n", err);
		return;
	}

	/* Enable SC_R_ENET_0 clock root */
	err = sc_pm_clock_enable(ipc, enet[index], 0, true, true);
	err |= sc_pm_clock_enable(ipc, enet[index], 2, true, true);
	err |= sc_pm_clock_enable(ipc, enet[index], 4, true, true);
	if (err != SC_ERR_NONE) {
		printf("\nSC_R_ENET_0 set clock enable failed! (error = %d)\n", err);
		return;
	}
}

