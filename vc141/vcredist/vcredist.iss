#define vcredist_name			      "Microsoft Visual C++ 2017 Redistributable"
#define vcredist_file			      "mfc140u.dll"

#define vcredist32_exe          "VC_redist.x86.exe"
#define vcredist32_title        vcredist_name + " (x86)"
#define vcredist32_url          "https://go.microsoft.com/fwlink/?LinkId=746571"

#define vcredist64_exe          "VC_redist.x64.exe"
#define vcredist64_title        vcredist_name + " (x64)"
#define vcredist64_url          "https://go.microsoft.com/fwlink/?LinkId=746572"

#include "idp.iss"
#include "dep.iss"

[Code]
Function InstallVCRedist(): Boolean;
Begin
  Result := False;

  if not FileExists( ExpandConstant( '{syswow64}\{#vcredist_file}' ) ) then begin
    AddProduct( '{#vcredist32_exe}', '/quiet /norestart', '{#vcredist32_title}', '{#vcredist32_url}', false, false );
    Result := True;
  end;

  if IsWin64 and not FileExists( ExpandConstant( '{sys}\{#vcredist_file}' ) ) then begin
    AddProduct( '{#vcredist64_exe}', '/quiet /norestart', '{#vcredist64_title}', '{#vcredist64_url}', false, false );
    Result := True;
  end;

End;
