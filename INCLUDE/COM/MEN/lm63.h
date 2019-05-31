/***********************  I n c l u d e  -  F i l e  ************************/
/*!  
 *        \file  lm63.h
 *
 *      \author  dieter.pfeuffer@men.de
 * 
 *  	 \brief  Header file for LM63 chip
 *                      
 *     Switches: -
 */
/*
 *---------------------------------------------------------------------------
 * Copyright (c) 2005-2019, MEN Mikro Elektronik GmbH
 ****************************************************************************/
/*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _LM63_H
#define _LM63_H

#ifdef __cplusplus
	extern "C" {
#endif


/*--------------------------------------+
|   DEFINES                             |
+--------------------------------------*/

/*! \defgroup xxxdefgroup header defines

  LM63 internal address
*/
/**@{*/
#define LM63_TEMP				0x00	/**< Local Temperature */
#define LM63_RMTTEMP_MSB		0x01	/**< Rmt Temp MSB */
#define LM63_ALERT_STATE		0x02	/**< ALERT Status */
#define LM63_CFG				0x03	/**< Configuration */
#define LM63_CONVRATE			0x04	/**< Conversion Rate */
#define LM63_TEMP_HIGH			0x05	/**< Local High Setpoint */
#define LM63_RMTTEMP_HIGH_MSB	0x07	/**< Rmt High Setpoint MSB */
#define LM63_RMTTEMP_LOW_MSB	0x08	/**< Rmt Low Setpoint MSB */
#define LM63_ONESHOT			0x0f	/**< One Shot */

#define LM63_RMTTEMP_LSB		0x10	/**< Rmt Temp LSB */
#define LM63_RMTTEMP_OFF_MSB	0x11	/**< Rmt Temp Offset MSB */
#define LM63_RMTTEMP_OFF_LSB	0x12	/**< Rmt Temp Offset LSB */
#define LM63_RMTTEMP_HIGH_LSB	0x13	/**< Rmt High Setpoint LSB */
#define LM63_RMTTEMP_LOW_LSB	0x14	/**< Rmt Low Setpoint LSB */
#define LM63_ALERT_MASK			0x16	/**< ALERT Mask */
#define LM63_RMTTEMP_TCRIT_SET	0x19	/**< Rmt TCRIT Setpoint */
#define LM63_RMTTEMP_TCRIT_HYS	0x21	/**< Rmt TCRIT Hysteresis */

#define LM63_TACH_COUNT_LSB		0x46	/**< Tach Count LSB */
#define LM63_TACH_COUNT_MSB		0x47	/**< Tach Count MSB */
#define LM63_TACH_LIMIT_LSB		0x48	/**< Tach Limit LSB */
#define LM63_TACH_LIMIT_MSB		0x49	/**< Tach Limit MSB */
#define LM63_PWM_RPM			0x4A	/**< PWM and RPM */
#define LM63_FAN_SPINUP_CFG		0x4B	/**< Fan Spin-Up Config */
#define LM63_PWM_VALUE			0x4C	/**< PWM Value */
#define LM63_PWM_FREQU			0x4D	/**< PWM Frequency */

#define LM63_LOOKUP_HYS			0x4F	/**< Lookup Table Hystersis */
#define LM63_LOOKUP_TBL_TEMP(n)	(0x50 + 0x02*(n))	/**< Lookup Table TEMP (n=0..7) */
#define LM63_LOOKUP_TBL_PWM(n)	(0x51 + 0x02*(n))	/**< Lookup Table PWM (n=0..7) */

#define LM63_RMTTEMP_FILTER		0xBF	/**< Rmt Diode Temp Filter */

#define LM63_MANUFACTURER_ID	0xFE	/**< Manufacturer’s ID */
#define LM63_STEPPING_DIE_REV	0xFF	/**< Stepping/Die Rev. ID */
/**@}*/


/*! \defgroup xxxdefgroup header defines

  LM63 config register bit definitions
*/
/**@{*/
#define LM63_CONF_START			0x01 /**< start operation - clear bit -> standby */
#define LM63_CONF_INT_ENABLE	0x02 /**< enable interrupt output */
#define LM63_CONF_INT_CLEAR 	0x08 /**< disable interrupt output */
#define LM63_CONF_RESET     	0x10 /**< reset pulse 20ms */
#define LM63_CONF_CI_CLEAR  	0x40 /**< 20ms CI pulse */
#define LM63_CONF_INIT      	0x80 /**< soft chip reset */
/**@}*/







