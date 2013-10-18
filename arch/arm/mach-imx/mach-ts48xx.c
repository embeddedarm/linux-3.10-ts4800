/*
 * Copyright 2009-2010 Freescale Semiconductor, Inc. All Rights Reserved.
 * Copyright (C) 2009-2010 Amit Kucheria <amit.kucheria@canonical.com>
 *
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/input.h>
#include <linux/spi/flash.h>
#include <linux/spi/spi.h>

#include <asm/setup.h>
#include <asm/mach-types.h>
#include <asm/mach/arch.h>
#include <asm/mach/time.h>

#include "common.h"
#include "devices-imx51.h"
#include "hardware.h"
#include "iomux-mx51.h"

#define TS48XX_USB_HUB_RESET	IMX_GPIO_NR(1, 7)
#define TS48XX_USBH1_STP	IMX_GPIO_NR(1, 27)
#define TS48XX_USB_PHY_RESET	IMX_GPIO_NR(2, 5)
#define TS48XX_FEC_PHY_RESET	IMX_GPIO_NR(2, 14)
#define TS48XX_ECSPI1_CS0	IMX_GPIO_NR(4, 24)

/* USB_CTRL_1 */
#define MX51_USB_CTRL_1_OFFSET			0x10
#define MX51_USB_CTRL_UH1_EXT_CLK_EN		(1 << 25)

#define	MX51_USB_PLLDIV_12_MHZ		0x00
#define	MX51_USB_PLL_DIV_19_2_MHZ	0x01
#define	MX51_USB_PLL_DIV_24_MHZ	0x02

