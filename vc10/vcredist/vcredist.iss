; Installing Visual Studio 2010 SP1 C++ CRT Libraries
#if PlatformName == "Win32"
  #define vcredist_exe          "vcredist_x86.exe"
  #define vcredist_title        "Microsoft Visual C++ 2010 SP1 Redistributable (x86)"
  #define vcredist_url          "http://download.microsoft.com/download/C/6/D/C6D0FD4E-9E53-4897-9B91-836EBA2AACD3/" + vcredist_exe
  #define vcredist_productcode  "{F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}"
#else
  #define vcredist_exe          "vcredist_x64.exe"
  #define vcredist_title        "Microsoft Visual C++ 2010 SP1 Redistributable (x64)"
  #define vcredist_url          "http://download.microsoft.com/download/A/8/0/A80747C3-41BD-45DF-B505-E9710D2744E0/" + vcredist_exe
  #define vcredist_productcode  "{1D8E6291-B0D5-35EC-8441-6616F567A0F7}"
#endif
