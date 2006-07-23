; *** Inno Setup version 5.1.0+ English messages ***
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.php
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).

[LangOptions]
; The following three entries are very important. Be sure to read and
; understand the '[LangOptions] section' topic in the help file.
LanguageName=<65E5><672C><8A9E>
LanguageID=$0411
LanguageCodePage=932
; If the language you are translating to requires special font faces or
; sizes, uncomment any of the following entries and change them accordingly.
DialogFontName=MS UI Gothic
DialogFontSize=10
WelcomeFontName=MS mincho
;WelcomeFontSize=12
TitleFontName=MS mincho
;TitleFontSize=29
CopyrightFontName=MS mincho
;CopyrightFontSize=8

[Messages]

; *** Application titles
SetupAppTitle=セットアップ
SetupWindowTitle=%1のセットアップ
UninstallAppTitle=アンインストール
UninstallAppFullTitle=%1のアンインストール

; *** Misc. common
InformationTitle=インフォメーション
ConfirmTitle=確認
ErrorTitle=エラー

; *** SetupLdr messages
SetupLdrStartupMessage=%1をインストールします。続けますか?
LdrCannotCreateTemp=一時ファイルを作成できませんでした。セットアップを中止します。
LdrCannotExecTemp=一時ファイルの実行に失敗しました。セットアップを終了します。

; *** Startup error messages
LastErrorMessage=%1.%n%nエラー %2: %3
SetupFileMissing=%1が見つかりません。
SetupFileCorrupt=セットアップファイルが破損しています。もう一度ファイルをダウンロードしなおしてください。
SetupFileCorruptOrWrongVer=セットアップファイルが破損しているか、バージョンが違います。正しいバージョンのファイルをダウンロードしなおしてください。
NotOnThisPlatform=このソフトウェアｰは%1では動作しません。
OnlyOnThisPlatform=このソフトウェアｰは%1でしか動作しません。
OnlyOnTheseArchitectures=このソフトウェアｰは次のWindows又はCPUアーキテクチャー上でのみ動作します:%n%n%1
MissingWOW64APIs=このWindowsには64ビットを使用するのに必要なものが足りません。使用するためにはサービスパック%1が必要です。
WinVersionTooLowError=このソフトは%1 version %2 以降が必要です。
WinVersionTooHighError=このソフトは %1 version %2 以降にはインストールできません。
AdminPrivilegesRequired=管理者権限でインストールしてください。
PowerUserPrivilegesRequired=このソフトのインストールは、管理者権限又はパワーユーザー権限が必要です。
SetupAppRunningError=セットアップは%1の作動を感知しました。セットアップを続行するには、%n%nすべての%1を終了してから"はい"をクリック、又はセットアップを中止してください。
UninstallAppRunningError=セットアップは%1の作動を感知しました。セットアップを続行するには、%n%n%1を終了してから"はい"をクリック、又はセットアップを中止してください。

; *** Misc. errors
ErrorCreatingDir=セットアップは"%1"フォルダーを作成できませんでした。
ErrorTooManyFilesInDir=セットアップは"%1"フォルダーにファイルを作成できませんでした。ファイルが多すぎます。

; *** Setup common messages
ExitSetupTitle=セットアップの中止
ExitSetupMessage=セットアップを中止します。中止するとインストールされません%n%n中止しますか?
AboutSetupMenuItem=セットアップについて...(&A)
AboutSetupTitle=セットアップについて
AboutSetupMessage=%1 version %2%n%3%n%n%1 home page:%n%4
AboutSetupNote=
TranslatorNote=

; *** Buttons
ButtonBack=< 戻る(&B)
ButtonNext=次へ(&N) >
ButtonInstall=インストール(&I)
ButtonOK=OK
ButtonCancel=キャンセル
ButtonYes=はい(&Y)
ButtonYesToAll=すべて "はい"(&A)
ButtonNo=いいえ(&N)
ButtonNoToAll=すべて "いいえ"(&o)
ButtonFinish=終了(&F)
ButtonBrowse=参照...(&B)
ButtonWizardBrowse=参照...(&r)
ButtonNewFolder=新しいフォルダー(&M)

; *** "Select Language" dialog messages
SelectLanguageTitle=言語選択
SelectLanguageLabel=セットアップに使用する言語を選んでください:

; *** Common wizard text
ClickNext=続けるには"次へ"をクリックしてください, 又は"中止"をクリックして、セットアップを中止してください。
BeveledLabel=
BrowseDialogTitle=フォルダーの参照
BrowseDialogLabel=リストからフォルダーを選んでOKをクリックしてください。
NewFolderName=新しいフォルダー

