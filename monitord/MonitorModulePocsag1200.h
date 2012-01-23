/* MyMonModulesPocsag1200.h
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

/* ---------------------------------------------------------------------- */// MyMonModulePocsag1200.h: Schnittstelle f�r die Klasse MonitorModulePocsag1200.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MYMONMODULEPOCSAG1200_H__572ACD5B_3877_41FC_9C6B_1073970E7100__INCLUDED_)
#define AFX_MYMONMODULEPOCSAG1200_H__572ACD5B_3877_41FC_9C6B_1073970E7100__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "MonitorModulePocsag.h"

class MonitorModulePocsag1200 : public MonitorModulePocsag
{
public:
	virtual void demod_se(float *buffer, int length);
	virtual void demod_mg(float *buffer, int length);
	MonitorModulePocsag1200(int sampleRate=22050, bool crcheck=true, bool errorcorrection=true, int minpreambel=300, int maxerrors=10, int algorithmus=1) ;
	MonitorModulePocsag1200(XMLNode *pConfig);
	virtual ~MonitorModulePocsag1200();
	void demod(float *buffer, int length) ;
protected:

};

#endif // !defined(AFX_MYMONMODULEPOCSAG1200_H__572ACD5B_3877_41FC_9C6B_1073970E7100__INCLUDED_)
