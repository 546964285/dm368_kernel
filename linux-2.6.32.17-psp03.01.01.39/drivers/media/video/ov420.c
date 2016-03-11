/*
 * Driver for OV420 CMOS Image Sensor from Micron
 *
 * Copyright (C) 2010, RidgeRun <www.ridgerun.com>
 *
 * Author: Miguel Aguilar <miguel.aguilar@ridgerun.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/videodev2.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/log2.h>
#include <linux/delay.h>

#include <media/v4l2-device.h>
#include <media/v4l2-common.h>
#include <media/v4l2-chip-ident.h>

/* ov420 i2c address 0x35
 * The platform has to define i2c_board_info
 * and call i2c_register_board_info() */

/* ov420 selected register addresses */
#define OV420_CHIP_VERSION            0x00
#define OV420_ROW_START           0x01
#define OV420_COLUMN_START            0x02
#define OV420_WINDOW_HEIGHT           0x03
#define OV420_WINDOW_WIDTH            0x04
#define OV420_HORIZONTAL_BLANK        0x05
#define OV420_VERTICAL_BLANK          0x06
#define OV420_OUTPUT_CONTROL          0x07
#define OV420_SHUTTER_WIDTH_UPPER     0x08
#define OV420_SHUTTER_WIDTH           0x09
#define OV420_PIXEL_CLOCK_CONTROL     0x0A
#define OV420_RESTART             0x0B
#define OV420_SHUTTER_DELAY           0x0C
#define OV420_RESET               0x0D
#define OV420_PLL_CONTROL         0x10
#define OV420_PLL_CONFIG_1            0x11
#define OV420_PLL_CONFIG_2            0x12
#define OV420_READ_MODE_1         0x1E
#define OV420_READ_MODE_2         0x20
#define OV420_ROW_ADDRESS_MODE        0x22
#define OV420_COLUMN_ADDRESS_MODE     0x23
#define OV420_GREEN_1_GAIN        0x2B
#define OV420_BLUE_GAIN           0x2C
#define OV420_RED_GAIN            0x2D
#define OV420_GREEN_2_GAIN        0x2E
#define OV420_GLOBAL_GAIN         0x35
#define OV420_TEST_PATTERN_CONTROL        0xA0
#define OV420_TEST_PATTERN_GREEN      0xA1
#define OV420_TEST_PATTERN_RED        0xA2
#define OV420_TEST_PATTERN_BLUE       0xA3
#define OV420_TEST_PATTERN_BAR_WIDTH      0xA4
#define OV420_CHIP_VERSION_ALT        0xFF

/* Macros for default register values */
#define OV420_ROW_SKIP            (0x0036)
#define OV420_COLUMN_SKIP         (0x0010)
//#define OV420_MAX_HEIGHT          (0x0797)
//#define OV420_MAX_WIDTH           (0x0A1F)
#define OV420_MAX_HEIGHT          (0x0180)
#define OV420_MAX_WIDTH           (0x0180)
#define OV420_MIN_HEIGHT          (0x0002)
#define OV420_MIN_WIDTH           (0x0002)
#define OV420_DEFAULT_WIDTH           (0x0180) /* 640 */
#define OV420_DEFAULT_HEIGHT          (0x0180) /* 480 */
#define OV420_HORIZONTAL_BLANK_DEFAULT    (0x0000)
#define OV420_VERTICAL_BLANK_DEFAULT      (0x0019)
#define OV420_OUTPUT_CONTROL_DEFAULT      (0x1F82)
#define OV420_SHUTTER_WIDTH_UPPER_DEFAULT (0x0000)
#define OV420_SHUTTER_WIDTH_DEFAULT       (0x01F4)
#define OV420_PIXEL_CLK_CTRL_DEFAULT      (0x0000)
#define OV420_FRAME_RESTART_DEFAULT       (0x0000)
#define OV420_SHUTTER_DELAY_DEFAULT       (0x0000)
#define OV420_PLL_CONTROL_DEFAULT     (0x0050)
#define OV420_PLL_CONTROL_POWER_UP        (0x0051)
#define OV420_PLL_CONTROL_USE_PLL_CLK     (0x0053)
#define OV420_PLL_CONFIG_1_96MHZ      (0x1001)
#define OV420_PLL_CONFIG_1_DEFAULT        (0x6404)
#define OV420_PLL_CONFIG_2_DEFAULT        (0x0000)
#define OV420_PLL_CONFIG_2_96MHZ      (0x0003)
#define OV420_READ_MODE1_DEFAULT      (0x4006)
#define OV420_READ_MODE2_DEFAULT      (0x0040)
#define OV420_ROW_ADDRESS_MODE_DEFAULT    (0x0000)
#define OV420_COLUMN_ADDR_MODE_DEFAULT    (0x0000)
#define OV420_GREEN_1_GAIN_DEFAULT        (0x0008)
#define OV420_BLUE_GAIN_DEFAULT       (0x0008)
#define OV420_RED_GAIN_DEFAULT        (0x0008)
#define OV420_GREEN_2_GAIN_DEFAULT        (0x0008)
#define OV420_GLOBAL_GAIN_DEFAULT     (0x0008)
#define OV420_TEST_PATTERN_CONTROL_DEFAULT    (0x0000)
#define OV420_TEST_PATTERN_GREEN_DEFAULT  (0x0000)
#define OV420_TEST_PATTERN_RED_DEFAULT    (0x0000)
#define OV420_TEST_PATTERN_BLUE_DEFAULT   (0x0000)
#define OV420_TEST_PATTERN_BAR_WIDTH_DEFAULT  (0x0000)

/*Minimum values*/
#define OV420_VERTICAL_BLANK_MINIMUM     (0x0008)

#define MT9T031_BUS_PARAM   (SOCAM_PCLK_SAMPLE_RISING | \
    SOCAM_PCLK_SAMPLE_FALLING | SOCAM_HSYNC_ACTIVE_HIGH |   \
    SOCAM_VSYNC_ACTIVE_HIGH | SOCAM_DATA_ACTIVE_HIGH |  \
    SOCAM_MASTER | SOCAM_DATAWIDTH_10)


#define MAX_FRMIVALS            3
#define CEIL(x) ((x - (int)x)==0 ? (int)x : (int)x+1)

/* Debug functions */
static int debug = 1;