; *** "Welcome" wizard page
WelcomeLabel1=[name]セットアップウィザードへようこそ
WelcomeLabel2=このセットアップは[name/ver]をインストールします。%n%n実行中のすべてのアプリケーションソフトを終了してから実行することが望ましいです。

; *** "Password" wizard page
WizardPassword=パスワード
PasswordLabel1=このインストーラーはパスワードで保護されています。
PasswordLabel3=続けるにはパスワードを入力して"次へ"をクリックしてください。%n注）パスワードは大文字小文字を区別します。
PasswordEditLabel=パスワード(&P):
IncorrectPassword=入力されたパスワードは正しくありません。正しいパスワードを入力してください。

; *** "License Agreement" wizard page
WizardLicense=規約
LicenseLabel=次の規約を読んでから同意又は、拒否してください
LicenseLabel3=次の規約を読んでから同意又は拒否してください。セットアップを続けるには同意してください。拒否した場合はセットアップを終了します。
LicenseAccepted=同意します(&a)
LicenseNotAccepted=同意しません(&d)

; *** "Information" wizard pages
WizardInfoBefore=インフォメーション
InfoBeforeLabel=セットアップの前に次のインフォメーションを良くお読みください。
InfoBeforeClickLabel=セットアップの準備ができたら、"次へ"をクリックしてください。
WizardInfoAfter=インフォメーション
InfoAfterLabel=セットアップを続行する前に次のインフォメーションを良くお読みください。
InfoAfterClickLabel=準備ができたら、"次へ"をクリックしてください。

; *** "User Information" wizard page
WizardUserInfo=使用者情報
UserInfoDesc=次の項目を記入してください。
UserInfoName=氏名(&U):
UserInfoOrg=会社名(&O):
UserInfoSerial=シリアルナンバー(&S):
UserInfoNameRequired=氏名を入力してください。

; *** "Select Destination Location" wizard page
WizardSelectDir=インストール先を選択してください。
SelectDirDesc=[name]をどこへインストールしますか？
SelectDirLabel3=セットアップは[name]を次の場所にインストールします。
SelectDirBrowseLabel=続けるには"次へ"をクリックしてください。ほかの場所にインストールしたい場合は参照してください。
DiskSpaceMBLabel=最低限[mb] MBの空き容量が必要です。
ToUNCPathname=インストールするにはUNCパスが必要です。もしもネットワーク共有にインストールしたい場合は、ネットワークドライブを割り当ててください。
InvalidPath=インストールにはドライブ名の入った完全なパスが必要です; 例:%n%nC:\APP%n%n又はUNCパス:%n%n\\server\share
InvalidDrive=指定されたドライブ又はUNCで指定されたネットワーク共有は、無効又は存在しません。正しい場所を指定してください。
DiskSpaceWarningTitle=空き容量が足りません
DiskSpaceWarning=セットアップは最低 %1 KB の空き容量を必要とします, しかし選択されたドライブにはは %2 KB しか空き容量がありません%n%nそれでも続けますか？
DirNameTooLong=フォルダー名又はファイル名が長すぎます。
InvalidDirName=指定されたフォルダー名は無効です。
BadDirName32=フォルダー名に次の文字は使用できません:%n%n%1
DirExistsTitle=フォルダーが存在します。
DirExists=フォルダー:%n%n%1%n%nはすでに存在しています。続けますか？
DirDoesntExistTitle=フォルダーが存在しません。
DirDoesntExist=フォルダー:%n%n%1%n%nは存在しません。このフォルダーを作成しますか？

; *** "Select Components" wizard page
WizardSelectComponents=コンポーネントの選択
SelectComponentsDesc=使用したいコンポーネントを選んでください。
SelectComponentsLabel2=インストールしたいコンポーネントを選択してください; インストールしたくないコンポーネントをクリアしてください。 準備ができたら"次へ"を選んでください。
FullInstallation=完全インストール
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=コンパクト
CustomInstallation=カスタム
NoUninstallWarningTitle=コンポーネントが既に存在します。
NoUninstallWarning=セットアップは次のコンポーネントが既に存在していることを検出しました:%n%n%1%n%n選択を解除したコンポーネントがアンインストールされることはありません%n%n続けますか？
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel=選択されたコンポーネントをインストールするには[mb] MBの空き容量が必要です。

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=追加作業
SelectTasksDesc=追加作業を選択してください。
SelectTasksLabel2=[name]のインストール中にセットアップにさせたい追加作業を選んで、"次へ"をクリックしてください。

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=スタートメニューのフォルダーを選んでください。
SelectStartMenuFolderDesc=どこにショートカットを作成しますか？
SelectStartMenuFolderLabel3=セットアップは次のフォルダーにショートカットを作ります。
SelectStartMenuFolderBrowseLabel=続けるには"次へ"をクリックしてください。ほかの場所を選ぶには"参照"してください。
MustEnterGroupName=フォルダーの名前を指定してください。
GroupNameTooLong=フォルダー名又はパスが長すぎます。
InvalidGroupName=指定されたフォルダー名は無効です。
BadGroupName=フォルダー名には次の文字は使用できません:%n%n%1
NoProgramGroupCheck2=スタートメニューにショートカットを作成しません(&D)

