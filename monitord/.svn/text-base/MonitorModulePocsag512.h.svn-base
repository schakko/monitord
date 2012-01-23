/* MyMonModulesPocsag512.h
 *
 *      This file is part of MyMonitor
 *
 *		Copyright (C) 1996
 *          Thomas Sailer (sailer@ife.ee.ethz.ch, hb9jnx@hb9w.che.eu)
 *
 *      Copyright (C) 1998-2002
 *          Markus Grohmann (markus_grohmann@gmx.de)
 *
 *      Copyright (c) 2002
 *          Stephan Effertz (info@stephan-effertz.de)
 *
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
 */

/* ---------------------------------------------------------------------- */
// MyMonModulesPocsag512.h: Schnittstelle f�r die Klasse CMyMonModulesPocsag512.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYMONMODULESPOCSAG512_H__EC75A96B_A1A1_4806_948A_A60F8A0503D6__INCLUDED_)
#define AFX_MYMONMODULESPOCSAG512_H__EC75A96B_A1A1_4806_948A_A60F8A0503D6__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "MonitorModulePocsag.h"

class MonitorModulePocsag512 : public MonitorModulePocsag
{
public:
	virtual void demod_se(float *buffer, int length);
	virtual void demod_mg(float *buffer, int length);
	MonitorModulePocsag512(int SampleRate=22050, bool crcheck=true, bool errorcorrection=true, int minpreambel=300, int maxerrors=10, int algorithmus=1);
	MonitorModulePocsag512(int SampleRate, XMLNode *pConfig);
	virtual ~MonitorModulePocsag512();

protected:
	/*******************************************************
	Decoder Variables & Coef. calculation
	*******************************************************/
	float lp1_c[3],lp2_c[3],bp0_c[3],bp1_c[3];
	float lp1_b[4],lp2_b[4],bp0_b[4],bp1_b[4];
	
	void set_filters(float f0, float f1, float dr);
	void gen_coef(int tipo, float f0, float Q, float *pcoef);
	float biq_hp(float x,float *pcoef,float *buf);
	float biq_bp(float x,float *pcoef,float *buf);
	float biq_lp(float x,float *pcoef,float *buf);

	
	
};

#endif // !defined(AFX_MYMONMODULESPOCSAG512_H__EC75A96B_A1A1_4806_948A_A60F8A0503D6__INCLUDED_)
