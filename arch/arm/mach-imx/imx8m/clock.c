/*
 * Copyright 2017 NXP
 *
 * Peng Fan <peng.fan@nxp.com>
 *
 * SPDX-License-Identifier:     GPL-2.0+
 */

#include <common.h>
#include <asm/arch/clock.h>
#include <asm/arch/imx-regs.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <errno.h>
#include <linux/iopoll.h>

DECLARE_GLOBAL_DATA_PTR;

static struct anamix_pll *ana_pll = (struct anamix_pll *)ANATOP_BASE_ADDR;

static u32 decode_frac_pll(enum clk_root_src frac_pll)
{
	u32 pll_cfg0, pll_cfg1, pllout;
	u32 pll_refclk_sel, pll_refclk;
	u32 divr_val, divq_val, divf_val, divff, divfi;
	u32 pllout_div_shift, pllout_div_mask, pllout_div;

	switch (frac_pll) {
	case ARM_PLL_CLK:
		pll_cfg0 = readl(&ana_pll->arm_pll_cfg0);
		pll_cfg1 = readl(&ana_pll->arm_pll_cfg1);
		pllout_div_shift = HW_FRAC_ARM_PLL_DIV_SHIFT;
		pllout_div_mask = HW_FRAC_ARM_PLL_DIV_MASK;
		break;
	default:
		printf("Frac PLL %d not supporte\n", frac_pll);
		return 0;
	}

	pllout_div = readl(&ana_pll->frac_pllout_div_cfg);
	pllout_div = (pllout_div & pllout_div_mask) >> pllout_div_shift;

	/* Power down */
	if (pll_cfg0 & FRAC_PLL_PD_MASK)
		return 0;

	/* output not enabled */
	if ((pll_cfg0 & FRAC_PLL_CLKE_MASK) == 0)
		return 0;

	pll_refclk_sel = pll_cfg0 & FRAC_PLL_REFCLK_SEL_MASK;

	if (pll_refclk_sel == FRAC_PLL_REFCLK_SEL_OSC_25M)
		pll_refclk = 25000000u;
	else if (pll_refclk_sel == FRAC_PLL_REFCLK_SEL_OSC_27M)
		pll_refclk = 27000000u;
	else if (pll_refclk_sel == FRAC_PLL_REFCLK_SEL_HDMI_PHY_27M)
		pll_refclk = 27000000u;
	else
		pll_refclk = 0;

	if (pll_cfg0 & FRAC_PLL_BYPASS_MASK)
		return pll_refclk;

	divr_val = (pll_cfg0 & FRAC_PLL_REFCLK_DIV_VAL_MASK) >>
		FRAC_PLL_REFCLK_DIV_VAL_SHIFT;
	divq_val = pll_cfg0 & FRAC_PLL_OUTPUT_DIV_VAL_MASK;

	divff = (pll_cfg1 & FRAC_PLL_FRAC_DIV_CTL_MASK) >>
		FRAC_PLL_FRAC_DIV_CTL_SHIFT;
	divfi = pll_cfg1 & FRAC_PLL_INT_DIV_CTL_MASK;

	divf_val = 1 + divfi + divff / (1 << 24);

	pllout = pll_refclk / (divr_val + 1) * 8 * divf_val /
		((divq_val + 1) * 2);

	return pllout / (pllout_div + 1);
}

