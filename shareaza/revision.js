//
// revision.js
//
// Copyright (c) Shareaza Development Team, 2009.
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

// This scipt compares current directory revision and revision saved in
// revision.h file using TortoiseSVN COM-interface.

// #TODO: Fix the revision 

var fso = WScript.CreateObject( "Scripting.FileSystemObject" );
var fpath = fso.GetAbsolutePathName( "." )
var fname = fso.GetAbsolutePathName( "revision.h" );

var date = Date.now;
var revision = "TODO:FIX"

try
{
	var tsw = fso.OpenTextFile( fname, 2, true );
	tsw.WriteLine( "// rev." + revision );
	tsw.WriteLine( "" );
	tsw.WriteLine( "#pragma once" );
	tsw.WriteLine( "" );
	tsw.WriteLine( "#define __REVISION__\t\t\"" + revision + "\"" );
	tsw.WriteLine( "#define __REVISION_DATE__\t\"" + date + "\"" );
	tsw.Close();
}
catch(e)
{
	WScript.Echo( "Update failed: \"" + fname + "\"" );
	WScript.Quit( 1 );
}

WScript.Quit( 0 );