; *** "Ready to Install" wizard page
WizardReady=インストールの準備ができました。
ReadyLabel1=セットアップは[name]のインストールする準備ができました。
ReadyLabel2a=続けるには"インストール"をクリックしてください、設定を変更するには"戻る"をクリックしてください。
ReadyLabel2b=続けるには"インストール"をクリックしてください。
ReadyMemoUserInfo=ユーザー情報:
ReadyMemoDir=インストール先:
ReadyMemoType=セットアップの種類:
ReadyMemoComponents=選択されたコンポーネント:
ReadyMemoGroup=スタートメニュー:
ReadyMemoTasks=追加作業:

; *** "Preparing to Install" wizard page
WizardPreparing=インストールの準備
PreparingDesc=セットアップは[name]のインストールを準備します。
PreviousInstallNotCompleted=古いバージョンのインストール又はアンインストールが終了していません。[name]のセットアップを実行する前に再起動してください。
CannotContinue=セットアップは完了しませでした。"キャンセル"をクリックして終了してください。

; *** "Installing" wizard page
WizardInstalling=インストール中
InstallingLabel=[name]のセットアップが終了するまでしばらくお待ちください。

; *** "Setup Completed" wizard page
FinishedHeadingLabel=[name]のセットアップウィザードを完了しています。
FinishedLabelNoIcons=[name]のインストールが終了しました。
FinishedLabel=[name]のインストールが終了しました。
ClickFinish=セットアップを閉じるには、"終了"をクリックしてください。
FinishedRestartLabel=[name]のセットアップを完了するためには、再起動する必要があります。今すぐ、再起動しますか？
FinishedRestartMessage=[name]のセットアップを完了するためには、再起動する必要があります。%n%n今すぐ、再起動しますか？
ShowReadmeCheck=はい、READMEを読みます。
YesRadio=はい、再起動します。(&Y)
NoRadio=いいえ、後で再起動します。(&N)
; used for example as 'Run MyProg.exe'
RunEntryExec=%1を実行
; used for example as 'View Readme.txt'
RunEntryShellExec=%1を表示

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=セットアップは次のディスクを必要としています。
SelectDiskLabel2=ディスク%1を挿入してOKをクリックしてください.%n%nもしも必要なファイルがほかの場所にある場合はその場所を指定、又は参照して選んでください。
PathLabel=&Path:
FileNotInDir2=ファイル"%1"が"%2"に存在しません。正しいディスクを挿入するか又は、ただし場所を指定してください。
SelectDirectoryLabel=次のディスクがセットされている場所を指定してください。

; *** Installation phase messages
SetupAborted=セットアップは完了しませんでした%n%n問題を修正してから、再度セットアップを実行してください。
EntryAbortRetryIgnore=Click Retry to try again, Ignore to proceed anyway, or Abort to cancel installation.

; *** Installation status messages
StatusCreateDirs=フォルダーを作成しています...
StatusExtractFiles=ファイルを解凍しています...
StatusCreateIcons=ショートカットを作成しています...
StatusCreateIniEntries=設定ファイルに項目を追加しています...
StatusCreateRegistryEntries=レジストリーに項目を追加しています...
StatusRegisterFiles=ファイルを登録しています...
StatusSavingUninstall=アンインストール情報を保存しています...
StatusRunProgram=インストールを終了しています...
StatusRollback=変更を復帰しています...

; *** Misc. errors
ErrorInternal2=内部エラー: %1
ErrorFunctionFailedNoCode=%1 の失敗
ErrorFunctionFailed=%1 の失敗; エラーコード %2
ErrorFunctionFailedWithMessage=%1 の失敗; エラーコード %2.%n%3
ErrorExecutingProgram=次のファイルを実行できませんでした:%n%1

; *** Registry errors
ErrorRegOpenKey=次のレジストリーキーを開けませんでした:%n%1\%2
ErrorRegCreateKey=次のレジストリーキーを作成できませんでした:%n%1\%2
ErrorRegWriteKey=次のレジストリーキーを変更できませんでした:%n%1\%2

