; *** Inno Setup version 4.2.2+ Portuguese (Standard) messages ***
;
; Author: NARS (http://nars.cjb.net)
; Last update: 29 Apr 2004
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.php
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; $jrsoftware: issrc/Files/Languages/portuguese-braz.isl,v 1.4 2004/05/18 20:46:55 nars Exp $

[LangOptions]
LanguageName=Portugu<00EA>s (Brasileiro)
LanguageID=$0416
LanguageCodePage=1252

[Messages]

; *** Application titles
SetupAppTitle=Instalação
SetupWindowTitle=%1 - Instalação
UninstallAppTitle=Desinstalação
UninstallAppFullTitle=%1 - Desinstalação

; *** Misc. common
InformationTitle=Informação
ConfirmTitle=Confirmação
ErrorTitle=Erro

; *** SetupLdr messages
SetupLdrStartupMessage=Esse procedimento instalará o %1. Deseja continuar?
LdrCannotCreateTemp=Não foi possível criar um arquivo temporário. Instalação abortada
LdrCannotExecTemp=Não foi possível executar o arquivo na pasta temporária. Instalação abortada

; *** Startup error messages
LastErrorMessage=%1.%n%nErro %2: %3
SetupFileMissing=O arquivo %1 não foi encontrado na pasta de instalação. Corrija o problema ou obtenha uma nova cópia do programa.
SetupFileCorrupt=Os arquivos de instalação estão corrompidos. Obtenha uma nova cópia do programa.
SetupFileCorruptOrWrongVer=Os arquivos de instalação estão corrompidos, ou são incompatíveis com esta versão do Assistente de Instalação. Corrija o problema ou obtenha uma nova cópia do programa.
NotOnThisPlatform=Este programa não pode ser executado no %1.
OnlyOnThisPlatform=Este programa deve ser executado no %1.
WinVersionTooLowError=Este programa necessita do %1 versão %2 ou mais recente.
WinVersionTooHighError=Este programa não pode ser instalado no %1 versão %2 ou mais recente.
AdminPrivilegesRequired=Deve iniciar sessão como administrador para instalar este programa.
PowerUserPrivilegesRequired=Deve iniciar sessão como administrador ou membro do grupo de Super Usuários para instalar este programa.
SetupAppRunningError=O Assistente de Instalação detectou que o %1 está em execução. Feche-o e de seguida clique em OK para continuar, ou clique em Cancelar para cancelar a instalação.
UninstallAppRunningError=O Assistente de Desinstalação detectou que o %1 está em execução. Feche-o e de seguida clique em OK para continuar, ou clique em Cancelar para cancelar a desinstalação.

; *** Misc. errors
ErrorCreatingDir=O Assistente de Instalação não consegue criar a pasta "%1"
ErrorTooManyFilesInDir=Não é possível criar um arquivo na pasta "%1" porque ela contém muitos arquivos

; *** Setup common messages
ExitSetupTitle=Terminar a instalação
ExitSetupMessage=A instalação não está completa. Se terminar agora, o programa não será instalado.%n%nMais tarde poderá executar novamente este Assistente de Instalação e concluir a instalação.%n%nDeseja terminar a instalação?
AboutSetupMenuItem=&Sobre...
AboutSetupTitle=Sobre o Assistente de Instalação
AboutSetupMessage=%1 versão %2%n%3%n%n%1 home page:%n%4
AboutSetupNote=Traduzido por Lukas Taves

; *** Buttons
ButtonBack=< &Anterior
ButtonNext=&Próximo >
ButtonInstall=&Instalar
ButtonOK=OK
ButtonCancel=Cancelar
ButtonYes=&Sim
ButtonYesToAll=Sim para &todos
ButtonNo=&Não
ButtonNoToAll=Nã&o para todos
ButtonFinish=&Concluir
ButtonBrowse=&Procurar...
ButtonWizardBrowse=P&rocurar...
ButtonNewFolder=&Criar Nova Pasta

; *** "Select Language" dialog messages
SelectLanguageTitle=Selecione o Idioma do Assistente de Instalação
SelectLanguageLabel=Selecione o idioma para usar durante a Instalação:

; *** Common wizard text
ClickNext=Clique em Próximo para continuar ou em Cancelar para cancelar a instalação.
BeveledLabel=
BrowseDialogTitle=Procurar Pasta
BrowseDialogLabel=Selecione uma pasta na lista abaixo e clique em OK.
NewFolderName=Nova Pasta

; *** "Welcome" wizard page
WelcomeLabel1=Bem-vindo ao Assistente de Instalação do [name]
WelcomeLabel2=O Assistente de Instalação irá instalar o [name/ver] em seu computador.%n%nÉ recomendado fechar todas as outras aplicações antes de continuar.

; *** "Password" wizard page
WizardPassword=Senha
PasswordLabel1=Esta instalação está protegida por senha.
PasswordLabel3=Insira a senha e depois clique em Próximo para continuar. Na senha existe diferença entre maiúsculas e minúsculas.
PasswordEditLabel=&Senha:
IncorrectPassword=A senha que introduziu não está correta. Tente novamente.

; *** "License Agreement" wizard page
WizardLicense=Licença de contrato
LicenseLabel=É importante ler as próximas informações antes de continuar.
LicenseLabel3=Leia atentamente o contrato de licença a seguir. Os termos do contrato devem ser aceitos para continuar a instalação.
LicenseAccepted=A&ceito o contrato
LicenseNotAccepted=&Não aceito o contrato

; *** "Information" wizard pages
WizardInfoBefore=Informação
InfoBeforeLabel=É importante ler as informações antes de continuar.
InfoBeforeClickLabel=Quando estiver pronto para continuar clique em Próximo.
WizardInfoAfter=Informação
InfoAfterLabel=É importante ler as informações antes de continuar.
InfoAfterClickLabel=Quando estiver pronto para continuar clique em Próximo.

; *** "User Information" wizard page
WizardUserInfo=Informações do usuário
UserInfoDesc=Introduza as suas informações.
UserInfoName=Nome do &usuário:
UserInfoOrg=&Organização:
UserInfoSerial=Número de &série:
UserInfoNameRequired=Deve introduzir um nome.

; *** "Select Destination Location" wizard page
WizardSelectDir=Selecione a localização de destino
SelectDirDesc=Onde deverá ser instalado o [name]?
SelectDirLabel3=O [name] será instalado na pasta seguinte.
SelectDirBrowseLabel=Para continuar, clique em Próximo. Se desejar selecionar uma pasta diferente, clique em Procurar.
DiskSpaceMBLabel=É necessário pelo menos [mb] MB de espaço livre em disco.
ToUNCPathname=O Assistente de Instalação não pode instalar num caminho do tipo UNC. Se está tentanado fazer a instalação numa rede, precisa mapear a unidade de rede.
InvalidPath=É necessário indicar o caminho completo com a letra de unidade; por exemplo:%n%nC:\APP%n%nou um caminho UNC no formato:%n%n\\servidor\compartilhado
InvalidDrive=A unidade ou pasta compartilhada UNC selecionada não existe ou não está acessível. Selecione outra.
DiskSpaceWarningTitle=Não há espaço suficiente no disco
DiskSpaceWarning=O Assistente de Instalação necessita de pelo menos %1 KB de espaço livre, mas a unidade seleccionada tem apenas %2 KB disponíveis.%n%nDeseja continuar de qualquer forma?
DirNameTooLong=O nome ou caminho para a pasta é muito longo.
InvalidDirName=O nome da pasta é inválido.
BadDirName32=O nome da pasta não pode conter nenhum dos próximos caracteres:%n%n%1
DirExistsTitle=A pasta já existe
DirExists=A pasta:%n%n%1%n%njá existe. Gostaria de instalar assim mesmo?
DirDoesntExistTitle=A pasta não existe
DirDoesntExist=A pasta:%n%n%1%n%nnão existe. Gostaria de criá-la?

; *** "Select Components" wizard page
WizardSelectComponents=Selecione os componentes
SelectComponentsDesc=Quais componentes deverão ser instalados?
SelectComponentsLabel2=Selecione os componentes que deseja instalar e de-selecione os componentes que não queira instalar. Clique em Próximo quando estiver pronto para continuar.
FullInstallation=Instalação Completa
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Instalação Compacta
CustomInstallation=Instalação Personalizada
NoUninstallWarningTitle=Componentes Encontrados
NoUninstallWarning=O Assistente de Instalação detectou que os próximos componentes estão instalados no seu computador:%n%n%1%n%nSe de-seleccionar estes componentes eles não serão desinstalados.%n%nDeseja continuar?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel=A seleção atual necessita de pelo menos [mb] MB de espaço em disco.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Selecione tarefas adicionais
SelectTasksDesc=Que tarefas adicionais deverão ser executadas?
SelectTasksLabel2=Selecione as tarefas adicionais que deseja que o Assistente de Instalação execute na instalação do [name] e em seguida clique em Próximo.

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Selecione a pasta do Menu Iniciar
SelectStartMenuFolderDesc=Onde deverão ser colocados os ícones de atalho do programa?
SelectStartMenuFolderLabel3=Os ícones de atalho do programa serão criados na seguinte pasta do Menu Iniciar.
SelectStartMenuFolderBrowseLabel=Para continuar, clique em Próximo. Se desejar selecionar uma pasta diferente, clique em Procurar.
NoIconsCheck=&Não criar ícones
MustEnterGroupName=É necessário nomear a pasta.
GroupNameTooLong=O nome ou caminho para a pasta é muito grande.
InvalidGroupName=O nome da pasta é inválido.
BadGroupName=O nome da pasta não pode conter nenhum dos próximos caracteres:%n%n%1
NoProgramGroupCheck2=&Não criar nenhuma pasta no Menu Iniciar

; *** "Ready to Install" wizard page
WizardReady=Pronto para Instalar
ReadyLabel1=O Assistente de Instalação está pronto para instalar o [name] no seu computador.
ReadyLabel2a=Clique em Instalar para continuar a instalação, ou clique em Anterior se desejar rever ou alterar alguma das configurações.
ReadyLabel2b=Clique em Instalar para continuar a instalação.
ReadyMemoUserInfo=Informações do utilizador:
ReadyMemoDir=Localização de destino:
ReadyMemoType=Tipo de instalação:
ReadyMemoComponents=Componentes selecionados:
ReadyMemoGroup=Pasta do Menu Iniciar:
ReadyMemoTasks=Tarefas adicionais:

; *** "Preparing to Install" wizard page
WizardPreparing=Preparando-se para instalar
PreparingDesc=Preparando-se para instalar o [name] no seu computador.
PreviousInstallNotCompleted=A instalação/remoção de um programa anterior não foi completada. Você precisa reiniciar o computador para completar essa instalação.%n%nDepois de reiniciar o computador, execute novamente este Assistente de Instalação para completar a instalação do [name].
CannotContinue=A Instalação não pode continuar. Clique em Cancelar para sair.

; *** "Installing" wizard page
WizardInstalling=Instalando
InstallingLabel=Aguarde enquanto o Assistente de Instalação instala o [name] em seu computador.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Instalação do [name] foi concluída
FinishedLabelNoIcons=O Assistente de Instalação concluiu a instalação do [name] no seu computador.
FinishedLabel=O Assistente de Instalação concluiu a instalação do [name] no seu computador. A aplicação pode ser iniciada através dos ícones instalados.
ClickFinish=Clique em Concluir para finalizar o Assistente de Instalação.
FinishedRestartLabel=Para completar a instalação do [name], o Assistente de Instalação deverá reiniciar o seu computador. Deseja reiniciar agora?
FinishedRestartMessage=Para completar a instalação do [name], o Assistente de Instalação deverá reiniciar o seu computador.%n%nDeseja reiniciar agora?
ShowReadmeCheck=Sim, desejo abrir o arquivo LEIAME
YesRadio=&Sim, desejo reiniciar o computador agora
NoRadio=&Não, desejo reiniciar o computador mais tarde
; used for example as 'Run MyProg.exe'
RunEntryExec=Executar %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Visualizar %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=O Assistente de Instalação precisa do próximo disco
SelectDiskLabel2=Introduza o disco %1 e clique em OK.%n%nSe os arquivos deste disco estiverem num local diferente do mostrado abaixo, indique o caminho correto ou clique em Procurar.
PathLabel=&Caminho:
FileNotInDir2=O arquivo "%1" não foi encontrado em "%2". Introduza o disco correto ou selecione outra pasta.
SelectDirectoryLabel=Indique a localização do próximo disco.

; *** Installation phase messages
SetupAborted=A instalação não está completa.%n%nCorrija o problema e execute o Assistente de Instalação novamente.
EntryAbortRetryIgnore=Clique em Repetir para tentar novamente, Ignorar para continuar de qualquer forma, ou Abortar para cancelar a instalação.

; *** Installation status messages
StatusCreateDirs=Criando pastas...
StatusExtractFiles=Extraindo arquivos...
StatusCreateIcons=Criando atalhos...
StatusCreateIniEntries=Criando entradas em INI...
StatusCreateRegistryEntries=Criando entradas no registo...
StatusRegisterFiles=Registrando arquivos...
StatusSavingUninstall=Guardando informações para desinstalação...
StatusRunProgram=Concluindo a instalação...
StatusRollback=Anulando as alterações...

; *** Misc. errors
ErrorInternal2=Erro interno: %1
ErrorFunctionFailedNoCode=%1 falhou
ErrorFunctionFailed=%1 falhou; código %2
ErrorFunctionFailedWithMessage=%1 falhou; código %2.%n%3
ErrorExecutingProgram=Não é possível executar o arquivo:%n%1

; *** Registry errors
ErrorRegOpenKey=Erro ao abrir a chave de registo:%n%1\%2
ErrorRegCreateKey=Erro ao criar a chave de registo:%n%1\%2
ErrorRegWriteKey=Erro ao escrever na chave de registo:%n%1\%2

; *** INI errors
ErrorIniEntry=Erro ao criar entradas em INI no arquivo "%1".

; *** File copying errors
FileAbortRetryIgnore=Clique em Repetir para tentar novamente, Ignorar para ignorar este arquivo (não recomendado), ou Abortar para cancelar a instalação.
FileAbortRetryIgnore2=Clique em Repetir para tentar novamente, Ignorar para continuar de qualquer forma (não recomendado), ou Abortar para cancelar a instalação.
SourceIsCorrupted=O arquivo de origem está corrompido
SourceDoesntExist=O arquivo de origem "%1" não existe
ExistingFileReadOnly=O arquivo existente tem o atributo "somente leitura".%n%nClique em Repetir para remover o atributo "só de leitura" e tentar novamente, Ignorar para ignorar este arquivo, ou Abortar para cancelar a instalação.
ErrorReadingExistingDest=Ocorreu um erro ao tentar ler o arquivo existente:
FileExists=O arquivo já existe.%n%nDeseja substituí-lo?
ExistingFileNewer=O arquivo existente é mais recente que o que está a ser instalado. É recomendado que mantenha o arquivo existente.%n%nDeseja manter o arquivo existente?
ErrorChangingAttr=Ocorreu um erro ao tentar alterar os atributos do arquivo existente:
ErrorCreatingTemp=Ocorreu um erro ao tentar criar um arquivo na pasta de destino:
ErrorReadingSource=Ocorreu um erro ao tentar ler o arquivo de origem:
ErrorCopying=Ocorreu um erro ao tentar copiar um arquivo:
ErrorReplacingExistingFile=Ocorreu um erro ao tentar substituir o arquivo existente:
ErrorRestartReplace=RestartReplace failed:
ErrorRenamingTemp=Ocorreu um erro ao tentar mudar o nome de um arquivo na pasta de destino:
ErrorRegisterServer=Não é possível registar o DLL/OCX: %1
ErrorRegisterServerMissingExport=Exportador DllRegisterServer export not found
ErrorRegisterTypeLib=Incapaz de registrar o tipo de biblioteca: %1

; *** Post-installation errors
ErrorOpeningReadme=Ocorreu um erro ao tentar abrir o arquivo LEIAME.
ErrorRestartingComputer=O Assistente de Instalação não conseguiu reiniciar o computador. Por favor reinicie manualmente.

; *** Uninstaller messages
UninstallNotFound=O arquivo "%1" não existe. Não é possível desinstalar.
UninstallOpenError=Não foi possível abrir o arquivo "%1". Não é possível desinstalar.
UninstallUnsupportedVer=O arquivo do relatório de desinstalação "%1" está num formato que não é reconhecido por esta versão do desinstalador. Não é possível desinstalar
UninstallUnknownEntry=Foi encontrada uma entrada desconhecida (%1) no arquivo do relatório de desinstalação
ConfirmUninstall=Tem a certeza que deseja remover completamente o %1 e todos os seus componentes?
OnlyAdminCanUninstall=Esta desinstalação só pode ser realizada por um usuário com direitos administrativos.
UninstallStatusLabel=Por favor aguarde enquanto o %1 está sendo removido de seu computador.
UninstalledAll=O %1 foi removido de seu computador com sucesso.
UninstalledMost=A desinstalação do %1 está concluída.%n%nAlguns elementos não puderam ser removidos. Estes elementos podem ser removidos manualmente.
UninstalledAndNeedsRestart=Para completar a desinstalação do %1, o computador deve ser reiniciado.%n%nDeseja reiniciar agora?
UninstallDataCorrupted=O arquivo "%1" está corrompido. Não é possível desinstalar

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Remover arquivo compartilhado?
ConfirmDeleteSharedFile2=O sistema indica que o próximo arquivo compartilhado não está sendo utilizado por nenhum outro programa. Deseja removê-lo?%n%nSe algum programa ainda necessitar deste arquivo, poderá não funcionar corretamente depois de o remover. Se não tiver a certeza, selecione Não. Manter o arquivo não causará nenhum problema.
SharedFileNameLabel=Nome do arquivo:
SharedFileLocationLabel=Localização:
WizardUninstalling=Estado da desinstalação
StatusUninstalling=Desinstalando o %1...

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1 versão %2
AdditionalIcons=Ícones adicionais:
CreateDesktopIcon=Mostrar um ícone na &área de trabalho
CreateQuickLaunchIcon=Mostrar um ícone na barra de &Inicialização Rápida
ProgramOnTheWeb=%1 na Web
UninstallProgram=Desinstalar o %1
LaunchProgram=Executar o %1
AssocFileExtension=&Associar o %1 aos arquivos com a extensão %2
AssocingFileExtension=A associar o %1 aos arquivos com a extensão %2...
