; This sub-script lists all Shareaza settings that have to be written at installation time

[Registry]
; Set speed rates in byte/sec
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Settings"; ValueType: dword; ValueName: "RatesInBytes"; ValueData: "{ini:{param:SETTINGS|},General,RatesInBytes|1}"; Flags: createvalueifdoesntexist uninsdeletekey
; Turn off verbose mode in network tab
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Settings"; ValueType: dword; ValueName: "VerboseMode"; ValueData: "{ini:{param:SETTINGS|},General,VerboseMode|0}"; Flags: createvalueifdoesntexist uninsdeletekey
; Turn on ShareazaOS skin
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Skins"; ValueType: dword; ValueName: "ShareazaOS\ShareazaOS.xml"; ValueData: "{ini:{param:SETTINGS|},Skins,ShareazaOS|1}"; Flags: createvalueifdoesntexist uninsdeletekey
; Set Downloads.SearchPeriod to 120
Root: HKCU; Subkey: "Software\Shareaza\Shareaza\Downloads"; ValueType: dword; ValueName: "SearchPeriod"; ValueData: "{ini:{param:SETTINGS|},Downloads,SearchPeriod|120000}"; Flags: deletevalue uninsdeletekey