static u32 decode_sscg_pll(enum clk_root_src sscg_pll)
{
	u32 pll_cfg0, pll_cfg1, pll_cfg2;
	u32 pll_refclk_sel, pll_refclk;
	u32 divr1, divr2, divf1, divf2, divq, div;
	u32 sse;
	u32 pll_clke;
	u32 pllout_div_shift, pllout_div_mask, pllout_div;
	u32 pllout;

	switch (sscg_pll) {
	case SYSTEM_PLL1_800M_CLK:
	case SYSTEM_PLL1_400M_CLK:
	case SYSTEM_PLL1_266M_CLK:
	case SYSTEM_PLL1_200M_CLK:
	case SYSTEM_PLL1_160M_CLK:
	case SYSTEM_PLL1_133M_CLK:
	case SYSTEM_PLL1_100M_CLK:
	case SYSTEM_PLL1_80M_CLK:
	case SYSTEM_PLL1_40M_CLK:
		pll_cfg0 = readl(&ana_pll->sys_pll1_cfg0);
		pll_cfg1 = readl(&ana_pll->sys_pll1_cfg1);
		pll_cfg2 = readl(&ana_pll->sys_pll1_cfg2);
		pllout_div_shift = HW_SSCG_SYSTEM_PLL1_DIV_SHIFT;
		pllout_div_mask = HW_SSCG_SYSTEM_PLL1_DIV_MASK;
		break;
	case SYSTEM_PLL2_1000M_CLK:
	case SYSTEM_PLL2_500M_CLK:
	case SYSTEM_PLL2_333M_CLK:
	case SYSTEM_PLL2_250M_CLK:
	case SYSTEM_PLL2_200M_CLK:
	case SYSTEM_PLL2_166M_CLK:
	case SYSTEM_PLL2_125M_CLK:
	case SYSTEM_PLL2_100M_CLK:
	case SYSTEM_PLL2_50M_CLK:
		pll_cfg0 = readl(&ana_pll->sys_pll2_cfg0);
		pll_cfg1 = readl(&ana_pll->sys_pll2_cfg1);
		pll_cfg2 = readl(&ana_pll->sys_pll2_cfg2);
		pllout_div_shift = HW_SSCG_SYSTEM_PLL2_DIV_SHIFT;
		pllout_div_mask = HW_SSCG_SYSTEM_PLL2_DIV_MASK;
		break;
	case SYSTEM_PLL3_CLK:
		pll_cfg0 = readl(&ana_pll->sys_pll3_cfg0);
		pll_cfg1 = readl(&ana_pll->sys_pll3_cfg1);
		pll_cfg2 = readl(&ana_pll->sys_pll3_cfg2);
		pllout_div_shift = HW_SSCG_SYSTEM_PLL3_DIV_SHIFT;
		pllout_div_mask = HW_SSCG_SYSTEM_PLL3_DIV_MASK;
		break;
	case DRAM_PLL1_CLK:
		pll_cfg0 = readl(&ana_pll->dram_pll_cfg0);
		pll_cfg1 = readl(&ana_pll->dram_pll_cfg1);
		pll_cfg2 = readl(&ana_pll->dram_pll_cfg2);
		pllout_div_shift = HW_SSCG_DRAM_PLL_DIV_SHIFT;
		pllout_div_mask = HW_SSCG_DRAM_PLL_DIV_MASK;
		break;
	default:
		printf("sscg pll %d not supporte\n", sscg_pll);
		return 0;
	}

	switch (sscg_pll) {
	case DRAM_PLL1_CLK:
		pll_clke = SSCG_PLL_DRAM_PLL_CLKE_MASK;
		div = 1;
		break;
	case SYSTEM_PLL3_CLK:
		pll_clke = SSCG_PLL_PLL3_CLKE_MASK;
		div = 1;
		break;
	case SYSTEM_PLL2_1000M_CLK:
	case SYSTEM_PLL1_800M_CLK:
		pll_clke = SSCG_PLL_CLKE_MASK;
		div = 1;
		break;
	case SYSTEM_PLL2_500M_CLK:
	case SYSTEM_PLL1_400M_CLK:
		pll_clke = SSCG_PLL_DIV2_CLKE_MASK;
		div = 2;
		break;
	case SYSTEM_PLL2_333M_CLK:
	case SYSTEM_PLL1_266M_CLK:
		pll_clke = SSCG_PLL_DIV3_CLKE_MASK;
		div = 3;
		break;
	case SYSTEM_PLL2_250M_CLK:
	case SYSTEM_PLL1_200M_CLK:
		pll_clke = SSCG_PLL_DIV4_CLKE_MASK;
		div = 4;
		break;
	case SYSTEM_PLL2_200M_CLK:
	case SYSTEM_PLL1_160M_CLK:
		pll_clke = SSCG_PLL_DIV5_CLKE_MASK;
		div = 5;
		break;
	case SYSTEM_PLL2_166M_CLK:
	case SYSTEM_PLL1_133M_CLK:
		pll_clke = SSCG_PLL_DIV6_CLKE_MASK;
		div = 6;
		break;
	case SYSTEM_PLL2_125M_CLK:
	case SYSTEM_PLL1_100M_CLK:
		pll_clke = SSCG_PLL_DIV8_CLKE_MASK;
		div = 8;
		break;
	case SYSTEM_PLL2_100M_CLK:
	case SYSTEM_PLL1_80M_CLK:
		pll_clke = SSCG_PLL_DIV10_CLKE_MASK;
		div = 10;
		break;
	case SYSTEM_PLL2_50M_CLK:
	case SYSTEM_PLL1_40M_CLK:
		pll_clke = SSCG_PLL_DIV20_CLKE_MASK;
		div = 20;
		break;
	default:
		printf("sscg pll %d not supporte\n", sscg_pll);
		return 0;
	}

	/* Power down */
	if (pll_cfg0 & SSCG_PLL_PD_MASK)
		return 0;

	/* output not enabled */
	if ((pll_cfg0 & pll_clke) == 0)
		return 0;

	pllout_div = readl(&ana_pll->sscg_pllout_div_cfg);
	pllout_div = (pllout_div & pllout_div_mask) >> pllout_div_shift;

	pll_refclk_sel = pll_cfg0 & SSCG_PLL_REFCLK_SEL_MASK;

	if (pll_refclk_sel == SSCG_PLL_REFCLK_SEL_OSC_25M)
		pll_refclk = 25000000u;
	else if (pll_refclk_sel == SSCG_PLL_REFCLK_SEL_OSC_27M)
		pll_refclk = 27000000u;
	else if (pll_refclk_sel == SSCG_PLL_REFCLK_SEL_HDMI_PHY_27M)
		pll_refclk = 27000000u;
	else
		pll_refclk = 0;

	/* We assume bypass1/2 are the same value */
	if ((pll_cfg0 & SSCG_PLL_BYPASS1_MASK) ||
	    (pll_cfg0 & SSCG_PLL_BYPASS2_MASK))
		return pll_refclk;

	divr1 = (pll_cfg2 & SSCG_PLL_REF_DIVR1_MASK) >>
		SSCG_PLL_REF_DIVR1_SHIFT;
	divr2 = (pll_cfg2 & SSCG_PLL_REF_DIVR2_MASK) >>
		SSCG_PLL_REF_DIVR2_SHIFT;
	divf1 = (pll_cfg2 & SSCG_PLL_FEEDBACK_DIV_F1_MASK) >>
		SSCG_PLL_FEEDBACK_DIV_F1_SHIFT;
	divf2 = (pll_cfg2 & SSCG_PLL_FEEDBACK_DIV_F2_MASK) >>
		SSCG_PLL_FEEDBACK_DIV_F2_SHIFT;
	divq = (pll_cfg2 & SSCG_PLL_OUTPUT_DIV_VAL_MASK) >>
		SSCG_PLL_OUTPUT_DIV_VAL_SHIFT;
	sse = pll_cfg1 & SSCG_PLL_SSE_MASK;

	if (sse)
		sse = 8;
	else
		sse = 2;

	pllout = pll_refclk / (divr1 + 1) * sse * (divf1 + 1) /
		(divr2 + 1) * (divf2 + 1) / (divq + 1);

	return pllout / (pllout_div + 1) / div;
}