#define dev_dbg(dev, format, arg...)		\
	dev_printk(KERN_DEBUG , dev , format , ## arg)

#define v4l2_dbg(level, debug, dev, fmt, arg...)			\
	do { 								\
		if (debug >= (level))					\
			v4l2_printk(KERN_DEBUG, dev, fmt , ## arg); 	\
	} while (0)

module_param(debug, bool, 0644);
MODULE_PARM_DESC(debug, "Debug level (0-1)");

const struct v4l2_queryctrl *ov420_find_qctrl(u32 id);
static int ov420_get_control(struct v4l2_subdev *, struct v4l2_control *);
static int ov420_set_control(struct v4l2_subdev *, struct v4l2_control *);
static int ov420_queryctrl(struct v4l2_subdev *, struct v4l2_queryctrl *);

const int hb_min[4][4] = {  {450, 430, 0, 420},
                            {796, 776, 0, 766},
                            {0,   0, 0,   0},
                            {1488,1468,0, 1458} };


static const struct v4l2_fmtdesc ov420_formats[] = {
    {
        .index = 0,
        .type = V4L2_BUF_TYPE_VIDEO_CAPTURE,
        .description = "Bayer (sRGB) 10 bit",
        .pixelformat = V4L2_PIX_FMT_SGRBG10,
    },
};
static const unsigned int ov420_num_formats = ARRAY_SIZE(ov420_formats);

struct capture_size {
    unsigned long width;
    unsigned long height;
};
struct frame_table {
    struct capture_size framesize;
    struct v4l2_fract frameintervals[MAX_FRMIVALS];
    unsigned int num_frmivals;
};

struct exp_data{
    u32 row_time;
    u32 shutter_overlay;
    u32 pix_clock;
};

/* Array of image sizes supported by OV420.  These must be ordered from
 * smallest image size to largest.
 */
const static struct frame_table ov420_frame_table[] = {
    {
    /* VGA -4X Binning+Skipping */
    .framesize = { 384, 384 },
    .frameintervals = {
        { .numerator =1, .denominator = 30 },
        },
    .num_frmivals = 1
    },
    {
    /* VGA -4X Binning+Skipping */
    .framesize = { 640, 480 },
    .frameintervals = {
        { .numerator =1, .denominator = 30 },
        },
    .num_frmivals = 1
    },
    {
    /* 120% of VGA for video stabilization*/
    .framesize = { 768, 576 },
    .frameintervals = {
        { .numerator =1, .denominator = 30 },
        },
    .num_frmivals = 1
    },
    {
    /* SVGA-2X Binning+Skipping */
    .framesize = { 800, 600 },
    .frameintervals = {
        { .numerator = 1, .denominator = 65 },
        },
    .num_frmivals = 1
    },
    {
    /* XGA -2X Binning+Skipping */
    .framesize = { 1024, 768 },
    .frameintervals = {
        {  .numerator = 1, .denominator = 47 },
        },
    .num_frmivals = 1
    },
    {
    /* 720P-HDTV -2X Binning+Skipping */
    .framesize = { 1280, 720 },
    .frameintervals = {
        {  .numerator = 1, .denominator = 23 },
        },
    .num_frmivals = 1
    },
    {
    /* 120% of 720P for video stabilization*/
    .framesize = { 1536, 864 },
    .frameintervals = {
        { .numerator =1, .denominator = 23 },
        },
    .num_frmivals = 1
    },
    {
    /* SXGA */
    .framesize = { 1280, 1024},
    .frameintervals = {
        {  .numerator = 1, .denominator = 42 },
        },
    .num_frmivals = 1
    },
    {
    /* UXGA */
    .framesize = { 1600, 1200},
    .frameintervals = {
        {  .numerator = 1, .denominator = 31 },
        },
    .num_frmivals = 1
    },
    {
    /* 1080P-HDTV */
    .framesize = { 1920, 1080},
    .frameintervals = {
        {  .numerator = 1, .denominator = 31 },
        },
    .num_frmivals = 1
    },
    {
    /* 1080P-HDTV height=1088 for dm36x encoding. Encoder needs
     * dimensions (width and height) multiples of 16*/
    .framesize = { 1920, 1088},
    .frameintervals = {
        {  .numerator = 1, .denominator = 30 },
        },
    .num_frmivals = 1
    },
    {
    /* QXGA */
    .framesize = { 2048, 1536},
    .frameintervals = {
        {  .numerator = 1, .denominator = 21 },
        },
    .num_frmivals = 1
    },
    {
    /* Full Resolution for DM365 that supports maximum 2176x2176  */
    .framesize = { 2176, 1944},
    .frameintervals = {
        {  .numerator = 1, .denominator = 14 },
        },
    .num_frmivals = 1
    },
    {
    /* Full Resolution */
    .framesize = { 2592, 1944},
    .frameintervals = {
        {  .numerator = 1, .denominator = 14 },
        },
    .num_frmivals = 1
    }
};
static const unsigned int ov420_num_frmsizes = ARRAY_SIZE(ov420_frame_table);

static const struct v4l2_queryctrl ov420_controls[] = {
    {
        .id     = V4L2_CID_VFLIP,
        .type       = V4L2_CTRL_TYPE_BOOLEAN,
        .name       = "Flip Vertically",
        .minimum    = 0,
        .maximum    = 1,
        .step       = 1,
        .default_value  = 0,
    }, {
        .id     = V4L2_CID_HFLIP,
        .type       = V4L2_CTRL_TYPE_BOOLEAN,
        .name       = "Flip Horizontally",
        .minimum    = 0,
        .maximum    = 1,
        .step       = 1,
        .default_value  = 0,
    }, {
        .id     = V4L2_CID_GAIN,
        .type       = V4L2_CTRL_TYPE_INTEGER,
        .name       = "Gain",
        .minimum    = 0,
        .maximum    = 1024,
        .step       = 1,
        .default_value  = 8,
        .flags      = V4L2_CTRL_FLAG_SLIDER,
    }, {
        .id     = V4L2_CID_EXPOSURE,
        .type       = V4L2_CTRL_TYPE_INTEGER,
        .name       = "Exposure",
        .minimum    = 1,
        .maximum    = (0x7fffffff),/*Original 255*/
        .step       = 1,
        .default_value  = 255,
        .flags      = V4L2_CTRL_FLAG_SLIDER,
    }, {
        .id     = V4L2_CID_EXPOSURE_AUTO,
        .type       = V4L2_CTRL_TYPE_BOOLEAN,
        .name       = "Automatic Exposure",
        .minimum    = 0,
        .maximum    = 1,
        .step       = 1,
        .default_value  = 0,
    }, {
        .id     = V4L2_CID_RED_BALANCE,
        .type       = V4L2_CTRL_TYPE_INTEGER,
        .name       = "Red Balance",
        .minimum    = 1,
        .maximum    = 1024,
        .step       = 1,
        .default_value  = 8
    }, {
        .id     = V4L2_CID_BRIGHTNESS,
        .type       = V4L2_CTRL_TYPE_INTEGER,
        .name       = "Brightness (Green 1 Balance)",
        .minimum    = 1,
        .maximum    = 1024,
        .step       = 1,
        .default_value  = 8
    }, {
        .id     = V4L2_CID_AUTOGAIN,
        .type       = V4L2_CTRL_TYPE_INTEGER,
        .name       = "Auto Gain (Green 2 Balance)",
        .minimum    = 1,
        .maximum    = 1024,
        .step       = 1,
        .default_value  = 8
    }, {
        .id     = V4L2_CID_BLUE_BALANCE,
        .type       = V4L2_CTRL_TYPE_INTEGER,
        .name       = "Blue Balance",
        .minimum    = 1,
        .maximum    = 1024,
        .step       = 1,
        .default_value  = 8
    },
};

static const unsigned int ov420_num_controls = ARRAY_SIZE(ov420_controls);

struct ov420 {
    struct v4l2_subdev sd;
    int model;  /* V4L2_IDENT_MT9T031* codes from v4l2-chip-ident.h */
    unsigned char autoexposure;
    u16 vblank;
    u16 xskip;
    u16 yskip;
    u32 xbin;
    u32 ybin;
    u32 width;
    u32 height;
    unsigned short x_min;           /* Camera capabilities */
    unsigned short y_min;
    unsigned short x_current;       /* Current window location */
    unsigned short y_current;
    unsigned short width_min;
    unsigned short width_max;
    unsigned short height_min;
    unsigned short height_max;
    unsigned short y_skip_top;      /* Lines to skip at the top */
    unsigned short gain;
    u32 exposure;
    unsigned short mirror_column;
    unsigned short mirror_row;
    struct exp_data exp;
};

static inline struct ov420 *to_ov420(struct v4l2_subdev *sd)
{
    return container_of(sd, struct ov420, sd);
}

static int reg_read(struct i2c_client *client, const u16 reg)
{
    //s32 data;
    char data;
    char tmp[2];
    u16 reg_tmp;

    reg_tmp=reg;
    tmp[1]=(0x00ff&reg_tmp);
    reg_tmp=reg;
    tmp[0]=(0xff00&reg_tmp)>>8;

    //data = i2c_smbus_read_word_data(client, reg);
    i2c_master_recv2(client,  tmp, &data, 2);
    //return data < 0 ? data : swab16(data);

    printk("\n*****ov420.reg_read(%8x)=%x*****\n", reg,data&0xff);
    return data&0xff;
}

static int reg_write(struct i2c_client *client, const u16 reg,
             const u8 data)
{
//	    //int ret;
//	
//	    //ret = reg_read(client, reg);
//	    //printk("\n***Register:0x%x actualvalue:0x%x, Value to be write:0x%x",reg,ret,data);
//	    return i2c_smbus_write_word_data(client, reg, swab16(data));

        char tmp[3];
        u16 reg_tmp;
        int ret;

        printk("\n*****ov420.reg_write(%8x, %4x)*****\n", reg, data);

        reg_tmp=reg;
        tmp[1]=(0x00ff&reg_tmp);
        reg_tmp=reg;
        tmp[0]=(0xff00&reg_tmp)>>8;
        tmp[2]=data;
        //ret = i2c_master_send2(client,  tmp, 2);

        return ret;
}

static int reg_set(struct i2c_client *client, const u8 reg,
           const u16 data)
{
    int ret;

    ret = reg_read(client, reg);
    dev_dbg(&client->dev, "ov420.reg_set().read: %8x from %8x", ret, reg);
    if (ret < 0)
        return ret;
    dev_dbg(&client->dev, "ov420.reg_set().write: %8x to %8x\n", data, reg);
    return reg_write(client, reg, ret | data);
}

static int reg_clear(struct i2c_client *client, const u8 reg,
             const u16 data)
{
    int ret;

    ret = reg_read(client, reg);
    dev_dbg(&client->dev, "ov420.reg_set().read: %8x from %8x", ret, reg);
    if (ret < 0)
        return ret;
    return reg_write(client, reg, ret & ~data);
}

static int set_shutter(struct v4l2_subdev *sd, const u32 data)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret;
    ret = reg_write(client, OV420_SHUTTER_WIDTH_UPPER, data >> 16);

    if (ret >= 0)
        ret = reg_write(client, OV420_SHUTTER_WIDTH, data & 0xffff);

    return ret;
}

static int get_shutter(struct v4l2_subdev *sd, u32 *data)
{
    int ret;
    struct i2c_client *client = v4l2_get_subdevdata(sd);

    ret = reg_read(client, OV420_SHUTTER_WIDTH_UPPER);
    dev_dbg(&client->dev, "ov420.get_shutter().read: %8x from %8x", ret, OV420_SHUTTER_WIDTH_UPPER);
    *data = ret << 16;

    if (ret >= 0)
        ret = reg_read(client, OV420_SHUTTER_WIDTH);
        dev_dbg(&client->dev, "ov420.get_shutter().read: %8x from %8x", ret, OV420_SHUTTER_WIDTH);
    *data |= ret & 0xffff;
    return ret < 0 ? ret : 0;
}

void calc_shutter(struct v4l2_subdev *sd,
    struct ov420 *ov420)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    u32 pix_clk, shutter_delay, so, max, hb, hbmin, hb_max, w, row_t, column_size;
    /* Pixclk Period = 10.42ns, fixed point 24:8 in ns */
    pix_clk = 2667;
    /* Shutter Delay, increasing this decrease the exposure time*/
    shutter_delay = reg_read(client, OV420_SHUTTER_DELAY) + 1;
    /* Shutter overhead */
    so = 208 * (ov420->ybin) + 98 + min((int)shutter_delay, 1504) - 94;
    /* Output image width */
    column_size = reg_read(client, OV420_WINDOW_WIDTH);
    w = 2 * CEIL((column_size + 1)/(2 * (ov420->xskip)));
    /* Horizontal blanking */
    hb = reg_read(client, OV420_HORIZONTAL_BLANK) + 1;
    /* Minimun horizontal blanking*/
    hbmin = hb_min[ov420->ybin-1][ov420->xbin-1];
    /* Row time calculation, fixed point 24:8 in us */
    hb_max = max(hb, hbmin);
    max = max((w/2 + hb_max),(41 + 346*(ov420->ybin) + 99));
    row_t = (2 * pix_clk * max)/1000;

    so = 1;
    column_size = 384;
    w=384;
    hb=10;
    hbmin=1;
    hb_max=10;
    max = max((w/2 + hb_max),(41 + 346*(ov420->ybin) + 99));
    //row_t = (2 * pix_clk * max)/1000;
    row_t = 384;
    pix_clk = 384;

    v4l2_dbg(1, debug, sd, "Frame time parameters (Table 8, datasheet)\n");
    v4l2_dbg(1, debug, sd,"shutter_delay %i\n", shutter_delay);
    v4l2_dbg(1, debug, sd,"ybin %i\n",ov420->ybin);
    v4l2_dbg(1, debug, sd,"xskip %i\n",ov420->xskip );
    v4l2_dbg(1, debug, sd,"so %i\n", so);
    v4l2_dbg(1, debug, sd,"column_size %i\n",column_size );
    v4l2_dbg(1, debug, sd,"w %i\n",w);
    v4l2_dbg(1, debug, sd,"hbmin %i\n", hbmin);
    v4l2_dbg(1, debug, sd,"hb %i\n",hb);
    v4l2_dbg(1, debug, sd,"pix_clock %i\n", pix_clk);
    v4l2_dbg(1, debug, sd,"row_time %i\n", row_t);
    ov420->exp.row_time = row_t;
    ov420->exp.shutter_overlay = so;
    ov420->exp.pix_clock = pix_clk;
    return;
}

