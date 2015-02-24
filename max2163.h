/**
 * ISDB-T 1Seg DTV USB device driver
 * IBAYO IB-200 / ZIROK DTV-1 - Zinwell chipset
 * max2163.h - definitions for MAX2163 ISDB-T 1-Segment Tuner IC
 *
 * Copyright (c) 2010, Lucas C. Villa Real <lucasvr@gobolinux.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Released under the GNU GPL version 
 *
 * For more information on this project, please visit the following URL:
 * http://groups.fsf.org/wiki/LinuxLibre:ISDB_USB_ZINWELL
 */
#ifndef __max2163_h
#define __max2163_h

#define MAX2163_I2C_WRITE_ADDR                 0xc0 /* may also be 0xc2 */
#define MAX2163_I2C_READ_ADDR                  0xc1 /* may also be 0xc3 */

#define MAX2163_I2C_IF_FILTER_REG              0x00
#define _IF_FILTER_REG_MASK                    0xff
#define _IF_FILTER_43MHZ_BANDWIDTH            (0x00 & 0x03)
#define _IF_FILTER_26MHZ_BANDWIDTH            (0x01 & 0x03)
#define _IF_FILTER_17MHZ_BANDWIDTH            (0x02 & 0x03)
#define _IF_FILTER_13MHZ_BANDWIDTH            (0x03 & 0x03)
#define _IF_FILTER_BIAS_CURRENT_00           ((0x00 << 2) & 0x0c)
#define _IF_FILTER_BIAS_CURRENT_01           ((0x01 << 2) & 0x0c)
#define _IF_FILTER_BIAS_CURRENT_10           ((0x02 << 2) & 0x0c)
#define _IF_FILTER_BIAS_CURRENT_11           ((0x03 << 2) & 0x0c)
#define _IF_FILTER_FLTS_INTERNAL             ((0x00 << 4) & 0x10)
#define _IF_FILTER_FLTS_MANUAL               ((0x01 << 4) & 0x10)
#define _IF_FILTER_CENTER_FREQUENCY_0_75     ((0x00 << 5) & 0xe0)
#define _IF_FILTER_CENTER_FREQUENCY_0_84     ((0x01 << 5) & 0xe0)
#define _IF_FILTER_CENTER_FREQUENCY_0_92     ((0x02 << 5) & 0xe0)
#define _IF_FILTER_CENTER_FREQUENCY_1_00     ((0x03 << 5) & 0xe0)
#define _IF_FILTER_CENTER_FREQUENCY_1_08     ((0x04 << 5) & 0xe0)
#define _IF_FILTER_CENTER_FREQUENCY_1_16     ((0x05 << 5) & 0xe0)
#define _IF_FILTER_CENTER_FREQUENCY_1_25     ((0x06 << 5) & 0xe0)
#define _IF_FILTER_CENTER_FREQUENCY_1_33     ((0x07 << 5) & 0xe0)

#define MAX2163_I2C_VAS_REG                    0x01
#define _VAS_REG_MASK                          0xff
#define _VAS_REG_AUTOSELECT_14336_WAIT_TIME   (0x00 & 0x03)
#define _VAS_REG_AUTOSELECT_24576_WAIT_TIME   (0x01 & 0x03)
#define _VAS_REG_AUTOSELECT_34816_WAIT_TIME   (0x02 & 0x03)
#define _VAS_REG_AUTOSELECT_45056_WAIT_TIME   (0x03 & 0x03)
#define _VAS_REG_DISABLE_ADC_READ            ((0x00 << 2) & 0x04)
#define _VAS_REG_ENABLE_ADC_READ             ((0x01 << 2) & 0x04)
#define _VAS_REG_DISABLE_ADC_LATCH           ((0x00 << 3) & 0x08)
#define _VAS_REG_ENABLE_ADC_LATCH            ((0x01 << 3) & 0x08)
#define _VAS_REG_CPS_MANUAL                  ((0x00 << 4) & 0x10)
#define _VAS_REG_CPS_AUTOMATIC               ((0x01 << 4) & 0x10)
#define _VAS_REG_DISABLE_VCO_AUTOSELECT      ((0x00 << 5) & 0x20)
#define _VAS_REG_ENABLE_VCO_AUTOSELECT       ((0x01 << 5) & 0x20)
#define _VAS_REG_START_AT_CURR_LOADED_REGS   ((0x00 << 6) & 0xc0)
#define _VAS_REG_START_AT_CURR_USED_REGS     ((0x01 << 6) & 0xc0)

