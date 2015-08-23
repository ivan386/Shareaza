; Installing Visual Studio 2015 C++ CRT Libraries
#ifndef WIN64
  #define vcredist_exe          "vc_redist.x86.exe"
  #define vcredist_title        "Microsoft Visual C++ 2015 Redistributable (x86)"
  #define vcredist_url          "http://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/" + vcredist_exe
  #define vcredist_productcode  "{A2563E55-3BEC-3828-8D67-E5E8B9E8B675}"
#else
  #define vcredist_exe          "vc_redist.x64.exe"
  #define vcredist_title        "Microsoft Visual C++ 2015 Redistributable (x64)"
  #define vcredist_url          "http://download.microsoft.com/download/9/3/F/93FCF1E7-E6A4-478B-96E7-D4B285925B00/" + vcredist_exe
  #define vcredist_productcode  "{0D3E9E15-DE7A-300B-96F1-B4AF12B96488}"
#endif
