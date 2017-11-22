/*
 * TI Booting and Flashing Utilities
 *
 * This file provides low-level init functions for use in the UBL for booting
 * an application.
 *
 * Copyright (C) 2009 Texas Instruments Incorporated - http://www.ti.com/
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation version 2.
 *
 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* --------------------------------------------------------------------------
  AUTHOR      : Platform Support Group
 --------------------------------------------------------------------------- */
 
// General type include
#include "tistdtypes.h"

// Device specific CSL
#include "device.h"

// Util functions
#include "util.h" 

//I2C functions
#include "i2c.h"

//TPS 65023 functions
#include "tps65070.h"

static TPS_ConfigObj tpsConfigObj[TPS_MAX_NUM_DEVICES];

Uint32 TPS65070_open(Uint32 instance)
{
  Uint32 status = E_PASS;
  
  switch(instance)
  {
	case 0:
		tpsConfigObj[0].i2cConfig.addrMode  = I2C_ADDRESSING_7BIT;
		tpsConfigObj[0].i2cConfig.i2cclkh   = DEVICE_I2C_CLKH;
  		tpsConfigObj[0].i2cConfig.i2cclkl   = DEVICE_I2C_CLKL;
  		tpsConfigObj[0].i2cConfig.ownAddr   = DEVICE_I2C_OWN_ADDRESS;
  		tpsConfigObj[0].i2cConfig.prescalar = DEVICE_I2C_CLK_PRESCALE;
  
		tpsConfigObj[0].hI2cInfo = NULL;
  		tpsConfigObj[0].hI2cInfo = \
    		I2C_open(0, I2C_ROLE_MASTER, I2C_MODE_AUTO_STOP, &tpsConfigObj[0].i2cConfig);
  
  		status = (tpsConfigObj[0].hI2cInfo ? E_PASS : E_FAIL) ;
  
  		if(status == E_PASS) {
			tpsConfigObj[0].hI2cInfo->slaveAddr = DEVICE_I2C_TPS65070_PMIC_SLAVE_ADDR;
  		}
	break;
	default:
		status = E_FAIL;
	break;
  }

  return status;
}

Uint8 TPS65070_reg_read(Uint32 instance, Uint8 regOffset, Uint8 *buf )
{
    Uint32 status = E_PASS;

	if(instance >= TPS_MAX_NUM_DEVICES) return E_FAIL;

    status = I2C_writeBytes(tpsConfigObj[instance].hI2cInfo,1u, &regOffset);

    if(status != E_PASS){
		status = I2C_readBytes(tpsConfigObj[instance].hI2cInfo,1u, buf);
	}
    
    return status;
}

Uint32 TPS65070_reg_write(Uint32 instance, Uint8 regOffset, Uint8 regVal)
{

  Uint32  status  = E_PASS;
  Uint8   buf[2];

  if(instance >= TPS_MAX_NUM_DEVICES) return E_FAIL; 

  buf[0] = regOffset;
  buf[1] = regVal;

  status = I2C_writeBytes(tpsConfigObj[instance].hI2cInfo, 2u, buf);    

  return status;
}