static u32 get_root_src_clk(enum clk_root_src root_src)
{
	switch (root_src) {
	case OSC_25M_CLK:
		return 25000000;
	case OSC_27M_CLK:
		return 25000000;
	case OSC_32K_CLK:
		return 32000;
	case ARM_PLL_CLK:
		return decode_frac_pll(root_src);
	case SYSTEM_PLL1_800M_CLK:
	case SYSTEM_PLL1_400M_CLK:
	case SYSTEM_PLL1_266M_CLK:
	case SYSTEM_PLL1_200M_CLK:
	case SYSTEM_PLL1_160M_CLK:
	case SYSTEM_PLL1_133M_CLK:
	case SYSTEM_PLL1_100M_CLK:
	case SYSTEM_PLL1_80M_CLK:
	case SYSTEM_PLL1_40M_CLK:
	case SYSTEM_PLL2_1000M_CLK:
	case SYSTEM_PLL2_500M_CLK:
	case SYSTEM_PLL2_333M_CLK:
	case SYSTEM_PLL2_250M_CLK:
	case SYSTEM_PLL2_200M_CLK:
	case SYSTEM_PLL2_166M_CLK:
	case SYSTEM_PLL2_125M_CLK:
	case SYSTEM_PLL2_100M_CLK:
	case SYSTEM_PLL2_50M_CLK:
	case SYSTEM_PLL3_CLK:
		return decode_sscg_pll(root_src);
	default:
		return 0;
	}

	return 0;
}

