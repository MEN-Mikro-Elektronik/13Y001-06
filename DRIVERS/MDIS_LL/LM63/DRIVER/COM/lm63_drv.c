/*********************  P r o g r a m  -  M o d u l e ***********************/
/*!
 *        \file  lm63_drv.c
 *
 *      \author  dieter.pfeuffer@men.de
 *        $Date: 2009/09/04 11:33:38 $
 *    $Revision: 1.4 $
 *
 *      \brief   Low-level driver for LM63 device on SMBus
 *
 *     Required: OSS, DESC, DBG, libraries
 *
 *     \switches _ONE_NAMESPACE_PER_DRIVER_
 */
 /*
 *---------------------------------------------------------------------------
 * (c) Copyright 2005..2008 by MEN Mikro Elektronik GmbH, Nuernberg, Germany
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

#define _NO_LL_HANDLE		/* ll_defs.h: don't define LL_HANDLE struct */

#ifdef LM63_SW				/* swapped variant */
#	define MAC_MEM_MAPPED
#	define ID_SW
#endif

#include <MEN/men_typs.h>   /* system dependent definitions */
#include <MEN/maccess.h>    /* hw access macros and types */
#include <MEN/dbg.h>        /* debug functions */
#include <MEN/oss.h>        /* oss functions */
#include <MEN/desc.h>       /* descriptor functions */
#include <MEN/mdis_api.h>   /* MDIS global defs */
#include <MEN/mdis_com.h>   /* MDIS common defs */
#include <MEN/mdis_err.h>   /* MDIS error codes */
#include <MEN/ll_defs.h>    /* low-level driver definitions */
#include <MEN/smb2.h>		/* SMB2 definitions */
#include <MEN/lm63.h>		/* LM63 definitions	*/

/*-----------------------------------------+
|  DEFINES                                 |
+-----------------------------------------*/
/* general defines */
#define CH_NUMBER			3			/**< Number of device channels */
#define CH_BYTES			2			/**< Number of bytes per channel */
#define USE_IRQ				FALSE		/**< Interrupt required  */
#define ADDRSPACE_COUNT		0			/**< Number of required address spaces */
#define ADDRSPACE_SIZE		0			/**< Size of address space */

/* debug defines */
#define DBG_MYLEVEL			llHdl->dbgLevel   /**< Debug level */
#define DBH					llHdl->dbgHdl     /**< Debug handle */

/*-----------------------------------------+
|  TYPEDEFS                                |
+-----------------------------------------*/
/** low-level handle */
typedef struct {
	/* general */
    int32           memAlloc;		/**< Size allocated for the handle */
    OSS_HANDLE      *osHdl;         /**< OSS handle */
    OSS_IRQ_HANDLE  *irqHdl;        /**< IRQ handle */
    DESC_HANDLE     *descHdl;       /**< DESC handle */
	MDIS_IDENT_FUNCT_TBL idFuncTbl;	/**< ID function table */
	/* debug */
    u_int32         dbgLevel;		/**< Debug level */
	DBG_HANDLE      *dbgHdl;        /**< Debug handle */
	/* lm63 specific */
	SMB_HANDLE		*smbH;			/**< ptr to SMB_HANDLE struct */
	u_int16			smbAddr;		/**< SMB address of LM63 */
	u_int32			tachPuls;		/**< pulse per revolution fan tachometer */
	int32			rmtTempOff;		/**< remote temp offset [°C] */
} LL_HANDLE;

/* include files which need LL_HANDLE */
#include <MEN/ll_entry.h>   /* low-level driver jump table  */
#include <MEN/lm63_drv.h>	/* LM63 driver header file */

/*-----------------------------------------+
|  PROTOTYPES                              |
+-----------------------------------------*/
static int32 LM63_Init(DESC_SPEC *descSpec, OSS_HANDLE *osHdl,
					   MACCESS *ma, OSS_SEM_HANDLE *devSemHdl,
					   OSS_IRQ_HANDLE *irqHdl, LL_HANDLE **llHdlP);
static int32 LM63_Exit(LL_HANDLE **llHdlP );
static int32 LM63_Read(LL_HANDLE *llHdl, int32 ch, int32 *value);
static int32 LM63_Write(LL_HANDLE *llHdl, int32 ch, int32 value);
static int32 LM63_SetStat(LL_HANDLE *llHdl,int32 ch, int32 code,
							INT32_OR_64 value32_or_64 );