Uint32 TPS65070_set_DCDC3_voltage(Uint32 instance, Uint16 volt)
{
  Uint32  status  = E_PASS;

  if(instance >= TPS_MAX_NUM_DEVICES) return E_FAIL;

  status = TPS65070_open(instance);
  
  switch (volt)
  {
    /* For valid values just go and set the register value */
    case TPS_VDCDC3_MILLIVOLT_725  :
    case TPS_VDCDC3_MILLIVOLT_750  :
    case TPS_VDCDC3_MILLIVOLT_775  :
    case TPS_VDCDC3_MILLIVOLT_800  :
    case TPS_VDCDC3_MILLIVOLT_825  :
    case TPS_VDCDC3_MILLIVOLT_850  :
    case TPS_VDCDC3_MILLIVOLT_875  :
    case TPS_VDCDC3_MILLIVOLT_900  :
    case TPS_VDCDC3_MILLIVOLT_925  :
    case TPS_VDCDC3_MILLIVOLT_950  :
    case TPS_VDCDC3_MILLIVOLT_975  :
    case TPS_VDCDC3_MILLIVOLT_1000 :
    case TPS_VDCDC3_MILLIVOLT_1025 :
    case TPS_VDCDC3_MILLIVOLT_1050 :
    case TPS_VDCDC3_MILLIVOLT_1075 :
    case TPS_VDCDC3_MILLIVOLT_1100 :
    case TPS_VDCDC3_MILLIVOLT_1125 :
    case TPS_VDCDC3_MILLIVOLT_1150 :
    case TPS_VDCDC3_MILLIVOLT_1175 :
    case TPS_VDCDC3_MILLIVOLT_1200 :
    case TPS_VDCDC3_MILLIVOLT_1225 :
    case TPS_VDCDC3_MILLIVOLT_1250 :
    case TPS_VDCDC3_MILLIVOLT_1275 :
    case TPS_VDCDC3_MILLIVOLT_1300 :
    case TPS_VDCDC3_MILLIVOLT_1325 :
    case TPS_VDCDC3_MILLIVOLT_1350 :
    case TPS_VDCDC3_MILLIVOLT_1375 :
    case TPS_VDCDC3_MILLIVOLT_1400 :
    case TPS_VDCDC3_MILLIVOLT_1425 :
    case TPS_VDCDC3_MILLIVOLT_1450 :
    case TPS_VDCDC3_MILLIVOLT_1475 :
    case TPS_VDCDC3_MILLIVOLT_1500 :
    case TPS_VDCDC3_MILLIVOLT_1550 :
    case TPS_VDCDC3_MILLIVOLT_1600 :
    case TPS_VDCDC3_MILLIVOLT_1650 :
    case TPS_VDCDC3_MILLIVOLT_1700 :
    case TPS_VDCDC3_MILLIVOLT_1750 :
    case TPS_VDCDC3_MILLIVOLT_1800 :
    case TPS_VDCDC3_MILLIVOLT_1850 :
    case TPS_VDCDC3_MILLIVOLT_1900 :
    case TPS_VDCDC3_MILLIVOLT_1950 :
    case TPS_VDCDC3_MILLIVOLT_2000 :
    case TPS_VDCDC3_MILLIVOLT_2050 :
    case TPS_VDCDC3_MILLIVOLT_2100 :
    case TPS_VDCDC3_MILLIVOLT_2150 :
    case TPS_VDCDC3_MILLIVOLT_2200 :
    case TPS_VDCDC3_MILLIVOLT_2250 :
    case TPS_VDCDC3_MILLIVOLT_2300 :
    case TPS_VDCDC3_MILLIVOLT_2350 :
    case TPS_VDCDC3_MILLIVOLT_2400 :
    case TPS_VDCDC3_MILLIVOLT_2450 :
    case TPS_VDCDC3_MILLIVOLT_2500 :
    case TPS_VDCDC3_MILLIVOLT_2550 :
    case TPS_VDCDC3_MILLIVOLT_2600 :
    case TPS_VDCDC3_MILLIVOLT_2650 :
    case TPS_VDCDC3_MILLIVOLT_2700 :
    case TPS_VDCDC3_MILLIVOLT_2750 :
    case TPS_VDCDC3_MILLIVOLT_2800 :
    case TPS_VDCDC3_MILLIVOLT_2850 :
    case TPS_VDCDC3_MILLIVOLT_2900 :
    case TPS_VDCDC3_MILLIVOLT_3000 :
    case TPS_VDCDC3_MILLIVOLT_3100 :
    case TPS_VDCDC3_MILLIVOLT_3200 :
    case TPS_VDCDC3_MILLIVOLT_3300 :
         /*
          1. Set the value of desired voltage in DEFDCDC3 register (DEFDCDC pin 
             is high, hence DEFDCDC3 register defines the output voltage.
         */
        
        status = TPS65070_reg_write(instance, TPS_REG_OFFSET_DEFDCDC3_HIGH, volt);
        
        if(status == E_PASS) {
        /* 
          2. Start the transition
          Set GO bit in CTRL2 register
         */
        	status = TPS65070_reg_write(instance, TPS_REG_OFFSET_CONCTRL2, 0x80);
        }
        
    break;

    default:
        status = E_FAIL;
    break;    
  }
  
  return status;
}
