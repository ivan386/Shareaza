-----------------------
Shareaza Skin Installer
-----------------------

This installer was developed by Robert Rainwater.
Version 1.0.9 and later are extensions by Jann Röder <jann_roeder@arcor.de>
1.0.12 version improved by Rolandas Rudomanskis.

History
=======
1.0.0:
 - Initial Release

1.0.1:
 - Use a global header file for local includes
 - Return error if unable to create skin directory
 - Return error if unable to create directories from skin file
 - File handles are closed on error extracting skin files
 - Added /install,/installsilent,/uninstall,/uninstallsilent switches
 - FindWindow fallback if the app is not running

1.0.2
 - Fix skin failing when creating sub directories

1.0.3:
 - ZLib compiled statically into the exe
 - Option to apply even if the app is closed.

1.0.4:
 - Exit options check whether or not app is running
 - Options dialog is opened if the users chooses yes

1.0.5:
 - Updated skin icon (thanks to Kiwi)

1.0.6:
 - Application is now placed in the Skins directory of Shareaza

1.0.7:
 - Initial Public Release
 - Typo in MessageBox
 - Removed creation of skin directory code and options
 - Updated MessageBox titles to say Shareaza
 - Error messages use exclamation icons
 - More messagebox string changes
 - Output directory is now set to skin.exe directory
 - Added XP Style support
 - Added right click + "Install Shareaza Skin" context menu item
 - Maximized state of Shareaza is preserved when opening options
 - Added version info to exe (can view version info from windows)
 - Fixed unzip logic to prevent unzipping invalid zips
 - Invalid skins are not installed
 - Use zlib.dll (smaller exe size)
 - Do not allow skins to install outside of skins directory
 - Better check for xml file inside skin
 - Don't extract skin.info from skin file (deprecated)
 - Uninstall now removes ALL associated keys from registry
 - Language skins now open new Language Selection dialog

1.0.8
 - [.1] Added user interface
 - [.1] Fixed memory leaks
 - [.1] Updated unzip algorithm
 - [.1] Shareaza window is brought to the front on configure or skin apply
 - [.2] Configure Language/Set as Default skin button closes installer
 - [.2] Close button was disabled on startup
 - [.3] Wrote custom parser for xml file
 - [.3] Diplay manifest info in dialog
 - [.4] Better parsing of xml file
 - [.4] Use Tahoma font
 - [.5] Decode html entities in manifest file
 - [.6] Don't allow '.' in the beginning of a filename
 - [.6] Added Set Skin as Default option
 - [.7] Fixed Set as Skin option
 - [.8] Window Title shows skin name/version
 - [.8] Re-added support for /installsilent (same as /install now)
 - [.9] Support /uninstallsilent

1.0.9
 - Read install location from registry with support for the "Path hack"
 - Updated graphics

1.0.10
 - Changed the way how files are extracted. The installer gets the skin name from the XML file and creates a directory with that name and puts all the files inside this directory. For language skins, all files are placed in the \Languages folder.
 - Updated Icon 

1.0.11
 - Fixed Bug with "Set Skin as Default". It now selects the skin and opens the skin settings window.
 - Renamed "Set Skin as Default" button to "Select Skin"

1.0.12
 - Linked against zlib 1.2.3 (winapi)
 - UTF-8 support implemented
 - "Updated by" authors are displayed
 - GUI fixed (white background)
 
ToDo
====

- Maybe disable all other skins when clicking on Select Skin

Copyright Information
=====================
This software is released into the public domain.  You are free to 
redistribute and modify without any restrictions with the exception of
the following:

The Zlib library is Copyright (C) 1995-2002 Jean-loup Gailly and Mark Adler.
The Unzip library is Copyright (C) 1998-2003 Gilles Vollant.
