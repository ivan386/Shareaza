; *** Inno Setup version 3.0.6 Portuguese (Brazilian) messages ***
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.htm
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; by Daniel Nogueira <danielnogueira@ism.com.br>
; based on original by Fabricio Biazzotto <sliphacker@yahoo.com.br>

[LangOptions]
LanguageName=br
LanguageID=$0416
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
;DialogFontName=MS Shell Dlg
;DialogFontSize=8
;DialogFontStandardHeight=13
;TitleFontName=Arial
;TitleFontSize=29
;WelcomeFontName=Verdana
;WelcomeFontSize=12
;CopyrightFontName=Arial
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=Programa de Instalação
SetupWindowTitle=Programa de Instalação - %1
UninstallAppTitle=Programa de Desinstalação
UninstallAppFullTitle=Programa de Desinstalação - %1

; *** Misc. common
InformationTitle=Informação
ConfirmTitle=Confirmação
ErrorTitle=Erro

; *** SetupLdr messages
SetupLdrStartupMessage=Este programa irá instalar o %1. Você deseja continuar?
LdrCannotCreateTemp=Não foi possível criar um arquivo temporário. Instalação abortada
LdrCannotExecTemp=Não foi possível executar um arquivo na pasta de arquivos temporários. Instalação abortada

; *** Startup error messages
LastErrorMessage=%1.%n%nErro %2: %3
SetupFileMissing=O arquivo %1 está faltando na pasta de instalação. Corrija o problema ou obtenha uma nova cópia do programa.
SetupFileCorrupt=Os arquivos de instalação estão corrompidos. Obtenha uma nova cópia do programa.
SetupFileCorruptOrWrongVer=Os arquivos de instalação estão corrompidos ou são incompatíveis com esta versão do Programa de Instalação. Corrija o problema ou obtenha uma cópia nova.
NotOnThisPlatform=Este programa não pode ser executado no %1.
OnlyOnThisPlatform=Este programa deverá ser executado no %1.
WinVersionTooLowError=Este programa exige o %1 versão %2 ou mais recente.
WinVersionTooHighError=Este programa não pode ser instalado no %1 versão %2 ou mais recente.
AdminPrivilegesRequired=Você deverá estar logado como administrador para instalar este programa.
PowerUserPrivilegesRequired=Você deve ser logado como um administrador ou como um membro do grupo "Power Users" quando instalar este programa.
SetupAppRunningError=O Programa de Instalação detectou que %1 está sendo executando.%n%nPor favor, feche todas as suas instâncias agora, e então clique em OK para continuar, ou Cancelar para sair.
UninstallAppRunningError=O Programa de Desinstalação detectou que %1 está sendo executando.%n%nPor favor, feche todas as suas instâncias agora, e então clique em OK para continuar, ou Cancelar para sair.

; *** Misc. errors
ErrorCreatingDir=O Programa de Instalação não pôde criar a pasta "%1"
ErrorTooManyFilesInDir=Não foi possível criar um arquivo no pasta "%1" pois ela contém arquivos demais

; *** Setup common messages
ExitSetupTitle=Sair do Programa de Instalação
ExitSetupMessage=A instalação não está completa. Se você terminar agora, o programa não será instalado.%n%nVocê pode executar o Programa de Instalação mais tarde para completar a instalação.%n%nSair do Programa de Instalação?
AboutSetupMenuItem=&Sobre o Programa de Instalação...
AboutSetupTitle=Sobre o Programa de Instalação
AboutSetupMessage=%1 versão %2%n%3%n%n%1 home page:%n%4
AboutSetupNote=

; *** Buttons
ButtonBack=< &Voltar
ButtonNext=&Avançar >
ButtonInstall=&Instalar
ButtonOK=OK
ButtonCancel=Cancelar
ButtonYes=&Sim
ButtonYesToAll=Sim para &Todos
ButtonNo=&Não
ButtonNoToAll=Nã&o para Todos
ButtonFinish=&Concluir
ButtonBrowse=&Procurar...

; *** "Select Language" dialog messages
SelectLanguageTitle=Selecione o idioma do Programa de Instalação
SelectLanguageLabel=Selecione o idioma a ser usado durante a instalação:

; *** Common wizard text
ClickNext=Clique Avançar para continuar, Cancelar para sair do Programa de Instalação.
BeveledLabel=

; *** "Welcome" wizard page
WelcomeLabel1=Bem-vindo ao Programa de Instalação do [name].
WelcomeLabel2=Este programa irá instalar o [name/ver] no seu computador.%n%nÉ recomendado que você feche todas as aplicações abertas antes de continuar. Isto evitará conflitos durante a instalação.