/* See Datasheet Table 17, Gain settings.
 * Since posible values goes from 1-128, and the received values goes from 8-1024
 * map  [1-1024] to [1-128] => Gain = ctrl->value/8
 * if Gain -> [1-8]  =>  AnalogGain -> [8-32] ,  steps of 0.125
 * if Gain -> [4.25-8] => AnalogGain -> [17-32] & AnalogMultipier=1, steps of 0.25
 * If Gain -> [9-128] => AnalogGain = 32 & AnalogMultipier=1 & DigitalGain=>[1-120], steps of 1
 */
int calc_gain(u32 data)
{
    int gain;
    if (data <= 32)
        /* received gain 9..32 -> 9..32 */
        gain = data;
    else if (data <= 64)
        /* received gain 33..64 -> 0x51..0x60 */
        gain = ((data + 1) >> 1) | 0x40;
    else if(data <= 1024)
        /* received gain 65..1024 -> (1..120) << 8 +0x60*/
        gain =(((data - 64 + 4) * 32) & 0xff00) | 0x60;
    else
        gain = 0x7860;

    return gain;
}
/*
 * ======== ov420_setpll =========
*/
/*   Function to set the clock pll   Freq_ext = 24Mhz */
/*   Freq = Freq_ext * m_factor / (n_factor + 1) / (p1_factor + 1) */
/*   2MHz < Freq_ext / (n_factor+1) < 13.5MHz */
/*   180 MHz < (Freq_ext * m_factor) / (n_factor+1)< 360 MHz  */

