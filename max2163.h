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

#define RDIVIDER_LSB_MASK      0x01
#define NDIVIDER_LSB_MASK      0xf0
#define PLL_MOST_NDIVIDER(num)   ((num >> 4) & 0xff)
#define PLL_LEAST_NDIVIDER(num)  ((num << 4) & 0xf0)

#define IF_FILTER_REG              0x00
#define BANDWIDTH_43MHZ           (0 << 0)
#define BANDWIDTH_26MHZ           (1 << 0)
#define BANDWIDTH_17MHZ           (2 << 0)
#define BANDWIDTH_13MHZ           (3 << 0)
#define IF_FILTER_REG_BANDWDITH_MASK 0x03
#define BIAS_CURRENT_00           (0 << 2)
#define BIAS_CURRENT_01           (1 << 2)
#define BIAS_CURRENT_10           (2 << 2)
#define BIAS_CURRENT_11           (3 << 2)
#define FLTS_INTERNAL             (0 << 4)
#define FLTS_MANUAL               (1 << 4)
#define CENTER_FREQUENCY_0_75     (0 << 5)
#define CENTER_FREQUENCY_0_84     (1 << 5)
#define CENTER_FREQUENCY_0_92     (2 << 5)
#define CENTER_FREQUENCY_1_00     (3 << 5)
#define CENTER_FREQUENCY_1_08     (4 << 5)
#define CENTER_FREQUENCY_1_16     (5 << 5)
#define CENTER_FREQUENCY_1_25     (6 << 5)
#define CENTER_FREQUENCY_1_33     (7 << 5)

#define VAS_REG                       0x01
#define AUTOSELECT_14336_WAIT_TIME   (0 << 0)
#define AUTOSELECT_24576_WAIT_TIME   (1 << 0)
#define AUTOSELECT_34816_WAIT_TIME   (2 << 0)
#define AUTOSELECT_45056_WAIT_TIME   (3 << 0)
#define DISABLE_ADC_READ             (0 << 2)
#define ENABLE_ADC_READ              (1 << 2)
#define DISABLE_ADC_LATCH            (0 << 3)
#define ENABLE_ADC_LATCH             (1 << 3)
#define CPS_MANUAL                   (0 << 4)
#define CPS_AUTOMATIC                (1 << 4)
#define DISABLE_VCO_AUTOSELECT       (0 << 5)
#define ENABLE_VCO_AUTOSELECT        (1 << 5)
#define START_AT_CURR_LOADED_REGS    (2 << 6)
#define START_AT_CURR_USED_REGS      (3 << 6)

#define VCO_REG                      0x02
#define VCOB_NORMAL_POWER           (0 << 0)
#define VCOB_LOW_POWER              (1 << 0)
#define SUB_BAND_0                  (0 << 1)
#define SUB_BAND_1                  (1 << 1)
#define SUB_BAND_2                  (2 << 1)
#define SUB_BAND_3                  (3 << 1)
#define SUB_BAND_4                  (4 << 1)
#define SUB_BAND_5                  (5 << 1)
#define SUB_BAND_6                  (6 << 1)
#define SUB_BAND_7                  (7 << 1)
#define SUB_BAND_8                  (8 << 1)
#define SUB_BAND_9                  (9 << 1)
#define SUB_BAND_10                 (10 << 1)
#define SUB_BAND_11                 (11 << 1)
#define SUB_BAND_12                 (12 << 1)
#define SUB_BAND_13                 (13 << 1)
#define SUB_BAND_14                 (14 << 1)
#define SUB_BAND_15                 (15 << 1)
#define VCO_0                       (0 << 5)
#define VCO_1                       (1 << 5)
#define VCO_2                       (2 << 5)
#define VCO_REG_NOT_USED            (3 << 5)
#define VCO_FACTORY_USE_ONLY        (1 << 7)

#define RF_FILTER_REG               0x03
#define FREQ_RANGE_MASK             0xf0
#define UHF_RANGE_470_488MHZ       (0 << 0)
#define UHF_RANGE_488_512MHZ       (1 << 0)
#define UHF_RANGE_512_542MHZ       (2 << 0)
#define UHF_RANGE_542_572MHZ       (3 << 0)
#define UHF_RANGE_572_608MHZ       (4 << 0)
#define UHF_RANGE_608_656MHZ       (5 << 0)
#define UHF_RANGE_656_710MHZ       (6 << 0)
#define UHF_RANGE_710_806MHZ       (7 << 0)
#define AGC_MINUS_66DBM            (0 << 3)
#define AGC_MINUS_64DBM            (1 << 3)
#define AGC_MINUS_62DBM            (2 << 3)
#define AGC_MINUS_60DBM            (3 << 3)
#define AGC_MINUS_58DBM            (4 << 3)
#define AGC_MINUS_56DBM            (5 << 3)
#define AGC_MINUS_54DBM            (6 << 3)
#define AGC_MINUS_52DBM            (7 << 3)
#define PWRDET_BUF_OFF             (0 << 6)
#define PWRDET_BUF_ON_RFAGC        (1 << 6)
#define UNUSED                     (2 << 6)
#define PWRDET_BUF_ON_GC1          (3 << 6)

