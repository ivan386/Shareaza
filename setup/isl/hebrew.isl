;*** Inno Setup version 4.2.2+ English messages ***
;
; To download user-contributed translations of this file, go to:
;   http://www.jrsoftware.org/is3rdparty.php
;
; Note: When translating this text, do not add periods (.) to the end of
; messages that didn't have them already, because on those messages Inno
; Setup adds the periods automatically (appending a period would result in
; two periods being displayed).
;
; $jrsoftware: issrc/Files/Default.isl,v 1.58 2004/04/07 20:17:13 jr Exp $

[LangOptions]
LanguageName=Hebrew
LanguageID=$040D
LanguageCodePage=1255
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
SetupAppTitle=התקנה
SetupWindowTitle=%1 - התקנה
UninstallAppTitle=הסרת התקנה
UninstallAppFullTitle=%1 הסרת התקנה של

; *** Misc. common
InformationTitle=מידע
ConfirmTitle=אישור
ErrorTitle=שגיאה

; *** SetupLdr messages
SetupLdrStartupMessage=?‏להמשיך‎ ‏מעוניין‎ ‏הינך‎ ‏האם‎ %1 ‏את‎ ‏יתקין‎ ‏הקובץ
LdrCannotCreateTemp=נכשל ליצור תיקייה זמנית, ההתקנה הופסקה
LdrCannotExecTemp=נכשל להריץ קובץ בתיקייה הזמנית, ההתקנה הופסקה

; *** Startup error messages
LastErrorMessage=%1.%n%n %2: %3 :שגיאה
SetupFileMissing=‏התוכנית‎ ‏של‎ ‏תקין‎ ‏עותק‎ ‏השג‎ ‏או‎ ‏לתיקייה‎ ‏הקובץ‎ ‏את‎ ‏הכנס‎ ‏אנא‎ ‏ההתקנה‎, ‏בספריית‎ ‏נמצא‎ ‏לא‎ %1 ‏הקובץ
SetupFileCorrupt=קבצי ההתקנה פגומים, אנא השג עותק תקין של התוכנית
SetupFileCorruptOrWrongVer=קבצי ההתקנה פגומים או לא מתאימים לגרסה של התוכנית, אנא תקן את הבעיה או השג עותק תקין של התוכנה
NotOnThisPlatform=%1 התוכנית לא תוכל לפעול על
OnlyOnThisPlatform=%1 התוכנית חייבת להיות מופעלת על
WinVersionTooLowError=%1 ‏של‎ ‏ואילך‎ %2 ‏גרסא‎ ‏דרוש‎ ‏לתוכנית
WinVersionTooHighError=%1 ‏של‎ ‏ואילך‎ %2 ‏גרסא‎ ‏על‎ ‏מותקנת‎ ‏להיות‎ ‏יכולה‎ ‏אינה‎ ‏התוכנית
AdminPrivilegesRequired=עלייך להיות רשום כמנהל בכדי להתקין את התוכנה
PowerUserPrivilegesRequired=עלייך להיות רשום כמנהל או כמשתמש מתקדם בכדי להתקין את התוכנה
SetupAppRunningError=תוכנית ההתקנה זיהתה שהתהליך %1 כרגע מופעל %n%nאנא סגור את התהליך ולחץ על 'הבא' להמשך, או לחץ 'ביטול' ליציאה.
UninstallAppRunningError=תוכנית ההתקנה זיהתה שהתהליך %1 כרגע מופעל %n%nאנא סגור את התהליך ולחץ על 'הבא' להמשך, או לחץ 'ביטול' ליציאה.

; *** Misc. errors
ErrorCreatingDir="%1" תוכנית ההתקנה נכשלה ליצור את הספרייה
ErrorTooManyFilesInDir=לא ניתן ליצור את הקובץ בתיקייה "%1" כי היא מכילה יותר מדי קבצים

; *** Setup common messages
ExitSetupTitle=צא מההתקנה
ExitSetupMessage=תהליך ההתקנה לא הושלם, אם תצא עכשיו, התוכנית לא תהיה מותקנת%n%nתוכל להפעיל מחדש את תוכנית ההתקנה פעם אחרת להשלמת ההתקנה%n%n?לצאת מתוכנית ההתקנה
AboutSetupMenuItem=א&ודות ההתקנה
AboutSetupTitle=אודות התקנה
AboutSetupMessage=%1 , %2%n%3%n%n%1 :גרסא %n%4 :עמוד בית
AboutSetupNote=

