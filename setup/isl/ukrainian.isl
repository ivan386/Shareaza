; *** Inno Setup version 4.1.8+ Ukrainian messages ***
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.htm
;
; *** Translated by Andriy Slezenko, student@uninet.kiev.ua ***
; *** Web-site: http://www.uninet.kiev.ua/~student ***
;
; *** Updated from version 3.0.2 to version 3.0.5 to version 4.1.8
; *** by Roman Kovbasiouk, romkov@ua.fm
;
; Last update: 2004.03.01

[LangOptions]
LanguageName=Ukrainian
LanguageID=$0422
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
SetupAppTitle=Інсталяція
SetupWindowTitle=Інсталяція - %1
UninstallAppTitle=Видалення
UninstallAppFullTitle=Видалення - %1

; *** Misc. common
InformationTitle=Інформація
ConfirmTitle=Підтвердження
ErrorTitle=Помилка

; *** SetupLdr messages
SetupLdrStartupMessage=Інсталяція програми "%1". Продовжити?
LdrCannotCreateTemp=Неможливо створити тимчасовий файл. Інсталяцію перервано
LdrCannotExecTemp=Неможливо виконати файл у тимчасовому каталозі. Інсталяцію перервано

; *** Startup error messages
LastErrorMessage=%1.%n%nПомилка %2: %3
SetupFileMissing=Файл %1 відсутній у каталозі інсталяції. Усуньте проблему або отримайте нову копію програми.
SetupFileCorrupt=Файли інсталяції пошкоджені. Отримайте нову копію програми.
SetupFileCorruptOrWrongVer=Файли інсталяції пошкоджені або несумісні з поточною версією програми інсталяції. Усуньте проблему або отримайте нову копію програми.
NotOnThisPlatform=Цю програму неможливо виконати в %1.
OnlyOnThisPlatform=Ця програма має виконуватися в %1.
WinVersionTooLowError=Ця програма потребує %1 версії %2 або вище.
WinVersionTooHighError=Ця програма не може бути інстальована в %1 версії %2 або вище.
AdminPrivilegesRequired=Для продовження інсталяції необхідно увійти в систему з правами адміністратора.
PowerUserPrivilegesRequired=Для продовження iнсталяцiї необхiдно увiйти в систему як адмiнiстратор або член групи Power Users.
SetupAppRunningError=Програма інсталяції виявила, що виконується програма "%1".%n%nЗакрийте всі її копії, потім натисніть "ОК" для продовження, або "Скасувати" для виходу.
UninstallAppRunningError=Програма видалення (деінсталятор) виявила, що виконується програма "%1".%n%nЗакрийте всі її копії, потім натисніть "ОК" для продовження, або "Скасувати" для виходу.

; *** Misc. errors
ErrorCreatingDir=Неможливо створити каталог "%1"
ErrorTooManyFilesInDir=Неможливо створити файл в каталозі "%1", тому що каталог містить забагато файлів

; *** Setup common messages
ExitSetupTitle=Вихід з інсталяції
ExitSetupMessage=Інсталяція не завершена. Якщо Ви її не завершите, програма не буде інстальована.%n%nВи можете виконати інсталяцію наступного разу для її завершення.
AboutSetupMenuItem=&Про...
AboutSetupTitle=Про інсталяцію
AboutSetupMessage=%1 версія %2%n%3%n%n%1 домівка:%n%4
AboutSetupNote=

; *** Buttons
ButtonBack=< &Назад
ButtonNext=&Далі >
ButtonInstall=&Встановити
ButtonOK=OK
ButtonCancel=Скасувати
ButtonYes=&Так
ButtonYesToAll=Так для &усіх
ButtonNo=&Ні
ButtonNoToAll=Н&і для усіх
ButtonFinish=&Завершити
ButtonBrowse=&Огляд...
ButtonWizardBrowse=О&гляд...
ButtonNewFolder=&Створити нову папку

; *** "Select Language" dialog messages
SelectLanguageTitle=Виберiть мову iнсталяцiї
SelectLanguageLabel=Виберiть мову для використання протягом iнсталяцiї

; *** Common wizard text
ClickNext=Натисніть "Далі" для продовження або "Скасувати" для виходу.
BeveledLabel=
BrowseDialogTitle=Виберiть папку
BrowseDialogLabel=Виберiть папку зi списку, потiм натиснiть OK.
NewFolderName=Нова папка

