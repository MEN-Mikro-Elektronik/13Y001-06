/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  lm63_doc.c
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2009/08/31 11:41:49 $
 *    $Revision: 1.2 $
 *
 *      \brief   User documentation for LM63 device driver
 *
 *     Required: -
 *
 *     \switches -
 */
 /*-------------------------------[ History ]--------------------------------
 *
 * $Log: lm63_doc.c,v $
 * Revision 1.2  2009/08/31 11:41:49  MRoth
 * R: Porting to MIDS5
 * M: Changed description to MDIS5; added compatibility hint
 *
 * Revision 1.1  2005/11/25 15:19:06  dpfeuffer
 * Initial Revision
 *
 *---------------------------------------------------------------------------
 * (c) Copyright 2005 by MEN Mikro Elektronik GmbH, Nuernberg, Germany
 ****************************************************************************/

/*! \mainpage
    This is the documentation of the MDIS5 low-level driver for the LM63 SMB device.

    The LM63 is a remote diode temperature sensor with integrated fan control.
	The LM63 accurately measures: its own temperature and the temperature of a
	diode-connected transistor, such as a thermal diode commonly found on
	Computer Processors.

	Note: MDIS5 32bit drivers are compatible to the MDIS4 drivers but must not
          be mixed with MDIS4 drivers at one target system.

    <b>Common Notes:</b>
	- The driver requires MDIS with SMB2 support
	- If this MEN driver is used, no other third-party driver or tool for the LM63
	  device shall be used.

    \n
    \section Variants Variants
    In order to support different LM63 modes, the LM63 driver can be built in some
	variants at compilation time:

    \code
    Driver              Variant Description
    --------            --------------------------------
    Standard            LM63 TACH mode
    \endcode

    \n \section FuncDesc Functional Description

    \n \subsection General General
	The driver supports reading of the current value of the current channel via M_read()
	and reading blocks of the current values from all channels via M_getblock(). The driver
	provides the values within a signed 32-bit data quadword.

	When the first path is opened to an LM63 device, the HW and the driver are being
	initialized with default values	(see section about \ref descriptor_entries).

    \n \subsection channels Logical channels
	The driver provides three logical channels:\n
	 ch 0: LM63 die temperature (-55..125) [degrees Celsius]\n
	 ch 1: remote temperature (-55..125) [degrees Celsius]\n
	 ch 2: fan speed [rpm]\n
	Note: The \ref channel_selector defines can be used from within user-mode applications to
	select the current channel.

    \n \section api_functions Supported API Functions

    <table border="0">
    <tr>
        <td><b>API Function</b></td>
        <td><b>Functionality</b></td>
        <td><b>Corresponding Low-Level Function</b></td></tr>

    <tr><td>M_open()</td><td>Open device</td><td>LM63_Init()</td></tr>

    <tr><td>M_close()     </td><td>Close device             </td>
    <td>LM63_Exit())</td></tr>
    <tr><td>M_read()      </td><td>Read from device         </td>
    <td>LM63_Read()</td></tr>
    <tr><td>M_setstat()   </td><td>Set device parameter     </td>
    <td>LM63_SetStat()</td></tr>
    <tr><td>M_getstat()   </td><td>Get device parameter     </td>
    <td>LM63_GetStat()</td></tr>
    <tr><td>M_getblock()  </td><td>Block read from device   </td>
    <td>LM63_BlockRead()</td></tr>
    <tr><td>M_errstringTs() </td><td>Generate error message </td>
    <td>-</td></tr>
    </table>

    \n \section descriptor_entries LM63 Descriptor Entries

    The low-level driver initialization routine decodes the following entries
    ("keys") in addition to the general descriptor keys:

    <table border="0">
    <tr><td><b>Descriptor entry</b></td>
        <td><b>Description</b></td>
        <td><b>Values</b></td>
    </tr>
    <tr><td>SMB_BUSNBR</td>
        <td>SMBus bus number</td>
        <td>0, 1, 2, ...\n
			Default: 0</td>
    </tr>
    <tr><td>SMB_DEVADDR</td>
        <td>SMBus address of LM63</td>
        <td>0x00, 0xff\n
			Default: 0x98</td>
    </tr>
    </tr>
    <tr><td>TACH_PULSE</td>
        <td>pulse per revolution fan tachometer</td>
        <td>1,2,3\n
			Default: 3</td>
    </tr>
    </tr>
    <tr><td>RMT_TEMP_OFFSET</td>
        <td>remote temp offset\n
		The remote temperature can be adjusted, since the specified value will
		automatically be subtracted from or added to the remote temperature reading.</td>
        <td>-55..125 [degrees Celsius]\n
			Default: 0</td>
    </tr>
    </table>

    \n \subsection lm63_min   Minimum descriptor
    lm63_min.dsc (see Examples section)\n
    Demonstrates the minimum set of options necessary for using the driver.

    \n \subsection lm63_max   Maximum descriptor
    lm63_max.dsc (see Examples section)\n
    Shows all possible configuration options for this driver.

    \n \section codes LM63 specific Getstat/Setstat codes
    none

    \n \section programs Overview of provided LM63 programs

    \subsection lm63_simp  Simple example of using the driver
    lm63_simp.c (see Examples section)
*/

/** \example lm63_simp.c
Simple example of driver usage
*/

/** \example lm63_min.dsc */
/** \example lm63_max.dsc */

/*! \page dummy
  \menimages
*/
