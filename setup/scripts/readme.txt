You can download Inno Setup 5 here:
http://files.jrsoftware.org/is/5/
You'll also need to install the PreProcessor:
http://files.jrsoftware.org/ispack/

Minimum version needed is 5.1.7 Beta.
Do not deploy IS installers of an old version ontop of newer versions!
Have the user do a clean install if you cannot avoid this scenario.

To compile a debug build without any files uncomment "#define debug" in main.iss.

Compile procedure for Shareaza:
1)	Place Shareaza.exe in \setup\builds\
2)	Place skin.exe in \setup\builds\
3)	Place Shareaza.pdb in \setup\builds\ only for the debug builds of Shareaza
4)	Compile repair.iss
5)	Compile main.iss

Compile procedure for TorrentWizard:
1)	Place TorrentWizard.exe in \setup\builds\
2)	Compile torrent.iss 

The output file should be in \setup\builds\