static iomux_v3_cfg_t mx51ts48xx_pads[] = {
	/* UART1 */
	MX51_PAD_UART1_RXD__UART1_RXD,
	MX51_PAD_UART1_TXD__UART1_TXD,
	MX51_PAD_UART1_RTS__UART1_RTS,
	MX51_PAD_UART1_CTS__UART1_CTS,

	/* UART2 */
	MX51_PAD_UART2_RXD__UART2_RXD,
	MX51_PAD_UART2_TXD__UART2_TXD,

	/* UART3 */
	MX51_PAD_EIM_D25__UART3_RXD,
	MX51_PAD_EIM_D26__UART3_TXD,
	MX51_PAD_EIM_D27__UART3_RTS,
	MX51_PAD_EIM_D24__UART3_CTS,

	/* I2C1 */
	MX51_PAD_EIM_D16__I2C1_SDA,
	MX51_PAD_EIM_D19__I2C1_SCL,

	/* I2C2 */
	MX51_PAD_KEY_COL4__I2C2_SCL,
	MX51_PAD_KEY_COL5__I2C2_SDA,

	/* HSI2C */
	MX51_PAD_I2C1_CLK__I2C1_CLK,
	MX51_PAD_I2C1_DAT__I2C1_DAT,

	/* USB HOST1 */
	MX51_PAD_USBH1_CLK__USBH1_CLK,
	MX51_PAD_USBH1_DIR__USBH1_DIR,
	MX51_PAD_USBH1_NXT__USBH1_NXT,
	MX51_PAD_USBH1_DATA0__USBH1_DATA0,
	MX51_PAD_USBH1_DATA1__USBH1_DATA1,
	MX51_PAD_USBH1_DATA2__USBH1_DATA2,
	MX51_PAD_USBH1_DATA3__USBH1_DATA3,
	MX51_PAD_USBH1_DATA4__USBH1_DATA4,
	MX51_PAD_USBH1_DATA5__USBH1_DATA5,
	MX51_PAD_USBH1_DATA6__USBH1_DATA6,
	MX51_PAD_USBH1_DATA7__USBH1_DATA7,

	/* USB HUB reset line*/
	MX51_PAD_GPIO1_7__GPIO1_7,

	/* USB PHY reset line */
	MX51_PAD_EIM_D21__GPIO2_5,

	/* FEC */
	MX51_PAD_EIM_EB2__FEC_MDIO,
	MX51_PAD_EIM_EB3__FEC_RDATA1,
	MX51_PAD_EIM_CS2__FEC_RDATA2,
	MX51_PAD_EIM_CS3__FEC_RDATA3,
	MX51_PAD_EIM_CS4__FEC_RX_ER,
	MX51_PAD_EIM_CS5__FEC_CRS,
	MX51_PAD_DISP2_DAT10__FEC_COL,
	MX51_PAD_DISP2_DAT11__FEC_RXCLK,
	MX51_PAD_DISP2_DAT14__FEC_RDAT0,
	MX51_PAD_DISP2_DAT15__FEC_TDAT0,
	MX51_PAD_NANDF_CS2__FEC_TX_ER,
	MX51_PAD_DI2_PIN2__FEC_MDC,
	MX51_PAD_DISP2_DAT6__FEC_TDATA1,
	MX51_PAD_DISP2_DAT7__FEC_TDATA2,
	MX51_PAD_DISP2_DAT8__FEC_TDATA3,
	MX51_PAD_DISP2_DAT9__FEC_TX_EN,
	MX51_PAD_DISP2_DAT13__FEC_TX_CLK,

	/* original fec
	MX51_PAD_EIM_EB2__FEC_MDIO,
	MX51_PAD_EIM_EB3__FEC_RDATA1,
	MX51_PAD_EIM_CS2__FEC_RDATA2,
	MX51_PAD_EIM_CS3__FEC_RDATA3,
	MX51_PAD_EIM_CS4__FEC_RX_ER,
	MX51_PAD_EIM_CS5__FEC_CRS,
	MX51_PAD_NANDF_RB2__FEC_COL,
	MX51_PAD_NANDF_RB3__FEC_RX_CLK,
	MX51_PAD_NANDF_D9__FEC_RDATA0,
	MX51_PAD_NANDF_D8__FEC_TDATA0,
	MX51_PAD_NANDF_CS2__FEC_TX_ER,
	MX51_PAD_NANDF_CS3__FEC_MDC,
	MX51_PAD_NANDF_CS4__FEC_TDATA1,
	MX51_PAD_NANDF_CS5__FEC_TDATA2,
	MX51_PAD_NANDF_CS6__FEC_TDATA3,
	MX51_PAD_NANDF_CS7__FEC_TX_EN,
	MX51_PAD_NANDF_RDY_INT__FEC_TX_CLK,
	*/

	/* FEC PHY reset line */
	MX51_PAD_EIM_A20__GPIO2_14,

	/* SD 1 */
	MX51_PAD_SD1_CMD__SD1_CMD,
	MX51_PAD_SD1_CLK__SD1_CLK,
	MX51_PAD_SD1_DATA0__SD1_DATA0,
	MX51_PAD_SD1_DATA1__SD1_DATA1,
	MX51_PAD_SD1_DATA2__SD1_DATA2,
	MX51_PAD_SD1_DATA3__SD1_DATA3,
	/* CD/WP from controller */
	MX51_PAD_GPIO1_0__SD1_CD,
	MX51_PAD_GPIO1_1__SD1_WP,

	/* eCSPI1 */
	MX51_PAD_CSPI1_MISO__ECSPI1_MISO,
	MX51_PAD_CSPI1_MOSI__ECSPI1_MOSI,
	MX51_PAD_CSPI1_SCLK__ECSPI1_SCLK,
	MX51_PAD_CSPI1_SS0__GPIO4_24,
	MX51_PAD_CSPI1_SS1__GPIO4_25,

	/* Audio */
	MX51_PAD_AUD3_BB_TXD__AUD3_TXD,
	MX51_PAD_AUD3_BB_RXD__AUD3_RXD,
	MX51_PAD_AUD3_BB_CK__AUD3_TXC,
	MX51_PAD_AUD3_BB_FS__AUD3_TXFS,
};

/* Serial ports */
static const struct imxi2c_platform_data ts48xx_i2c_data __initconst = {
	.bitrate = 100000,
};

static const struct imxi2c_platform_data ts48xx_hsi2c_data __initconst = {
	.bitrate = 400000,
};

static struct i2c_board_info ts48xx_i2c_devices[] = {
	{
		I2C_BOARD_INFO("sgtl5000-i2c", 0x0a),
	}, {
		I2C_BOARD_INFO("m41t00", 0x68),
	},
};