static u32 get_root_clk(enum clk_root_index clock_id)
{
	enum clk_root_src root_src;
	u32 post_podf, pre_podf, root_src_clk;

	if (clock_root_enabled(clock_id) <= 0)
		return 0;

	if (clock_get_prediv(clock_id, &pre_podf) < 0)
		return 0;

	if (clock_get_postdiv(clock_id, &post_podf) < 0)
		return 0;

	if (clock_get_src(clock_id, &root_src) < 0)
		return 0;

	root_src_clk = get_root_src_clk(root_src);

	return root_src_clk / (post_podf + 1) / (pre_podf + 1);
}

#ifdef CONFIG_SECURE_BOOT
void hab_caam_clock_enable(unsigned char enable)
{
	/* The CAAM clock is always on for iMX8M */
}
#endif

#ifdef CONFIG_MXC_OCOTP
void enable_ocotp_clk(unsigned char enable)
{
	clock_enable(CCGR_OCOTP, !!enable);
}
#endif

int enable_i2c_clk(unsigned char enable, unsigned int i2c_num)
{
	/* 0 - 3 is valid i2c num */
	if (i2c_num > 3)
		return -EINVAL;

	clock_enable(CCGR_I2C1 + i2c_num, !!enable);

	return 0;
}

unsigned int mxc_get_clock(enum clk_root_index clk)
{
	u32 val;

	if (clk >= CLK_ROOT_MAX)
		return 0;

	if (clk == MXC_ARM_CLK)
		return get_root_clk(ARM_A53_CLK_ROOT);

	if (clk == MXC_IPG_CLK) {
		clock_get_target_val(IPG_CLK_ROOT, &val);
		val = val & 0x3;
		return get_root_clk(AHB_CLK_ROOT) / (val + 1);
	}

	return get_root_clk(clk);
}

u32 imx_get_uartclk(void)
{
	return mxc_get_clock(UART1_CLK_ROOT);
}

void mxs_set_lcdclk(u32 base_addr, u32 freq)
{
	/*
	 * LCDIF_PIXEL_CLK: select 800MHz root clock,
	 * select pre divider 8, output is 100 MHz
	 */
	clock_set_target_val(LCDIF_PIXEL_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(4) |
			     CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV8));
}

void init_wdog_clk(void)
{
	clock_enable(CCGR_WDOG1, 0);
	clock_enable(CCGR_WDOG2, 0);
	clock_enable(CCGR_WDOG3, 0);
	clock_set_target_val(WDOG_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(0));
	clock_set_target_val(WDOG_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(0));
	clock_set_target_val(WDOG_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(0));
	clock_enable(CCGR_WDOG1, 1);
	clock_enable(CCGR_WDOG2, 1);
	clock_enable(CCGR_WDOG3, 1);
}

void init_usb_clk(void)
{
	if (!is_usb_boot()) {
		clock_enable(CCGR_USB_CTRL1, 0);
		clock_enable(CCGR_USB_CTRL2, 0);
		clock_enable(CCGR_USB_PHY1, 0);
		clock_enable(CCGR_USB_PHY2, 0);
		/* 500MHz */
		clock_set_target_val(USB_BUS_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(1));
		/* 100MHz */
		clock_set_target_val(USB_CORE_REF_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(1));
		/* 100MHz */
		clock_set_target_val(USB_PHY_REF_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(1));
		clock_enable(CCGR_USB_CTRL1, 1);
		clock_enable(CCGR_USB_CTRL2, 1);
		clock_enable(CCGR_USB_PHY1, 1);
		clock_enable(CCGR_USB_PHY2, 1);
	}
}

void init_uart_clk(u32 index)
{
	/* Set uart clock root 25M OSC */
	switch (index) {
	case 0:
		clock_enable(CCGR_UART1, 0);
		clock_set_target_val(UART1_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_UART1, 1);
		return;
	case 1:
		clock_enable(CCGR_UART2, 0);
		clock_set_target_val(UART2_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_UART2, 1);
		return;
	case 2:
		clock_enable(CCGR_UART3, 0);
		clock_set_target_val(UART3_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_UART3, 1);
		return;
	case 3:
		clock_enable(CCGR_UART4, 0);
		clock_set_target_val(UART4_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(0));
		clock_enable(CCGR_UART4, 1);
		return;
	default:
		printf("Invalid uart index\n");
		return;
	}
}

