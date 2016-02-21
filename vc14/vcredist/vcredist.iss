; Visual C++ Redistributable for Visual Studio 2015 Update 1 (14.0.23506)
; https://www.microsoft.com/en-us/download/confirmation.aspx?id=49984
#if PlatformName == "Win32"
  #define vcredist_exe          "VC_redist.x86.exe"
  #define vcredist_title        "Microsoft Visual C++ 2015 Update 1 Redistributable (x86)"
  #define vcredist_url          "https://download.microsoft.com/download/C/E/5/CE514EAE-78A8-4381-86E8-29108D78DBD4/" + vcredist_exe
  #define vcredist_productcode  "{65AD78AD-D23D-3A1E-9305-3AE65CD522C2}"
#else
  #define vcredist_exe          "VC_redist.x64.exe"
  #define vcredist_title        "Microsoft Visual C++ 2015 Update 1 Redistributable (x64)"
  #define vcredist_url          "https://download.microsoft.com/download/C/E/5/CE514EAE-78A8-4381-86E8-29108D78DBD4/" + vcredist_exe
  #define vcredist_productcode  "{A1C31BA5-5438-3A07-9EEE-A5FB2D0FDE36}"
#endif
