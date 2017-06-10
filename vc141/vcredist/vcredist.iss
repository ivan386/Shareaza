#define vcredist_name			      "Microsoft Visual C++ 2017 Redistributable 14.10.25017"
#define vcredist_file			      "mfc140u.dll"

#define vcredist32_exe          "vc_redist.x86.exe"
#define vcredist32_title        vcredist_name + " (x86)"
#define vcredist32_url          "https://download.microsoft.com/download/2/2/8/228b5ade-2bcf-41ca-b934-e161de218cfc/vc_redist.x86.exe"
#define vcredist32_productcode  "{e89464af-e7f0-4ed3-bf43-f1a5986113db}"

#define vcredist64_exe          "vc_redist.x64.exe"
#define vcredist64_title        vcredist_name + " (x64)"
#define vcredist64_url          "https://download.microsoft.com/download/9/7/b/97bfa482-9ae0-4d20-8f4a-5920e26b6d7c/vc_redist.x64.exe"
#define vcredist64_productcode  "{d3ea57b6-46d6-4824-a20f-6b8213001903}"

#include "idp.iss"
#include "dep.iss"

[Code]
Function InstallVCRedist(): Boolean;
Begin
  Result := False;

  if not MsiProduct( '{#vcredist32_productcode}' ) and not FileExists( ExpandConstant( '{syswow64}\{#vcredist_file}' ) ) then begin
    AddProduct( '{#vcredist32_exe}', '/quiet /norestart', '{#vcredist32_title}', '{#vcredist32_url}', false, false );
    Result := True;
  end;

  if IsWin64 and not MsiProduct( '{#vcredist64_productcode}' ) and not FileExists( ExpandConstant( '{sys}\{#vcredist_file}' ) ) then begin
    AddProduct( '{#vcredist64_exe}', '/quiet /norestart', '{#vcredist64_title}', '{#vcredist64_url}', false, false );
    Result := True;
  end;

End;