static int ov420_setpll(struct i2c_client *client, unsigned char m_factor,
                unsigned char n_factor, unsigned char p1_factor)
{
    int ret;

    ret = reg_write(client, OV420_PLL_CONTROL,
                  OV420_PLL_CONTROL_POWER_UP);

    if (ret >= 0)
        ret = reg_write(client, OV420_PLL_CONFIG_1,
                  (m_factor << 8) | (n_factor & 0x003f));
    if (ret >= 0)
        ret = reg_write(client, OV420_PLL_CONFIG_2,
                  p1_factor & 0x001f);

    msleep(10);

    if (ret >= 0)
        ret = reg_write(client, OV420_PLL_CONTROL,
                  OV420_PLL_CONTROL_USE_PLL_CLK);

    return ret >= 0 ? 0 : -EIO;
}

static int ov420_init(struct v4l2_subdev *sd, u32 val)
{
    int ret, shutter;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct v4l2_control ctrl;
    struct ov420 *ov420 = to_ov420(sd);
    u32 i;
    const u16 vblank = OV420_VERTICAL_BLANK_DEFAULT;

    /* Disable chip output, synchronous option update */
    ret = reg_write(client, OV420_RESET, 1);
    if (ret >= 0)
        ret = reg_write(client, OV420_RESET, 0);

    /*Soft Standby*/
    if (ret >= 0)
        ret = reg_clear(client, OV420_OUTPUT_CONTROL, 2);

    /*if (ret >= 0)
        ret = reg_set(client, OV420_OUTPUT_CONTROL, 1);*/

    /* Set defaults of the controls*/
    for (i = 0; i < ov420_num_controls; i++) {
        if (ov420_controls[i].id == V4L2_CID_EXPOSURE)
            i++;
        ctrl.id = ov420_controls[i].id;
        ctrl.value = ov420_controls[i].default_value;
        ov420_set_control(sd,&ctrl);
    }
    /*if (ret >= 0)
        ret = reg_clear(client, OV420_OUTPUT_CONTROL, 1);*/
    shutter = 479 + vblank;
    ret = set_shutter(sd, shutter);
    calc_shutter(sd, ov420);
    ov420->exposure = shutter * ov420->exp.row_time -
       (ov420->exp.shutter_overlay * 2 * ov420->exp.pix_clock)/1000;
    return ret >= 0 ? 0 : -EIO;
}

static int ov420_s_stream(struct v4l2_subdev *sd, int enable)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret;

    /* Switch to master "normal" mode - StreamOn*/
    if (enable) {

        /*Enable Chip*/
        if (reg_set(client, OV420_OUTPUT_CONTROL, 2) < 0)
            return -EIO;

        /*Restart Pause*/
        ret = reg_set(client, OV420_RESTART,2);
        ret = reg_set(client, OV420_RESTART,1);
        /*Setup PLL to generate  96MHz from extclk at 24MHz*/
        ret = ov420_setpll(client, 16, 1, 1);
        /*Resume*/
        ret = reg_clear(client, OV420_RESTART,2);

    } else {
    /* Switch to master "" mode - StreamOff*/
        /*Restart Pause*/
        ret = reg_set(client, OV420_RESTART,2);
        ret = reg_set(client, OV420_RESTART,1);

        if (reg_clear(client, OV420_OUTPUT_CONTROL, 2) < 0)
            return -EIO;

        /*Resume*/
        ret = reg_clear(client, OV420_RESTART,2);
    }
    return 0;
}

/* Round up minima and round down maxima */
static void recalculate_limits(struct ov420 *ov420,
                   u16 xskip, u16 yskip)
{

    ov420->x_min = (OV420_COLUMN_SKIP + 2 * xskip - 1) & ~(2 * xskip - 1);
    ov420->y_min = (OV420_ROW_SKIP + 2 * yskip - 1) & ~(2 * yskip - 1);
    ov420->width_min = (OV420_MIN_WIDTH + 2 * xskip - 1) & ~(2 * xskip - 1);
    ov420->height_min = (OV420_MIN_HEIGHT + 2 * yskip - 1) & ~(2 * yskip - 1);
    ov420->width_max = OV420_MAX_WIDTH / xskip;
    ov420->height_max = OV420_MAX_HEIGHT / yskip;

    printk("\nin recalculate_limits()\n");
    printk("ov420->x_min=%d\n",ov420->x_min);
    printk("ov420->y_min=%d\n",ov420->y_min);
    printk("ov420->width_min=%d\n",ov420->width_min);
    printk("ov420->height_min=%d\n",ov420->height_min);
    printk("ov420->width_max=%d\n",ov420->width_max);
    printk("ov420->height_max=%d\n",ov420->height_max);
}