; *** "Password" wizard page
WizardPassword=Senha
PasswordLabel1=Esta instalação está protegida por senha.
PasswordLabel3=Por favor digite a senha e então clique em Avançar para continuar. Maiúsculas e minúsculas são diferenciadas.
PasswordEditLabel=&Senha:
IncorrectPassword=A senha que você digitou não está correta. Tente novamente.

; *** "License Agreement" wizard page
WizardLicense=Licença de Uso
LicenseLabel=Leia as seguintes informações importantes antes de continuar.
LicenseLabel3=Leia a Licença de Uso seguinte. Você precisa aceitar os termos desta licença antes de continuar com a instalação
LicenseAccepted=Eu &aceito a licença
LicenseNotAccepted=Eu &não aceito a licença

; *** "Information" wizard pages
WizardInfoBefore=Informação
InfoBeforeLabel=Leia as seguintes informações importantes antes de continuar.
InfoBeforeClickLabel=Quando você estiver pronto para continuar clique em Avançar.
WizardInfoAfter=Informação
InfoAfterLabel=Leia as seguintes informações importantes antes de continuar.
InfoAfterClickLabel=Quando você estiver pronto para continuar clique em Avançar.

; *** "User Information" wizard page
WizardUserInfo=Informações do Usuário
UserInfoDesc=Informe seus dados.
UserInfoName=&Nome de Usuário:
UserInfoOrg=&Organização:
UserInfoSerial=Número de &série:
UserInfoNameRequired=Você precisa informar um nome.

; *** "Select Destination Directory" wizard page
WizardSelectDir=Escolha a pasta de destino
SelectDirDesc=Onde o [name] será instalado?
SelectDirLabel=Escolha a pasta onde você quer instalar o [name] e então clique em Avançar.
DiskSpaceMBLabel=Este programa exige no mínimo [mb] MB de espaço.
ToUNCPathname=O Programa de Instalação não pode instalar em um caminho UNC. Se você está tentando instalar em uma rede, você precisa mapear uma unidade da rede.
InvalidPath=Você deve entrar um caminho completo com a letra da unidade; por exemplo:%nC:\APP%n%nou um caminho UNC na forma:%n%n\\servidor\estação
InvalidDrive=A unidade ou estação UNC que você selecionou não existe. Escolha outra.
DiskSpaceWarningTitle=Não há espaço suficiente
DiskSpaceWarning=O Programa de Instalação exige %1 KB de espaço livre para instalar, mas a unidade selecionada tem somente %2 KB disponíveis.%n%você quer continuar?
BadDirName32=O nome da pasta não pode conter os seguintes caracteres:%n%n%1
DirExistsTitle=Pasta Existente
DirExists=A pasta%n%n%1%n%njá existe. Você gostaria de instalar nesta pasta mesmo assim?
DirDoesntExistTitle=Pasta Não-Existente
DirDoesntExist=A pasta:%n%n%1%n%nnão existe. Você deseja que ela seja criada?

; *** "Select Components" wizard page
WizardSelectComponents=Selecione Componentes
SelectComponentsDesc=Que componentes deverão ser instalados?
SelectComponentsLabel2=Selecione os componentes que você quer instalar, desmarque os componentes que você não quer instalar. Clique em Avançar quando você estiver pronto para continuar.
FullInstallation=Instalação Completa
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Instalação Compacta
CustomInstallation=Instalação Personalizada
NoUninstallWarningTitle=Componentes Encontrados
NoUninstallWarning=O Programa de Instalação detectou que os seguintes componentes estão instalados em seu computador:%n%n%1%n%nDesmarcar estes componentes não fará com que eles sejam desinstalados.%n%nVocê gostaria de continuar mesmo assim?
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel=Seleção atual requer [mb] MB de espaço.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Selecione Tarefas Adicionais
SelectTasksDesc=Que tarefas adicionais serão executadas?
SelectTasksLabel2=Selecione as tarefas adicionais que você gostaria que o Programa de Instalação execute enquanto instala o [name] e então clique em Avançar.

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Escolha a pasta do Menu Iniciar
SelectStartMenuFolderDesc=Onde o Programa de Instalação deverá criar os ícones do programa?
SelectStartMenuFolderLabel=Escolha a pasta do Menu Iniciar onde o Programa de Instalação irá criar os ícones do programa e então clique em Avançar
NoIconsCheck=&Não criar ícones
MustEnterGroupName=Você deve digitar um nome de uma pasta do Menu Iniciar.
BadGroupName=O nome do pasta não pode incluir os seguintes caracteres:%n%n%1
NoProgramGroupCheck2=&Não criar uma pasta do Menu Iniciar