static struct gpio mx51_ts48xx_usbh1_gpios[] = {
	{ TS48XX_USBH1_STP, GPIOF_OUT_INIT_LOW, "usbh1_stp" },
	{ TS48XX_USB_PHY_RESET, GPIOF_OUT_INIT_LOW, "usbh1_phy_reset" },
};

static int gpio_usbh1_active(void)
{
	iomux_v3_cfg_t usbh1stp_gpio = MX51_PAD_EIM_A27__GPIO2_21;
	int ret;

	/* Set USBH1_STP to GPIO and toggle it */
	mxc_iomux_v3_setup_pad(usbh1stp_gpio);
	ret = gpio_request_array(mx51_ts48xx_usbh1_gpios,
					ARRAY_SIZE(mx51_ts48xx_usbh1_gpios));

	if (ret) {
		pr_debug("failed to get USBH1 pins: %d\n", ret);
		return ret;
	}

	msleep(100);
	gpio_set_value(TS48XX_USBH1_STP, 1);
	gpio_set_value(TS48XX_USB_PHY_RESET, 1);
	gpio_free_array(mx51_ts48xx_usbh1_gpios,
					ARRAY_SIZE(mx51_ts48xx_usbh1_gpios));
	return 0;
}

static inline void ts48xx_usbhub_reset(void)
{
	int ret;

	/* Reset USB hub */
	ret = gpio_request_one(TS48XX_USB_HUB_RESET,
					GPIOF_OUT_INIT_LOW, "GPIO1_7");
	if (ret) {
		printk(KERN_ERR"failed to get GPIO_USB_HUB_RESET: %d\n", ret);
		return;
	}

	msleep(2);
	/* Deassert reset */
	gpio_set_value(TS48XX_USB_HUB_RESET, 1);
}

static inline void ts48xx_fec_reset(void)
{
	int ret;

	/* reset FEC PHY */
	ret = gpio_request_one(TS48XX_FEC_PHY_RESET,
					GPIOF_OUT_INIT_LOW, "fec-phy-reset");
	if (ret) {
		printk(KERN_ERR"failed to get GPIO_FEC_PHY_RESET: %d\n", ret);
		return;
	}
	msleep(1);
	gpio_set_value(TS48XX_FEC_PHY_RESET, 1);
}

/* This function is board specific as the bit mask for the plldiv will also
be different for other Freescale SoCs, thus a common bitmask is not
possible and cannot get place in /plat-mxc/ehci.c.*/
static int initialize_otg_port(struct platform_device *pdev)
{
	u32 v;
	void __iomem *usb_base;
	void __iomem *usbother_base;

	usb_base = ioremap(MX51_USB_OTG_BASE_ADDR, SZ_4K);
	if (!usb_base)
		return -ENOMEM;
	usbother_base = usb_base + MX5_USBOTHER_REGS_OFFSET;

	/* Set the PHY clock to 19.2MHz */
	v = __raw_readl(usbother_base + MXC_USB_PHY_CTR_FUNC2_OFFSET);
	v &= ~MX5_USB_UTMI_PHYCTRL1_PLLDIV_MASK;
	v |= MX51_USB_PLL_DIV_19_2_MHZ;
	__raw_writel(v, usbother_base + MXC_USB_PHY_CTR_FUNC2_OFFSET);
	iounmap(usb_base);

	mdelay(10);

	return mx51_initialize_usb_hw(0, MXC_EHCI_INTERNAL_PHY);
}

static int initialize_usbh1_port(struct platform_device *pdev)
{
	u32 v;
	void __iomem *usb_base;
	void __iomem *usbother_base;

	usb_base = ioremap(MX51_USB_OTG_BASE_ADDR, SZ_4K);
	if (!usb_base)
		return -ENOMEM;
	usbother_base = usb_base + MX5_USBOTHER_REGS_OFFSET;

	/* The clock for the USBH1 ULPI port will come externally from the PHY. */
	v = __raw_readl(usbother_base + MX51_USB_CTRL_1_OFFSET);
	__raw_writel(v | MX51_USB_CTRL_UH1_EXT_CLK_EN, usbother_base + MX51_USB_CTRL_1_OFFSET);
	iounmap(usb_base);

	mdelay(10);

	return mx51_initialize_usb_hw(1, MXC_EHCI_POWER_PINS_ENABLED |
			MXC_EHCI_ITC_NO_THRESHOLD);
}