; *** Buttons
ButtonBack=< הק&ודם
ButtonNext=הב&א >
ButtonInstall=הת&קנה
ButtonOK=אישור
ButtonCancel=ביטול
ButtonYes=כ&ן
ButtonYesToAll=&כן להכל
ButtonNo=&לא
ButtonNoToAll=לא לה&כל
ButtonFinish=ס&יים
ButtonBrowse=&סייר
ButtonWizardBrowse=סי&יר
ButtonNewFolder=צ&ור תיקייה חדשה

; *** "Select Language" dialog messages
SelectLanguageTitle=בחר את שפת ההתקנה
SelectLanguageLabel=בחר את השפה לשימוש במהלך ההתקנה:

; *** Common wizard text
ClickNext=.לחץ על 'הבא' להמשך או על 'ביטול' ליציאה מההתקנה
BeveledLabel=
BrowseDialogTitle=סייר בספרייה
BrowseDialogLabel=בחר ספרייה ברשימה להלן, ולחץ על אישור
NewFolderName=ספרייה חדשה

; *** "Welcome" wizard page
WelcomeLabel1=[name] ברוך הבא לתוכנית ההתקנה של
WelcomeLabel2=,על מחשבך [name/ver] התוכנית תתקין את%n%n.מומלץ שתסגור את כל התוכניות האחרות לפני שתמשיך

; *** "Password" wizard page
WizardPassword=סיסמה
PasswordLabel1=ההתקנה מוגנת על-ידי סיסמה
PasswordLabel3=אנא הכנס סיסמה, ולחץ על 'הבא' כדי להמשיך. סיסמאות הן תלויות-רשית
PasswordEditLabel=סיסמ&א
IncorrectPassword=הסיסמה שהקלדת אינה נכונה, אנא נסה שנית

; *** "License Agreement" wizard page
WizardLicense=הסכם רשיון
LicenseLabel=אנא קרא את המידע החשוב הבא לפני שתמשיך
LicenseLabel3=אנא קרא את הסכם הרשיון, עלייך להסכים לכל תנאיו לפני שתמשיך
LicenseAccepted=אני מק&בל את ההסכם
LicenseNotAccepted=אני  ל&א מקבל את ההסכם

; *** "Information" wizard pages
WizardInfoBefore=מידע
InfoBeforeLabel=אנא קרא את המידע החשוב הבא לפני שתמשיך
InfoBeforeClickLabel='כשאתה מוכן להמשיך עם ההתקנה, לחץ על 'הבא
WizardInfoAfter=מידע
InfoAfterLabel=אנא קרא את המידע החשוב הבא לפני שתמשיך
InfoAfterClickLabel='כשאתה מוכן להמשיך עם ההתקנה, לחץ על 'הבא

; *** "User Information" wizard page
WizardUserInfo=מידע משתמש
UserInfoDesc=אנא הכנס את המידע שלך
UserInfoName=ש&ם משתמש:
UserInfoOrg=א&ירגון:
UserInfoSerial=מ&ספר סיראלי:
UserInfoNameRequired=עלייך להכניס שם משתמש

; *** "Select Destination Location" wizard page
WizardSelectDir=בחר מיקום ידע
SelectDirDesc=?[name] היכן תרצה להתקין את
SelectDirLabel3=לספריה להלן [name] התוכנית תתקין את
SelectDirBrowseLabel='להמשך לחץ על 'הבא' אם תרצה לבחור ספרייה אחרת לחץ על 'סייר
DiskSpaceMBLabel=‏בדיסק‎ ‏פנוי‎ ‏מקום‎ ‏של‎ [mb] MB ‏לפחות‎ ‏דרוש
ToUNCPathname=Setup cannot install to a UNC pathname. If you are trying to install to a network, you will need to map a network drive.
InvalidPath=You must enter a full path with drive letter; for example:%n%nC:\APP%n%nor a UNC path in the form:%n%n\\server\share
InvalidDrive=The drive or UNC share you selected does not exist or is not accessible. Please select another.
DiskSpaceWarningTitle=אין מספיק מקום פנוי בדיסק
DiskSpaceWarning=Setup requires at least %1 KB of free space to install, but the selected drive only has %2 KB available.%n%n ?האם תרצה להמשיך בכל זאת
DirNameTooLong=שם/כתובת הספרייה ארוך מדי
InvalidDirName=שם הספרייה איננו חוקי
BadDirName32=%n%n%1:שם תיקייה לא יכול להכיל את התווים הבאים
DirExistsTitle=הספרייה כבר קיימת
DirExists=?כבר קיימת, האם תרצה להתקין לספרייה בכל זאת %n%n%1%n%n הספרייה
DirDoesntExistTitle=הספרייה אינה קיימת
DirDoesntExist=?לא קיימת, האם תרצה ליצור את הספרייה %n%n%1%n%n הספרייה

