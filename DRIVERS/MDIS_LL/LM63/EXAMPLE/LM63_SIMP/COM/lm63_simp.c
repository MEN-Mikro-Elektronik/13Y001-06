/****************************************************************************
 ************                                                    ************
 ************                   LM63_SIMP                         ************
 ************                                                    ************
 ****************************************************************************/
/*!
 *         \file lm63_simp.c
 *       \author dieter.pfeuffer@men.de
 *
 *        \brief Simple example program for the LM63 driver.
 *
 *               Reads from all channels.
 *
 *     Required: libraries: mdis_api, usr_oss
 *     \switches (none)
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


#include <stdio.h>
#include <string.h>
#include <MEN/men_typs.h>
#include <MEN/mdis_api.h>
#include <MEN/usr_oss.h>
#include <MEN/lm63_drv.h>

static const char IdentString[]=MENT_XSTR(MAK_REVISION);

/*--------------------------------------+
|   PROTOTYPES                          |
+--------------------------------------*/
static void PrintError(char *info);

/********************************* main ************************************/
/** Program main function
 *
 *  \param argc       \IN  argument counter
 *  \param argv       \IN  argument vector
 *
 *  \return	          success (0) or error (1)
 */
int main(int argc, char *argv[])
{
	MDIS_PATH path;
	char	  *device;
	int32	  ch, nbrOfCh, value;

	if (argc < 2 || strcmp(argv[1],"-?")==0) {
		printf("Syntax: lm63_simp <device>\n");
		printf("Function: LM63 example for reading all channels\n");
		printf("Options:\n");
		printf("    device       device name\n");
		printf("%s\n", IdentString );
		return(1);
	}

	device = argv[1];

	/*--------------------+
    |  open path          |
    +--------------------*/
	if ((path = M_open(device)) < 0) {
		PrintError("open");
		return(1);
	}

	/* get number of channels */
	if ((M_getstat(path, M_LL_CH_NUMBER, &nbrOfCh)) < 0) {
		PrintError("setstat M_MK_IO_MODE");
		goto abort;
	}

	/*--------------------+
    |  config             |
    +--------------------*/
	/* repeat until keypress */
	do {

		for( ch=0; ch<nbrOfCh; ch++ ){

			/* set channel number */
			if ((M_setstat(path, M_MK_CH_CURRENT, ch)) < 0) {
				PrintError("setstat M_MK_CH_CURRENT");
				goto abort;
			}

			/* read value from current channel (then increment channel)  */
			if ((M_read(path, &value ))  < 0 ){
				PrintError("read");
				goto abort;
			}

			switch( ch ){
			case LM63_CH_TEMP:
				printf("LM63 die temperature : %d degree celcius\n", (int)value);
				break;
			case LM63_CH_RMTTEMP:
				printf("Remote temperature   : %d degree celcius\n", (int)value);
				break;
			case LM63_CH_FANSPEED:
				printf("Fanspeed             : %drpm\n", (int)value);
				break;
			}
		}

		UOS_Delay(500);

	} while( UOS_KeyPressed() == -1 );

	/*--------------------+
    |  cleanup            |
    +--------------------*/
	abort:
	if (M_close(path) < 0)
		PrintError("close");

	return(0);
}

/********************************* PrintError ******************************/
/** Print MDIS error message
 *
 *  \param info       \IN  info string
*/
static void PrintError(char *info)
{
	printf("*** can't %s: %s\n", info, M_errstring(UOS_ErrnoGet()));
}

