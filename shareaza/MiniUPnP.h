//
// MiniUPnP.h
//
// Copyright (c) Shareaza Development Team, 2014.
// This file is part of SHAREAZA (shareaza.sourceforge.net)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#pragma once

#include "UPnP.h"

class CMiniUPnP : public CUPnP, public CThreadImpl
{
public:
	CMiniUPnP();
	virtual ~CMiniUPnP();

	virtual void StartDiscovery();
	virtual void StopAsyncFind();
	virtual void DeletePorts();
	virtual bool IsAsyncFindRunning();

protected:
	WORD		m_nExternalTCPPort;
	WORD		m_nExternalUDPPort;
	CStringA	m_sControlURL;
	CStringA	m_sServiceType;
	CStringA	m_sExternalAddress;

	void OnRun();
};
