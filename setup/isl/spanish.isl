; *** Inno Setup version 4.2.2+ Spanish Messages ***
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.php
;
; Hecho por: Luis Carlos Colunga (Help)
; Para: Shareaza Installation
; Made by: Luis Carlos Colunga (Help
; For: Shareaza Installation
;
; $jrsoftware: issrc/Files/Default.isl,v 1.58 2004/04/07 20:17:13 jr Exp $

[LangOptions]
LanguageName=Espa<00F1>ol
LanguageID=$1034
LanguageCodePage=0
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=
;DialogFontSize=8
;WelcomeFontName=Verdana
;WelcomeFontSize=12
;TitleFontName=Arial
;TitleFontSize=29
;CopyrightFontName=Arial
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=Instalación
SetupWindowTitle=Instalación - %1
UninstallAppTitle=Desinstalar
UninstallAppFullTitle=Desinstalar %1

; *** Misc. common
InformationTitle=Información
ConfirmTitle=Confirmar
ErrorTitle=Error

; *** SetupLdr messages
SetupLdrStartupMessage=Esto va instalar %1. ¿Deseas continuar?
LdrCannotCreateTemp=No sé pudo crear el archivo temporal. Instalación Abortada
LdrCannotExecTemp=No sé pudo ejecutar el archivo de la carpeta temporal. Instalación Abortada

; *** Startup error messages
LastErrorMessage=%1.%n%nError %2: %3
SetupFileMissing=El archivo %1 no está en la carpeta del Instalador. Por favor corrige o consigue una nueva copia del programa.
SetupFileCorrupt=Los archivos del Instalador son corruptos. Por favor consigue una nueva copia del programa.
SetupFileCorruptOrWrongVer=Los archivos del Instalador son corruptos, o son incompatibles con esta versión del Instalador. Por favor corrige o consigue una nueva copia del programa.
NotOnThisPlatform=Este programa no va a correr en %1.
OnlyOnThisPlatform=Este programa debe ser corrido en %1.
WinVersionTooLowError=Este programa requiere la versión %1 %2 o más nueva.
WinVersionTooHighError=Este programa no puede ser instalado en la %1 versión %2 o más nueva.
AdminPrivilegesRequired=Debes de estar logeado como administrador para instalar este programa.
PowerUserPrivilegesRequired=Usted debe estar logeado como Administrador o Power Member al instalar este programa.
SetupAppRunningError=El Instalador ha detectado que %1 esta siendo ejecutado.%n%nPor favor cierralo, y después haz click a OK para continuar, o Cancelar para salir.
UninstallAppRunningError=La desinstalación ha detectado que %1 esta corriendo.%n%nPor favor ciérralo, y después haz click a OK para continuar, o Cancelar para salir.

; *** Misc. errors
ErrorCreatingDir=El Instalador no pudo crear la carpeta "%1"
ErrorTooManyFilesInDir=No se pudo crear la carpeta "%1" porque contiene muchos archivos

; *** Setup common messages
ExitSetupTitle=Salir del Instalador
ExitSetupMessage=La instalación no esta completa. Si lo quitas ahora, el programa no va a ser instalado.%n%nPuedes volver a correr el instalador otra vez en otro tiempo para completar la instalación.%n%n¿Salir de la instalación?
AboutSetupMenuItem=&About Setup...
AboutSetupTitle=About Setup
AboutSetupMessage=%1 version %2%n%3%n%n%1 home page:%n%4
AboutSetupNote=

; *** Buttons
ButtonBack=< &Atrás
ButtonNext=&Siguiente >
ButtonInstall=&Instalar
ButtonOK=OK
ButtonCancel=Cancelar
ButtonYes=&Si
ButtonYesToAll=Si &a todo
ButtonNo=&No
ButtonNoToAll=N&o a todo
ButtonFinish=Terminar
ButtonBrowse=&Explorar..
ButtonWizardBrowse=E&plorar...
ButtonNewFolder=&Hacer nueva carpeta

; *** "Select Language" dialog messages
SelectLanguageTitle=Seleccionar lenguaje de instalación
SelectLanguageLabel=Seleccione el lenguaje que desea usar durante la instalación:

; *** Common wizard text
ClickNext=Oprima Siguiente para continuar, o Cancelar para salir de la instalación.
BeveledLabel=
BrowseDialogTitle=Explorar Carpeta
BrowseDialogLabel=Selecciona una carpeta de la lista siguiente, después haz click a OK.
NewFolderName=Nueva Carpeta

; *** "Welcome" wizard page
WelcomeLabel1=Bienvenido al asistente de instalación de [name]
WelcomeLabel2=Este proceso instalará [name/ver] en su computadora.%n%nSe recomienda que cierre el resto de las aplicaciones antes de continuar.

; *** "Password" wizard page
WizardPassword=Contraseña
PasswordLabel1=Esta instalación está protegida con contraseña.
PasswordLabel3=PasswordLabel3=Por favor escriba su contraseña y después haga click al botón Siguiente para continuar.  Las contraseñas son sensitivas a las mayúsculas.
PasswordEditLabel=&Contraseña:
IncorrectPassword=La contraseña es incorrecta.  Intente de nuevo.

; *** "License Agreement" wizard page
WizardLicense=Acuerdo de licencia
LicenseLabel=Lea cuidadosamente la siguiente información antes de continuar.
LicenseLabel3=Lea el siguiente acuerdo de licencia.  Deberá aceptar los términos de este acuerdo antes de continuar con la instalación.
LicenseAccepted=&Acepto el acuerdo
LicenseNotAccepted=No acepto el acuer&do

; *** "Information" wizard pages
WizardInfoBefore=Información
InfoBeforeLabel=Lea la siguiente información antes de continuar.
InfoBeforeClickLabel=Cuando esté listo para continuar la Instalación, haga click a Siguiente.
WizardInfoAfter=Informacíon
InfoAfterLabel=Cuando esté listo para continuar la instalación, haga click al botón Siguiente.
InfoAfterClickLabel=Cuando esté listo para continuar la instalación, haga click al botón Siguiente.

; *** "User Information" wizard page
WizardUserInfo=Información del usuario
UserInfoDesc=Favor de teclear su información.
UserInfoName=Nombre del &usuario:
UserInfoOrg=&Organización:
UserInfoSerial=Número de &serie:
UserInfoNameRequired=Debe dar un nombre.

; *** "Select Destination Location" wizard page
WizardSelectDir= *** Página de "Selección de carpeta destino" del asistente
SelectDirDesc=¿Donde desea instalar [name]?
SelectDirLabel3=El Instalador va a instalar [name] en la siguiente carpeta
SelectDirBrowseLabel=Para continuar, haga click a Siguiente. Si deseas instalar el programa en otra carpeta, haga click a Explorar.
DiskSpaceMBLabel=Por lo menos [mb] MB de espacio libre en su disco duro es requerido.
ToUNCPathname=El Instalador no puede instalar a un destino UNC. Si estas tratando de instalar a una red, debes de mapear la letra de la unidad de la red.
InvalidPath=Debes dar un destino completo con una letra de unidad; por ejemplo:%n%nC:\APP%n%no un destino UNC como:%n%n\\server\share
InvalidDrive=La unidad o un UNC que seleccionaste no existe o no es accesible. Por favor selecciona otro.
DiskSpaceWarningTitle=No hay suficiente espacio en el disco duro
DiskSpaceWarning=El Instalador requiere por lo menos %1 KB de espacio libre para instalar, pero la unidad seleccionada solo contiene %2 KB disponible.%n%nDeseas continuar?
DirNameTooLong=El nombre de la carpeta o el destino es muy largo.
InvalidDirName=El nombre de la carpeta no es valido.
BadDirName32=Los nombres de la carpeta no pueden incluir alguno de estos caracteres:%n%n%1
DirExistsTitle=La Carpeta Existe
DirExists=La Carpeta:%n%n%1%n%nya existe. ¿Desearía instalarlo en es carpeta de todas maneras?
DirDoesntExistTitle=Folder Does Not Exist
DirDoesntExist=The folder:%n%n%1%n%ndoes not exist. ¿Desearías que la carpeta sea creada?

; *** "Select Components" wizard page
WizardSelectComponents=Seleccionar Componentes
SelectComponentsDesc=Cuales componentes deseas que sean instalados?
SelectComponentsLabel2=Selecciona los componentes que quieres instalar; Quita los componentes que no deseas que sean instalados. Haga click a Siguiente para continuar.
FullInstallation=Instalación Completa
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Instalación Compacta
CustomInstallation=Instalación Personalizada
NoUninstallWarningTitle=Los componentes existen
NoUninstallWarning=El Instalador ha detectado que los siguientes componentes existen en tu computadora:%n%n%1%n%nDeseleccionando estos componentes no los va a desinstalar.%n%n¿Deseas continuar de todas maneras?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel=La seleccion que tienes requiere al menos [mb] MB de espacio de disco duro.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Seleccionar Tareas Adicionales
SelectTasksDesc=¿Cuales tareas adicionales deseas que sean ejecutadas?
SelectTasksLabel2=Selecciona las tareas adicionales que deseas que la instalación ejecute [name], después haz click a Siguiente.

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Selecciona el Menu Inicio
SelectStartMenuFolderDesc=¿Donde debe la instalación poner los accesos directos del programa?
SelectStartMenuFolderLabel3=El Instalador va a crear los accesos directs del programa en el Menu Inicio.
SelectStartMenuFolderBrowseLabel=Para continuar, haz click a Siguiente. Si deseas seleccionar una carpeta diferente , haga click a Explorar.
NoIconsCheck=&No crear iconos
MustEnterGroupName=Debes dar un nombre a la carpeta.
GroupNameTooLong=El nombre de la carpeta o el destino es muy largo.
InvalidGroupName=El nombre de la carpeta es invalido.
BadGroupName=El nombre de la carpeta no debe incluir los siguientes caracteres:%n%n%1
NoProgramGroupCheck2=&No crear la carpeta en el Menu Inicio

; *** "Ready to Install" wizard page
WizardReady=Listo para instalar
ReadyLabel1=Instalador esta listo para instalar [name] en tu computadora.
ReadyLabel2a=Haga click a Instalar para continuar con la instalación, o haga click a Atrás si quieres checar alguna configuración o cambiarla.
ReadyLabel2b=Haga click a  Instalar para continuar con la instalación.
ReadyMemoUserInfo=Información de Usuario:
ReadyMemoDir=Locación de destino:
ReadyMemoType=Tipo de instalación:
ReadyMemoComponents=Components seleccionados:
ReadyMemoGroup=Carpeta Menu Inicio:
ReadyMemoTasks=Tareas Adicionales:

; *** "Preparing to Install" wizard page
WizardPreparing=Preparando para Instalar
PreparingDesc=El Instalador esta preparando para instalar [name] en tu computadora.
PreviousInstallNotCompleted=La instalación/Desinstalación del siguiente programa no ha sido completado. Vas a necesitar reiniciar tu computadora para completar con la Instalación.%n%nDespués de reiniciar tu computadora, ejecuta la instalación para completar la instalación de [name].
CannotContinue=La instalación. Por favor haz click a Cancelar.

; *** "Installing" wizard page
WizardInstalling=Instalando
InstallingLabel=Por favor espere mientras se instala [name] en tu computadora.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Completando Instalación de [name] 
FinishedLabelNoIcons=El Instalador ha terminado de instalar [name] en tu computadora.
FinishedLabel=El Instalador ha terminado de instalar [name] en tu computadora. La aplicación puede ser lanzada seleccionando los iconos instalados.
ClickFinish=Haz click aquí para cerrar el instalador.
FinishedRestartLabel=Para completar la instalación de [name], El Instalador debe reiniciar tu computadora. ¿Desearías reiniciar ahora?
FinishedRestartMessage=Para completar la instalación de [name],  El Instalador debe reiniciar tu computadora.%n%n¿Desearías reiniciar ahora?
ShowReadmeCheck=Si, Desearía leer el archivo README
YesRadio=&Si, reiniciar mi computadora ahora
NoRadio=&No, Voy a reiniciar mi computadora después
; used for example as 'Run MyProg.exe'
RunEntryExec=Correr %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Ver %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=El Instalador necesita el siguiente disco
SelectDiskLabel2=Por favor inserta el disco %1 y haz click a OK.%n%nSi los archivos pueden ser encontrados en una carpeta otra que la que se muestra abajo, da el destino correcto o haz click a Explorar.
PathLabel=&Path:
FileNotInDir2=El archivo "%1" no pudo ser localizado en "%2". Por favor inserta el disco correcto o selecciona otra carpeta.
SelectDirectoryLabel=Por favor especifica la locación del siguiente disco.

; *** Installation phase messages
SetupAborted=La Instalación no ha sido completa.%n%nPor favor arregla el problema y vuelve a correr el Instalador.
EntryAbortRetryIgnore=Haz click a volver a tratar, Ignorar para proceder de todas maneras, o Abortar para cancelar instalación.

; *** Installation status messages
StatusCreateDirs=Creando directorios...
StatusExtractFiles=Extrayendo archivos...
StatusCreateIcons=Creando accesos directos...
StatusCreateIniEntries=Creado entradas INI...
StatusCreateRegistryEntries=Creando entradas de registro...
StatusRegisterFiles=Registrando archivos...
StatusSavingUninstall=Guardando información de desinstalación...
StatusRunProgram=Terminando instalción...
StatusRollback=Quitando cambios...

; *** Misc. errors
ErrorInternal2=Error Interno: %1
ErrorFunctionFailedNoCode=%1 falló
ErrorFunctionFailed=%1 falló; codigo %2
ErrorFunctionFailedWithMessage=%1 fallado; codigo %2.%n%3
ErrorExecutingProgram=No fue posible ejecutar archivo:%n%1

; *** Registry errors
ErrorRegOpenKey=Error al abrir clave de registro:%n%1\%2
ErrorRegCreateKey=Error creando clave de registro:%n%1\%2
ErrorRegWriteKey=Error escribiendo a clave de registro:%n%1\%2

; *** INI errors
ErrorIniEntry=Error creando entrada INI a archivo"%1".

; *** File copying errors
FileAbortRetryIgnore=Haga click a volver a tratar, Ignorar para saltarse este archivo (no recomendado), o Abortar para cancelar la instalación.
FileAbortRetryIgnore2=Haga click a volver a tratar, Ignorar para proceder de cualquier forma (no recomendado), o Abortar para cancelar la instalación.
SourceIsCorrupted=El archivo fuente esta corrupto
SourceDoesntExist=El archivo fuente %1 no existe
ExistingFileReadOnly=El archivo existente esta marcado como solo lectura.%n%Haga click a volver a tratar para remover el atributo y tratar de nuevo, Ignorar para pasarde este archivo, o Abortar para cancelar la instalación 
ErrorReadingExistingDest=Ocurrio un error al tratar de leer el archivo existente:
FileExists=Este archivo ya existe.%n%¿Deseas que la instalación lo reescriba?
ExistingFileNewer=El archivo existente es mas nuevo que el que la instalacon esta tratando de instalar. It is recommended that you keep the existing file.%n%nDo you want to keep the existing file?
ExistingFileNewer=El archivo existente es más reciente del que se está tratando de instalar.  Se recomienda que mantenga el archivo existente.%n%n¿Desea mantener el archivo existente?
ErrorChangingAttr=Ocurrió un error al tratar de cambiar los atributos del archivo:
ErrorCreatingTemp=Ocurrió un error al tratar de crear un archivo en el directorio destino:
ErrorCopying=Ocurrió un error al tratar de copiar un archivo:
ErrorReplacingExistingFile==Ocurrió un error al tratar de reemplazar el archivo existente:
ErrorReadingSource=Ocurrió un error al tratar de leer el archivo fuente:
ErrorRenamingTemp=Ocurrió un error al tratar de renombrar un archivo en el directorio destino:
ErrorRegisterServer=Imposible registrar el DLL/OCX: %1
ErrorRegisterServerMissingExport=El módulo de registro DllRegisterServer no fué encontrado
ErrorRegisterTypeLib=Imposible registrar el tipo de librería: %1

; *** Post-installation errors
ErrorOpeningReadme=Ocurrió un error al tratar de abrir el archivo LEEME.
ErrorRestartingComputer=El instalador no pudo reiniciar la computadora.  Hágalo manualmente.

; *** Uninstaller messages
UninstallNotFound=El archivo %1 no existe.  No se puede desinstalar.
UninstallOpenError=El archivo %1 no pudo ser abierto.  No se puede desinstalar.
UninstallUnsupportedVer=La bitácora de desinstalación %1 está en un formato no reconocido por esta versión del desinstalador.  No es posible desinstalar.
UninstallUnknownEntry=Una entrada desconocida (%1) fué encontrada en la bitácora de desinstalación.
ConfirmUninstall=¿Está seguro que desea eliminar completamente %1 y todos sus componentes?
OnlyAdminCanUninstall=Este sistema solo puede ser desinstalado por un usuario con privilegios de administración.
UninstallStatusLabel=Por favor espere mientras %1 es eliminado de su computadora.
UninstalledAll=%1 fué exitosamente eliminado de su computadora.
UninstalledMost=%1 desinstalación terminada.%n%nAlgunos elementos no pudieron ser eliminados.  Deberán ser borrados manualmente.
UninstalledAndNeedsRestart=Para completar la desinstalación de %1, su computadora deberá ser reiniciada.%n%n¿Desea hacerlo ahora?
UninstallDataCorrupted="%1" archivo corrupto.  No se puede desinstalar.

; *** Uninstallation phase messages
; *** Mensajes de fase de desinstalación
ConfirmDeleteSharedFileTitle=¿Borrar los archivos compartidos?
ConfirmDeleteSharedFile2=El sistema indica que el siguiente archivo compartido ya no es usado por ningún programa. ¿Desea que la desinstalación borre este archivo compartido?%n%nSi algún programa aún lo utiliza y es borrado, ese programa no funcionará correctamente.  Si no está seguro, elija No.  Dejar el archivo en su sistema no causará ningún daño.
SharedFileNameLabel=Nombre del archivo:
SharedFileLocationLabel=Ubicación:
WizardUninstalling=Estado de la desinstalación
StatusUninstalling=Desinstalando %1...

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1 versión %2
AdditionalIcons=Iconos adiccionales:
CreateDesktopIcon=Mostrar/Quitar icono de &Escritorio
CreateQuickLaunchIcon=Mostrar/Quitar icono de &Inicio Rápido
ProgramOnTheWeb=%1 en la web
UninstallProgram=Desinstalar %1
LaunchProgram=Lanzar %1
AssocFileExtension=&Asociar %1 con %2 extensión de archivo
AssocingFileExtension=Asociando %1 con %2 extensión de archivo...


