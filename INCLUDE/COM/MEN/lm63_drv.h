/***********************  I n c l u d e  -  F i l e  ***********************/
/*!
 *        \file  lm63_drv.h
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2009/08/31 11:41:53 $
 *    $Revision: 3.2 $
 *
 *       \brief  Header file for LM63 driver containing
 *               LM63 specific status codes and
 *               LM63 function prototypes
 *
 *    \switches  _ONE_NAMESPACE_PER_DRIVER_
 *               _LL_DRV_
 */
 /*
 *---------------------------------------------------------------------------
 * Copyright (c) 2019, MEN Mikro Elektronik GmbH
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

#ifndef _LM63_DRV_H
#define _LM63_DRV_H

#ifdef __cplusplus
      extern "C" {
#endif

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/** \name LM63 specific Getstat/Setstat codes
 *  \anchor getstat_setstat_codes
 *  Note: not yet used</td>
 */
/**@{*/
#define LM63_XXX		M_DEV_OF+0x00   /**< G,S: description\n
											      Values: \n
												  Default: \n*/

/* LM63 specific Getstat/Setstat block codes */
#define LM63_BLK_XXX	M_DEV_BLK_OF+0x00   /**< G: description*/
/**@}*/


/** \name LM63 channel selector
 *  \anchor channel_selector
 */
/**@{*/
#define LM63_CH_TEMP		0	/**< LM63 die temperature (-55..125) [degrees Celsius] */
#define LM63_CH_RMTTEMP		1	/**< remote temperature (-55..125) [degrees Celsius] */
#define LM63_CH_FANSPEED	2	/**< fan speed [rpm] */
/**@}*/


#ifndef  LM63_VARIANT
# define LM63_VARIANT LM63
#endif

# define _LM63_GLOBNAME(var,name) var##_##name
#ifndef _ONE_NAMESPACE_PER_DRIVER_
# define LM63_GLOBNAME(var,name) _LM63_GLOBNAME(var,name)
#else
# define LM63_GLOBNAME(var,name) _LM63_GLOBNAME(LM63,name)
#endif

#define __LM63_GetEntry    LM63_GLOBNAME(LM63_VARIANT,GetEntry)

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
#ifdef _LL_DRV_
#ifndef _ONE_NAMESPACE_PER_DRIVER_
	extern void __LM63_GetEntry(LL_ENTRY* drvP);
#endif
#endif /* _LL_DRV_ */

/*-----------------------------------------+
|  BACKWARD COMPATIBILITY TO MDIS4         |
+-----------------------------------------*/
#ifndef U_INT32_OR_64
 /* we have an MDIS4 men_types.h and mdis_api.h included */
 /* only 32bit compatibility needed!                     */
 #define INT32_OR_64  int32
 #define U_INT32_OR_64 u_int32
 typedef INT32_OR_64  MDIS_PATH;
#endif /* U_INT32_OR_64 */


#ifdef __cplusplus
      }
#endif

#endif /* _LM63_DRV_H */