static const struct mxc_usbh_platform_data dr_utmi_config __initconst = {
	.init		= initialize_otg_port,
	.portsc	= MXC_EHCI_UTMI_16BIT,
};

static const struct fsl_usb2_platform_data usb_pdata __initconst = {
	.operating_mode	= FSL_USB2_DR_DEVICE,
	.phy_mode	= FSL_USB2_PHY_UTMI_WIDE,
};

static const struct mxc_usbh_platform_data usbh1_config __initconst = {
	.init		= initialize_usbh1_port,
	.portsc	= MXC_EHCI_MODE_ULPI,
};

static bool otg_mode_host __initdata;

static int __init ts48xx_otg_mode(char *options)
{
	if (!strcmp(options, "host"))
		otg_mode_host = true;
	else if (!strcmp(options, "device"))
		otg_mode_host = false;
	else
		pr_info("otg_mode neither \"host\" nor \"device\". "
			"Defaulting to device\n");
	return 1;
}
__setup("otg_mode=", ts48xx_otg_mode);

static int mx51_ts48xx_spi_cs[] = {
	TS48XX_ECSPI1_CS0,
};

static const struct spi_imx_master mx51_ts48xx_spi_pdata __initconst = {
	.chipselect     = mx51_ts48xx_spi_cs,
	.num_chipselect = ARRAY_SIZE(mx51_ts48xx_spi_cs),
};

static const struct esdhc_platform_data mx51_ts48xx_sd1_data __initconst = {
	.cd_type = ESDHC_CD_PERMANENT,
};

void __init ts48xx_common_init(void)
{
	mxc_iomux_v3_setup_multiple_pads(mx51ts48xx_pads,
					 ARRAY_SIZE(mx51ts48xx_pads));
}

static void ts48xx_fixup(struct tag *tags, char **from, struct meminfo *meminfo)
{
	struct tag *t;
	for_each_tag(t, tags) {
		if (t->hdr.tag == ATAG_MEM) {
			t->u.mem.size = SZ_256M;
			break;
		}
	}
}

/*
 * Board specific initialization.
 */
static void __init ts48xx_init(void)
{
	iomux_v3_cfg_t usbh1stp = MX51_PAD_USBH1_STP__USBH1_STP;

	imx51_soc_init();

	ts48xx_common_init();

	imx51_add_imx_uart(0, NULL);
	imx51_add_imx_uart(1, NULL);
	imx51_add_imx_uart(2, NULL);

	ts48xx_fec_reset();
	imx51_add_fec(NULL);

	imx51_add_imx_i2c(1, &ts48xx_i2c_data); 
	imx51_add_hsi2c(&ts48xx_hsi2c_data);

	i2c_register_board_info(0, ts48xx_i2c_devices,
		ARRAY_SIZE(ts48xx_i2c_devices));

	if (otg_mode_host)
		imx51_add_mxc_ehci_otg(&dr_utmi_config);
	else {
		initialize_otg_port(NULL);
		imx51_add_fsl_usb2_udc(&usb_pdata);
	}

	gpio_usbh1_active();
	imx51_add_mxc_ehci_hs(1, &usbh1_config);
	//setback USBH1_STP to be function
	mxc_iomux_v3_setup_pad(usbh1stp);

	imx51_add_sdhci_esdhc_imx(0, &mx51_ts48xx_sd1_data);

	/*spi_register_board_info(mx51_ts48xx_spi_board_info,
		ARRAY_SIZE(mx51_ts48xx_spi_board_info));
	imx51_add_ecspi(0, &mx51_ts48xx_spi_pdata);*/
}

static void __init ts48xx_timer_init(void)
{
	mx51_clocks_init(32768, 24000000, 22579200, 24576000);
}

MACHINE_START(TS48XX, "Technologic Systems TS48XX")
	.atag_offset = 0x100,
	.map_io = mx51_map_io,
	.init_early = imx51_init_early,
	.init_irq = mx51_init_irq,
	.handle_irq = imx51_handle_irq,
	.init_time	= ts48xx_timer_init,
	.init_machine = ts48xx_init,
	.init_late	= imx51_init_late,
	.fixup = ts48xx_fixup,
	.restart	= mxc_restart,
MACHINE_END