void init_clk_usdhc(u32 index)
{
	/*
	 * set usdhc clock root
	 * sys pll1 400M
	 */
	switch (index) {
	case 0:
		clock_enable(CCGR_USDHC1, 0);
		clock_set_target_val(USDHC1_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(1) |
				     CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV2));
		clock_enable(CCGR_USDHC1, 1);
		return;
	case 1:
		clock_enable(CCGR_USDHC2, 0);
		clock_set_target_val(USDHC2_CLK_ROOT, CLK_ROOT_ON |
				     CLK_ROOT_SOURCE_SEL(1) |
				     CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV2));
		clock_enable(CCGR_USDHC2, 1);
		return;
	default:
		printf("Invalid usdhc index\n");
		return;
	}
}

int set_clk_qspi(void)
{
	/*
	 * set qspi root
	 * sys pll1 100M
	 */
	clock_enable(CCGR_QSPI, 0);
	clock_set_target_val(QSPI_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(7));
	clock_enable(CCGR_QSPI, 1);

	return 0;
}

#ifdef CONFIG_FEC_MXC
int set_clk_enet(enum enet_freq type)
{
	u32 target;
	u32 enet1_ref;

	switch (type) {
	case ENET_125MHZ:
		enet1_ref = ENET1_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_125M_CLK;
		break;
	case ENET_50MHZ:
		enet1_ref = ENET1_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_50M_CLK;
		break;
	case ENET_25MHZ:
		enet1_ref = ENET1_REF_CLK_ROOT_FROM_PLL_ENET_MAIN_25M_CLK;
		break;
	default:
		return -EINVAL;
	}

	/* disable the clock first */
	clock_enable(CCGR_ENET1, 0);
	clock_enable(CCGR_SIM_ENET, 0);

	/* set enet axi clock 266Mhz */
	target = CLK_ROOT_ON | ENET_AXI_CLK_ROOT_FROM_SYS1_PLL_266M |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(ENET_AXI_CLK_ROOT, target);

	target = CLK_ROOT_ON | enet1_ref |
		 CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		 CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1);
	clock_set_target_val(ENET_REF_CLK_ROOT, target);

	target = CLK_ROOT_ON |
		ENET1_TIME_CLK_ROOT_FROM_PLL_ENET_MAIN_100M_CLK |
		CLK_ROOT_PRE_DIV(CLK_ROOT_PRE_DIV1) |
		CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV4);
	clock_set_target_val(ENET_TIMER_CLK_ROOT, target);

	/* enable clock */
	clock_enable(CCGR_SIM_ENET, 1);
	clock_enable(CCGR_ENET1, 1);

	return 0;
}
#endif

u32 imx_get_fecclk(void)
{
	return get_root_clk(ENET_AXI_CLK_ROOT);
}

