; This sub-script lists all Shareaza settings that have to be written at installation time

[Registry]
; Set speed rates in byte/sec
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Settings"; ValueType: dword; ValueName: "RatesInBytes"; ValueData: "1"; Flags: createvalueifdoesntexist uninsdeletekey
; Turn off verbose mode in network tab
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Settings"; ValueType: dword; ValueName: "VerboseMode"; ValueData: "0"; Flags: createvalueifdoesntexist uninsdeletekey
