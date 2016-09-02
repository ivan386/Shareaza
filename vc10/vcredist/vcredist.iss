; Visual C++ Redistributable for Visual Studio 2010 SP1 (10.0.40219)

#define vcredist32_exe          "vcredist_x86.exe"
#define vcredist32_title        "Microsoft Visual C++ 2010 Redistributable (x86)"
#define vcredist32_url          "http://download.microsoft.com/download/C/6/D/C6D0FD4E-9E53-4897-9B91-836EBA2AACD3/" + vcredist32_exe
#define vcredist32_productcode  "{F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}"

#define vcredist64_exe          "vcredist_x64.exe"
#define vcredist64_title        "Microsoft Visual C++ 2010 Redistributable (x64)"
#define vcredist64_url          "http://download.microsoft.com/download/A/8/0/A80747C3-41BD-45DF-B505-E9710D2744E0/" + vcredist64_exe
#define vcredist64_productcode  "{1D8E6291-B0D5-35EC-8441-6616F567A0F7}"

#include "idp.iss"
#include "dep.iss"

[Code]
Function InstallVCRedist(): Boolean;
Begin
  Result := False;

  if ( not MsiProduct( '{#vcredist32_productcode}' ) ) then begin
    AddProduct( '{#vcredist32_exe}', '/quiet /norestart', '{#vcredist32_title}', '{#vcredist32_url}', false, false );
    Result := True;
  end;

  if IsWin64 and ( not MsiProduct( '{#vcredist64_productcode}' ) ) then begin
    AddProduct( '{#vcredist64_exe}', '/quiet /norestart', '{#vcredist64_title}', '{#vcredist64_url}', false, false );
    Result := True;
  end;

End;