#ifdef CONFIG_SPL_BUILD
void dram_pll_init(enum sscg_pll_out_val pll_val)
{
	struct src *src = (struct src *)SRC_BASE_ADDR;
	void __iomem *pll_control_reg = &ana_pll->dram_pll_cfg0;
	void __iomem *pll_cfg_reg2 = &ana_pll->dram_pll_cfg2;
	u32 pwdn_mask = 0, pll_clke = 0, bypass1 = 0, bypass2 = 0;
	u32 val;
	int ret;

	setbits_le32(GPC_BASE_ADDR + 0xEC, BIT(7));
	setbits_le32(GPC_BASE_ADDR + 0xF8, BIT(5));

	pwdn_mask = SSCG_PLL_PD_MASK;
	pll_clke = SSCG_PLL_DRAM_PLL_CLKE_MASK;
	bypass1 = SSCG_PLL_BYPASS1_MASK;
	bypass2 = SSCG_PLL_BYPASS2_MASK;

	/* Enable DDR1 and DDR2 domain */
	writel(SRC_DDR1_ENABLE_MASK, &src->ddr1_rcr);
	writel(SRC_DDR1_ENABLE_MASK, &src->ddr2_rcr);

	/* Bypass */
	setbits_le32(pll_control_reg, bypass1);
	setbits_le32(pll_control_reg, bypass2);

	switch (pll_val) {
		case SSCG_PLL_OUT_400M:
			val = readl(pll_cfg_reg2);
			val &= ~(SSCG_PLL_OUTPUT_DIV_VAL_MASK | SSCG_PLL_FEEDBACK_DIV_F2_MASK);
			val |= SSCG_PLL_OUTPUT_DIV_VAL(1);
			val |= SSCG_PLL_FEEDBACK_DIV_F2_VAL(11);
			writel(val, pll_cfg_reg2);
			break;
		case SSCG_PLL_OUT_600M:
			val = readl(pll_cfg_reg2);
			val &= ~(SSCG_PLL_OUTPUT_DIV_VAL_MASK | SSCG_PLL_FEEDBACK_DIV_F2_MASK);
			val |= SSCG_PLL_OUTPUT_DIV_VAL(1);
			val |= SSCG_PLL_FEEDBACK_DIV_F2_VAL(17);
			writel(val, pll_cfg_reg2);
			break;
		case SSCG_PLL_OUT_800M:
			val = readl(pll_cfg_reg2);
			val &= ~(SSCG_PLL_OUTPUT_DIV_VAL_MASK | SSCG_PLL_FEEDBACK_DIV_F2_MASK);
			val |= SSCG_PLL_OUTPUT_DIV_VAL(0);
			val |= SSCG_PLL_FEEDBACK_DIV_F2_VAL(11);
			writel(val, pll_cfg_reg2);
			break;
	}

	/* Clear power down bit */
	clrbits_le32(pll_control_reg, pwdn_mask);
	/* Eanble ARM_PLL/SYS_PLL  */
	setbits_le32(pll_control_reg, pll_clke);

	/* Clear bypass */
	clrbits_le32(pll_control_reg, bypass1);
	__udelay(100);
	clrbits_le32(pll_control_reg, bypass2);
	/* Wait lock */
	ret = readl_poll_timeout(pll_control_reg, val,
				 val & SSCG_PLL_LOCK_MASK, 1);
	if (ret)
		printf("%s timeout\n", __func__);
}

int frac_pll_init(u32 pll, enum frac_pll_out_val val)
{
	void __iomem *pll_cfg0, __iomem *pll_cfg1;
	u32 val_cfg0, val_cfg1;
	int ret;

	switch (pll) {
	case ANATOP_ARM_PLL:
		pll_cfg0 = &ana_pll->arm_pll_cfg0;
		pll_cfg1 = &ana_pll->arm_pll_cfg1;

		if (val == FRAC_PLL_OUT_1000M)
			val_cfg1 = FRAC_PLL_INT_DIV_CTL_VAL(49);
		else
			val_cfg1 = FRAC_PLL_INT_DIV_CTL_VAL(79);
		val_cfg0 = FRAC_PLL_CLKE_MASK | FRAC_PLL_REFCLK_SEL_OSC_25M |
			FRAC_PLL_LOCK_SEL_MASK | FRAC_PLL_NEWDIV_VAL_MASK |
			FRAC_PLL_REFCLK_DIV_VAL(4) |
			FRAC_PLL_OUTPUT_DIV_VAL(0);
		break;
	default:
		return -EINVAL;
	}

	/* bypass the clock */
	setbits_le32(pll_cfg0, FRAC_PLL_BYPASS_MASK);
	/* Set the value */
	writel(val_cfg1, pll_cfg1);
	writel(val_cfg0 | FRAC_PLL_BYPASS_MASK, pll_cfg0);
	val_cfg0 = readl(pll_cfg0);
	/* unbypass the clock */
	clrbits_le32(pll_cfg0, FRAC_PLL_BYPASS_MASK);
	ret = readl_poll_timeout(pll_cfg0, val_cfg0,
				 val_cfg0 & FRAC_PLL_LOCK_MASK, 1);
	if (ret)
		printf("%s timeout\n", __func__);
	clrbits_le32(pll_cfg0, FRAC_PLL_NEWDIV_VAL_MASK);

	return 0;
}

