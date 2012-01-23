// MyMonModuleZVEI.h: Schnittstelle f�r die Klasse MonitorModuleZVEI.
/*
 *      based on: demod_zvei.c -- ZVEI signalling demodulator/decoder
 *
 *      Copyright (C) 1996
 *          Thomas Sailer (sailer@ife.ee.ethz.ch, hb9jnx@hb9w.che.eu)
 *
 *      Copyright (C) 1998-2002
 *          Markus Grohmann (markus_grohmann@gmx.de)
 *
 *      Copyright (c) 2002-2007
 *          Stephan Effertz (info@stephan-effertz.de)
 *
 *		Edited 11/2007 - demod partly rewritten, fully functional but to be tested "in the wild":
 *			Martin Diedrich (martin@mdiedrich.de)
 *
 *		(Demodulation parts taken from monitor (c) Markus Grohmann, Thomas Sailor)
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * ------------------------------------------------------------------------- */

#if !defined(AFX_MYMONMODULEZVEI_H__AE2C7DF3_BFD5_4A65_A86B_09BD277F35FD__INCLUDED_)
#define AFX_MYMONMODULEZVEI_H__AE2C7DF3_BFD5_4A65_A86B_09BD277F35FD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MonitorModules.h"

#define COSTABSIZE 0x400

class MonitorModuleZVEI : public MonitorModule
{
public:
	int find_max_index(const float *totte, int index1, int index2);
	void demod(float* buffer, int length);
	MonitorModuleZVEI(int sampleRate,XMLNode *pConfig);
	int debugmodus;
	float squelch;
	// MonitorModuleZVEI(XMLNode *pConfig);
	virtual ~MonitorModuleZVEI();

protected:
	void StoreResult(std::string Adresse,int typ, std::string text);
	void DisplayResult(std::string Adresse, int typ, std::string Text);
	int *process_block(int *found_tones);

	int BLOCKLEN;
	int PHINC(int x);

	short int pause_length;
	short int lastch;
	short int lastout;
	short int zvei_folge[5]; // array length of ZVEI-Tonfolge - change when needed longer!
	unsigned int folge_position;
	unsigned int maxlength;
	bool timeout;
	int* found_tones;
	short int tone_count;
	int detected_seven[7][7];
	short int seven_count;
	int fuzzyseven();
	bool zvei_ok();
	short int siren_count;

	char zvei_character[20];
	unsigned int zvei_freq[20] ;
	unsigned int	ph[20];
	float			energy[4],tenergy[4][40];
	int				blkcount;
	float costabf[COSTABSIZE];

};

#endif // !defined(AFX_MYMONMODULEZVEI_H__AE2C7DF3_BFD5_4A65_A86B_09BD277F35FD__INCLUDED_)