; *** "Welcome" wizard page
WelcomeLabel1=Вас вітає програма інсталяції "[name]"
WelcomeLabel2=[name/ver] буде проінстальована на Ваш комп`ютер.%n%nРекомендується закрити всі інші програми перед тим, як продовжити інсталяцію.

; *** "Password" wizard page
WizardPassword=Серійний номер
PasswordLabel1=Для продовження інсталяції потрібен серійний номер. Якщо Ви не маєте його, то уважно прочитайте Ліцензійну угоду і потім зверніться до розроблювачів.
PasswordLabel3=Будь ласка, введіть серійний номер і натисніть "Далі" для продовження інсталяції.%nУВАГА! Серійний номер чутливий до регістру літер!
PasswordEditLabel=&Серійний номер:
IncorrectPassword=Ви ввели невірний серійний номер. Спробуйте ще раз.

; *** "License Agreement" wizard page
WizardLicense=Ліцензійна угода
LicenseLabel=Будь ласка, перш, ніж продовжити, ознайомтеся з наступною інформацією.
LicenseLabel3=Будь ласка, прочитайте Ліцензійну угоду. Для того, щоб продовжити інсталяцію, Ви повинні погодитися з умовами даної угоди.
LicenseAccepted=Я &згодний з угодою
LicenseNotAccepted=Я &не погоджуюся з угодою

; *** "Information" wizard pages
WizardInfoBefore=Інформація
InfoBeforeLabel=Будь ласка, перш, ніж продовжити, ознайомтеся з наступною інформацією.
InfoBeforeClickLabel=Коли будете готові, натисніть "Далі" для продовження інсталяції.
WizardInfoAfter=Додаткова інформація
InfoAfterLabel=Будь ласка, перш, ніж продовжити, ознайомтеся з наступною інформацією.
InfoAfterClickLabel=Коли будете готові, натисніть "Далі" для продовження інсталяції.

; *** "User Information" wizard page
WizardUserInfo=Інформація про користувача
UserInfoDesc=Будь ласка введіть наступну інформацію.
UserInfoName=&Ім`я користувача:
UserInfoOrg=&Організація:
UserInfoSerial=&Серійний номер:
UserInfoNameRequired=Ви повинні ввести ім`я.

; *** "Select Destination Location" wizard page
WizardSelectDir=Виберіть мiсце на диску для інсталяції
SelectDirDesc=Куди проінсталювати програму "[name]"?
SelectDirLabel3="[name]" буде встановлено у вказану папку.
SelectDirBrowseLabel=Для продовження натисніть "Далі". Якщо бажаєте вибрати iншу папку, натиснiть "Огляд".
DiskSpaceMBLabel=Потрiбно щонайменше [mb] МБ вільного дискового простору.
ToUNCPathname=Неможливо проінсталювати в шлях, який використовує UNC. Якщо Ви намагаєтесь інсталювати програму в мережі, необхідно підключити мережний диск.
InvalidPath=Необхідно вказати повний шлях разом з літерою диску, наприклад:%nC:\Program Files\App
InvalidDrive=Вибраного Вами диску не існує. Виберіть інший.
DiskSpaceWarningTitle=Недостатньо місця на диску
DiskSpaceWarning=Інсталяція потребує щонайменше %1 Кб вільного місця, але на вибраному диску доступно лише %2 Кб.%n%nПродовжити всеодно?
DirNameTooLong=Iм'я папки або шляху занадто довгi.
InvalidDirName=Помилкове iм'я папки.
BadDirName32=Назва папки не може містити наступних символів:%n%n%1
DirExistsTitle=Папка вже існує
DirExists=Папка:%n%n%1%n%nвже існує. Проінсталювати в неї?
DirDoesntExistTitle=Папка не існує
DirDoesntExist=Папка:%n%n%1%n%nне існує. Створити?

; *** "Select Program Group" wizard page
WizardSelectComponents=Виберіть компоненти
SelectComponentsDesc=Які компоненти треба інсталювати?
SelectComponentsLabel2=Виберіть компоненти для інсталяції;%nзніміть позначку з тих, що Ви не бажаєте інсталювати. Коли будете готові, натисніть "Далі" для продовження інсталяції.
FullInstallation=Повна інсталяція
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=Компактна інсталяція
CustomInstallation=Вибіркова інсталяція
NoUninstallWarningTitle=Наявні компоненти
NoUninstallWarning=Програма інсталяції виявила, що наступні компоненти вже проінстальовані на Вашому комп`ютері:%n%n%1%n%nНе позначення цих компонентів не видалить їх.%n%nПродовжити?
ComponentSize1=%1 Кб
ComponentSize2=%1 Мб
ComponentsDiskSpaceMBLabel=Вибрані компоненти потребують щонайменше [mb] МБ місця на диску.

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Вибір додаткових завдань
SelectTasksDesc=Які додаткові завдання потрібно виконати?
SelectTasksLabel2=Виберіть додаткові завдання, які необхідно виконати при інсталяції програми "[name]", потім натисніть "Далі".

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=Виберіть програмну групу
SelectStartMenuFolderDesc=Де програма інсталяції має створити значки нової програми?
SelectStartMenuFolderLabel3=Iнсталяцiйна програма створить значки у вказанiй папцi меню "Пуск".
SelectStartMenuFolderBrowseLabel=Для продовження натисніть "Далі". Якщо бажаєте вибрати iншу папку, натиснiть "Огляд".
NoIconsCheck=Не створювати значки
MustEnterGroupName=Необхідно ввести ім`я групи.
GroupNameTooLong=Iм'я папки або шляху занадто довгi.
InvalidGroupName=Помилкове iм'я папки.
BadGroupName=Ім`я групи не може містити жодного з наступних символів:%n%n%1
NoProgramGroupCheck2=&Не створювати каталог у меню "Пуск"

