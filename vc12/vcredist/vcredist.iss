; Installing Visual Studio 2013 C++ CRT Libraries
#ifndef WIN64
  #define vcredist_exe          "vcredist_x86.exe"
  #define vcredist_title        "Microsoft Visual C++ 2013 Redistributable (x86)"
  #define vcredist_url          "http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/" + vcredist_exe
  #define vcredist_productcode  "{13A4EE12-23EA-3371-91EE-EFB36DDFFF3E}"
#else
  #define vcredist_exe          "vcredist_x64.exe"
  #define vcredist_title        "Microsoft Visual C++ 2013 Redistributable (x64)"
  #define vcredist_url          "http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/" + vcredist_exe
  #define vcredist_productcode  "{A749D8E6-B613-3BE3-8F5F-045C84EBA29B}"
#endif