/*! \defgroup xxxdefgroup header defines

  LM63 config register bit definitions
*/
/**@{*/
#define LM63_CONF_START			0x01 /**< start operation - clear bit -> standby */
#define LM63_CONF_INT_ENABLE	0x02 /**< enable interrupt output */
#define LM63_CONF_INT_CLEAR 	0x08 /**< disable interrupt output */
#define LM63_CONF_RESET     	0x10 /**< reset pulse 20ms */
#define LM63_CONF_CI_CLEAR  	0x40 /**< 20ms CI pulse */
#define LM63_CONF_INIT      	0x80 /**< soft chip reset */
/**@}*/

/*! \defgroup xxxdefgroup header defines

  LM63 interrupt status 1 register bit definitions
*/
/**@{*/
#define LM63_ISTAT1_2_5V			0x01 /**< 2.5 V mask */
#define LM63_ISTAT1_VCCP_1			0x02 /**< VCCP1 mask */
#define LM63_ISTAT1_3_3V			0x04 /**< 3.3 V mask */
#define LM63_ISTAT1_5V				0x08 /**< 5 V mask */
#define LM63_ISTAT1_TEMP			0x10 /**< temperatur mask */
#define LM63_ISTAT1_RESERVED		0x20 /**< reserved mask */
#define LM63_ISTAT1_FAN1			0x40 /**< FAN1 mask */
#define LM63_ISTAT1_FAN2			0x80 /**< FAN2 mask */
/**@{*/

/*! \defgroup xxxdefgroup header defines

  LM63 interrupt status 2 register bit definitions
*/
/**@{*/
#define LM63_ISTAT2_12V   			0x01 /**< 12 V mask */
#define LM63_ISTAT2_VCCP_2			0x02 /**< VCCP2 mask */
#define LM63_ISTAT2_RES1			0x04 /**< reserved mask */
#define LM63_ISTAT2_RES2			0x08 /**< reserved mask */
#define LM63_ISTAT2_CI				0x10 /**< CI mask */
#define LM63_ISTAT2_RES3			0x20 /**< reserved mask */
#define LM63_ISTAT2_RES4			0x40 /**< reserved mask */
#define LM63_ISTAT2_TLOW			0x80 /**< temperature low mask */
/**@{*/

/*! \defgroup xxxdefgroup header defines

  LM63 interrupt mask 1 register bit definitions
*/
/**@{*/
#define LM63_IMASK1_2_5V			LM63_ISTAT1_2_5V	 /**< 2.5 V mask */
#define LM63_IMASK1_VCCP_1			LM63_ISTAT1_VCCP_1	 /**< VCCP1 mask */
#define LM63_IMASK1_3_3V			LM63_ISTAT1_3_3V	 /**< 3.3 V mask */
#define LM63_IMASK1_5V				LM63_ISTAT1_5V		 /**< 5 V mask */
#define LM63_IMASK1_TEMP			LM63_ISTAT1_TEMP	 /**< temperatur mask */
#define LM63_IMASK1_RESERVED		LM63_ISTAT1_RESERVED /**< reserved mask */
#define LM63_IMASK1_FAN1			LM63_ISTAT1_FAN1	 /**< FAN1 mask */
#define LM63_IMASK1_FAN2			LM63_ISTAT1_FAN2	 /**< FAN2 mask */
/**@{*/

/*! \defgroup xxxdefgroup header defines

  LM63 interrupt mask 2 register bit definitions
*/
/**@{*/
#define LM63_IMASK2_12V   			LM63_ISTAT2_12V   	 /**< 12 V mask */
#define LM63_IMASK2_VCCP_2			LM63_ISTAT2_VCCP_2	 /**< VCCP2 mask */
#define LM63_IMASK2_RES1			LM63_ISTAT2_RES1	 /**< reserved mask */
#define LM63_IMASK2_RES2			LM63_ISTAT2_RES2	 /**< reserved mask */
#define LM63_IMASK2_CI				LM63_ISTAT2_CI		 /**< CI mask */
#define LM63_IMASK2_RES3			LM63_ISTAT2_RES3	 /**< reserved mask */
#define LM63_IMASK2_RES4			LM63_ISTAT2_RES4	 /**< reserved mask */
#define LM63_IMASK2_TLOW			LM63_ISTAT2_TLOW	 /**< temperature low mask */
/**@{*/


/*! \defgroup xxxdefgroup header defines

  LM63 interrupt mask 2 register bit definitions
*/
/**@{*/
#define	LM63_TEM_CFG_MODE_REPETITIVE 	0x00	/**< repetitive interrupt mode */
#define	LM63_TEM_CFG_MODE_ONE_TIME	 	0x01	/**< one time interrupt mode */
#define	LM63_TEM_CFG_MODE_COMPERATOR	0x02	/**< comperator interrupt mode */
/**@{*/


/*! \defgroup xxxdefgroup header defines

  LM63 IIC bus address
*/
/**@{*/
#define LM63_IIC_ADDR       0x98				/**< IIC bus address */
/**@{*/

#ifdef __cplusplus
	}
#endif

#endif	/* _LM63_H */
