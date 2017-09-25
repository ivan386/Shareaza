Compiling Shareaza

	svn://svn.code.sf.net/p/shareaza/code/trunk
	
	Note: Assuming Shareaza sources checked out to at least second level nested folder i.e. like C:\Projects\Shareaza\trunk\

Preparation:

	1. Download and install Microsoft Visual Studio Community 2015 - https://www.visualstudio.com/
	2. Download and install 7-Zip - http://www.7-zip.org/
	3. Download and unpack Boost - http://www.boost.org/ (no compilation needed, just add Boost folder to the Include directories in the Project Properties Manager)
	4. Download and install InnoSetup - http://www.jrsoftware.org/isdl.php (unicode version)
	5. Download and install TortoiseSVN - http://tortoisesvn.net/ (Shareaza uses its COM-interface to retrieve source tree revision)

Manual compilation:

	Open and compile vc14\Shareaza.sln

Automatic compilation of daily builds:

	Run vc14\DailyBuild.cmd
	
	It produces a 3 files in the ..\Releases\Snapshots\{revision}\ folder:
	
		Shareaza_{version}_Win32_Debug_{revision}_{date}.exe
		Shareaza_Win32_Debug_{revision}_{date}_Symbols.7z
		Shareaza_{version}_Source.7z

Automatic compilation of release builds:

	Run vc14\ReleaseBuildAll.cmd
	
	It produces a 5 files in the ..\Releases\Shareaza-{version}\ folder:
	
		Shareaza_{version}_Win32.exe
		Shareaza_{version}_Win32_Symbols.7z
		Shareaza_{version}_x64.exe
		Shareaza_{version}_x64_Symbols.7z
		Shareaza_{version}_Source.7z
		
	Also it creates a copy of unpacked binary and symbols files in the next folders:

		..\Shareaza_{version}_Win32_Symbols\
		..\Shareaza_{version}_x64_Symbols\

Automatic compilation of LAN Mode builds:

	Run vc14\LanMode.cmd

	It produces a 3 files in the ..\Releases\Shareaza-{version}-LAN\ folder:
	
		Shareaza_{version}_Win32_LAN.exe
		Shareaza_{version}_Win32_LAN_Symbols.7z
		Shareaza_{version}_Source.7z
