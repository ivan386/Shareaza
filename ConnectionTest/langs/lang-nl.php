<?php
/*
   Copyright (c) jonne, 2004 - 2008
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
   Language file for: Nederlands (Dutch)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula." (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Nederlandse vertaling door Jonne");

/* the title (used in various places) */
define("_MSG_TITLE", "Shareaza Verbindingstest");
define("_MSG_TITLE_WITH_LINK", "<a href='http://shareaza.sourceforge.net/'>Shareaza</a> Verbindingstest");

define("_MSG_LANGUAGES", "Deze verbindingstest is ook beschikbaar in:");
define("_MSG_FOOTER", "Vragen en suggesties over deze test zijn altijd welkom in de
<a href='http://shareaza.sourceforge.net/phpbb/'>Shareaza Forums</a>.");

/* stats line */
define("_MSG_STATS", "Deze test werd reeds %d keer uitgevoerd sinds %s. Geslaagde tests: Beide tests %d%%, enkel TCP: %d%%, enkel UDP: %d%%, geen: %d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Fout");

define("_MSG_ERROR_PORT_INVALID", "Het poortnummer dat je ingaf is geen geldige poort. Een geldige poort is een getal tussen 1 en 65535.");

define("_MSG_ERROR_PORT_ZERO", "De poort die je ingaf is geen geldige poort. Indien de poortinstelling in Shareaza '0' is, betekent dit dat een willekeurige poort gebruikt wordt. Indien je een firewall of router gebruikt, kan je geen willekeurige poort gebruiken. In dat geval moet je <b>willekeurig</b> uitvinken in de Shareaza instellingen en een poortnummer invullen, dat je ook instelt in je firewall en/of router. De standaard poort is 6346, maar je mag ook een andere poort kiezen. Als je geen firewall of router hebt, mag je willekeurige poort gebruiken als je wil, maar deze test werkt enkel als je weet welke poort Shareaza op dit moment gebruikt. <i>Let op: als je het poortnummer verandert, moet je de verbinding met het netwerk verbreken en opnieuw verbinden om de verandering toe te passen.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "De test kon je IP-adres niet te weten komen. Gelieve dit probleem te melden.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Indien je hulp nodig hebt met het instellen van je firewall of router, bekijk dan deze pagina in de
wiki: <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters/nl'>FAQ:&nbsp;Routers/Firewalls</a>.");

/* progress box */
define("_MSG_PROGRESS", "De connectietest is nu bezig; dit kan enkele seconden duren, even geduld...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Gedetailleerd logboek");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Het gedetailleerd logboek is verborgen, klik hier om het te laten verschijnen.");
define("_MSG_DETAIL_HIDE", "Verberg het gedetailleerd logboek.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Resultaten");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "De test werd niet gedaan.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Gelieve dit probleem te melden met een kopie van het gedetailleerde logboek.");

/* internal error */
define("_MSG_RESULTS_IE", "Deze test kon niet uitgevoerd worden door een interne fout.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "De TCP verbinding heeft de wachttijd overschreden");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Mogelijke oorzaken zijn een 'stealth' firewall, of een slecht ingestelde router.");

define("_MSG_RESULTS_TCP_REFUSED_1", "De TCP verbinding werd geweigerd, de poort is gesloten.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Ofwel werd geen firewall of router op deze poort ingesteld, ofwel maakt er momenteel geen enkele applicatie gebruik van. Zorg ervoor dat je firewall of router ingesteld is om deze poort niet te blokkeren of deze poort te forwarden, en dat Shareaza op deze poort is ingesteld en verbonden is.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "De TCP verbinding werd aanvaard door je computer.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Maar er werd geen antwoord gegeven aan ons verzoek. Dit is waarschijnlijk geen probleem. Controleer of Shareaza deze poort wel gebruikt, en geen ander programma.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "De TCP verbinding werd aanvaard door je computer, en ons verzoek werd beantwoord.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Dit betekent dat andere clients op het netwerk probleemloos met je kunnen verbinden");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "Er was een onbekende fout bij het verbinden met je IP-adres");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Geen antwoord werd ontvangen van je client.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Dit kan verschillende redenen hebben, zoals een slecht ingestelde router of firewall, of Shareaza loopt niet op deze poort, of is niet verbonden met de netwerken.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Een antwoord werd verkregen van je client!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Dit betekent dat Shareaza probleemloos UDP paketten kan ontvangen van het netwerk.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Proficiat. Alles lijkt in orde en Shareaza zou probleemloos met het netwerk moeten verbinden.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Er is minstens één probleem gevonden, en je moet naar alle waarschijnlijkheid je firewall en router instellen.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Doe de verbindingstest");

define("_MSG_FORM_TEXT", "Deze test werkt enkel als Shareaza aanstaat. Als het nog niet aanstaat, start Shareaza en laat deze verbinden met 1 of meer netwerken (Shareaza moet niet verbonden zijn, als het probeert te verbinden is het al goed). Daarna vul je Shareaza's poortnummer in en klik je op 'Test'.");

define("_MSG_FORM_IP", "Je IP is %s.");
define("_MSG_FORM_PROXY", "Je proxy is %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Poort:");
define("_MSG_FORM_LTO", "Lange wachttijd");
/* the button */
define("_MSG_FORM_BUTTON", "Test");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Hoe weet je welke poort Shareaza gebruikt?");
define("_MSG_FORM_PORT_HOWTO", "Indien je niet weet welke poort Shareaza gebruikt, moet je onder <b>hulpmiddelen</b> in de <b>Shareaza instellingen</b> kijken. Klik daar op <b>verkeer>verbinding</b>, en kijk in het veld <b>poort</b>. Dit is het poortnummer dat Shareaza gebruikt.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Over proxy servers...");
define("_MSG_FORM_PROXY_HOWTO", "Als je in je browserinstellingen een proxyserver ingesteld hebt, is het mogelijk dat deze test je IP-adres niet kan te weten komen. Indien de test er niet in slaagt om je proxy te detecteren, zal die het IP-adres van je proxy testen, in plaats van je eigen IP, wat natuurlijk niet zal werken. Schakel je proxy tijdelijk uit om deze test te doen.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Moet ik 'lange wachttijd' aanvinken?");
define("_MSG_FORM_LTO_HOWTO", "Gewoonlijk is dit niet nodig. Deze test wacht standaard 5 seconden voor de TCP en UDP tests om te zien of alles zoals verwacht werkt. Dit zou in de meeste gevallen genoeg moeten zijn, maar als je computer onder zware druk staat, met iets anders bezig is, of als je een onbetrouwbare internetverbinding hebt, zou het kunnen dat Shareaza meer tijd nodig heeft om op de test te reageren, waardoor 5 seconden niet genoeg is. Als je de lange wachttijd aanvinkt, zal de test 10 seconden wachten. De test neemt dan natuurlijk meer tijd in beslag.");

/* ======== end of file */

?>