static int32 LM63_GetStat(LL_HANDLE *llHdl, int32 ch, int32 code,
							INT32_OR_64 *value32_or_64P );
static int32 LM63_BlockRead(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							int32 *nbrRdBytesP);
static int32 LM63_BlockWrite(LL_HANDLE *llHdl, int32 ch, void *buf, int32 size,
							 int32 *nbrWrBytesP);
static int32 LM63_Irq(LL_HANDLE *llHdl );
static int32 LM63_Info(int32 infoType, ... );

static char* Ident( void );
static int32 Cleanup(LL_HANDLE *llHdl, int32 retCode);

/* LM63 specific helper functions */
static int32 ReadValue(
   LL_HANDLE    *llHdl,
   int32		ch,
   int32		*valueP);

/****************************** LM63_GetEntry ********************************/
/** Initialize driver's jump table
 *
 *  \param drvP     \OUT Pointer to the initialized jump table structure
 */
#ifdef _ONE_NAMESPACE_PER_DRIVER_
    void LL_GetEntry( LL_ENTRY* drvP )
#else
    void __LM63_GetEntry( LL_ENTRY* drvP )
#endif /* _ONE_NAMESPACE_PER_DRIVER_ */
{
    drvP->init        = LM63_Init;
    drvP->exit        = LM63_Exit;
    drvP->read        = LM63_Read;
    drvP->write       = LM63_Write;
    drvP->blockRead   = LM63_BlockRead;
    drvP->blockWrite  = LM63_BlockWrite;
    drvP->setStat     = LM63_SetStat;
    drvP->getStat     = LM63_GetStat;
    drvP->irq         = LM63_Irq;
    drvP->info        = LM63_Info;
}

/******************************** LM63_Init **********************************/
/** Allocate and return low-level handle, initialize hardware
 *
 * The function initializes the LM63 device with the definitions made
 * in the descriptor.
 *
 * The function decodes \ref descriptor_entries "these descriptor entries"
 * in addition to the general descriptor keys.
 *
 *  \param descP      \IN  Pointer to descriptor data
 *  \param osHdl      \IN  OSS handle
 *  \param ma         \IN  HW access handle
 *  \param devSemHdl  \IN  Device semaphore handle
 *  \param irqHdl     \IN  IRQ handle
 *  \param llHdlP     \OUT Pointer to low-level driver handle
 *
 *  \return           \c 0 On success or error code
 */