; *** "Select Components" wizard page
WizardSelectComponents=בחר רכיבים
SelectComponentsDesc=אילו רכיבים תרצה להתקין
SelectComponentsLabel2=סמן את הרכיבים שתרצה להתקין, ובטל את הסימון באלה שלא תרצה להתקין, לחץ על 'הבא' להמשך בתהליך ההתקנה
FullInstallation=התקנה מלאה
; if possible don't translate 'Compact' as 'Minimal' (I mean 'Minimal' in your language)
CompactInstallation=התקנה בסיסית
CustomInstallation=התקנה מותאמת אישית
NoUninstallWarningTitle=רכיבים שקיימים
NoUninstallWarning=תוכנית ההתקנה זיהתה שהרכיבים הבאים כבר מותקנים על מחשבך:%n%n%1%n%nאם תבטל את סימונם של רכיבים אלה הם לא יותקנו%n%n?האם תרצה להמשיך בכל מקרה
ComponentSize1=%1 KB
ComponentSize2=%1 MB
ComponentsDiskSpaceMBLabel=[mb] MB :לפי ההגדרות התקנה הנוכחיות נדרשת הכמות הבאה של מקום פנוי בדיסק

; *** "Select Additional Tasks" wizard page
WizardSelectTasks=Select Additional Tasks
SelectTasksDesc=?אילו משימות נוספות דרושות להתבצע
SelectTasksLabel2='תבצע ולחץ על 'הבא [name] בחר את המשימות הנוספות שתרצה שההתקנת

; *** "Select Start Menu Folder" wizard page
WizardSelectProgramGroup=בחר תיקיית תפריט התחל
SelectStartMenuFolderDesc=?היכן על ההתקנה ליצור את קישורי הקיצור לתוכנה
SelectStartMenuFolderLabel3=תוכנית ההתקנה תיצור את קיצורי הדרך בתיקיית תפריט התחל הבאה
SelectStartMenuFolderBrowseLabel='להמשך לחץ על 'הבא' אם תרצה לבחור תיקייה אחרת לחץ על 'סייר
NoIconsCheck=א&ל תיצור קיצורי דרך
MustEnterGroupName=אתה מוכרח להכניס שם תיקייה
GroupNameTooLong=שם או כתובת התיקייה ארוך מדי
InvalidGroupName=שם התיקייה אינו חוקי
BadGroupName=%n%n%1 :שם התיקייה לא יכל להכיל את התווים
NoProgramGroupCheck2=אל תיצ&ור תיקייה בתפריט התחל

; *** "Ready to Install" wizard page
WizardReady=מוכן להתקנה
ReadyLabel1=על מחשבך [name] תוכנית ההתקנה כעת מוכנה להתקין את
ReadyLabel2a=לחץ על 'התקנה' להמשך בהתקנה או לחץ על 'הקודם' אם תרצה לשנות הגדרות
ReadyLabel2b=לחץ על 'התקנה' להמשך בהתקנה
ReadyMemoUserInfo=מידע משתמש:
ReadyMemoDir=מיקום יעד:
ReadyMemoType=סוג ההתקנה:
ReadyMemoComponents=רכיבים נבחרים:
ReadyMemoGroup=תייקית תפריט התחל:
ReadyMemoTasks=משימות נוספות:

; *** "Preparing to Install" wizard page
WizardPreparing=מתכונן להתקנה
PreparingDesc=על מחשבך [name] תוכנית ההתקנה מתכוננת להתקין את
PreviousInstallNotCompleted=הסרה או התקנה של תוכנית קודמת לא הושלמה, יהיה עלייך להפעיל מחדש את המחשב לביצוע התקנה%n%n.[name] לאחר הפעלה מחדש הפעל את תוכנית ההתקנה שנית בכדי לסיים את תהליך ההתקנה של
CannotContinue=תוכנית ההתקנה אינה יכולה להמשיך, אנא לך על 'ביטול' ליציאה

; *** "Installing" wizard page
WizardInstalling=מתקין
InstallingLabel=על מחשבך [name] אנא המתן בעת שתוכנית ההתקנה מתקינה את