#define MAX2163_I2C_VCO_REG                    0x02
#define _VCO_REG_MASK                          0x7f
#define _VCO_REG_VCOB_NORMAL_POWER            (0x00 & 0x01)
#define _VCO_REG_VCOB_LOW_POWER               (0x01 & 0x01)
#define _VCO_REG_SUB_BAND_0                  ((0x00 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_1                  ((0x01 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_2                  ((0x02 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_3                  ((0x03 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_4                  ((0x04 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_5                  ((0x05 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_6                  ((0x06 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_7                  ((0x07 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_8                  ((0x08 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_9                  ((0x09 << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_10                 ((0x0a << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_11                 ((0x0b << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_12                 ((0x0c << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_13                 ((0x0d << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_14                 ((0x0e << 1) & 0x1e)
#define _VCO_REG_SUB_BAND_15                 ((0x0f << 1) & 0x1e)
#define _VCO_REG_VCO_0                       ((0x00 << 5) & 0x20)
#define _VCO_REG_VCO_1                       ((0x01 << 5) & 0x20)
#define _VCO_REG_VCO_2                       ((0x02 << 5) & 0x20)
#define _VCO_REG_NOT_USED                    ((0x03 << 5) & 0x20)
#define _VCO_FACTORY_USE_ONLY                ((0x01 << 7) & 0x80)

#define MAX2163_I2C_RF_FILTER_REG              0x03
#define _RF_FILTER_REG_MASK                    0xff
#define _RF_FILTER_UHF_RANGE_470_488MHZ       (0x00 & 0x07)
#define _RF_FILTER_UHF_RANGE_488_512MHZ       (0x01 & 0x07)
#define _RF_FILTER_UHF_RANGE_512_542MHZ       (0x02 & 0x07)
#define _RF_FILTER_UHF_RANGE_542_572MHZ       (0x03 & 0x07)
#define _RF_FILTER_UHF_RANGE_572_608MHZ       (0x04 & 0x07)
#define _RF_FILTER_UHF_RANGE_608_656MHZ       (0x05 & 0x07)
#define _RF_FILTER_UHF_RANGE_656_710MHZ       (0x06 & 0x07)
#define _RF_FILTER_UHF_RANGE_710_806MHZ       (0x07 & 0x07)
#define _RF_FILTER_AGC_MINUS_66DBM           ((0x00 << 3) & 0x38)
#define _RF_FILTER_AGC_MINUS_64DBM           ((0x01 << 3) & 0x38)
#define _RF_FILTER_AGC_MINUS_62DBM           ((0x02 << 3) & 0x38)
#define _RF_FILTER_AGC_MINUS_60DBM           ((0x03 << 3) & 0x38)
#define _RF_FILTER_AGC_MINUS_58DBM           ((0x04 << 3) & 0x38)
#define _RF_FILTER_AGC_MINUS_56DBM           ((0x05 << 3) & 0x38)
#define _RF_FILTER_AGC_MINUS_54DBM           ((0x06 << 3) & 0x38)
#define _RF_FILTER_AGC_MINUS_52DBM           ((0x07 << 3) & 0x38)
#define _RF_FILTER_PWRDET_BUF_OFF            ((0x00 << 6) & 0xc0)
#define _RF_FILTER_PWRDET_BUF_ON_RFAGC       ((0x01 << 6) & 0xc0)
#define _RF_FILTER_UNUSED                    ((0x02 << 6) & 0xc0)
#define _RF_FILTER_PWRDET_BUF_ON_GC1         ((0x03 << 6) & 0xc0)

#define MAX2163_I2C_MODE_REG                   0x04
#define _MODE_REG_MASK                         0xe0
#define _MODE_REG_FACTORY_USE                 (0x1f)
#define _MODE_REG_LOW_SIDE_INJECTION         ((0x01 << 5) & 0x20)
#define _MODE_REG_HIGH_SIDE_INJECTION        ((0x00 << 5) & 0x20)
#define _MODE_REG_ENABLE_RF_FILTER           ((0x00 << 6) & 0x40)
#define _MODE_REG_DISABLE_RF_FILTER          ((0x01 << 6) & 0x40)
#define _MODE_REG_ENABLE_3RD_STAGE_RFVGA     ((0x00 << 7) & 0x80)
#define _MODE_REG_DISABLE_3RD_STAGE_RFVG     ((0x01 << 7) & 0x80)

#define MAX2163_I2C_RDIVIDER_MSB_REG           0x05
#define _RDIVIDER_MSB_REG_MASK                 0xff
/* 
 * All bits are used to set the PLL reference divider (R) number.
 * default R divide value is 126 decimal. R can range from 16 to 511 decimal.
 */