; *** "Ready to Install" wizard page
WizardReady=Pronto para Instalar
ReadyLabel1=O Programa de Instalação está pronto para começar a instalar o  [name] no seu computador
ReadyLabel2a=Clique em Instalar para iniciar a instalação, ou clique Voltar se você quer rever ou modificar suas opções
ReadyLabel2b=Clique em Instalar para iniciar a instalação
ReadyMemoUserInfo=Informações do Usuário:
ReadyMemoDir=Diretório de destino:
ReadyMemoType=Tipo de Instalação:
ReadyMemoComponents=Componentes Selecionados:
ReadyMemoGroup=Pasta do Menu Iniciar:
ReadyMemoTasks=Tarefas Adicionais:

; *** "Preparing to Install" wizard page
WizardPreparing=Preparando para Instalar
PreparingDesc=Instalação está preparando para instalar o [name] em seu computador.
PreviousInstallNotCompleted=A instalação/remoção do programa anterior não foi completada. Você precisa reiniciar o computador para completar esta instalação. %n%nApós reiniciar seu computador, rode o Programa de Instalação novamente para completar a instalação do [name].
CannotContinue=A instalação não pode continuar. Clique em Cancelar para sair.

; *** "Installing" wizard page
WizardInstalling=Instalando
InstallingLabel=Aguarde enquanto o Programa de Instalação instala o [name] em seu computador

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Completando a instalação do [name]
FinishedLabelNoIcons=O Programa de Instalação terminou de instalar o [name] no seu computador.
FinishedLabel=O Programa de Instalação terminou de instalar o [name] no seu computador. O programa pode ser iniciado escolhendo os ícones instalados.
ClickFinish=Clique em Concluir para finalizar o Programa de Instalação.
FinishedRestartLabel=Para completar a instalação do [name], o Programa de Instalação deverá reiniciar o seu computador. Você gostaria de reiniciar agora?
FinishedRestartMessage=Para completar a instalação do [name], o Programa de Instalação deverá reiniciar o seu computador. Você gostaria de reiniciar agora?
ShowReadmeCheck=Sim, eu quero ver o arquivo LEIAME
YesRadio=&Sim, reiniciar o computador agora
NoRadio=&Não, eu reiniciarei o computador mais tarde
; used for example as 'Run MyProg.exe'
RunEntryExec=Executar %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Visualizar %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=O Programa de Instalação precisa do próximo disco
SelectDirectory=Escolha a Pasta
SelectDiskLabel2=Insira o disco %1 e clique OK.%n%nSe os arquivos deste disco estiverem em uma pasta diferente da mostrada abaixo, digite o caminho correto ou clique em Procurar.
PathLabel=&Caminho:
FileNotInDir2=O arquivo "%1" não pôde ser localizado em "%2". Insira o disco correto ou escolha outra pasta.
SelectDirectoryLabel=Indique a localização do próximo disco.

; *** Installation phase messages
SetupAborted=A instalação não foi completada.%n%nCorrija o problema e execute o Programa de Instalação novamente.
EntryAbortRetryIgnore=Clique Repetir para tentar novamente, Ignorar para continuar assim mesmo, ou Anular para cancelara instalação.

; *** Installation status messages
StatusCreateDirs=Criando pastas...
StatusExtractFiles=Extraindo arquivos...
StatusCreateIcons=Criando ícones...
StatusCreateIniEntries=Criando entradas INI...
StatusCreateRegistryEntries=Criando entradas no registro...
StatusRegisterFiles=Registrando arquivos...
StatusSavingUninstall=Salvando informação para desinstalação...
StatusRunProgram=Terminando a instalação...
StatusRollback=Revertendo as mudanças...

; *** Misc. errors
ErrorInternal2=Erro interno: %1
ErrorFunctionFailedNoCode=%1 falhou
ErrorFunctionFailed=%1 falhou; código %2
ErrorFunctionFailedWithMessage=%1 falhou; código %2.%n%3
ErrorExecutingProgram=Não foi possível executar o arquivo:%n%1

; *** Registry errors
ErrorRegOpenKey=Erro ao abrir a chave de registro:%n%1\%2
ErrorRegCreateKey=Erro ao criar a chave de registro:%n%1\%2
ErrorRegWriteKey=Erro ao escrever na chave de registro:%n%1\%2