int sscg_pll_init(u32 pll)
{
	void __iomem *pll_cfg0, __iomem *pll_cfg1, __iomem *pll_cfg2;
	u32 val_cfg0, val_cfg1, val_cfg2, val;
	u32 bypass1_mask = 0x20, bypass2_mask = 0x10;
	int ret;

	switch (pll) {
	case ANATOP_SYSTEM_PLL1:
		pll_cfg0 = &ana_pll->sys_pll1_cfg0;
		pll_cfg1 = &ana_pll->sys_pll1_cfg1;
		pll_cfg2 = &ana_pll->sys_pll1_cfg2;
		/* 800MHz */
		val_cfg2 = SSCG_PLL_FEEDBACK_DIV_F1_VAL(3) |
			SSCG_PLL_FEEDBACK_DIV_F2_VAL(3);
		val_cfg1 = 0;
		val_cfg0 = SSCG_PLL_CLKE_MASK | SSCG_PLL_DIV2_CLKE_MASK |
			SSCG_PLL_DIV3_CLKE_MASK | SSCG_PLL_DIV4_CLKE_MASK |
			SSCG_PLL_DIV5_CLKE_MASK | SSCG_PLL_DIV6_CLKE_MASK |
			SSCG_PLL_DIV8_CLKE_MASK | SSCG_PLL_DIV10_CLKE_MASK |
			SSCG_PLL_DIV20_CLKE_MASK | SSCG_PLL_LOCK_SEL_MASK |
			SSCG_PLL_REFCLK_SEL_OSC_25M;
		break;
	case ANATOP_SYSTEM_PLL2:
		pll_cfg0 = &ana_pll->sys_pll2_cfg0;
		pll_cfg1 = &ana_pll->sys_pll2_cfg1;
		pll_cfg2 = &ana_pll->sys_pll2_cfg2;
		/* 1000MHz */
		val_cfg2 = SSCG_PLL_FEEDBACK_DIV_F1_VAL(3) |
			SSCG_PLL_FEEDBACK_DIV_F2_VAL(4);
		val_cfg1 = 0;
		val_cfg0 = SSCG_PLL_CLKE_MASK | SSCG_PLL_DIV2_CLKE_MASK |
			SSCG_PLL_DIV3_CLKE_MASK | SSCG_PLL_DIV4_CLKE_MASK |
			SSCG_PLL_DIV5_CLKE_MASK | SSCG_PLL_DIV6_CLKE_MASK |
			SSCG_PLL_DIV8_CLKE_MASK | SSCG_PLL_DIV10_CLKE_MASK |
			SSCG_PLL_DIV20_CLKE_MASK | SSCG_PLL_LOCK_SEL_MASK |
			SSCG_PLL_REFCLK_SEL_OSC_25M;
		break;
	case ANATOP_SYSTEM_PLL3:
		pll_cfg0 = &ana_pll->sys_pll3_cfg0;
		pll_cfg1 = &ana_pll->sys_pll3_cfg1;
		pll_cfg2 = &ana_pll->sys_pll3_cfg2;
		/* 800MHz */
		val_cfg2 = SSCG_PLL_FEEDBACK_DIV_F1_VAL(3) |
			SSCG_PLL_FEEDBACK_DIV_F2_VAL(3);
		val_cfg1 = 0;
		val_cfg0 = SSCG_PLL_PLL3_CLKE_MASK |  SSCG_PLL_LOCK_SEL_MASK |
			SSCG_PLL_REFCLK_SEL_OSC_25M;
		break;
	default:
		return -EINVAL;
	}

	/*bypass*/
	setbits_le32(pll_cfg0, bypass1_mask | bypass2_mask);
	/* set value */
	writel(val_cfg2, pll_cfg2);
	writel(val_cfg1, pll_cfg1);
	/*unbypass1 and wait 70us */
	writel(val_cfg0 | bypass2_mask, pll_cfg1);

	__udelay(70);

	/* unbypass2 and wait lock */
	writel(val_cfg0, pll_cfg1);
	ret = readl_poll_timeout(pll_cfg0, val, val & SSCG_PLL_LOCK_MASK, 1);
	if (ret)
		printf("%s timeout\n", __func__);

	return ret;
}