; *** INI errors
ErrorIniEntry=設定ファイル"%1"に項目を追加できませんでした。

; *** File copying errors
FileAbortRetryIgnore=もう一度試みるには"再試行"、スキップするには"無視"(お勧めできません)、 又は中止するには"キャンセル"をクリックして下さい。
FileAbortRetryIgnore2=もう一度試みるには"再試行"、スキップするには"無視"(お勧めできません)、 又は中止するには"キャンセル"をクリックして下さい。
SourceIsCorrupted=このファイルは破損しています。
SourceDoesntExist=ファイル"%1"は存在しません。
ExistingFileReadOnly=既存のファイルは読取専用属性が付加されています%n%nもう一度試みるには"再試行""、スキップするには"無視"(お勧めできません)、 又は中止するには"キャンセル"をクリックして下さい。
ErrorReadingExistingDest=既存のファイルの読み込み中にエラーが発生しました:
FileExists=ファイルが既に存在します%n%n上書きしますか？
ExistingFileNewer=既存のファイルはインストールされるファイルより新しいです。既存のファイルを残しておく事が望ましいです。%n%n既存のファイルをそのまま使いますか？
ErrorChangingAttr=既存のファイルの属性を変更中にエラーが発生しました:
ErrorCreatingTemp=インストール先のフォルダーを作成中にエラーが発生しました:
ErrorReadingSource=インストールするファイルの読み込み中にエラーが発生しました:
ErrorCopying=ファイルのコピー中にエラーが発生しました:
ErrorReplacingExistingFile=既存のファイルの入れ替え中にエラーが発生しました:
ErrorRestartReplace=RestartReplace failed:
ErrorRenamingTemp=インストール先のファイル名の変更中にエラーが発生しました:
ErrorRegisterServer=DLL/OCXファイル %1 の登録が出来ませんでした。
ErrorRegisterServerMissingExport=DLL内にDllRegisterServerファンクションがありません。
ErrorRegisterTypeLib=タイプ・ライブラリー %1 の登録が出来ませんでした。

; *** Post-installation errors
ErrorOpeningReadme=READMEファイルの読み込み中にエラーが発生しました。
ErrorRestartingComputer=セットアップはコンピューターの再起動を出来ませんでした。手動で再起動してください。

; *** Uninstaller messages
UninstallNotFound=File "%1"が存在しません。アンインストールできません。
UninstallOpenError=File "%1"を開けません、アンインストールできません。
UninstallUnsupportedVer=アンインストールファイル"%1"のバージョンが違います。アンインストールできません。
UninstallUnknownEntry=アンインストールログ(%1)に未確認項目を発見しました。
ConfirmUninstall=%1とそのコンポーネントをアンインストールしますか？
UninstallOnlyOnWin64=このアンインストーラーは64ビットWindows以外では実行できません。
OnlyAdminCanUninstall=このソフトウェアのアンインストールには管理者権限が必要です。
UninstallStatusLabel=%1を削除しています。しばらくお待ちください。
UninstalledAll=%1のアンインストールが正常に終了しました。
UninstalledMost=%1のアンインストールが終了しました%n%n一部のファイルを削除できませんでした。これらのファイルは手動で削除できます。
UninstalledAndNeedsRestart=%1のアンインストールを完了するには、再起動する必要があります。%n%n今すぐ再起動しますか？
UninstallDataCorrupted=ファイル"%1"は破損しています、アンインストール出来ませんでした。

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=共有ファイルを削除しますか?
ConfirmDeleteSharedFile2=次の共有ファイルを必要とされないことを検出しました。削除しますか？%n%nもし、ほかのアプリケーションソフトで必要な場合そのソフトは正常に動作しなくなります。もし不確かな場合は"いいえ"を選んでください。%n%n不必要なファイルが残っていても支障はありません。
SharedFileNameLabel=ファイル名:
SharedFileLocationLabel=場所:
WizardUninstalling=アンインストール状態
StatusUninstalling=%1を案インストールしています...

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1 version %2
AdditionalIcons=追加アイコン:
CreateDesktopIcon=デスクトップに追加(&d)
CreateQuickLaunchIcon=クイックランチにアイコンを追加(&Q)
ProgramOnTheWeb=%1 on the Web
UninstallProgram=%1のアンインストール
LaunchProgram=%1の起動
AssocFileExtension=%1を%2ファイルと関連付けます(&A)
AssocingFileExtension=%1を%2関連付けています...
