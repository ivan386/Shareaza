//
// StdAfx.cpp
//
// Copyright (c) Shareaza Development Team, 2007-2014.
// This file is part of Shareaza Torrent Wizard (shareaza.sourceforge.net).
//
// Shareaza Torrent Wizard is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Torrent Wizard is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#include "StdAfx.h"

// Disable exceptions if the memory allocation fails

class NoThrowNew
{
public:
	NoThrowNew() throw()
	{
		std::set_new_handler( &NoThrowNew::OutOfMemoryHandlerStd );
		_set_new_handler( &NoThrowNew::OutOfMemoryHandler );
		AfxSetNewHandler( &NoThrowNew::OutOfMemoryHandlerAfx );
	}

private:
	static void __cdecl OutOfMemoryHandlerStd() throw()
	{
	}

	static int __cdecl OutOfMemoryHandler(size_t /* nSize */) throw()
	{
		return 0;
	}

	static int __cdecl OutOfMemoryHandlerAfx(size_t /* nSize */) throw()
	{
		return 0;
	}
};

NoThrowNew initNoThrowNew;