const struct v4l2_queryctrl *ov420_find_qctrl(u32 id)
{
    int i;

    for (i = 0; i < ov420_num_controls; i++) {
        if (ov420_controls[i].id == id){
            return &ov420_controls[i];
        }
    }
    return NULL;
}

static int get_frame_interval(int width, int height){

    struct capture_size sizei;
    int index = 0;
    int fps = 0;
    while (index < ov420_num_frmsizes) {
        sizei.width = ov420_frame_table[index].framesize.width;
        sizei.height = ov420_frame_table[index].framesize.height;

        if (((sizei.width == width) &&
            (sizei.height == height))){
            break;
        }
        index++;
    }

    fps = ov420_frame_table[index].frameintervals[0].denominator /
        ov420_frame_table[index].frameintervals[0].numerator;
    return fps;
}
static int ov420_set_params(struct v4l2_subdev *sd,
                  struct v4l2_rect *rect, u16 xskip, u16 yskip)
{
    struct ov420 *ov420 = to_ov420(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    int ret, shutter = 1;
    u16 xbin=1, ybin=1, width, height, left, top;
    const u16 hblank = OV420_HORIZONTAL_BLANK_DEFAULT;
    int vblank = OV420_VERTICAL_BLANK_DEFAULT;
    int fps;

    /* We center the capture region */
    rect->left =
        (ov420->width_max - rect->width) / 2 + ov420->x_min;

    rect->top =
        (ov420->height_max - rect->height) / 2 + ov420->y_min;

    width = rect->width * xskip;
    height = rect->height * yskip;
    left = rect->left;
    top = rect->top;
    /* See Datasheet Table 14, Legal Skip/Bin Values
     * Remember for xskip=2X,Row Skip=1,
     *          for xskip=3X,Row Skip=2, and so on.
     */
    if(xskip & 1)
        xbin = 1;
    else if (xskip & 2)
        xbin = 2;
    else if (xskip & 4)
        xbin = 4;

    if(yskip & 1)
        ybin = 1;
    else if (yskip & 2)
        ybin = 2;
    else if (yskip & 4)
        ybin = 4;

    v4l2_dbg(1, debug, sd, "xskip %u, width %u/%u, yskip %u,"
        "height %u/%u\n", xskip, width, rect->width, yskip,
        height, rect->height);
    /*Rounded down to the nearest multiple of 4 times the bin factor*/
    left = ((left) & ~(2 * xbin -1)) + 1 * xbin * ov420->mirror_row;
    top = ((top) & ~(2 * ybin -1)) + 1 * ybin * ov420->mirror_column;

    /* Disable register update, reconfigure atomically */
    ret = reg_set(client, OV420_OUTPUT_CONTROL, 1);
    if (ret < 0)
        return ret;

    /* Blanking and start values - default... */
    ret = reg_write(client, OV420_HORIZONTAL_BLANK, hblank);
    if (ret >= 0)
        ret = reg_write(client, OV420_VERTICAL_BLANK, vblank);

    if (yskip != ov420->yskip || xskip != ov420->xskip) {
        /* Binning, skipping */
        if (ret >= 0)
            ret = reg_write(client, OV420_COLUMN_ADDRESS_MODE,
                    ((xbin - 1) << 4) | (xskip - 1));
        if (ret >= 0)
            ret = reg_write(client, OV420_ROW_ADDRESS_MODE,
                    ((ybin - 1) << 4) | (yskip - 1));
    }

    v4l2_dbg(1, debug, sd, "new physical left %u, top %u\n", left, top);

    /* The caller provides a supported format, as guaranteed by
     * icd->try_fmt_cap(), soc_camera_s_crop() and soc_camera_cropcap() */
    if (ret >= 0)
        ret = reg_write(client, OV420_COLUMN_START, left);
    if (ret >= 0)
        ret = reg_write(client, OV420_ROW_START, top);
    if (ret >= 0)
        ret = reg_write(client, OV420_WINDOW_WIDTH, width - 1);
    if (ret >= 0)
        ret = reg_write(client, OV420_WINDOW_HEIGHT,
                height + ov420->y_skip_top - 1);

    if (ret >= 0) {
        shutter = rect->height-1 + vblank;
        ret = set_shutter(sd, shutter);
    }
    if (ret >= 0)
        ret = reg_set(client, OV420_READ_MODE_2, 0x0020);

    /* Re-enable register update, commit all changes */
    if (ret >= 0) {
        ret = reg_clear(client, OV420_OUTPUT_CONTROL, 1);
        fps = get_frame_interval(rect->width, rect->height);
        height = 2*((height + ov420->y_skip_top)/(2*yskip) + 1) ;
        /* update the values */
        ov420->width  = rect->width,
        ov420->height = height,
        ov420->x_current = rect->left;
        ov420->y_current = rect->top;
        ov420->ybin = ybin;
        ov420->xbin = xbin;
        ov420->xskip = xskip;
        ov420->yskip = yskip;

        calc_shutter(sd, ov420);
//	        vblank = 1000000/((fps * ov420->exp.row_time)>>8) - ov420->height - 1;
//	        if(vblank < OV420_VERTICAL_BLANK_MINIMUM){
//	            vblank = OV420_VERTICAL_BLANK_MINIMUM;
//	        }
        vblank=8;
//	        ret = reg_write(client, OV420_VERTICAL_BLANK, vblank);
        ov420->vblank = vblank;
        ov420->exposure = shutter * ov420->exp.row_time -
            (ov420->exp.shutter_overlay * 2 * ov420->exp.pix_clock)/1000;

        printk("ov420->vblank=%d\n",ov420->vblank);
        printk("ov420->exposure=%d\n",ov420->exposure);
        printk("shutter=%d\n",shutter);
        printk("ov420->exp.row_time=%d\n",ov420->exp.row_time);
        printk("ov420->exp.shutter_overlay=%d\n",ov420->exp.shutter_overlay);
        printk("ov420->exp.pix_clock=%d\n",ov420->exp.pix_clock);
        

    }

    //v4l2_dbg(1, debug, sd, "\n", );
    
    return ret < 0 ? ret : 0;

}

static int ov420_set_fmt(struct v4l2_subdev *sd,
               struct v4l2_format *f)
{
    struct ov420 *ov420 = to_ov420(sd);
    int ret;
    u16 xskip, yskip;

    printk("\nov420->x_current=%d\n",ov420->x_current);
    printk("ov420->y_current=%d\n",ov420->y_current);
    printk("f->fmt.pix.width=%d\n",f->fmt.pix.width);
    printk("f->fmt.pix.height=%d\n",f->fmt.pix.height);