static int32 LM63_Init(
    DESC_SPEC       *descP,
    OSS_HANDLE      *osHdl,
    MACCESS         *ma,
    OSS_SEM_HANDLE  *devSemHdl,
    OSS_IRQ_HANDLE  *irqHdl,
    LL_HANDLE       **llHdlP
)
{
    LL_HANDLE	*llHdl = NULL;
    u_int32		gotsize, smbBusNbr;
    int32		error;
    u_int32		value;
	u_int8		pwmRpm;

    /*------------------------------+
    |  prepare the handle           |
    +------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */

	/* alloc */
    if((llHdl = (LL_HANDLE*)OSS_MemGet(
    				osHdl, sizeof(LL_HANDLE), &gotsize)) == NULL)
       return(ERR_OSS_MEM_ALLOC);

	/* clear */
    OSS_MemFill(osHdl, gotsize, (char*)llHdl, 0x00);

	/* init */
    llHdl->memAlloc   = gotsize;
    llHdl->osHdl      = osHdl;
    llHdl->irqHdl     = irqHdl;

    /*------------------------------+
    |  init id function table       |
    +------------------------------*/
	/* driver's ident function */
	llHdl->idFuncTbl.idCall[0].identCall = Ident;
	/* library's ident functions */
	llHdl->idFuncTbl.idCall[1].identCall = DESC_Ident;
	llHdl->idFuncTbl.idCall[2].identCall = OSS_Ident;
	/* terminator */
	llHdl->idFuncTbl.idCall[3].identCall = NULL;

    /*------------------------------+
    |  prepare debugging            |
    +------------------------------*/
	DBG_MYLEVEL = OSS_DBG_DEFAULT;	/* set OS specific debug level */
	DBGINIT((NULL,&DBH));

    /*------------------------------+
    |  scan descriptor              |
    +------------------------------*/
	/* prepare access */
    if((error = DESC_Init(descP, osHdl, &llHdl->descHdl)))
		return( Cleanup(llHdl,error) );

    /* DEBUG_LEVEL_DESC */
    if((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&value, "DEBUG_LEVEL_DESC")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

	DESC_DbgLevelSet(llHdl->descHdl, value);	/* set level */

    /* DEBUG_LEVEL */
    if((error = DESC_GetUInt32(llHdl->descHdl, OSS_DBG_DEFAULT,
								&llHdl->dbgLevel, "DEBUG_LEVEL")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    DBGWRT_1((DBH, "LL - LM63_Init\n"));

    /* SMB_BUSNBR (required) */
    if((error = DESC_GetUInt32(llHdl->descHdl, 0,
								&smbBusNbr, "SMB_BUSNBR")))
		return( Cleanup(llHdl,error) );

    /* SMB_DEVADDR (required) */
    if((error = DESC_GetUInt32(llHdl->descHdl, 0,
								&value, "SMB_DEVADDR")))
		return( Cleanup(llHdl,error) );
	llHdl->smbAddr = (u_int16)value;

    /* TACH_PULSE */
    if((error = DESC_GetUInt32(llHdl->descHdl, 3,
								&llHdl->tachPuls, "TACH_PULSE")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );
	if( (llHdl->tachPuls < 1) && (llHdl->tachPuls > 3) ){
		return( Cleanup(llHdl,ERR_LL_DESC_PARAM) );
	}

    /* RMT_TEMP_OFFSET */
    if((error = DESC_GetUInt32(llHdl->descHdl, 0,
					(u_int32*)(&llHdl->rmtTempOff), "RMT_TEMP_OFFSET")) &&
		error != ERR_DESC_KEY_NOTFOUND)
		return( Cleanup(llHdl,error) );

    /*------------------------------+
    |  init hardware                |
    +------------------------------*/
	if((error = OSS_GetSmbHdl( llHdl->osHdl, smbBusNbr, (void**)&llHdl->smbH) ))
		return( Cleanup(llHdl,error) );

#if 0
	/* just for debugging - dump all LM63 registers */
	{
	u_int8	regVal;
    int32	regOff;

	for( regOff=0; regOff<=0xff; regOff++ ){
		ReadByteData( llHdl->smbH, 0, llHdl->smbAddr, (u_int8)regOff, &regVal );
		DBGWRT_2((DBH, "LM63 regOff=0x%02x : regVal=0x%02x\n", regOff, regVal));
	}
	}
#endif

	/*
	 * PWM and RPM register:
	 *   bit - value: used configuration
	 *   5   -  1: the PWM Value (register 4C) and the Lookup Table (50 5F) are read/write enabled
	 *   4   -  0: the PWM output pin will be 0 V for fan OFF and open for fan ON
	 *   3   -  1: master PWM clock is 1.4 kHz
	 *   2   -  0: unused
	 *   1:0 - 10: most accurate readings, FFFF reading when under minimum detectable RPM
	 */
	pwmRpm = 0x2a;
	if( (error = llHdl->smbH->WriteByteData( llHdl->smbH, 0, llHdl->smbAddr,
							  LM63_PWM_RPM, pwmRpm )) )
		return( Cleanup(llHdl,error) );

	/*
	 * Fan Spin-Up Config register:
	 *   use POR value
	 */
	if( (error = llHdl->smbH->WriteByteData( llHdl->smbH, 0, llHdl->smbAddr,
							  LM63_FAN_SPINUP_CFG, 0x3F )) )
		return( Cleanup(llHdl,error) );

	/*
	 * PWM Frequency register:
	 *   use POR value
	 */
	if( (error = llHdl->smbH->WriteByteData( llHdl->smbH, 0, llHdl->smbAddr,
							  LM63_PWM_FREQU, 0x17 )) )
		return( Cleanup(llHdl,error) );

	/*
	 * PWM Value register:
	 *   use POR value
	 */
	if( (error = llHdl->smbH->WriteByteData( llHdl->smbH, 0, llHdl->smbAddr,
							  LM63_PWM_VALUE, 0x00 )) )
		return( Cleanup(llHdl,error) );

	/*
	 * PWM and RPM register:
	 *   bit - value: used configuration
	 *   5   -  0: the PWM Value (register 4C) and the Lookup Table (50 5F) are read-only
	 */
	pwmRpm &= ~0x20;
	if( (error = llHdl->smbH->WriteByteData( llHdl->smbH, 0, llHdl->smbAddr,
							  LM63_PWM_RPM, pwmRpm )) )
		return( Cleanup(llHdl,error) );

	/*
	 * Remote temp offset:
	 */
	if( (error = llHdl->smbH->WriteByteData( llHdl->smbH, 0, llHdl->smbAddr,
							  LM63_RMTTEMP_OFF_LSB, 0 )) )
		return( Cleanup(llHdl,error) );

	if( (error = llHdl->smbH->WriteByteData( llHdl->smbH, 0, llHdl->smbAddr,
							  LM63_RMTTEMP_OFF_MSB,
							  (u_int8)llHdl->rmtTempOff)) )
		return( Cleanup(llHdl,error) );

	*llHdlP = llHdl;	/* set low-level driver handle */

	return(ERR_SUCCESS);
}

/****************************** LM63_Exit ************************************/
/** De-initialize hardware and clean up memory
 *
 *  The function deinitializes the LM63 device.
 *
 *  \param llHdlP      \IN  Pointer to low-level driver handle
 *
 *  \return           \c 0 On success or error code
 */
static int32 LM63_Exit(
   LL_HANDLE    **llHdlP
)
{
    LL_HANDLE *llHdl = *llHdlP;
	int32 error = 0;

    DBGWRT_1((DBH, "LL - LM63_Exit\n"));

    /*------------------------------+
    |  de-init hardware             |
    +------------------------------*/

    /*------------------------------+
    |  clean up memory               |
    +------------------------------*/
	*llHdlP = NULL;		/* set low-level driver handle to NULL */
	error = Cleanup(llHdl,error);

	return(error);
}

/****************************** LM63_Read ************************************/
/** Read a value from the device
 *
 *  The function reads the current value of the current channel and stores
 *  it as signed 32-bit value in valueP:
 *   ch=0,1 : temperature [degrees Celsius]
 *   ch=2   : pulse frequency [rpm]
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param ch         \IN  Current channel
 *  \param valueP     \OUT Read value
 *
 *  \return           \c 0 On success or error code
 */
static int32 LM63_Read(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 *valueP
)
{
    DBGWRT_1((DBH, "LL - LM63_Read: ch=%d\n",ch));

	return ReadValue( llHdl, ch, valueP );
}

/****************************** LM63_Write ***********************************/
/** Write a value to the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param ch         \IN  Current channel
 *  \param value      \IN  Read value
 *
 *  \return           \c ERR_LL_ILL_FUNC
 */
static int32 LM63_Write(
    LL_HANDLE *llHdl,
    int32 ch,
    int32 value
)
{
    DBGWRT_1((DBH, "LL - LM63_Write: ch=%d\n",ch));

	return(ERR_LL_ILL_FUNC);
}

/****************************** LM63_SetStat *********************************/
/** Set the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl  	  \IN  Low-level handle
 *  \param code       \IN  \ref getstat_setstat_codes "status code"
 *  \param ch         \IN  Current channel
 *  \param value32_or_64  \IN  Data or pointer to block data structure (M_SG_BLOCK)
 *                             for block status codes
 *  \return           \c 0 On success or error code
 */
static int32 LM63_SetStat(
    LL_HANDLE *llHdl,
    int32  code,
    int32  ch,
    INT32_OR_64 value32_or_64
)
{
	int32 error = ERR_SUCCESS;
	int32 value	= (int32)value32_or_64;	/* 32bit value */

    DBGWRT_1((DBH, "LL - LM63_SetStat: ch=%d code=0x%04x value=0x%x\n",
			  ch,code,value));

    switch(code) {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
            llHdl->dbgLevel = value;
            break;
        /*--------------------------+
        |  channel direction        |
        +--------------------------*/
        case M_LL_CH_DIR:
			if( value != M_CH_IN )
				error = ERR_LL_ILL_DIR;
            break;
        /*--------------------------+
        |  unknown                  |
        +--------------------------*/
        default:
			error = ERR_LL_UNK_CODE;
    }

	return(error);
}

/****************************** LM63_GetStat *********************************/
/** Get the driver status
 *
 *  The driver supports \ref getstat_setstat_codes "these status codes"
 *  in addition to the standard codes (see mdis_api.h).
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param code       \IN  \ref getstat_setstat_codes "status code"
 *  \param ch         \IN  Current channel
 *  \param value32_or_64P  \IN  Pointer to block data structure (M_SG_BLOCK) for
 *                              block status codes
 *  \param value32_or_64P  \OUT Data pointer or pointer to block data structure
 *                              (M_SG_BLOCK) for block status codes
 *
 *  \return           \c 0 On success or error code
 */
static int32 LM63_GetStat(
    LL_HANDLE *llHdl,
    int32  code,
    int32  ch,
    INT32_OR_64 *value32_or_64P
)
{
	int32 error = ERR_SUCCESS;

	int32		*valueP	  = (int32*)value32_or_64P;	/* pointer to 32bit value  */
	INT32_OR_64	*value64P = value32_or_64P;		 	/* stores 32/64bit pointer */

    DBGWRT_1((DBH, "LL - LM63_GetStat: ch=%d code=0x%04x\n",
			  ch,code));

    switch(code)
    {
        /*--------------------------+
        |  debug level              |
        +--------------------------*/
        case M_LL_DEBUG_LEVEL:
            *valueP = llHdl->dbgLevel;
            break;
        /*--------------------------+
        |  number of channels       |
        +--------------------------*/
        case M_LL_CH_NUMBER:
            *valueP = CH_NUMBER;
            break;
        /*--------------------------+
        |  channel direction        |
        +--------------------------*/
        case M_LL_CH_DIR:
            *valueP = M_CH_IN;
            break;
        /*--------------------------+
        |  channel length [bits]    |
        +--------------------------*/
        case M_LL_CH_LEN:
            *valueP = 32;
            break;
        /*--------------------------+
        |  channel type info        |
        +--------------------------*/
        case M_LL_CH_TYP:
            *valueP = M_CH_ANALOG;
            break;
        /*--------------------------+
        |   ident table pointer     |
        |   (treat as non-block!)   |
        +--------------------------*/
        case M_MK_BLK_REV_ID:
           *value64P = (INT32_OR_64)&llHdl->idFuncTbl;
           break;
        /*--------------------------+
        |  unknown                  |
        +--------------------------*/
        default:
			error = ERR_LL_UNK_CODE;
    }

	return(error);
}

/******************************* LM63_BlockRead ******************************/
/** Read a data block from the device
 *
 *  The function reads the current value of channels 0..x where x depends on
 *  the buffer size n. For each channel to read the buffer must provide four
 *  bytes.
 *
 *                +--------------+
 *                | bytes   0..3 | channel 0
 *                +--------------+
 *                .              .
 *                +--------------+
 *                | bytes n-3..n | channel x
 *                +--------------+
 *
 *  The read values will be stored in the buffer as signed 32-bit values:
 *   ch=0,1 : temperature [degrees Celsius]
 *   ch=2   : pulse frequency [rpm]
 *
 *  \param llHdl       \IN  Low-level handle
 *  \param ch          \IN  Current channel
 *  \param buf         \IN  Data buffer
 *  \param size        \IN  Data buffer size
 *  \param nbrRdBytesP \OUT Number of read bytes
 *
 *  \return            \c 0 On success or error code
 */
static int32 LM63_BlockRead(
     LL_HANDLE *llHdl,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrRdBytesP
)
{
	int32	status=ERR_SUCCESS;
	int32	chan, ch2read;
	int32	*bufP = (int32*)buf;

    DBGWRT_1((DBH, "LL - LM63_BlockRead: ch=%d, size=%d\n",ch,size));

	/* return number of read bytes */
	*nbrRdBytesP = 0;

	if ( size < (CH_NUMBER * 4) )
		ch2read =  size / 4;
	else
		ch2read = CH_NUMBER;

	for(chan=0; chan<ch2read; chan++) {

		if( (status=ReadValue( llHdl, ch, bufP++ )) )
			return status;

		*nbrRdBytesP += 4;
	}

	return(0);
}

/****************************** LM63_BlockWrite ******************************/
/** Write a data block from the device
 *
 *  The function is not supported and always returns an ERR_LL_ILL_FUNC error.
 *
 *  \param llHdl  	   \IN  Low-level handle
 *  \param ch          \IN  Current channel
 *  \param buf         \IN  Data buffer
 *  \param size        \IN  Data buffer size
 *  \param nbrWrBytesP \OUT Number of written bytes
 *
 *  \return            \c ERR_LL_ILL_FUNC
 */
static int32 LM63_BlockWrite(
     LL_HANDLE *llHdl,
     int32     ch,
     void      *buf,
     int32     size,
     int32     *nbrWrBytesP
)
{
    DBGWRT_1((DBH, "LL - LM63_BlockWrite: ch=%d, size=%d\n",ch,size));

	/* return number of written bytes */
	*nbrWrBytesP = 0;

	return(ERR_LL_ILL_FUNC);
}


/****************************** LM63_Irq ************************************/
/** Interrupt service routine - unused
 *
 *  If the driver can detect the interrupt's cause it returns
 *  LL_IRQ_DEVICE or LL_IRQ_DEV_NOT, otherwise LL_IRQ_UNKNOWN.
 *
 *  \param llHdl  	   \IN  Low-level handle
 *  \return LL_IRQ_DEVICE	IRQ caused by device
 *          LL_IRQ_DEV_NOT  IRQ not caused by device
 *          LL_IRQ_UNKNOWN  Unknown
 */
static int32 LM63_Irq(
   LL_HANDLE *llHdl
)
{

	return(LL_IRQ_DEV_NOT);
}

/****************************** LM63_Info ***********************************/
/** Get information about hardware and driver requirements
 *
 *  The following info codes are supported:
 *
 * \code
 *  Code                      Description
 *  ------------------------  -----------------------------
 *  LL_INFO_HW_CHARACTER      Hardware characteristics
 *  LL_INFO_ADDRSPACE_COUNT   Number of required address spaces
 *  LL_INFO_ADDRSPACE         Address space information
 *  LL_INFO_IRQ               Interrupt required
 *  LL_INFO_LOCKMODE          Process lock mode required
 * \endcode
 *
 *  The LL_INFO_HW_CHARACTER code returns all address and
 *  data modes (ORed) which are supported by the hardware
 *  (MDIS_MAxx, MDIS_MDxx).
 *
 *  The LL_INFO_ADDRSPACE_COUNT code returns the number
 *  of address spaces used by the driver.
 *
 *  The LL_INFO_ADDRSPACE code returns information about one
 *  specific address space (MDIS_MAxx, MDIS_MDxx). The returned
 *  data mode represents the widest hardware access used by
 *  the driver.
 *
 *  The LL_INFO_IRQ code returns whether the driver supports an
 *  interrupt routine (TRUE or FALSE).
 *
 *  The LL_INFO_LOCKMODE code returns which process locking
 *  mode the driver needs (LL_LOCK_xxx).
 *
 *  \param infoType	   \IN  Info code
 *  \param ...         \IN  Argument(s)
 *
 *  \return            \c 0 On success or error code
 */
static int32 LM63_Info(
   int32  infoType,
   ...
)
{
    int32   error = ERR_SUCCESS;
    va_list argptr;

    va_start(argptr, infoType );

    switch(infoType) {
		/*-------------------------------+
        |  hardware characteristics      |
        |  (all addr/data modes ORed)   |
        +-------------------------------*/
        case LL_INFO_HW_CHARACTER:
		{
			u_int32 *addrModeP = va_arg(argptr, u_int32*);
			u_int32 *dataModeP = va_arg(argptr, u_int32*);

			*addrModeP = MDIS_MA08;
			*dataModeP = MDIS_MD08 | MDIS_MD16;
			break;
	    }
		/*-------------------------------+
        |  nr of required address spaces |
        |  (total spaces used)           |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE_COUNT:
		{
			u_int32 *nbrOfAddrSpaceP = va_arg(argptr, u_int32*);

			*nbrOfAddrSpaceP = ADDRSPACE_COUNT;
			break;
	    }
		/*-------------------------------+
        |  address space type            |
        |  (widest used data mode)       |
        +-------------------------------*/
        case LL_INFO_ADDRSPACE:
		{
			error = ERR_LL_ILL_PARAM;
			break;
	    }
		/*-------------------------------+
        |   interrupt required           |
        +-------------------------------*/
        case LL_INFO_IRQ:
		{
			u_int32 *useIrqP = va_arg(argptr, u_int32*);

			*useIrqP = USE_IRQ;
			break;
	    }
		/*-------------------------------+
        |   process lock mode            |
        +-------------------------------*/
        case LL_INFO_LOCKMODE:
		{
			u_int32 *lockModeP = va_arg(argptr, u_int32*);

			*lockModeP = LL_LOCK_CALL;
			break;
	    }
		/*-------------------------------+
        |   (unknown)                    |
        +-------------------------------*/
        default:
          error = ERR_LL_ILL_PARAM;
    }

    va_end(argptr);
    return(error);
}

/*******************************  Ident  ***********************************/
/** Return ident string
 *
 *  \return            Pointer to ident string
 */
static char* Ident( void )
{
    return( "LM63 - LM63 low-level driver: $Id: lm63_drv.c,v 1.4 2009/09/04 11:33:38 MRoth Exp $" );
}

/********************************* Cleanup *********************************/
/** Close all handles, free memory and return error code
 *
 *	\warning The low-level handle is invalid after this function is called.
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param retCode    \IN  Return value
 *
 *  \return           \IN   retCode
 */
static int32 Cleanup(
   LL_HANDLE    *llHdl,
   int32        retCode
)
{
    /*------------------------------+
    |  close handles                |
    +------------------------------*/
	/* clean up desc */
	if(llHdl->descHdl)
		DESC_Exit(&llHdl->descHdl);

	/* clean up debug */
	DBGEXIT((&DBH));

    /*------------------------------+
    |  free memory                  |
    +------------------------------*/
    /* free my handle */
    OSS_MemFree(llHdl->osHdl, (int8*)llHdl, llHdl->memAlloc);

    /*------------------------------+
    |  return error code            |
    +------------------------------*/
	return(retCode);
}

/********************************* ReadValue *********************************/
/** Read measured value from LM63
 *
 *  \param llHdl      \IN  Low-level handle
 *  \param ch         \IN  Current channel
 *  \param valueP     \OUT Read value
 *
 *  \return            \c 0 On success or error code
 */
static int32 ReadValue(
   LL_HANDLE    *llHdl,
   int32		ch,
   int32		*valueP
)
{
	u_int8	lowByte, highByte;
	u_int16	word;
	int32	error, sign, round;

	switch( ch ){

		/* local temp [°C] */
		case LM63_CH_TEMP:
			if( (error = llHdl->smbH->ReadByteData( llHdl->smbH, 0,
							llHdl->smbAddr, LM63_TEMP, &lowByte )) )
				return( error );
			*valueP = (int32)((int8)lowByte);
			DBGWRT_2((DBH, " local temp [°C] = %d\n", *valueP));
			break;

		/* remote temp [°C] */
		case LM63_CH_RMTTEMP:
			if( (error = llHdl->smbH->ReadByteData( llHdl->smbH, 0,
							llHdl->smbAddr, LM63_RMTTEMP_MSB, &highByte )) )
				return( error );
			if( (error = llHdl->smbH->ReadByteData( llHdl->smbH, 0,
							llHdl->smbAddr, LM63_RMTTEMP_LSB, &lowByte )) )
				return( error );
			word = (u_int16)(((u_int16)highByte<<8) | lowByte);

			/* absolute temp */
			if( word & 0x8000 ){
				sign = -1;
				word = (u_int16)(~word +1);
			}
			else
				sign = 1;

			/* round: <0.5=0, >=0.5=1 */
			round = (word >> 7) & 0x1;

			/* compute temp [°C] */
			*valueP = (int32)( sign * ( (u_int32)(word >> 8) + round) );
			DBGWRT_2((DBH, " remote temp [°C] = %d\n", *valueP));
			break;

		/* fan speed [rpm] */
		case LM63_CH_FANSPEED:
			if( (error = llHdl->smbH->ReadByteData( llHdl->smbH, 0,
							llHdl->smbAddr, LM63_TACH_COUNT_MSB, &highByte )) )
				return( error );
			if( (error = llHdl->smbH->ReadByteData( llHdl->smbH, 0,
							llHdl->smbAddr, LM63_TACH_COUNT_LSB, &lowByte )) )
				return( error );
			word = (u_int16)(((u_int16)highByte<<8) | lowByte);

			if ( (word==0xffff) | (word==0) ){
				*valueP = 0;	/* 0Hz */
			}
			else{
				*valueP = (2 * 5400000) / (llHdl->tachPuls * word);
			}
			DBGWRT_2((DBH, " fan speed [rpm] = %d\n", *valueP));
			break;
		}

	return(0);
}