; *** "Setup Completed" wizard page
FinishedHeadingLabel=[name] משלים את תהליך ההתקנה של
FinishedLabelNoIcons=על מחשבך [name] תוכנית ההתקנה סיימה להתקין את
FinishedLabel=על מחשבך, ניתן להפעיל את התוכנית על ידי קיצורי הדרך שנוצרו [name] תוכנית ההתקנה סיימה להתקין את
ClickFinish=לחץ על 'סיום' ליציאה מתוכנית ההתקנה
FinishedRestartLabel=?תוכנית ההתקנה חייבת להפעיל מחדש את המחשב, האם תרצה להפעיל מחדש כעת [name] לסיום ההתקנה של
FinishedRestartMessage=.תוכנית ההתקנה חייבת להפעיל מחדש את המחשב [name] לסיום ההתקנה של %n%n ?האם תרצה להפעיל מחדש כעת
ShowReadmeCheck=Readme כן, אני מעוניין לקרוא את קובץ המידע
YesRadio=כ&ן, הפעל מחדש את המחשב
NoRadio=ל&א, אני אפעיל את המחשב מחדש מאוחר יותר
; used for example as 'Run MyProg.exe'
RunEntryExec=%1 הרץ את
; used for example as 'View Readme.txt'
RunEntryShellExec=%1 הצג את

; *** "Setup Needs the Next Disk" stuff
ChangeDiskTitle=ההתקנה צריכה את הדיסק הבא
SelectDiskLabel2=Please insert Disk %1 and click OK.%n%nIf the files on this disk can be found in a folder other than the one displayed below, enter the correct path or click Browse.
PathLabel=&Path:
FileNotInDir2=The file "%1" could not be located in "%2". Please insert the correct disk or select another folder.
SelectDirectoryLabel=אנא ציין את המיקום של הדיסק הבא

; *** Installation phase messages
SetupAborted=ההתקנה לא הושלמה %n%n אנא תקן את הבעיה והפעל את ההתקנה שנית
EntryAbortRetryIgnore=לחץ על 'נסה שנית' בכדי לנסות שוב, לחץ על 'התעלם' בכדי להמשיך בכל מקרה או לחץ על 'בטל' לביטול ההתקנה

; *** Installation status messages
StatusCreateDirs=...יוצר ספריות
StatusExtractFiles=...מחלץ קבצים
StatusCreateIcons=...יוצר קיצורי דרך
StatusCreateIniEntries=...INI יוצר ערכי רישום בקבצי
StatusCreateRegistryEntries=...יוצר ערכי רישום ברגיסטרי
StatusRegisterFiles=...יוצר רישום לקבצים
StatusSavingUninstall=...שומר מידע להסרת התקנה
StatusRunProgram=...מסיים התקנה
StatusRollback=...מבטל את השינויים

; *** Misc. errors
ErrorInternal2=%1 :שגיאה פנימית
ErrorFunctionFailedNoCode=%1 נכשלה הפעולה של
ErrorFunctionFailed=%1 נכשלה הפעולה של;code %2
ErrorFunctionFailedWithMessage=%1 נכשלה הפעולה של; code %2.%n%3
ErrorExecutingProgram=נכשל להריץ את הקובץ:%n%1

; *** Registry errors
ErrorRegOpenKey=שגיאה בפתיחת ערך לרגיסטרי:%n%1\%2
ErrorRegCreateKey=שגיאה ביצירת ערך לרגיסטרי:%n%1\%2
ErrorRegWriteKey=שגיאה ברישום ערך לרגיסטרי:%n%1\%2

; *** INI errors
ErrorIniEntry=Error creating INI entry in file "%1".