int clock_init(void)
{
	u32 grade;

	clock_set_target_val(ARM_A53_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(0));

	/*
	 * 8MQ only supports two grades: consumer and industrial.
	 * We set ARM clock to 1Ghz for consumer, 800Mhz for industrial
	 */
	grade = get_cpu_temp_grade(NULL, NULL);
	if (!grade) {
		frac_pll_init(ANATOP_ARM_PLL, FRAC_PLL_OUT_1000M);
		clock_set_target_val(ARM_A53_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(1) |
			     CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV1));
	} else {
		frac_pll_init(ANATOP_ARM_PLL, FRAC_PLL_OUT_1600M);
		clock_set_target_val(ARM_A53_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(1) |
			     CLK_ROOT_POST_DIV(CLK_ROOT_POST_DIV2));
	}
	/*
	 * According to ANAMIX SPEC
	 * sys pll1 fixed at 800MHz
	 * sys pll2 fixed at 1GHz
	 * Here we only enable the outputs.
	 */
	setbits_le32(&ana_pll->sys_pll1_cfg0, SSCG_PLL_CLKE_MASK |
		     SSCG_PLL_DIV2_CLKE_MASK | SSCG_PLL_DIV3_CLKE_MASK |
		     SSCG_PLL_DIV4_CLKE_MASK | SSCG_PLL_DIV5_CLKE_MASK |
		     SSCG_PLL_DIV6_CLKE_MASK | SSCG_PLL_DIV8_CLKE_MASK |
		     SSCG_PLL_DIV10_CLKE_MASK | SSCG_PLL_DIV20_CLKE_MASK);

	setbits_le32(&ana_pll->sys_pll2_cfg0, SSCG_PLL_CLKE_MASK |
		     SSCG_PLL_DIV2_CLKE_MASK | SSCG_PLL_DIV3_CLKE_MASK |
		     SSCG_PLL_DIV4_CLKE_MASK | SSCG_PLL_DIV5_CLKE_MASK |
		     SSCG_PLL_DIV6_CLKE_MASK | SSCG_PLL_DIV8_CLKE_MASK |
		     SSCG_PLL_DIV10_CLKE_MASK | SSCG_PLL_DIV20_CLKE_MASK);

	clock_set_target_val(NAND_USDHC_BUS_CLK_ROOT, CLK_ROOT_ON |
			     CLK_ROOT_SOURCE_SEL(1));

	init_wdog_clk();
	clock_enable(CCGR_TSENSOR, 1);

	return 0;
}
#endif

/*
 * Dump some clockes.
 */
#ifndef CONFIG_SPL_BUILD
int do_mx8m_showclocks(cmd_tbl_t *cmdtp, int flag, int argc,
		       char * const argv[])
{
	u32 freq;

	freq = decode_frac_pll(ARM_PLL_CLK);
	printf("ARM_PLL    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL1_800M_CLK);
	printf("SYS_PLL1_800    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL1_400M_CLK);
	printf("SYS_PLL1_400    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL1_266M_CLK);
	printf("SYS_PLL1_266    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL1_200M_CLK);
	printf("SYS_PLL1_200    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL1_160M_CLK);
	printf("SYS_PLL1_160    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL1_133M_CLK);
	printf("SYS_PLL1_133    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL1_100M_CLK);
	printf("SYS_PLL1_100    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL1_80M_CLK);
	printf("SYS_PLL1_80    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL1_40M_CLK);
	printf("SYS_PLL1_40    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL2_1000M_CLK);
	printf("SYS_PLL2_1000    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL2_500M_CLK);
	printf("SYS_PLL2_500    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL2_333M_CLK);
	printf("SYS_PLL2_333    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL2_250M_CLK);
	printf("SYS_PLL2_250    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL2_200M_CLK);
	printf("SYS_PLL2_200    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL2_166M_CLK);
	printf("SYS_PLL2_166    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL2_125M_CLK);
	printf("SYS_PLL2_125    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL2_100M_CLK);
	printf("SYS_PLL2_100    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL2_50M_CLK);
	printf("SYS_PLL2_50    %8d MHz\n", freq / 1000000);
	freq = decode_sscg_pll(SYSTEM_PLL3_CLK);
	printf("SYS_PLL3       %8d MHz\n", freq / 1000000);
	freq = mxc_get_clock(UART1_CLK_ROOT);
	printf("UART1          %8d MHz\n", freq / 1000000);
	freq = mxc_get_clock(USDHC1_CLK_ROOT);
	printf("USDHC1         %8d MHz\n", freq / 1000000);
	freq = mxc_get_clock(QSPI_CLK_ROOT);
	printf("QSPI           %8d MHz\n", freq / 1000000);
	return 0;
}

U_BOOT_CMD(
	clocks,	CONFIG_SYS_MAXARGS, 1, do_mx8m_showclocks,
	"display clocks",
	""
);
#endif