; *** "Ready to Install" wizard page
WizardReady=Все підготовлено для інсталяції
ReadyLabel1=Все підготовлено для інсталяції "[name]" на Ваш комп`ютер.
ReadyLabel2a=Натисніть "Інсталювати" для продовження інсталяції або "Назад", якщо необхідно подивитися чи змінити настроювання.
ReadyLabel2b=Натисніть "Інсталювати" для того, щоб продовжити інсталяцію.
ReadyMemoUserInfo=Інформація про користувача:
ReadyMemoDir=Мiсце призначення:
ReadyMemoType=Тип інсталяції:
ReadyMemoComponents=Вибрані компоненти:
ReadyMemoGroup=Каталог у меню "Пуск":
ReadyMemoTasks=Додаткові завдання:

; *** "Preparing to Install" wizard page
WizardPreparing=Підготування до інсталяції
PreparingDesc=Все підготовлено для інсталяції "[name]" на Ваш комп`ютер.
PreviousInstallNotCompleted=Інсталяція/видалення попередньої програми не завершене. Ви повинні перезавантажити комп`ютер для її завершення.%n%nПісля перезавантаження комп`ютера, продовжіть інсталяцію "[name]" знову.
CannotContinue=Інсталяцію неможливо продовжити. Будь ласка натисніть "Скасувати" для виходу.

; *** "Installing" wizard page
WizardInstalling=Інсталяція
InstallingLabel=Зачекайте, доки "[name]" інсталюється на Ваш комп`ютер.

; *** "Setup Completed" wizard page
FinishedHeadingLabel=Завершується iнсталяцiя програми "[name]"
FinishedLabelNoIcons=Інсталяцію програми "[name]" завершено.
FinishedLabel=Інсталяцію "[name]" завершено. Програму можна виконати створеним під час інсталяції значком.
ClickFinish=Натисніть "Завершити" для виходу з програми інсталяції.
FinishedRestartLabel=Для завершення інсталяції "[name]" необхідно перезавантажити Ваш комп`ютер. Зробити це негайно?
FinishedRestartMessage=Для завершення інсталяції "[name]" необхідно перезавантажити Ваш комп`ютер.%n%nЗробити це негайно?
ShowReadmeCheck=Так, я бажаю переглянути README файл
YesRadio=&Так, перезавантажити комп`ютер зараз
NoRadio=&Ні, я зроблю це пізніше
; used for example as 'Run MyProg.exe'
RunEntryExec=Виконати %1
; used for example as 'View Readme.txt'
RunEntryShellExec=Прочитати %1

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=Потрібен наступний диск
SelectDiskLabel2=Будь ласка, вставте диск %1 і натисніть ОК.%n%nЯкщо файли на цьому диску знаходяться в каталозі, що відрізняється від зазначеного нижче, введіть вірний шлях або натисніть "Огляд".
PathLabel=&Шлях:
FileNotInDir2=Файл "%1" не був знайдений в "%2". Вставте необхідний диск або виберіть інший каталог.
SelectDirectoryLabel=Будь ласка, визначте місцезнаходження наступного диску.

; *** Installation phase messages
SetupAborted=Інсталяцію не завершено.%n%nУсуньте проблеми, що виникли, та виконайте інсталяцію знову.
EntryAbortRetryIgnore=Натисніть "Повторити" - для того, щоб спробувати знову, Пропустити - для продовження або Перервати - для скасування інсталяції.

; *** Installation status messages
StatusCreateDirs=Створення каталогів...
StatusExtractFiles=Розпакування файлів...
StatusCreateIcons=Створення значків...
StatusCreateIniEntries=Внесення змін до INI файлів...
StatusCreateRegistryEntries=Внесення змін до реєстру...
StatusRegisterFiles=Реєстрація файлів...
StatusSavingUninstall=Збереження інформації для деінсталяції...
StatusRunProgram=Завершення інсталяції...
StatusRollback=Повернення зроблених змін...

