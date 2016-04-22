; Visual C++ Redistributable for Visual Studio 2015 Update 2 (14.0.23918)

#define vcredist32_exe          "VC_redist.x86.exe"
#define vcredist32_title        "Microsoft Visual C++ 2015 Redistributable (x86)"
#define vcredist32_url          "http://download.microsoft.com/download/F/3/9/F39B30EC-F8EF-4BA3-8CB4-E301FCF0E0AA/" + vcredist32_exe
#define vcredist32_Minimum      "{B5FC62F5-A367-37A5-9FD2-A6E137C0096F}"
#define vcredist32_Additional   "{BD9CFD69-EB91-354E-9C98-D439E6091932}"

#define vcredist64_exe          "VC_redist.x64.exe"
#define vcredist64_title        "Microsoft Visual C++ 2015 Redistributable (x64)"
#define vcredist64_url          "http://download.microsoft.com/download/4/C/B/4CBD5757-0DD4-43A7-BAC0-2A492CEDBACB/" + vcredist64_exe
#define vcredist64_Minimum      "{7B50D081-E670-3B43-A460-0E2CDB5CE984}"
#define vcredist64_Additional   "{DFFEB619-5455-3697-B145-243D936DB95B}"

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
