; Visual C++ Redistributable for Visual Studio 2013 (12.0.21005)

#define vcredist32_exe          "vcredist_x86.exe"
#define vcredist32_title        "Microsoft Visual C++ 2013 Redistributable (x86)"
#define vcredist32_url          "http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/" + vcredist32_exe
#define vcredist32_Minimum      "{13A4EE12-23EA-3371-91EE-EFB36DDFFF3E}"
#define vcredist32_Additional   "{F8CFEB22-A2E7-3971-9EDA-4B11EDEFC185}"

#define vcredist64_exe          "vcredist_x64.exe"
#define vcredist64_title        "Microsoft Visual C++ 2013 Redistributable (x64)"
#define vcredist64_url          "http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/" + vcredist64_exe
#define vcredist64_Minimum      "{A749D8E6-B613-3BE3-8F5F-045C84EBA29B}"
#define vcredist64_Additional   "{929FBD26-9020-399B-9A7A-751D61F0B942}"

#include "idp.iss"
#include "dep.iss"

[Code]
Function InstallVCRedist(): Boolean;
Begin
  Result := False;

  if ( not MsiProduct( '{#vcredist32_Minimum}' ) ) or ( not MsiProduct( '{#vcredist32_Additional}' ) ) then begin
    AddProduct( '{#vcredist32_exe}', '/quiet /norestart', '{#vcredist32_title}', '{#vcredist32_url}', false, false );
    Result := True;
  end;

  if IsWin64 and ( ( not MsiProduct( '{#vcredist64_Minimum}' ) ) or ( not MsiProduct( '{#vcredist64_Additional}' ) ) ) then begin
    AddProduct( '{#vcredist64_exe}', '/quiet /norestart', '{#vcredist64_title}', '{#vcredist64_url}', false, false );
    Result := True;
  end;

End;
