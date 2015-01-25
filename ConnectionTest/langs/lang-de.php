<?php
/*
   Copyright (c) jlh, 2004 - 2008
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
   Language file for: Deutsch (German)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English, for other languages do it like this:
   "Hawaiian translation by Hulahula." (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Deutsche Übersetzung: jlh");

define("_MSG_TITLE", "Shareaza Verbindungstest");
define("_MSG_TITLE_WITH_LINK", "<a href='http://shareaza.sourceforge.net/'>Shareaza</a> Verbindungstest");
define("_MSG_LANGUAGES", "Diesen Verbindungstest gibt es auch in:");
define("_MSG_FOOTER", "Fragen und Kommentare zu diesem Verbindungstest sind auf
den <a href='http://shareaza.sourceforge.net/phpbb/'>Shareaza Foren</a> willkommen.");

/* stats line */
define("_MSG_STATS", "Dieser Test wurde %d&nbsp;mal durchgeführt, seit dem %s.
Erfolgsrate: Beide&nbsp;Tests:&nbsp;%d%%, nur&nbsp;TCP:&nbsp;%d%%,
nur&nbsp;UDP:&nbsp;%d%%, weder noch:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Fehler");

define("_MSG_ERROR_PORT_INVALID", "Der eingegebene Port ist kein gültiger Port;
gültige Ports befinden sich im Bereich 1&nbsp;-&nbsp;65535.  Falls Sie nicht
wissen welchen Port Sie eingeben müssen, lesen Sie weiter unten, um es
herauszufinden.");

define("_MSG_ERROR_PORT_ZERO", "Der eingegebene Port ist kein gültiger Port.
Wenn die Porteinstellung in Shareaza '0' lautet, dann bedeutet dies, dass
Shareaza jedes Mal ein zufällig ausgewählter Port benutzt.  Wenn Sie eine
Firewall oder einen Router haben, dann können Sie diese Einstellung nicht
benutzen.  In diesem Fall entfernen Sie bei <b>Zufällig</b> das Häckchen und
geben einen Port ein, den Sie dann entsprechend an der Firewall und/oder am
Router konfigurieren.  Der Standardport ist 6346, aber irgend ein anderer
funktioniert genauso.  Wenn Sie keine Firewall und keinen Router haben, dann
können Sie die Option <b>Zufällig</b> benutzen, aber dieser Verbindungstest
funktioniert nur, wenn Sie angeben können, welchen Port Shareaza momentan
gerade benutzt.  <i>Bemerkung: Wenn Sie die Porteinstellung in Shareaza ändern,
dann müssen Sie die Netzwerkverbindungen trennen und neu verbinden, damit die
Änderung wirksam wird.</i>");

/* when IP can't be found, unlikely to happen */
define("_MSG_ERROR_IP", "Der Verbindungstest konnte Ihre IP Adresse nicht
herausfinden.  Bitte melden Sie dieses Problem.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Falls Sie bei der Konfiguration Ihrer Firewall oder
Ihres Routers Hilfe benötigen, dann besuchen Sie folgende Seite der Shareaza Wiki:
<a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters/de'>FAQ:&nbsp;Router/Firewall</a>.");

/* progress box */
define("_MSG_PROGRESS", "Der Verbindungstest ist jetzt am laufen; dies kann
einige Sekunden dauern, bitte gedulden Sie solange...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Details");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Die Details sind verborgen, hier klicken um sie anzuzeigen.");
define("_MSG_DETAIL_HIDE", "Details verbergen.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Testergebnisse");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Dieser Test wurde nicht durchgeführt.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Bitte melden Sie dieses Problem mit einer Kopie der Details.");

/* internal error */
define("_MSG_RESULTS_IE", "Dieser Test konnte nicht durchgeführt werden, ein interner Fehler trat auf.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "Eine Zeitüberschreitung ist während der TCP Verbindung aufgetreten.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Das kann daran liegen, dass Ihre Firewall
(oder Ihr Router) 'stealth' ist, d.h. es wurde überhaupt keine Antwort
bekommen.  Konfigurieren Sie diese Geräte für Shareaza's Port.");

define("_MSG_RESULTS_TCP_REFUSED_1", "Die TCP Verbindung wurde verweigert, dieser Port ist geschlossen.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Entweder ist Ihre Firewall oder Ihr
Router für diesen Port nicht konfiguriert worden, oder dann braucht ihn
momentan kein Programm.  Stellen Sie sicher dass Ihre Geräte korrekt für
Shareaza konfiguriert sind und dass Shareaza läuft, mit den Netzwerken am
verbinden ist und auf diesem Port konfiguriert ist.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "Die TCP Verbindung wurde von Ihrem Computer akzeptiert.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Aber keine Antwort wurde
zurückgesendet.  Wahrscheinlich ist trotzdem alles in Ordnung.  Stellen Sie
sicher, dass Shareaza diesen Port benutzt und nicht ein anderes Programm.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "Die TCP Verbindung wurde von Ihrem
Computer akzeptiert und eine Antwort wurde erhalten.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Dies bedeutet, dass andere Clients
problemlos Verbindungen zu Ihnen aufbauen können.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "Ein unbekannter Fehler trat auf, während
versucht wurde, eine Verbindung mit ihrem Computer aufzubauen.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Es wurde von Ihrem Computer keine Antwort empfangen.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Dafür kann es verschiedene Gründe geben,
z.B. könnte Ihre Firewall oder Ihr Router nicht korrekt für Shareaza
konfiguriert sein, oder Shareaza ist nicht auf diesen Port konfiguriert worden,
oder ist momentan nicht am verbinden.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Eine Antwort wurde von Ihrem Computer empfangen!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Dies bedeutet, dass Shareaza problemlos
UDP Pakete vom Netzwerk empfangen kann.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Gratulation, alles scheint in bester Ordnung
zu sein und Shareaza sollte problemlos funktionieren.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Mindestens ein Problem wurde
festgestellt und Sie müssen wahrscheinlich Ihre Fireall oder Ihr Router für
Shareaza konfigurieren.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Den Verbindungstest durchführen");

define("_MSG_FORM_TEXT", "Damit dieser Verbindungstest funktioniert, muss
Shareaza gestartet werden.  Wenn dies nicht der Fall ist, starten Sie es jetzt
und verbinden Sie es mit den Netzwerken.  (Es ist nicht nötig dass es
erfolgreich verbindet, es genügt, wenn es versucht zu verbinden.)  Dann geben
Sie hier unten ins Feld den Port ein, auf dem Shareaza konfiguriert ist und
klicken auf 'Test'.");

define("_MSG_FORM_IP", "Ihre IP ist %s.");
define("_MSG_FORM_PROXY", "Ihr Proxy ist %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Port:");
define("_MSG_FORM_LTO", "Lange Zeitüberschreitung");
/* the button */
define("_MSG_FORM_BUTTON", "Test");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Wie finde ich heraus, welchen Port Shareaza benutzt?");
define("_MSG_FORM_PORT_HOWTO", "Wenn Sie nicht wissen auf welchem Port Shareaza
konfiguriert wurde, dann wechseln Sie zu Shareaza und wählen
<b>Einstellungen...</b> aus dem Menü <b>Extras</b>.  Dort klicken Sie auf der
linken Seite auf <b>Internet&nbsp;&gt;&nbsp;Verbindung</b> und sehen rechts
unter <b>Port</b> nach.  Das ist der Port, den Shareaza benutzt.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Bemerkung zu Proxy Servern");
define("_MSG_FORM_PROXY_HOWTO", "Falls Sie Ihren Browser konfiguriert haben,
einen Proxy Server zu benutzen, dann kann es sein, dass dieser Test ihre IP
nicht feststellen kann.  Es sollte nur selten der Fall sein, aber wenn es den
Proxy nicht erkennt, dann testet es die IP des Proxies, statt ihrer IP, was
natürlich den Test zum scheitern führt.  Schalten Sie den Proxy temporär aus,
um diesen Test durchzuführen.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Muss ich 'Lange Zeitüberschreitung' einschalten?");
define("_MSG_FORM_LTO_HOWTO", "Das ist normalerweise nicht nötig.  Der
Verbindungstest wartet maximal 5 Sekunden auf eine Antwort für die TCP und UDP
Tests.  In normalen Fällen genügt das, aber falls Ihr Computer im Stress ist,
sehr beschäftigt ist oder Ihre Internetverbindung nicht ganz zuverlässig ist,
dann reichen diese 5 Sekunden manchmal nicht aus.  Wenn Sie 'Lange
Zeitüberschreitung' einschalten, dann wartet der Test 10 Sekunden lang.
Natürlich braucht der Test dann eventuell länger um durchgeführt zu werden.");

/* ======== end of file */

?>