; *** Misc. errors
ErrorInternal2=Внутрішня помилка: %1
ErrorFunctionFailedNoCode=невдача %1
ErrorFunctionFailed=невдача %1; код %2
ErrorFunctionFailedWithMessage=невдача %1; код %2.%n%3
ErrorExecutingProgram=Неможливо виконати файл:%n%1

; *** Registry errors
ErrorRegOpenKey=Помилка при відкритті ключа системного реєстру:%n%1\%2
ErrorRegCreateKey=Помилка при створенні ключа системного реєстру:%n%1\%2
ErrorRegWriteKey=Помилка запису до системного реєстру:%n%1\%2

; *** INI errors
ErrorIniEntry=Помилка запису у файл INI "%1".

; *** File copying errors
FileAbortRetryIgnore=Натисніть "Повторити" - для того, щоб спробувати знову, "Пропустити" - щоб пропустити файл (не рекомендується), або "Перервати" - для скасування інсталяції.
FileAbortRetryIgnore2=Натисніть "Повторити" - для того, щоб спробувати знову, "Пропустити" - щоб продовжити (не рекомендується), або "Перервати" - для скасування інсталяції.
SourceIsCorrupted=Вихідний файл пошкоджений
SourceDoesntExist=Вихідного файлу "%1" не існує
ExistingFileReadOnly=Існуючий файл має атрибут "Тільки для читання".%n%nНатисніть "Повторити" - для того, щоб зняти атрибут і спробувати знову, "Пропустити" - щоб пропустити файл, або "Перервати" - для скасування інсталяції.
ErrorReadingExistingDest=Відбулася помилка при спробі прочитати існуючий файл:
FileExists=Файл вже існує.%n%nПерезаписати його?
ExistingFileNewer=Існуючий файл новіше, ніж той, що потрібно проінсталювати. Рекомендується залишити існуючий.%n%nЗалишити існуючий файл?
ErrorChangingAttr=Відбулася помилка при спробі змінити атрибут існуючого файла:
ErrorCreatingTemp=Відбулася помилка при спробі створити файл в каталозі:
ErrorReadingSource=Відбулася помилка при спробі прочитати вихідний файл:
ErrorCopying=Відбулася помилка при спробі копіювання файла:
ErrorReplacingExistingFile=Відбулася помилка при спробі замінити існуючий файл:
ErrorRestartReplace=Неможливо замінити файл:
ErrorRenamingTemp=Відбулася помилка при спробі перейменувати файл в каталозі:
ErrorRegisterServer=Неможливо зареєструвати DLL/OCX: %1
ErrorRegisterServerMissingExport=Не знайдено експортування DllRegisterServer
ErrorRegisterTypeLib=Неможливо зареєструвати бібліотеку типів: %1

; *** Post-installation errors
ErrorOpeningReadme=Неможливо відкрити файл README:
ErrorRestartingComputer=Програмі інсталяції не вдалося перезавантажити комп`ютер. Зробіть краще це самі.

; *** Uninstaller messages
UninstallNotFound=Файл "%1" не існує. Видалення неможливе.
UninstallOpenError=Файл "%1" не вдається вiдкрити. Видалення неможливе
UninstallUnsupportedVer=Файл деінсталяції "%1" має непридатний формат для цієї версії програми видалення. Видалення неможливе.
UninstallUnknownEntry=Невідомий запис (%1) у файлі деінсталяції
ConfirmUninstall=Ви дійсно бажаєте повністю видалити програму "%1" та всі її компоненти?
OnlyAdminCanUninstall=Дана програма може бути видалена тільки користувачем з правами адміністратора.
UninstallStatusLabel=Будь ласка, зачекайте поки %1 видаляється з Вашого комп`ютера.
UninstalledAll=Програму "%1" було успішно видалено з Вашого комп`ютера.
UninstalledMost=Видалення програми "%1" завершено.%n%nДеякі елементи не вдалося видалити. Вони можуть бути видалені власноручно.
UninstalledAndNeedsRestart=Для завершення деінсталяції "%1", потрібно перезавантажити комп`ютер.%n%nЗробити це зараз?
UninstallDataCorrupted=Файл "%1" пошкоджений. Видалення неможливе

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=Видалити загальні (shared) файли?
ConfirmDeleteSharedFile2=Система показує, що наступний загальний (shared) файл більше не використовується жодною програмою. Чи бажаєте Ви видалити цей файл?%n%n%1%n%nЯкщо все ж деякі програми його використовують, а файл буде видалено, то ці програми не зможуть нормально функціонувати. Якщо Ви не впевнені, натисніть "Ні". Залишення файлу в системі не буде мати негативних наслідків.
SharedFileNameLabel=Ім`я файлу:
SharedFileLocationLabel=Місцезнаходження:
WizardUninstalling=Стан видалення
StatusUninstalling=Видаляється %1...