    printk("\nThen reset params\n");
    ov420->x_current = 16;
    ov420->y_current = 54;
    f->fmt.pix.width = 384;
    f->fmt.pix.height = 384;
    printk("\nov420->x_current=%d\n",ov420->x_current);
    printk("ov420->y_current=%d\n",ov420->y_current);
    printk("f->fmt.pix.width=%d\n",f->fmt.pix.width);
    printk("f->fmt.pix.height=%d\n",f->fmt.pix.height);

    struct v4l2_rect rect = {
        .left   = ov420->x_current,
        .top    = ov420->y_current,
        .width  = f->fmt.pix.width,
        .height = f->fmt.pix.height,
    };
    
    /*
     * try_fmt has put rectangle within limits.
     * S_FMT - use binning and skipping for scaling, recalculate
     * limits, used for cropping
     */
    /* Is this more optimal than just a division */
    for (xskip = 8; xskip > 1; xskip--)
        if (rect.width * xskip <= OV420_MAX_WIDTH)
            break;

    for (yskip = 8; yskip > 1; yskip--)
        if (rect.height * yskip <= OV420_MAX_HEIGHT)
            break;
	
	if (xskip != yskip){
		if (xskip > yskip)
			xskip = yskip;
		else 
			yskip = xskip;
	}

    printk("\nxskip=%d\n",xskip);
    printk("\nyskip=%d\n",yskip);
    
    recalculate_limits(ov420, xskip, yskip);
    ret = ov420_set_params(sd, &rect, xskip, yskip);
    return ret;
}

static int ov420_try_fmt(struct v4l2_subdev *sd,
               struct v4l2_format *f)
{
    struct v4l2_pix_format *pix = &f->fmt.pix;

    if (pix->height < OV420_MIN_HEIGHT)
        pix->height = OV420_MIN_HEIGHT;
    if (pix->height > OV420_MAX_HEIGHT)
        pix->height = OV420_MAX_HEIGHT;
    if (pix->width < OV420_MIN_WIDTH)
        pix->width = OV420_MIN_WIDTH;
    if (pix->width > OV420_MAX_WIDTH)
        pix->width = OV420_MAX_WIDTH;

    pix->width &= ~0x01; /* has to be even */
    pix->height &= ~0x01; /* has to be even */
    return 0;
}

static int ov420_enum_framesizes(struct v4l2_subdev *sd,
                struct v4l2_frmsizeenum *frms)
{
    int ifmt;
    for (ifmt = 0; ifmt < ov420_num_formats; ifmt++) {
        if (frms->pixel_format == ov420_formats[ifmt].pixelformat) {
            break;
        }
    }
    /* Is requested pixelformat not found on sensor? */
    if (ifmt == ov420_num_formats) {
        v4l2_err(sd, "pixel format %d, not found on sensor\n",frms->pixel_format);
        return -EINVAL;
    }

    /* Do we already reached all discrete framesizes? */
    if (frms->index >= ov420_num_frmsizes){
        return -EINVAL;
    }

    frms->type = V4L2_FRMSIZE_TYPE_DISCRETE;
    frms->discrete.width = ov420_frame_table[frms->index].framesize.width;
    frms->discrete.height = ov420_frame_table[frms->index].framesize.height;

    return 0;
}

static int ov420_enum_frameintervals(struct v4l2_subdev *sd,
                struct v4l2_frmivalenum *frmi)
{
    int ifmt;
    struct v4l2_frmsizeenum frms;

    for (ifmt = 0; ifmt < ov420_num_formats; ifmt++) {
        if (frmi->pixel_format == ov420_formats[ifmt].pixelformat) {
            break;
        }
    }
    /* Is requested pixelformat not found on sensor? */
    if (ifmt == ov420_num_formats) {
        v4l2_err(sd, "pixel format %d, not found on sensor\n",frms.pixel_format);
        return -EINVAL;
    }

    frms.index = 0;
    frms.pixel_format = frmi->pixel_format;

    /* Do we already reached all discrete framesizes? */
    while (ov420_enum_framesizes(sd,&frms) >= 0) {
        if (((frmi->width == frms.discrete.width) &&
            (frmi->height == frms.discrete.height))){
            break;
        }
        frms.index++;
    }
    if (frms.index >= ov420_num_frmsizes){
        v4l2_err(sd, "Frame size:width=%d and height=%d, not supported on sensor\n",
                frmi->width,frmi->height);
        return -EINVAL;
    }

    if (frmi->index >= ov420_frame_table[frms.index].num_frmivals)
        return -EINVAL;

    frmi->type = V4L2_FRMSIZE_TYPE_DISCRETE;
    frmi->discrete.numerator =
                ov420_frame_table[frms.index].frameintervals[frmi->index].numerator;
    frmi->discrete.denominator =
                ov420_frame_table[frms.index].frameintervals[frmi->index].denominator;

    return 0;
}

static int ov420_get_chip_id(struct v4l2_subdev *sd,
                   struct v4l2_dbg_chip_ident *id)
{
    struct ov420 *ov420 = to_ov420(sd);
    struct i2c_client *client = v4l2_get_subdevdata(sd);;

    if (id->match.type != V4L2_CHIP_MATCH_I2C_ADDR)
        return -EINVAL;

    if (id->match.addr != client->addr)
        return -ENODEV;

    id->ident   = ov420->model;
    id->revision    = 0;

    return 0;
}

#ifdef CONFIG_VIDEO_ADV_DEBUG
static int ov420_get_register(struct v4l2_subdev *sd,
                struct v4l2_dbg_register *reg)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);;
    struct ov420 *ov420 = to_ov420(sd);

    if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->reg > 0xff)
        return -EINVAL;

    if (reg->match.addr != client->addr)
        return -ENODEV;

    reg->val = reg_read(client, reg->reg);

    if (reg->val > 0xffff)
        return -EIO;

    return 0;
}

static int ov420_set_register(struct v4l2_subdev *sd,
                struct v4l2_dbg_register *reg)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct ov420 *ov420 = to_ov420(sd);

    if (reg->match.type != V4L2_CHIP_MATCH_I2C_ADDR || reg->reg > 0xff)
        return -EINVAL;

    if (reg->match.addr != client->addr)
        return -ENODEV;

    if (reg_write(client, reg->reg, reg->val) < 0)
        return -EIO;

    return 0;
}
#endif


static const struct v4l2_subdev_core_ops ov420_core_ops = {
    .g_chip_ident = ov420_get_chip_id,
    .init = ov420_init,
    .queryctrl = ov420_queryctrl,
    .g_ctrl = ov420_get_control,
    .s_ctrl = ov420_set_control,
#ifdef CONFIG_VIDEO_ADV_DEBUG
    .get_register = ov420_get_register,
    .set_register = ov420_set_register,
#endif
};