#define MODE_REG                   0x04
#define MODE_REG_FACTORY_USE       0x1f
#define HIGH_SIDE_INJECTION        (0 << 5)
#define LOW_SIDE_INJECTION         (1 << 5)
#define ENABLE_RF_FILTER           (0 << 6)
#define DISABLE_RF_FILTER          (1 << 6)
#define ENABLE_3RD_STAGE_RFVGA     (0 << 7)
#define DISABLE_3RD_STAGE_RFVG     (1 << 7)

#define RDIVIDER_MSB_REG           0x05
#define RDIVIDER_MSB_REG_MASK      0xff

/* 
 * All bits are used to set the PLL reference divider (R) number.
 * default R divide value is 126 decimal. R can range from 16 to 511 decimal.
 */

#define RDIVIDER_LSB_REG                   0x06
#define RDIVIDER_LSB_REG_FACTORY_USE_1    (1 << 1)
#define RFDA_37DB                         (0 << 2)
#define RFDA_34DB                         (1 << 2)
#define RFDA_31DB                         (2 << 2)
#define RFDA_28DB                         (3 << 2)
#define ENABLE_RF_DETECTOR                (0 << 4)
#define DISABLE_RF_DETECTOR               (1 << 4)
#define RDIVIDER_LSB_REG_FACTORY_USE_2    (1 << 5)
#define CHARGE_PUMP_1_5MA                 (0 << 6)
#define CHARGE_PUMP_2MA                   (1 << 6)
#define CHARGE_PUMP_2_5MA                 (2 << 6)
#define CHARGE_PUMP_3MA                   (3 << 6)

#define PLL_MOST_RDIVIDER(num)    ((num >> 1) & 0xff)
#define PLL_LEAST_RDIVIDER(num)   (num & RDIVIDER_LSB_MASK)

#define NDIVIDER_MSB_REG           0x07
/*
 * All bits are used to set the most significant bits of the PLL integer 
 * divide number (N). Default integer divide value is N = 1952 decimal. N 
 * can range from 1314 to 2687.
 */

#define NDIVIDER_LSB_REG               0x08
#define STBY_NORMAL                   (0 << 0)
#define STBY_DISABLED                 (1 << 0)
#define RFVGA_NORMAL                  (0 << 1)
#define RFVGA_HIGH                    (1 << 1)
#define MIX_NORMAL                    (0 << 2)
#define MIX_HIGH                      (1 << 2)
#define NDIVIDER_LSB_REG_FACTORY_USE  (1 << 3)

/* Read-only register */
#define STATUS_REG                 0x09
#define STATUS_REG_MASK            0x3f
#define PWR_CYCLE                  0x01
#define CHARGE_PUMP_SETTING        0x06
#define VTUNE_ADC_CONVERSION       0x38 /* PLL lock info */
#define STATUS_REG_UNUSED          0xc0

/* Read-only register */
#define VAS_STATUS_REG         0x0a
#define VAS_STATUS_REG_MASK    0xff
#define VASE                   0x01
#define VASA                   0x02
#define VCO_SUBBAND            0x3c
#define VCO_AUTOSELECT         0xc0

#define RESERVED_0B_REG              0x0b
#define RESERVED_0C_REG              0x0c
#define RESERVED_0D_REG              0x0d
#define RESERVED_0E_REG              0x0e
#define RESERVED_0F_REG              0x0f
#define RESERVED_10_REG              0x10
#define RESERVED_11_REG              0x11

#define RESERVED_0B_REG_MASK         0x00
#define RESERVED_0C_REG_MASK         0x00
#define RESERVED_0D_REG_MASK         0x00
#define RESERVED_0E_REG_MASK         0x00
#define RESERVED_0F_REG_MASK         0x00
#define RESERVED_10_REG_MASK         0x00
#define RESERVED_11_REG_MASK         0x00

#endif /* __max2163_h */
