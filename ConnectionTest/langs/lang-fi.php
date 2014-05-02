<?php
/*
   Copyright (c) Warial Brute (http://fissionx.net/) 2005 - 2008
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
   Language file for: Suomi (Finnish)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Testin suomennos by <a href=\"http://fissionx.net/\">Warial Brute</a>");

/* the title (used in various places) */
define("_MSG_TITLE", "Shareaza Yhteystesti");
define("_MSG_TITLE_WITH_LINK", "<a href='http://shareaza.sourceforge.net/'>Shareaza</a> Yhteystesti");

define("_MSG_LANGUAGES", "Test also in:");
define("_MSG_FOOTER", "Kysymykset ja kommentit:
<a href='http://shareaza.sourceforge.net/phpbb/'>Shareaza Forum (EN)</a>.");

/* stats line */
define("_MSG_STATS", "T&auml;m&auml; testi on tehty %d kertaa %s l&auml;htien. Onnistumisarvio: TCP &amp; UPD %d%%,
Vain TCP %d%%, Vain UDP %d%%, Ei kumpikaan %d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Virhe");

define("_MSG_ERROR_PORT_INVALID", "Antamasi portti ei ole toimiva portti.
Portin tulee olla v&auml;lilt&auml; 1 - 65535. Jos et tied&auml; mik&auml; portti Shareaza:lla on k&auml;yt&ouml;ss&auml;,
katso kohta <i>'Mist&auml; tied&auml;n mi&auml; porttia Shareaza k&auml;ytt&auml;&auml;?'</i>.");

define("_MSG_ERROR_PORT_ZERO", "Antamasi portti on 0 joka tarkoittaa Shareazan kohdalla satunnaista porttia (esim. joka k&auml;ynnistyksell&auml; Shareaza arpoo uuden portin ja k&auml;ytt&auml;&auml; sit&auml;).
Jos k&auml;yt&ouml;ss&auml;si on joko palomuuri (firewall), reitin (router) tai molemmat on suositeltavaa ettet k&auml;yt&auml; satunnaista porttia.
Poistaaksesi t&auml;m&auml;n ominaisuuden k&auml;yt&ouml;st&auml; poista valinta <b>Random</b> asetusten kohdasta <b>Internet &gt; Connection &gt; Inbound Address</b> ja laita haluamasi portti kentt&auml;&auml; sen vieress&auml; (esim. 6346, Shareazan oletusportti),
T&auml;m&auml; portti tulee olla avattuna sek&auml; reitittimess&auml;, ett&auml; palomuurissa. Kaikki portit k&auml;yv&auml;t, mutta sen t&auml;ytyy olla v&auml;lilt&auml; 1 - 65535.
Jos sinulla ei ole palomuuria (mik&auml; nyky&auml;&auml;n kannattaisi olla) tai reititint&auml; voit k&auml;ytt&auml;&auml; satunnaista porttia, mutta testi ei onnistu ellet tied&auml; mit&auml; porttia Shareaza t&auml;ll&auml; hetkell&auml; k&auml;ytt&auml;&auml;.
<i>Huom!: Ennen kuin vaihdat portin numeroa, katkaise yhteys kaikkiin P2P-verkkoihin joihin olet yhteydess&auml; Shareazalla.
Kun olet m&auml;&auml;ritt&auml;nyt uuden portin voit palauttaa yhteydet verkkoihin. Jos t&auml;t&auml; ei tehd&auml;, uutta porttia ei oteta k&auml;ytt&ouml;&ouml;n koska edellinen on jo k&auml;yt&ouml;ss&auml; ja sit&auml; ei pysty vaihtamaan 'lennosta'.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "Yhteystesti ei pystynyt selvitt&auml;m&auml;&auml;n IP-osoitettasi. Ole hyv&auml; ja v&auml;lit&auml; ilmoitus t&auml;st&auml; virheest&auml; Shareazan foorumeille (linkki alempana).");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Jos tarvitset apua palomuurin (firewall) tai reitittimen (router) asetusten kanssa, k&auml;y Shareazan Wiki:ss&auml;:
<a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters'>FAQ:&nbsp;Routers/Firewalls</a>.");

/* progress box */
define("_MSG_PROGRESS", "Yhteystesti on nyt k&auml;ynniss&auml;, t&auml;m&auml; voi vied&auml; muutaman sekunnin. Ole hyv&auml; ja odota...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Tapahtumaloki");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Testin tapahtumaloki on piilotettu. Jos haluat n&auml;hd&auml; sen klikkaa t&auml;st&auml;.");
define("_MSG_DETAIL_HIDE", "Piilota tapahtumaloki.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Tulokset");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "T&auml;t&auml; testi&auml; ei ole suoritettu.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Ole hyv&auml; ja ilmoita t&auml;m&auml; ongelma testin tapahtumalokin t&auml;ydellisell&auml; kopiolla Shareazan Foorumeihin (linkki alempana).");

/* internal error */
define("_MSG_RESULTS_IE", "T&auml;t&auml; testi&auml; ei voitu suorittaa sis&auml;isen virheen takia.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "TCP yhteys on aikakatkaistu.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "T&auml;m&auml; voi johtua ns. \"stealth\" palomuurista (t&auml;m&auml; k&auml;yt&auml;nn&ouml;ss&auml; pillottaa koneesi niin ettei sit&auml; n&auml;e netist&auml; katsottuna) ja/tai reitittimest&auml; jota ei ole m&auml;&auml;ritetty oikein Shareazan toimintaa varten.");

define("_MSG_RESULTS_TCP_REFUSED_1", "TCP yhteys on hyl&auml;tty, portti on kiinni.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Joko palomuuria ja/tai reititint&auml; ei ole m&auml;&auml;ritetty oikein antamallesi portille,
tai Shareaza ei k&auml;yt&auml; sit&auml;. Varmista, ett&auml; sinun palomuuristasi ja/tai reitittimest&auml;si on avattu portti Shareazaa varten,
Shareaza on k&auml;ynniss&auml; ja k&auml;ytt&auml;&auml; palomuurista/reitittimest&auml; avattua porttia.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "TCP yhteys hyv&auml;ksytty.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Mutta pyynt&ouml;&ouml;n ei saatu vastausta. T&auml;m&auml; on yleens&auml; normaalia.
Varmista, ett&auml; vain Shareaza k&auml;ytt&auml;&auml; antamaasi porttia.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "TCP yhteys on hyv&auml;ksytty ja pyynt&ouml;&ouml;n on vastattu.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "T&auml;m&auml; tarkoittaa ett&auml; muut Gnutella (ja muiden P2P) ohjelmien k&auml;ytt&auml;j&auml;t voivat ottaa yhteyden koneeseesi.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "Tapahtui virhe yritett&auml;ess&auml; yhdist&auml;&auml; IP osoitteeseesi (kyll&auml;, olen yht&auml; ymm&auml;ll&auml;ni t&auml;st&auml; kuin sin&auml;kin).");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Yht&auml;&auml;n vastausta ei saatu koneeltasi.");
define("_MSG_RESULTS_UDP_NOTHING_2", "T&auml;m&auml; voi johtua monista syist&auml;,
esim. palomuurisi ja/tai reitittimesi ei ole m&auml;&auml;ritetty Shareazalle,
Shareaza ei k&auml;yt&auml; t&auml;t&auml; porttia tai et ole yhteydess&auml; P2P-verkkoihin.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Vastaus on saatu koneeltasi!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "T&auml;m&auml; tarkoittaa ett&auml; Shareaza voi vastaanottaa UPD-paketteja verkosta.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Onneksi olkoon, kaikki n&auml;ytt&auml;&auml; olevan kunnossa
ja Shareazan pit&auml;isi toimia oikein.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "V&auml;hint&auml;&auml;n yksi ongelma on havaittu.
Sinun t&auml;ytyy m&auml;&auml;ritt&auml;&auml; palomuurisi ja/tai reitittimesi Shareazalle (l&auml;hinn&auml; avata portti jota Shareaza k&auml;ytt&auml;&auml; yhdist&auml;miseen).");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Suorita yhteystesti");

define("_MSG_FORM_TEXT", "Jotta testi toimisi (ja onnistuisi) tulee sinun pit&auml;&auml; Shareaza p&auml;&auml;ll&auml; koko testin ajan (n. 5-10s).  Jos Shareaza ei ole k&auml;ynniss&auml;, k&auml;ynnist&auml; se ja yhdist&auml; johonkin P2P-verkkoon
(Shareazan ei tarvitse olla yhteydess&auml; verkkoon, riitt&auml;&auml; ett&auml; se yritt&auml;&auml; edes yhdist&auml;&auml;).  Laita Shareazan k&auml;ytt&auml;m&auml;n portin numero alla olevaan kentt&auml;&auml;n ja
paina 'Suorita' painiketta.");

define("_MSG_FORM_IP", "IP-osoitteesi on %s.");
define("_MSG_FORM_PROXY", "V&auml;lityspalvelimesi (proxy) on %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Portti:");
define("_MSG_FORM_LTO", "Pitk&auml; aikakatkaisu");
/* the button */
define("_MSG_FORM_BUTTON", "Suorita");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Mist&auml; tied&auml;n mit&auml; porttia Shareaza k&auml;ytt&auml;&auml;?");
define("_MSG_FORM_PORT_HOWTO", "T&auml;m&auml;n selvitt&auml;minen k&auml;y helposti: avaa Asetukset-valikko <b>Tools &gt; Shareaza Settings</b>.
T&auml;m&auml;n j&auml;lkeen avaa <b>Internet &gt; Connection</b> ja etsi <b>Port</b>-kentt&auml; <b>Inbound Address</b> paneelista.
Shareaza k&auml;ytt&auml;&auml; t&auml;t&auml; porttia.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Huomautus v&auml;lityspalvelimista (proxy)");
define("_MSG_FORM_PROXY_HOWTO", "Jos sinulla on selaimessasi k&auml;yt&ouml;ss&auml; v&auml;lityspalvelin,
on mahdollista ettei testi voi ottaa yhteytt&auml; koneesi IP:hen vaan yritt&auml;&auml; suorittaa testin v&auml;lityspalvelimelle.
Jos testi ei huomaa v&auml;lityspalvelimen olevan k&auml;yt&ouml;ss&auml; (esim. k&auml;yt&ouml;ss&auml; on ns. 'anonymizer proxy' joka piilottaa k&auml;ytt&auml;j&auml;n alkuper&auml;isen IP:n),
poista v&auml;lityspalvelin k&auml;yt&ouml;st&auml; v&auml;liaikaisesti testin ajaksi.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Pit&auml;isik&ouml; minun k&auml;ytt&auml;&auml; 'Pitk&auml; aikakatkaisu'-valintaa?");
define("_MSG_FORM_LTO_HOWTO", "Yleen&auml;s t&auml;m&auml; ei ole tarpeen.
Normaalisti testi odottaa TCP- ja UPD-yhteyksiin vastausta 5 sekuntia, jotta se tiet&auml;&auml; toimiiko Shareaza nykyisill&auml; asetuksilla.
Joskus t&auml;m&auml; ei ole tarpeeksi. Syin&auml; voivat olla koneen kuormitus, hidas tai katkeileva yhteys, liian suuri liikenne l&auml;hiverkossa tai operaattorin p&auml;&auml;ss&auml; (operaattorin kohdalla t&auml;m&auml; on yleens&auml; vain kaapeliverkossa).
Jos tilanne on n&auml;in, on suositeltavaa k&auml;ytt&auml;&auml; pitk&auml;&auml; aikakatkaisua jotta testin tulos olisi tarkempi. Koska testi odottaa nyt 10 sekuntia vastauksia yhteyksiin, menee testill&auml; kauemmin valmistua.");

/* ======== end of file */

?>