; *** INI errors
ErrorIniEntry=Erro ao criar entrada INI no arquivo "%1".

; *** File copying errors
FileAbortRetryIgnore=Clique Repetir para tentar novamente, Ignorar para pular este arquivo (não recomendado), ou Anular para cancelar a instalação.
FileAbortRetryIgnore2=Clique Repetir para tentar novamente, Ignorar para continuar assim mesmo (não recomendado), ou Anular para cancelar a instalação.
SourceIsCorrupted=O arquivo de origem está corrompido
SourceDoesntExist=O arquivo de origem "%1" não existe
ExistingFileReadOnly=O arquivo existente no seu computador está marcado como somente leitura.%n%nClique em Repetir para remover o atributo de somente leitura e tentar novamente, Ignorar para pular este arquivo, ou Anular para cancelar a instalação.
ErrorReadingExistingDest=Um erro ocorreu ao tentar ler o arquivo existente no seu computador:
FileExists=O arquivo já existe.%n%nVocê gostaria de sobrescrevê-lo?
ExistingFileNewer=O arquivo existente no seu computador é mais novo que aquele que o Programa de Instalação está tentando instalar. É recomendado que você mantenha o arquivo existente.%n%nVocê deseja manter o arquivo existente?
ErrorChangingAttr=Um erro ocorreu ao tentar mudar os atributos do arquivo existente no seu computador:
ErrorCreatingTemp=Um erro ocorreu ao tentar criar um arquivo na pasta destino:
ErrorReadingSource=Um erro ocorreu ao tentar ler o arquivo de origem:
ErrorCopying=Um erro ocorreu ao tentar copiar um arquivo:
ErrorReplacingExistingFile=Um erro ocorreu ao tentar substituir o arquivo existente:
ErrorRestartReplace=RestartReplace falhou:
ErrorRenamingTemp=Um erro ocorreu ao tentar renomear um arquivo na pasta de destino:
ErrorRegisterServer=Não foi possível registrar a DLL/OCX: %1
ErrorRegisterServerMissingExport=DllRegisterServer não encontrado
ErrorRegisterTypeLib=Não foi possível registrar a biblioteca de tipos: %1

; *** Post-installation errors
ErrorOpeningReadme=Um erro ocorreu ao tentar abrir o arquivo LEIAME.
ErrorRestartingComputer=O Programa de Instalação não conseguiu reiniciar o computador. Por favor faça isso manualmente.

; *** Uninstaller messages
UninstallNotFound=O arquivo "%1" não existe. Não é possível desinstalar.
UninstallOpenError=O arquivo "%1" não pôde ser aberto. Não é possível desinstalar
UninstallUnsupportedVer=O arquivo de log de desinstação "%1" está em um formato que não é reconhecido por esta versão do desinstalador. Não é possível desinstalar
UninstallUnknownEntry=Uma entrada desconhecida (%1) foi encontrada no log de desinstalação
ConfirmUninstall=Você tem certeza que quer remover completamente o %1 e todos os seus componentes?
OnlyAdminCanUninstall=Está instalação só pode ser desinstalada por um usuário com privilégios administrativos.
UninstallStatusLabel=Por favor, aguarde enquanto o %1 é removido do seu computador.
UninstalledAll=O %1 foi removido com sucesso do seu computador.
UninstalledMost=A desinstalação do %1 terminou.%n%nAlguns elementos não puderam ser removidos. Estes elementos podem ser removidos manualmente.
UninstalledAndNeedsRestart=Para completar a desintalação do %1, você precisa reiniciar seu computador. %n%nVocê gostaria de reiniciar agora?
UninstallDataCorrupted=O arquivo "%1" está corrompido. Não é possível  desinstalar

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Remover arquivo compartilhado?
ConfirmDeleteSharedFile2=O sistema indicou que o seguinte arquivo compartilhado não está mais sendo usando por nenhum outro programa. Você gostaria de remover este arquivo compartilhado?%n%n%Se qualquer programa ainda estiver usando este arquivo e ele for removido, este programa poderá  não funcionar corretamente. Se você não tiver certeza, escolha Não. Manter o arquivo no computador não causará nenhum problema.
SharedFileNameLabel=Nome do Arquivo:
SharedFileLocationLabel=Localização:
WizardUninstalling=Progresso da Desinstalação
StatusUninstalling=Desinstalando o %1...