static const struct v4l2_subdev_video_ops ov420_video_ops = {
    .s_fmt = ov420_set_fmt,
    .try_fmt = ov420_try_fmt,
    .s_stream = ov420_s_stream,
    .enum_framesizes = ov420_enum_framesizes,
    .enum_frameintervals = ov420_enum_frameintervals,
};

static const struct v4l2_subdev_ops ov420_ops = {
    .core = &ov420_core_ops,
    .video = &ov420_video_ops,
};

static int ov420_queryctrl(struct v4l2_subdev *sd,
                struct v4l2_queryctrl *qctrl)
{
    const struct v4l2_queryctrl *temp_qctrl;

    temp_qctrl = ov420_find_qctrl(qctrl->id);
    if (!temp_qctrl) {
        v4l2_dbg(1, debug,sd, "control id %d not supported", qctrl->id);
        return -EINVAL;
    }
    memcpy(qctrl, temp_qctrl, sizeof(*qctrl));
    return 0;
}

static int ov420_get_control(struct v4l2_subdev *sd,
                   struct v4l2_control *ctrl)
{
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    struct ov420 *ov420 = to_ov420(sd);
    int data;
    u8 reg = -1;
    bool gain = false;

    switch (ctrl->id) {
    case V4L2_CID_VFLIP:
        data = reg_read(client, OV420_READ_MODE_2);
        if (data < 0)
            return -EIO;
        ctrl->value = !!(data & 0x8000);
        break;
    case V4L2_CID_HFLIP:
        data = reg_read(client, OV420_READ_MODE_2);
        if (data < 0)
            return -EIO;
        ctrl->value = !!(data & 0x4000);
        break;
    case V4L2_CID_EXPOSURE:
        ctrl->value = ov420->exposure;
        break;
    case V4L2_CID_EXPOSURE_AUTO:
        ctrl->value = ov420->autoexposure;
        break;
    case V4L2_CID_RED_BALANCE:
        reg = OV420_RED_GAIN;
        gain = true;
        break;
    case V4L2_CID_BLUE_BALANCE:
        reg = OV420_BLUE_GAIN;
        gain = true;
        break;
    case V4L2_CID_BRIGHTNESS:
        reg = OV420_GREEN_1_GAIN;
        gain = true;
        break;
    case V4L2_CID_AUTOGAIN:
        reg = OV420_GREEN_2_GAIN;
        gain = true;
        break;
    }
    if (gain){
        data = reg_read(client, reg);
        if ( (data & 0x7f40)  == 0 )
            ctrl->value = data;
        else if ((data & 0x7f00) == 0)
            ctrl->value = ((data & 0x003f) << 1);
        else
            ctrl->value = ((data & 0xff00)>>5) + 64;
    }
    return 0;
}

static int ov420_set_control(struct v4l2_subdev *sd,
                   struct v4l2_control *ctrl)
{
    struct ov420 *ov420 = to_ov420(sd);
    const struct v4l2_queryctrl *qctrl = NULL;
    int data = 0, ret = 0;
    struct i2c_client *client = v4l2_get_subdevdata(sd);
    u32 old, sw;
    int vbmin;
    if (NULL == ctrl)
        return -EINVAL;

    ret = reg_set(client, OV420_OUTPUT_CONTROL, 1);

    qctrl = ov420_find_qctrl(ctrl->id);
    if (!qctrl) {
        v4l2_err(sd, "control id %d not supported", ctrl->id);
        return -EINVAL;
    }

    switch (ctrl->id) {
    case V4L2_CID_VFLIP:
        if (ctrl->value){
            data = reg_set(client, OV420_READ_MODE_2, 0x8000);
            ov420->mirror_column = 1;
        }else{
            data = reg_clear(client, OV420_READ_MODE_2, 0x8000);
            ov420->mirror_column = 0;
        }
        if (data < 0)
            return -EIO;
        break;
    case V4L2_CID_HFLIP:
        if (ctrl->value){
            data = reg_set(client, OV420_READ_MODE_2, 0x4000);
            ov420->mirror_row = 1;
        }else{
            data = reg_clear(client, OV420_READ_MODE_2, 0x4000);
            ov420->mirror_row = 0;
        }
        if (data < 0)
            return -EIO;
        break;
    case V4L2_CID_GAIN:
        /* This control will be used to modify Gain. */
        if (ctrl->value > qctrl->maximum || ctrl->value < qctrl->minimum){
            printk("I receive a value that exceeds the range:%d\n",(int)ctrl->value);
            return -EINVAL;
        }
        data = calc_gain(ctrl->value);
        v4l2_dbg(1, debug, sd, "Setting gain from 0x%x to 0x%x\n",
                 reg_read(client, OV420_GLOBAL_GAIN), data);
        ret= reg_write(client, OV420_GLOBAL_GAIN, data);
        if (ret < 0)
            return -EIO;
        /* Success */
        ov420->gain = ctrl->value;
        break;
    case V4L2_CID_EXPOSURE:
        /* ov420 has maximum == default */
        if (ctrl->value > qctrl->maximum ||
            ctrl->value < qctrl->minimum){
            printk("Exposure range is exceed:%d\n",(int)ctrl->value);
            return -EINVAL;
        }else {
            ov420->autoexposure = 0;
            get_shutter(sd, &old);
            /* Shutter width calculation,
             * ctrl->value must be in a fixed point 24:8 in us*/
            sw = (ctrl->value + (ov420->exp.shutter_overlay*2*
                ov420->exp.pix_clock)/1000)/ov420->exp.row_time;
            /* Shutter width must be greater than 1*/
            if (sw < 1)
                sw = 1;
            vbmin = sw - ov420->height + 1;
            if (vbmin < 8) {
                vbmin = ov420->vblank;
            }
            if (ov420->vblank < (unsigned int)vbmin){
                sw = ov420->vblank + ov420->height - 1;
            }
            ret = set_shutter(sd, sw);
            if (ret < 0){
                printk("I could not write shutter width register\n");
                return -EIO;
            } else {
                if (ov420->vblank < (unsigned int)vbmin){
                    ov420->exposure = sw * ov420->exp.row_time -
                        (ov420->exp.shutter_overlay * 2 *
                        ov420->exp.pix_clock)/1000;
                    v4l2_dbg(1, debug, sd,
                        "Limiting exposure to %d\n", ov420->exposure);
                    return -EINVAL;
                }
                else
                    ov420->exposure = ctrl->value;
            }
            v4l2_dbg(1, debug, sd,
                "Setting shutter width from %u to %u\n",
                old, ctrl->value);
        }
        break;
    case V4L2_CID_EXPOSURE_AUTO:
        if (ctrl->value) {
            const u16 vblank = OV420_VERTICAL_BLANK_DEFAULT;
            const u32 shutter_max = OV420_MAX_HEIGHT + vblank;
            if (set_shutter(sd, ov420->height +
                    ov420->y_skip_top + vblank) < 0)
                return -EIO;

            qctrl = ov420_find_qctrl(V4L2_CID_EXPOSURE);
            ov420->exposure
             =
                (shutter_max / 2 + (ov420->height +
                ov420->y_skip_top + vblank - 1) *
                (qctrl->maximum - qctrl->minimum)) /
                shutter_max + qctrl->minimum;
            ov420->autoexposure = 1;
        } else{
            ov420->autoexposure = 0;
        }break;
    case V4L2_CID_RED_BALANCE:
    case V4L2_CID_BLUE_BALANCE:
    case V4L2_CID_BRIGHTNESS:
    case V4L2_CID_AUTOGAIN:
        /* This control will be used to modify Gain */
        if (ctrl->value > qctrl->maximum || ctrl->value < qctrl->minimum){
            printk("I receive a value that exceeds the range:%d\n",(int)ctrl->value);
            return -EINVAL;
        }
        data = calc_gain(ctrl->value);
        v4l2_dbg(1, debug, sd, "Setting gain %d\n", data);
        switch (ctrl->id) {
            case V4L2_CID_RED_BALANCE:
                ret = reg_write(client, OV420_RED_GAIN, data);
                if (ret < 0){
                    printk("Fail setting Red Gain register\n");
                    return -EIO;
                }
                break;
            case V4L2_CID_BLUE_BALANCE:
                ret = reg_write(client, OV420_BLUE_GAIN, data);
                if (ret < 0){
                    printk("Fail setting Blue Gain register\n");
                    return -EIO;
                }
                break;
            case V4L2_CID_BRIGHTNESS:
                ret = reg_write(client, OV420_GREEN_1_GAIN, data);
                if (ret < 0){
                    printk("Fail setting Green1 Gain register\n");
                    return -EIO;
                }
                break;
            case V4L2_CID_AUTOGAIN:
                ret = reg_write(client, OV420_GREEN_2_GAIN, data);
                if (ret < 0){
                    printk("Fail setting Green2 Gain register\n");
                    return -EIO;
                }
                break;
        }
        break;
    }
    reg_clear(client, OV420_OUTPUT_CONTROL, 1);