; *** File copying errors
FileAbortRetryIgnore=לחץ על 'נסה שנית' בכדי לנסות שוב, לחץ על 'התעלם' בכדי לדלג על הקובץ (לא מומלץ) או לחץ על 'בטל' לביטול ההתקנה
FileAbortRetryIgnore2=לחץ על 'נסה שנית' בכדי לנסות שוב, לחץ על 'התעלם' בכדי להמשיך (לא מומלץ) בכל מקרה או לחץ על 'בטל' לביטול ההתקנה
SourceIsCorrupted=קובץ המקור פגום
SourceDoesntExist=לא קיים "%1" קובץ המקור
ExistingFileReadOnly=The existing file is marked as read-only.%n%nClick Retry to remove the read-only attribute and try again, Ignore to skip this file, or Abort to cancel installation.
ErrorReadingExistingDest=An error occurred while trying to read the existing file:
FileExists=The file already exists.%n%nWould you like Setup to overwrite it?
ExistingFileNewer=The existing file is newer than the one Setup is trying to install. It is recommended that you keep the existing file.%n%nDo you want to keep the existing file?
ErrorChangingAttr=שגיאה ארעה בעת נסיון לשנות מאפיינים של קובץ בתיקיית היעד:
ErrorCreatingTemp=שגיאה ארעה בעת נסיון ליצור קובץ בתיקיית היעד:
ErrorReadingSource=שגיאה ארעה בעת נסיון להעתיק  לקרוא את הקובץ:
ErrorCopying=שגיאה ארעה בעת נסיון להעתיק קובץ:
ErrorReplacingExistingFile=שגיאה ארעה בעת נסיון להחליף קובץ קיים בתיקיית היעד:
ErrorRestartReplace=RestartReplace failed:
ErrorRenamingTemp=שגיאה ארעה בעת נסיון לשנות את שם הקובץ בתיקיית היעד:
ErrorRegisterServer=Unable to register the DLL/OCX: %1
ErrorRegisterServerMissingExport=DllRegisterServer export not found
ErrorRegisterTypeLib=%1 :נכשל להכניס לרישום את

; *** Post-installation errors
ErrorOpeningReadme=Readme שגיאה ארעה בנסיון לפתוח את קובץ
ErrorRestartingComputer=ההתקנה נכשלה להפעיל מחדש את המחשב, אנא עשה זאת באופן ידני

; *** Uninstaller messages
UninstallNotFound=הקובץ  "%1" חסר, לא ניתן להסיר התקנה
UninstallOpenError=הקובץ  "%1" לא ניתן לפתיחה, לא ניתן להסיר התקנה
UninstallUnsupportedVer=The uninstall log file "%1" is in a format not recognized by this version of the uninstaller. Cannot uninstall
UninstallUnknownEntry=נמצא רישום לא ידוע (%1) בקובץ הרישום של תוכנית הסרת ההתקנה
ConfirmUninstall=?האם אתה בטוח שתרצה להסיר את %1 וכל מרכיביו
OnlyAdminCanUninstall=התוכנית יכולה להיות מוסרת אך ורק על ידי משתמש עם זכויות ניהול
UninstallStatusLabel=אנא המתן בעת שהתוכנית %1 מוסרת ממחשבך
UninstalledAll=%1 הוסרה בהצלחה התוכנית
UninstalledMost=הסרת ההתקנה של %1 הושלמה.%n%n כמה אלמנטים לא נמחקו, ניתן למחוק אותם באופן ידני
UninstalledAndNeedsRestart=להשלמת ההתקנה של %1 עלייך להפעיל מחדש את המחשב %n%n ?האם תרצה להפעיל מחדש כעת
UninstallDataCorrupted=הקובץ "%1" פגום, לא ניתן להסיר את ההתקנה

; *** Uninstallation phase messages
ConfirmDeleteSharedFileTitle=?להסיר קובץ משותף
ConfirmDeleteSharedFile2=?לפי המערכת הקובץ לא בשימוש על ידי אף תוכנה, האם אתה מעוניין להסירו%n%nאם הקובץ עדיין בשימוש על ידי תוכנה כלשהי, הסרת הקובץ עלולה לגרום לפגיעה בפונקציונליות התוכנה, השארת הקובץ על המחשב לא תגרום לאף נזק
SharedFileNameLabel=שם קובץ:
SharedFileLocationLabel=מיקום:
WizardUninstalling=מצב הסרת התקנה
StatusUninstalling=...%1 מסיר התקנה של

; The custom messages below aren't used by Setup itself, but if you make
; use of them in your scripts, you'll want to translate them.

[CustomMessages]

NameAndVersion=%1 גרסא %2 של
AdditionalIcons=:סמלים נוספים
CreateDesktopIcon= הצג סמל על ש&ולחן עבודה
CreateQuickLaunchIcon=הצג סמל על ה&פעלה מהירה
ProgramOnTheWeb=%1 :אתר אינטרנט
UninstallProgram=%1 הסר התקנה של
LaunchProgram=%1 הרץ את
AssocFileExtension=%1 שייך את הס&יומת %2  עם
AssocingFileExtension=%1 משייך את הס&יומת %2  עם
