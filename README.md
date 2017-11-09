# Compiling Shareaza
## Microsoft Visual Studio

### Install Visual Studio
 
 Shareaza  can be compiled by VS2008 or by VS2010. http://www.microsoft.com/visualstudio/en-us/products

 (VS2003 and VS2005 was dropped)

#### Install Visual Studio 2008
 
##### Microsoft Visual Studio 2008 Full Edition

 This needs to be a Full Edition preferably Team Edition and not the Express Release.

 If you want to compile to 64-bit, make sure to enable it during the install process under Add or Remove Features (```Language Tools -> Visual C++ -> X64 Compilers and Tools```).  

##### Microsoft Visual C++ 2008 Feature Pack
 This fixes a lot of VS2008 errors so it is a must have.
 Install it **before** "Microsoft Visual Studio 2008 Service Pack 1".
 You can download the pack from
 http://www.microsoft.com/downloads/details.aspx?FamilyId=D466226B-8DAB-445F-A7B4-448B326C48E7&displaylang=en
 
 
##### Windows SDK for Windows Server 2008 (v6.01) and .NET Framework 3.5
 If you are installing a Full Edition of Visual Studio you should by default get Windows SDK installed.
 In my case Microsoft Windows v6.0A SDK was installed by default with Microsoft.Visual.Studio.Team.System.2008.Team.Suite.
 If v6.01 was not installed with your Visual Studio 2008 installation then you will need to download and install SDK v6.1 from http://www.microsoft.com/downloads/details.aspx?FamilyID=e6e1c3df-a74f-4207-8586-711ebe331cdc&displaylang=en Please note that a full download of all installation options of SDK v6.1 is 1.2GB. For compiling purposes “Documentation” and “Samples” are not required. In installation options deselect all “Documentation” and “Samples” to reduce the instillation download size to 83.3MB.
 
##### Microsoft DirectX SDK
Must be August 2007 or earlier version, newer versions including current March 2008, are missing a needed file — dxtrans.h 
You can download August 2007 from
http://www.microsoft.com/downloads/details.aspx?FamilyID=529f03be-1339-48c4-bd5a-8506e5acf571&displaylang=en
or by using this direct link
http://download.microsoft.com/download/3/3/f/33f1af6e-c61b-4f14-a0de-3e9096ed4b3a/dxsdk_aug2007.exe
 

##### Microsoft Visual Studio 2008 Service Pack 1
 This fixes a lot of VS2008 errors and adds TR1 so it is a must have. Please note you must (re)install SP1 **after** SDK 6.01 to avoid CRT bugs.
 You can download the pack from http://www.microsoft.com/downloads/details.aspx?FamilyId=FBEE1648-7106-44A7-9649-6D9F6D58056E&displaylang=en

#### Install Visual Studio 2010

 **Microsoft Visual Studio 2010**
 Any version fit except "Test Professional".

#### Microsoft Visual Studio Community 2015

Download and install **Microsoft Visual Studio Community 2015** - https://www.visualstudio.com/

### gzip

 Download & copy gzip.exe into your windows path eg. ```C:\WINDOWS\system32```

 I am using version 1.2.4 which you can download from http://www.gzip.org/.

 On some systems, possibly related to Windows 7 or Windows 64-bit, placing gzip in system32 will not be recognized.  In that case, just put it somewhere else in Visual Studio's executable path, such as under 'Microsoft Visual Studio 9.0/Common7/Tools'.

### 7-Zip
Download and install 7-Zip - http://www.7-zip.org/

### boost

To setup the [boost](http://www.boost.org/) library download it from
 https://sourceforge.net/projects/boost/files/boost/

 You will only need to extract the zipped files and **NOT** compile the library.
 Uncompress ```boost_1_54_0.zip``` to a drive/folder on your PC.
 ie ```C:\Program Files\boost_1_54_0```

### Add VS2008 Include Dir Listings


 You need to make sure that
 Visual Studio is pointing to and including boost & DirectX. 

 Go to ```Tools -> Options -> Projects and Solutions -> VC++ Directories -> Win 32 - Include Files```

 Make sure the first two entries in this list point to boost & DirectX. If the enties are not there add them. 

 ie ```C:\Program Files\boost_1_54_0```  
 ie ```C:\Program Files\Microsoft DirectX SDK (August 2007)\Include```

 Do the same for Tools -> Options -> Projects and Solutions -> VC++ Directories -> x64 - Include Files

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

Open in VS2008 the following source file ```\vc9\Shareaza.sln```
This will open the "Solution"
Go into ```Build -> Configuration Manager``` and set Shareaza build config
eg ```Configuration = Debug / Platform = Win32``` or ```Configuration = Debug / Platform = x64```.
Then select "Rebuild Solution" to start to compile. 
At the end of a successful compile you will find a newly compiled installer in \setup\builds folder.


Note: If you try compiling an x64 build on a 32bit machine you will get the following repeated error "Project : error PRJ0019: A tool returned an error code from "Performing registration". This error occurs because you can not register 64bit DLLs on a 32bit machine! It is not advisable to compile a 64-bit build on 32-bit windows, as you cannot test what you are compiling.


As a side note, I found setting up with VS2008 was less problematic than VS2005.


Enjoy ;)