    return 0;
}

/* Interface active, can use i2c. If it fails, it can indeed mean, that
 * this wasn't our capture interface, so, we wait for the right one */
static int ov420_detect(struct i2c_client *client, int *model)
{
    s32 data;

    /* Enable the chip */
//  data = reg_write(client, OV420_CHIP_ENABLE, 1);
//  dev_dbg(&client->dev, "write: %d\n", data);

    /* Read out the chip version register */
    //data = reg_read(client, OV420_CHIP_VERSION);  // 0x3010--HB, 0x3011--LB
    data = reg_read(client, 0x3011); 

    switch (data) {
    case 0x21:
        //*model = V4L2_IDENT_OV420;
        break;
    default:
        dev_err(&client->dev,
            "No OV420 chip detected, register read %x\n", data);
//        return -ENODEV;  // comment by andy
    }

    dev_info(&client->dev, "Detected a OV420 chip ID %x, ***\n", data);
    return 0;
}

static int ov420_probe(struct i2c_client *client,
             const struct i2c_device_id *did)
{
    struct ov420 *ov420;
    struct v4l2_subdev *sd;
    int pclk_pol;
    int ret;
    if (!i2c_check_functionality(client->adapter,
                     I2C_FUNC_SMBUS_WORD_DATA)) {
        dev_warn(&client->dev,
             "I2C-Adapter doesn't support I2C_FUNC_SMBUS_WORD\n");
        return -EIO;
    }

    if (!client->dev.platform_data) {
        dev_err(&client->dev, "No platform data!!\n");
        return -ENODEV;
    }

    pclk_pol = (int)client->dev.platform_data;

    ov420 = kzalloc(sizeof(struct ov420), GFP_KERNEL);
    if (!ov420)
        return -ENOMEM;

    ret = ov420_detect(client, &ov420->model);
    if (ret)
        goto clean;

    ov420->x_min      = OV420_COLUMN_SKIP;
    ov420->y_min      = OV420_ROW_SKIP;
    ov420->width      = OV420_DEFAULT_WIDTH;
    ov420->height     = OV420_DEFAULT_HEIGHT;
    ov420->x_current  = ov420->x_min;
    ov420->y_current  = ov420->y_min;
    ov420->width_min  = OV420_MIN_WIDTH;
    ov420->width_max  = OV420_MAX_WIDTH;
    ov420->height_min = OV420_MIN_HEIGHT;
    ov420->height_max = OV420_MAX_HEIGHT;
    ov420->y_skip_top = 10; //Originally it had 10, minimun value(6)which works
    ov420->autoexposure = 1;
    ov420->xskip = 1;
    ov420->yskip = 1;
    ov420->xbin = 1;
    ov420->ybin = 1;
    ov420->mirror_column = 0;
    ov420->mirror_row = 0;

    /* Register with V4L2 layer as slave device */
    sd = &ov420->sd;
    v4l2_i2c_subdev_init(sd, client, &ov420_ops);
    if (!pclk_pol)
        reg_clear(v4l2_get_subdevdata(sd),
              OV420_PIXEL_CLOCK_CONTROL, 0x8000);
    else
        reg_set(v4l2_get_subdevdata(sd),
        OV420_PIXEL_CLOCK_CONTROL, 0x8000);

    ret = ov420_init(sd,1);
    v4l2_info(sd, "%s decoder driver registered !!\n", sd->name);
    return 0;

clean:
    kfree(ov420);
    return ret;
}

static int ov420_remove(struct i2c_client *client)
{
    struct v4l2_subdev *sd = i2c_get_clientdata(client);
    struct ov420 *ov420 = to_ov420(sd);

    v4l2_device_unregister_subdev(sd);

    kfree(ov420);
    return 0;
}

static const struct i2c_device_id ov420_id[] = {
    { "ov420", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, ov420_id);

static struct i2c_driver ov420_i2c_driver = {
    .driver = {
        .name = "ov420",
    },
    .probe      = ov420_probe,
    .remove     = ov420_remove,
    .id_table   = ov420_id,
};

static int __init ov420_mod_init(void)
{
    return i2c_add_driver(&ov420_i2c_driver);
}
module_init(ov420_mod_init);

static void __exit ov420_mod_exit(void)
{
    i2c_del_driver(&ov420_i2c_driver);
}
module_exit(ov420_mod_exit);

MODULE_DESCRIPTION("Micron OV420 Camera driver");
MODULE_AUTHOR("Miguel Aguilar <miguel.aguilar@ridgerun.com>");
MODULE_LICENSE("GPL");