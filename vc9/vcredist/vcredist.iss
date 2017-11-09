; Installing Visual Studio 2008 C++ CRT Libraries
; http://download.microsoft.com/download/0/5/c/05c912c6-ba17-4909-9371-cad89bd8dcef/vcredist_x86.exe
; http://download.microsoft.com/download/d/2/4/d242c3fb-da5a-4542-ad66-f9661d0a8d19/vcredist_x64.exe

[Files]
#if PlatformName == "Win32"
  Source: "vc9\vcredist\vcredist_x86.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; AfterInstall: ExecTemp( 'vcredist_x86.exe', '/passive /promptrestart' );
#else
  Source: "vc9\vcredist\vcredist_x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; AfterInstall: ExecTemp( 'vcredist_x64.exe', '/passive /promptrestart' );
#endif

[Code]
procedure ExecTemp(File, Params : String);
var
	nCode: Integer;
begin
	Exec( ExpandConstant( '{tmp}' ) + '\' + File, Params, ExpandConstant( '{tmp}' ), SW_SHOW, ewWaitUntilTerminated, nCode );
end;
