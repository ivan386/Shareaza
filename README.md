# Compiling Shareaza

## Microsoft Visual Studio

### Install Visual Studio

This release of Shareaza _must_ be compiled by VS2022. http://www.microsoft.com/visualstudio/en-us/products
(Previous releases was dropped)

#### Install Visual Studio 2022

##### Microsoft Visual Studio 2022 Full Edition

This needs to be a Full Edition preferably Team Edition and not the Express Release.

If you want to compile to 64-bit, make sure to enable it during the install process under Add or Remove Features (`Language Tools -> Visual C++ -> X64 Compilers and Tools`).

##### Windows SDK for Windows Server 2022

If you are installing a Full Edition of Visual Studio you should by default get Windows SDK installed.

##### Microsoft DirectX SDK

Must be August 2007 or earlier version, newer versions including current March 2008, are missing a needed file � dxtrans.h
You can download August 2007 from
http://www.microsoft.com/downloads/details.aspx?FamilyID=529f03be-1339-48c4-bd5a-8506e5acf571&displaylang=en
or by using this direct link
http://download.microsoft.com/download/3/3/f/33f1af6e-c61b-4f14-a0de-3e9096ed4b3a/dxsdk_aug2007.exe

### gzip

Download & copy gzip.exe into your windows path eg. `C:\WINDOWS\system32`

I am using version 1.2.4 which you can download from http://www.gzip.org/.

On some systems, possibly related to Windows 7 or Windows 64-bit, placing gzip in system32 will not be recognized. In that case, just put it somewhere else in Visual Studio's executable path, such as under 'Microsoft Visual Studio 9.0/Common7/Tools'.

### Download and install Unicode Inno Setup QuickStart Pack self-installing package

from http://www.jrsoftware.org/isdl.php.

Make sure you install the "Unicode Inno Setup QuickStart Pack" not the standard setup/install.

The "Unicode Inno Setup QuickStart Pack" incudes additional componets that are required that the standard setup does not include.

Also make sure you install Inno on your C drive. I generally install my programs on another drive but found when setting up VS2005 last time that the visual studio project file used to build shareaza used a script reference to access Inno. This script assumed that Inno was installed on C drive, so not having Inno installed on C drive caused an error in compiling. To avoid this potential problem I installed Inno on my C drive this time and had no problems.

### Download and install TortoiseSVN

from http://tortoisesvn.net.

Use [TortoiseSVN](http://tortoisesvn.net/) "Checkout" with URL repository
https://shareaza.svn.sourceforge.net/svnroot/shareaza/trunk/
to download current code.

[TortoiseSVN](http://tortoisesvn.net) will notify you of updates to the program as they are made available.

Please note, the [TortoiseSVN](http://tortoisesvn.net/) required for compilation, Shareaza uses its COM-interface to retrieve source tree revision.

### Compile

Once all the above is in place you can compile.

Open in VS2022 the following source file `\vc143\Shareaza.sln`
This will open the "Solution"
Go into `Build -> Configuration Manager` and set Shareaza build config
eg `Configuration = Debug / Platform = Win32` or `Configuration = Debug / Platform = x64`.
Then select "Rebuild Solution" to start to compile.
At the end of a successful compile you will find a newly compiled installer in \setup\builds folder.

Note: If you try compiling an x64 build on a 32bit machine you will get the following repeated error "Project : error PRJ0019: A tool returned an error code from "Performing registration". This error occurs because you can not register 64bit DLLs on a 32bit machine! It is not advisable to compile a 64-bit build on 32-bit windows, as you cannot test what you are compiling.

Enjoy ;)