#define MAX2163_I2C_RDIVIDER_LSB_REG           0x06
#define _RDIVIDER_LSB_REG_MASK                 0xdd
#define _RDIVIDER_LSB_REG_RO_MASK             (0x01)
#define _RDIVIDER_LSB_REG_FACTORY_USE_1      ((0x01 << 1) & 0x02)
#define _RDIVIDER_LSB_REG_RFDA_37DB          ((0x00 << 2) & 0x0c)
#define _RDIVIDER_LSB_REG_RFDA_34DB          ((0x01 << 2) & 0x0c)
#define _RDIVIDER_LSB_REG_RFDA_31DB          ((0x02 << 2) & 0x0c)
#define _RDIVIDER_LSB_REG_RFDA_28DB          ((0x03 << 2) & 0x0c)
#define _RDIVIDER_LSB_REG_ENABLE_RF_DETECTOR ((0x00 << 4) & 0x10)
#define _RDIVIDER_LSB_REG_DISABLE_RF_DETECTOR ((0x01<< 4) & 0x10)
#define _RDIVIDER_LSB_REG_FACTORY_USE_2      ((0x01 << 5) & 0x20)
#define _RDIVIDER_LSB_REG_CHARGE_PUMP_1_5MA  ((0x00 << 6) & 0xc0)
#define _RDIVIDER_LSB_REG_CHARGE_PUMP_2MA    ((0x01 << 6) & 0xc0)
#define _RDIVIDER_LSB_REG_CHARGE_PUMP_2_5MA  ((0x02 << 6) & 0xc0)
#define _RDIVIDER_LSB_REG_CHARGE_PUMP_3MA    ((0x03 << 6) & 0xc0)

#define PLL_MOST_RDIVIDER(num)   ((num >> 1) & 0xff)
#define PLL_LEAST_RDIVIDER(num)   (num & _RDIVIDER_LSB_REG_RO_MASK)

#define MAX2163_I2C_NDIVIDER_MSB_REG           0x07
#define _NDIVIDER_MSB_REG_MASK                 0xff
/*
 * All bits are used to set the most significant bits of the PLL integer 
 * divide number (N). Default integer divide value is N = 1952 decimal. N 
 * can range from 1314 to 2687.
 */

#define MAX2163_I2C_NDIVIDER_LSB_REG           0x08
#define _NDIVIDER_LSB_REG_MASK                 0xf7
#define _NDIVIDER_LSB_REG_STBY_NORMAL         (0x00 & 0x01)
#define _NDIVIDER_LSB_REG_STBY_DISABLED       (0x01 & 0x01)
#define _NDIVIDER_LSB_REG_RFVGA_NORMAL       ((0x00 << 1) & 0x02)
#define _NDIVIDER_LSB_REG_RFVGA_HIGH         ((0x01 << 1) & 0x02)
#define _NDIVIDER_LSB_REG_MIX_NORMAL         ((0x00 << 2) & 0x04)
#define _NDIVIDER_LSB_REG_MIX_HIGH           ((0x01 << 2) & 0x04)
#define _NDIVIDER_LSB_REG_FACTORY_USE        ((0x01 << 3) & 0x08)


#define PLL_MOST_NDIVIDER(num)   ((num >> 4) & 0xff)
#define PLL_LEAST_NDIVIDER(num)  ((num << 4) & 0xf0)

/* Read-only register */
#define MAX2163_I2C_STATUS_REG                 0x09
#define _STATUS_REG_MASK                       0x3f
#define _STATUS_REG_PWR_CYCLE                  0x01
#define _STATUS_REG_CHARGE_PUMP_SETTING        0x06
#define _STATUS_REG_VTUNE_ADC_CONVERSION       0x38 /* PLL lock info */
#define _STATUS_REG_UNUSED                     0xc0

/* Read-only register */
#define MAX2163_I2C_VAS_STATUS_REG             0x0a
#define _VAS_STATUS_REG_MASK                   0xff
#define _VAS_STATUS_REG_VASE                   0x01
#define _VAS_STATUS_REG_VASA                   0x02
#define _VAS_STATUS_REG_VCO_SUBBAND            0x3c
#define _VAS_STATUS_REG_VCO_AUTOSELECT         0xc0

#define MAX2163_I2C_RESERVED1_REG              0x0b
#define _RESERVED1_REG_MASK                    0x00
#define MAX2163_I2C_RESERVED2_REG              0x0c
#define _RESERVED2_REG_MASK                    0x00
#define MAX2163_I2C_RESERVED3_REG              0x0d
#define _RESERVED3_REG_MASK                    0x00
#define MAX2163_I2C_RESERVED4_REG              0x0e
#define _MAX2163_I2C_RESERVED4_REG_MASK        0x00
#define MAX2163_I2C_RESERVED5_REG              0x0f
#define _RESERVED5_REG_MASK                    0x00
#define MAX2163_I2C_RESERVED6_REG              0x10
#define _RESERVED6_REG_MASK                    0x00
#define MAX2163_I2C_RESERVED7_REG              0x11
#define _RESERVED7_REG_MASK                    0x00

#endif /* __max2163_h */
