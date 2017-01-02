Compiling Shareaza

	svn://svn.code.sf.net/p/shareaza/code/trunk

Preparation

	1. Download and install Microsoft Visual Studio Community 2015 - https://www.visualstudio.com/
	2. Download and unpack gzip.exe - http://www.gzip.org/ (copy it to Windows directory)
	3. Download and unpack Boost - http://www.boost.org/ (no compilation needed, just add Boost folder to the Include directories in the Project Properties Manager)
	4. Download and install InnoSetup - http://www.jrsoftware.org/isdl.php (unicode version)
	5. Download and install TortoiseSVN - http://tortoisesvn.net/ (Shareaza uses its COM-interface to retrieve source tree revision)

Manual compilation

	Open vc14\Shareaza.sln and compile
	
	Note: All VS2015 solutions placed in the same folder i.e. in vc14

Automatic compilation

	Run vc14\DailyBuild.cmd

	Note: Assuming Shareaza sources checked out to at least second level nested folder i.e. like C:\Projects\Shareaza\trunk\
