; Installing Visual Studio 2013 C++ CRT Libraries
; http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x86.exe
; http://download.microsoft.com/download/2/E/6/2E61CFA4-993B-4DD4-91DA-3737CD5CD6E3/vcredist_x64.exe

[Files]
#if PlatformName == "Win32"
  Source: "vc12\vcredist\vcredist_x86.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; AfterInstall: ExecTemp( 'vcredist_x86.exe', '/passive' );
#else
  Source: "vc12\vcredist\vcredist_x64.exe"; DestDir: "{tmp}"; Flags: deleteafterinstall; AfterInstall: ExecTemp( 'vcredist_x64.exe', '/passive' );
#endif

[Code]
procedure ExecTemp(File, Params : String);
var
	nCode: Integer;
begin
	Exec( ExpandConstant( '{tmp}' ) + '\' + File, Params, ExpandConstant( '{tmp}' ), SW_SHOW, ewWaitUntilTerminated, nCode );
end;
