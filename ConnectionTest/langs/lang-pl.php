<?php
/*
   Copyright (c) Brov, 2004 - 2008
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
   Language file for: Polski (Polish)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula." (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Polskie tłumaczenie: Brov");

/* the title (used in various places) */
define("_MSG_TITLE", "Test Połączenia Shareazy");
define("_MSG_TITLE_WITH_LINK", "Test Połączenia <a href='http://shareaza.sourceforge.net/'>Shareazy</a>");

define("_MSG_LANGUAGES", "Ten test jest dostępny również w następujących językach:");
define("_MSG_FOOTER", "Pytania i komentarze odnośnie tego testu są mile widziane na
<a href='http://shareaza.sourceforge.net/phpbb/'>Forum Shareazy</a>.");

/* stats line */
define("_MSG_STATS", "Ten test został przeprowadzony %d&nbsp;razy od %s.  Sukcesywność:
 Oba&nbsp;testy:&nbsp;%d%%, tylko&nbsp;TCP:&nbsp;%d%%,
tylko&nbsp;UDP:&nbsp;%d%%, żaden:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Błąd");

define("_MSG_ERROR_PORT_INVALID", "Numer portu, który podałeś jest nieprawudłowy, wpisz proszę poprawny z zakresu 1&nbsp;-&nbsp;65535.");

define("_MSG_ERROR_PORT_ZERO", "Numer portu, który podałeś jest nieprawidłowy.
Jeśli w ustawieniach Shareazy widzisz port '0', znaczy to, że Shareaza używa losowego portu.
Jeśli posiadasz firewalla bądź router nie możesz używać losowych portów.
W tym przypadku odznacz opcje <b>Random</b> i wprowadź numer portu, który
skonfigurujesz na swoim firewallu bądź routerze. Domyślnym portem jest 6346, ale każdy port będzie działać.
Jeśli nie masz firewalla bądź routera, możesz używać portów losowych jeśli chcesz, ale ten test może być wykonany
jeśli wiesz jak znaleźć port, który jest teraz używany przez Shareaze.
<i>Notka: Jeśli zmienisz numer portu w ustawieniach, musisz się rozłączyć i połączyć ponownie, aby zmiany odniosły skutek.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "Ten test nie może znaleźć Twojego adresu IP. Proszę zgłosić ten problem.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Jeśli potrzebujesz pomocy z konfiguracją Twojego firewalla bądź routera - zajrzyj do
wiki: <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters/pl'>FAQ:&nbsp;Routers/Firewalls</a>.");

/* progress box */
define("_MSG_PROGRESS", "Test połączenia jest właśnie wykonywany; może to zająć kilka sekund, proszę czekać...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Dziennik szczegółowy");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Dziennik szczegółowy jest ukryty, kliknij tutaj, aby go pokazać.");
define("_MSG_DETAIL_HIDE", "Ukryj dziennik szczegółowy.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Wyniki");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Ten test nie został wykonany.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Proszę zgłosić ten problem z pełną kopią dziennika szczegółowego.");

/* internal error */
define("_MSG_RESULTS_IE", "Ten test nie może być wykonany, ponieważ wystąpił błąd wewnętrzny.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "Przekroczony został limit czasu połączenia TCP.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Może to być spowodowane przez niepoprawnie skonfigurowany firewall lub router.");

define("_MSG_RESULTS_TCP_REFUSED_1", "Połączenie TCP zostało odrzucone, port jest zamknięty.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Firewall bądź router nie został skonfigurowany na tym porcie, lub żadna aplikacja aktualnie nie używa tego portu.
Upewnij się czy firewall bądź router jest skonfigurowany tak, aby nie blokował bądź przekierowywał ten port i czy Shareaza działa i jest skonfigurowana, aby używać tego portu.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "Połączenie TCP zostało zaakceptowane przez Twój komputer.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Ale nie otrzymano odpowiedzi na wysłane żądanie.
Prawdopodobnie wszystko jest dobrze. Po prostu upewnij się, że Shareaza, a nie inna aplikacja używa tego portu. ");

define("_MSG_RESULTS_TCP_ANSWERED_1", "Połączenie TCP zostało zaakceptowane przez Twój komputer i otrzymano odpowiedź na wysłane żądanie.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Oznacza to, że inni mogą się poprawnie łączyć do Ciebie.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "Wystąpił nieokreślony błąd przy próbie połączenia się z Twoim komputerem.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Nie odebrano żadnej odpowiedzi od Twojego klienta.");
define("_MSG_RESULTS_UDP_NOTHING_2", "To może mieć różne przyczyny, takie jak Twój firewall bądź router nie jest poprawnie skonfigurowany dla Shareazy, lub Shareaza nie używa tego portu, lub nie łączy się do sieci.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Otrzymano odpowiedź od Twojego klienta!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Oznacza to, że Shareaza może odbierać pakiety UDP z sieci.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Gratulacje, wszystko wygląda dobrze i Shareaza powinna działać poprwanie.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Przynajmniej jeden problem został wykryty. Prawdopodobnie potrzebujesz skonfigurować Twój firewall bądź router dla Shareazy.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Zrób test połączenia");

define("_MSG_FORM_TEXT", "Aby ten test zadziałał, musisz mieć Shareaze włączoną.
Jeśli jeszcze nie jest, uruchom ją i połącz się do sieci.
(Nie ma znaczenia czy się poprawnie połączy czy nie, wystarczy, że próbuje.)  Następnie wpisz w poniższe pole numer portu Shareazy i kliknij 'Sprawdź!'.");

define("_MSG_FORM_IP", "Twój adres IP to %s.");
define("_MSG_FORM_PROXY", "Twój serwer PROXY to %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Port:");
define("_MSG_FORM_LTO", "Długi czas oczekiwania");
/* the button */
define("_MSG_FORM_BUTTON", "Sprawdź!");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Jak sprawdzić jakiego portu używa Shareaza?");
define("_MSG_FORM_PORT_HOWTO", "Jeśli nie wiesz jakiego portu używa Shareaza to
przełącz się na Shareaze i otwórz okno z ustawieniami wybierając <b>Shareaza&nbsp;Settings</b> z menu <b>Tools</b>.  Następnie w lewym panelu kliknij na <b>Internet&nbsp;&gt;&nbsp;Connection</b> i popatrz na pole
<b>Port</b> z prawej strony.  To jest port, który jest używany przez Shareaze.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Notka o serwerach proxy");
define("_MSG_FORM_PROXY_HOWTO", "Jeśli włączyłeś serwer proxy w ustawieniach Twojej przeglądarki, to jest możliwe, że ten test nie będzie w stanie znaleźć adresu IP Twojego komputera. Jeśli test nie wykryje serwera proxy to przetestuje IP Twojego serwera proxy zamiast Twojego, które oczywiście nie zadziała.
Wyłącz Twoje proxy na czas tego testu.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Czy powinienem zaznaczyć 'Długi czas oczekiwania'?");
define("_MSG_FORM_LTO_HOWTO", "Zwykle to nie jest potrzebne. Domyślnie ten test poczeka 5 sekund dla każdego z testów, aby sprawdzić czy wszystko działa jak należy. Zwykle to powinno wystarczyć, ale jeśli Twój komputer jest bardzo obciążony, bardzo zajęty lub masz zawodne połączenie z internetem, to może zająć Shareazie więcej czasu, aby poprawnie odpowiedzieć na test, w takim wypadku 5 sekund może nie byc wystarczające. Jeśli włączysz długi czas oczekiwania, test zaczeka 10 sekund. Oczywiście test zajmie więcej czasu.");

/* ======== end of file */

?>