<?php
/*
   Copyright (c) __peer__, omega09 2005 - 2008
   This file is part of the Shareaza Connection Test

   The Shareaza Connection Test is free software; you can redistribute
   it and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Shareaza Connection Test is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Shareaza; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Also visit http://www.gnu.org/
*/

/*
   Shareaza Connection Test
   Language file for: עברית (Hebrew)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 1);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "תרגום לעברית על ידי __peer__ ו-omega09");

/* the title (used in various places) */
define("_MSG_TITLE", "בדיקת החיבור של שרזה");
define("_MSG_TITLE_WITH_LINK", "בדיקת החיבור של שרזה");

define("_MSG_LANGUAGES", "בדיקת החיבור זמינה גם בשפות הבאות:");
define("_MSG_FOOTER", "שאלות והערות לגבי הבדיקה יתקבלו בברכה
<a href='http://shareaza.sourceforge.net/phpbb/'>בפורום שרזה</a>.");

/* stats line */
define("_MSG_STATS", "מאז %d הבדיקה התבצעה %s פעמים.
,%d%%&nbsp;:רק בבדיקת הורדה ,%d%%&nbsp;:אחוזי הצלחה: בבדיקת ההעלאה וגם בהורדה
.%d%%&nbsp;:באף בדיקה ,%d%%&nbsp;:רק בבדיקת העלאה");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "תקלה");

define("_MSG_ERROR_PORT_INVALID", "מספר הפתחה שהכנסת לא חוקי
1&nbsp;-&nbsp;65535  אנא הכנס יציאה חוקית בטווח של
<i>'איך לדעת באיזו פתחה שרזה משתמשת'</i> :אנא קרא להלן אם אינך יודע באיזו פתחה שרזה משתמשת.");

define("_MSG_ERROR_PORT_ZERO", "מספר הפתחה שהכנסת לא חוקי.
אם הגדרת הפתחה בשרזה קבועה כ-'0', אזי יציאה רנדומלית בשימוש.
אם יש לך חומת אש או נתב אז אין לך אפשרות להשתמש בפתחה רנדומלית.
במקרה זה בטל את הסימון ב-<b>רנדומלי</b> והכנס מספר יציאה, אשר אותה
תגדיר על חומת האש או הראוטר שלך. יציאת ברירת המחדל היא 6346,
אבל כל מספר פתחה אחר יתאים. אם אין לך חומת אש או נתב, תוכל להשתמש
במספר פתחה רנדומלי אם תרצה. אך בדיקה זאת יכולה להיות מבוצעת
רק אם תדע למצוא באיזה מספר פתחה שרזה משתמשת ברגע זה.
הערה: כשמשנים מספר פתחה בהגדרות של שרזה, עלייך להתחבר מחדש בכדי
שהשינויים ייכנסו לתוקף.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "בדיקת החיבור לא הצליחה לזהות את מספר ה-IP שלך, אנא דווח על התקלה.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "<a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters'>שאלות נפוצות:&nbsp;נתבים/חומות אש</a>. :אם אתה צריך עזרה בהגדרת חומת האש או הנתב שלך בקר בעמוד זה במערכת הויקי");

/* progress box */
define("_MSG_PROGRESS", "בדיקת החיבור מתבצעת כעת, הבדיקה תמשך מספר שניות, אנא המתן...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "רישום פרטים");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "רישום הפרטים מוסתר, לחץ כאן להצגתו.");
define("_MSG_DETAIL_HIDE", "הסתר את רישום הפרטים.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "תוצאות");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "הבדיקה לא בוצעה.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "אנא דווח על התקלה בצירוף קובץ רישום הפרטים המלא.");

/* internal error */
define("_MSG_RESULTS_IE", "ביצוע הבדיקה נכשל בשל תקלה פנימית במערכת.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "חיבור ה-TCP לא בוצע בפרק זמן סביר");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "הדבר יכול לנבוע מחומת אש או נתב שלא הוגדרו כראוי לעבוד עם שרזה");

define("_MSG_RESULTS_TCP_REFUSED_1", "חיבור ה-TCP נדחה, היציאה נסגרה.");
define("_MSG_RESULTS_TCP_REFUSED_2", "פתחה זו לא הוגדרה בחומת האש או הנתב
או שאין תוכנית שמשתמשת בה כעת. ודא שחומת האש או הנתב שלך מוגדרים כדאוי,
בכדי לתת נגישות למספר הפתחה שמוגדרת בשרזה.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "חיבור ה-TCP התקבל על ידי המחשב שלך,");
define("_MSG_RESULTS_TCP_CONNECTED_2", "אך לא התקבלה אף תשובה לבקשה שנשלחה
ככל הנאה הדבר תקין. רק ודא ששרזה משתמשת במספר הפתחה שהכנסת,
ולא תוכנה אחרת.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "חיבור ה-TCP התקבל על ידי המחשב שלך,
והבקשה שנשלחה נענתה.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "משתמשים אחרים ברשת
יכולים להתחבר אליך כראוי.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "הייתה תקלה בעת החיבור לכתובת ה-IP שלך מסיבה לא ידועה.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "חיבור ה-UDP לא התקבל על ידי המחשב שלך.");
define("_MSG_RESULTS_UDP_NOTHING_2", "הדבר יכול לנבוע ממספר סיבות, או שחומת האש/נתב לא מוגדר
כראוי לעבודה עם שרזה , או ששרזה לא מוגדרת לעבוד עם המספר הפתחה שהכנסת כאן,
או ששרזה לא מנסה כרגע להתחבר לאף רשת שיתוף.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "חיבור ה-UDP התקבל על ידי המחשב שלך");
define("_MSG_RESULTS_UDP_ANSWERED_2", "משתמשים אחרים ברשת
יכולים להתחבר אליך כראוי");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "מזל טוב, נראה שהכל עובד כראוי ושרזה אמורה לפעול היטב.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "לפחות בעיה אחת התגלתה וכנראה שעלייך
להגדיר כראוי את חומת האש או הנתב שלך בכדי לעבוד היטב עם שרזה.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "בצע את בדיקת החיבור");

define("_MSG_FORM_TEXT", "בכדי שהבדיקה תתבצע, עלייך לוודא ששרזה רצה
.אם שרזה לא רצה כרגע, הפעל אותה ונסה להתחבר לרשתות. (ללא קשר להצלחת החיבור) לאחר מכן הכנס את מספר הפתחה של שרזה בשדה שלהלן
ולחץ על לחצן 'בצע בדיקה'.");

define("_MSG_FORM_IP", "מספר ה-IP שלך הוא %s");
define("_MSG_FORM_PROXY", "הפרוקסי שלך הוא %s");
/* just before the port box */
define("_MSG_FORM_PORT", "מספר פתחה:");
define("_MSG_FORM_LTO", "חריגה גדולה מזמן סביר");
/* the button */
define("_MSG_FORM_BUTTON", "בצע בדיקה");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "איך לדעת באיזה מספר פתחה שרזה משתמשת?");
define("_MSG_FORM_PORT_HOWTO", "אם אינך יודע באיזה מספר פתחה שרזה משתמשת,
עבור להגדרות שרזה על ידי בחירת <b>שרזה&nbsp;הגדרות שרזה</b> מתפריט <b>כלים</b>.
לאחר מכן לחץ על <b>אינטרנט&nbsp;&gt;&nbsp;חיבורים</b> והסתכל על שדה
<b>מספר פתחה</b> מימין. זהו מספר הפתחה שבו שרזה משתמשת..");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "הערה לגבי שרתי פרוקסי");
define("_MSG_FORM_PROXY_HOWTO", "אם יש לך שרת פרוקסי מוגדר בדפדפן ייתכן שבדיקת החיבור לא תוכל לזהות את כתובת ה-IP של המחשב שלך. אם הבדיקה לא מזהה שהחיבור דרך פרוקסי היא תבדוק את ה-IP של שרת הפרוקסי במקום את שלך והבדיקה תכשל. בטל זמנית את הגדרת הפרוקסי כדי לבצע את הבדיקה.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "האם עלי לאפשר 'חריגה גדולה מזמן סביר '?");
define("_MSG_FORM_LTO_HOWTO", "בדרך כלל אין צורך בכך. כברירת מחדל הבדיקה תחכה 5 שניות כדי לקבל תוצאות של TCP ו-UDP. בדרך כלל זמן זה מספיק, אבל אם המחשב שלך עמוס או עסוק או אם חיבור האינטרנט שלך לא אמין ייתכן שיהיה צריך יותר מ-5 שניות. אם אפשרת חריגה גדולה מזמן סביר הבדיקה תחכה 10 שניות. כמובן, שלבדיקה ייקח יותר זמן להסתיים.");

/* ======== end of file */

?>