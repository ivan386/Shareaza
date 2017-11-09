//
// Hresults.cs
//
// Copyright (c) Shareaza Development Team, 2008.
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

using System;

namespace Shareaza
{
	internal sealed class Hresults
	{
		public const int E_FAIL = unchecked((int)0x80004005);
        public const int E_INVALIDARG = unchecked((int)0x80070057);
		public const int E_UNEXPECTED = unchecked((int)0x8000FFFF);
		public const int E_NOINTERFACE = unchecked((int)0x80004002);
		public const int E_OUTOFMEMORY = unchecked((int)0x8007000E);
		public const int E_NOTIMPL = unchecked((int)0x80004001);
		public const int S_OK = 0;
		public const int S_FALSE = 1;
	}
}
