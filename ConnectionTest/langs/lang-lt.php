<?php
/*
   Copyright (c) Rolandas, 2005 - 2008
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
   Language file for: Lietuviškai (Lithuanian)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "į lietuvių kalbą išvertė Rolandas Rudomanskis");

/* the title (used in various places) */
define("_MSG_TITLE", "Ryšio testas, skirtas „Shareaza“ programai");
define("_MSG_TITLE_WITH_LINK", "Ryšio testas, skirtas „<a href='http://shareaza.sourceforge.net/'>Shareaza</a>“ programai");

define("_MSG_LANGUAGES", "Šį puslapį galite skaityti ir šiomis kalbomis:");
define("_MSG_FOOTER", "Klausimus ir pastabas apie šį testą siųskite į:
<a href='http://shareaza.sourceforge.net/phpbb/'>„Shareaza“ forumus</a>.");

/* stats line */
define("_MSG_STATS", "Šis testas buvo atliktas %d&nbsp;kartus(-ą,-ų) nuo %s.  Sėkmingi 
testai: Abu&nbsp;testai:&nbsp;%d%%, Tiktai&nbsp;TCP:&nbsp;%d%%,
Tiktai&nbsp;UDP:&nbsp;%d%%, nė vieno:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Klaida");

define("_MSG_ERROR_PORT_INVALID", "Prievado numeris, kurį Jūs įvedėte, yra neteisingas,
įveskite prievadą, esantį skaičių 1&nbsp;-&nbsp;65535 rėžiuose.  Jei nežinote, kokį skaičių
reikėtų suvesti, perskaitykite žemiau apie <i>„Iš kur sužinoti, kokį prievadą „Shareaza“ naudoja?“</i>.");

define("_MSG_ERROR_PORT_ZERO", "Prievado numeris, kurį Jūs įvedėte, yra neteisingas.  Jei
„Shareaza“ programos nuostatose Jūs matote „0“, tai reiškia, jog naudojamas atsitiktinio numerio
prievadas.  Jei Jūs turite ugniasienę ar maršrutizatorių, tai Jūs negalite naudoti atsitiktinių prievadų.  Tuo
atveju atžymėkite nuostatą <b>Atsitiktinis</b> ir įveskite numerį, kurį Jūs įvesite į
savo ugniasienės ar maršrutizatoriaus nuostatas.  Numatytasis prievado numeris yra 6346, bet tai yra nesvarbu — 
Jūs galite įvesti bet kokį numerį.  Jei Jūs neturite ugniasienės arba maršrutizatoriaus, tada galite naudoti atsitiktinį
prievado numerį, jei norite, bet turėkite omenyje, kad šio testo negalima atlikti, jei nežinote prievado
numerio, kurį naudoja „Shareaza“ programa.  <i>Pastaba: Kad pakeistumėte prievado numerį nuostatose, Jums reikia
programoje atsijungti nuo tinklo ir vėl po to prisijungti, tam, kad Jūsų pakeitimai įsigaliotų.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "Ryšio testas negalėjo nustatyti Jūsų kompiuterio IP adreso.
  Prašytume pranešti apie šią problemą.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Jei Jums reikalinga pagalba, kaip susiderinti ugniasienę arba maršrutizatorių, aplankykite wiki
puslapį: <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters'>FAQ:&nbsp;Routers/Firewalls</a>. Rašykite lietuviškai, jei nemokate
anglų kalbos, atsiras bet kas, kas galės atsakyti.");

/* progress box */
define("_MSG_PROGRESS", "Ryšio testas vykdomas; tai gali užimti kelias sekundes, prašome palaukti...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Išsamus žurnalas");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Išsamus žurnalas paslėptas, spragtelkite čia, jei norite peržiūrėti.");
define("_MSG_DETAIL_HIDE", "Paslėpti išsamų žurnalą.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Rezultatai");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Testas dar nebuvo atliktas.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Prašome pranešti apie problemą, išsiųsdami išsamaus žurnalo turinį.");

/* internal error */
define("_MSG_RESULTS_IE", "Nebuvo galima atlikti testo, įvyko vidinė klaida.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "TCP ryšio užmezgimas užėmė per daug laiko.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Tai gali būti susiję su klaidinančia (stealth) ugniasiene arba
maršrutizatoriumi, kurie nebuvo tinkamai suderinti su „Shareaza“ programa.");

define("_MSG_RESULTS_TCP_REFUSED_1", "TCP ryšio buvo atsakyta, prievadas uždarytas.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Su šiuo prievadu susieti ugniasienė ar maršrutizatorius arba
jokia programa nenaudoja jo. Įsitikinkite, jog Jūsų ugniasienė ar maršrutizatorius tinkamai suderinti
ir neblokuoja prievado, o „Shareaza“ programa veikia ir nustatyta naudoti šį prievadą.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "TCP ryšys buvo užmegztas su Jūsų kompiuteriu.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Bet jokio atsako negauta į užklausą.
Tai galbūt nėra blogai. Tik įsitikinkite, jog būtent „Shareaza“ programa naudoja šį prievadą,
o ne kokia nors kita programa.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "TCP ryšys buvo užmegztas su Jūsų kompiuteriu, ir buvo atsakyta
į užklausą.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Tai reiškia, jog kiti klientai iš tinklų galės laisvai prisijungti
prie Jūsų kompiuterio.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "Įvyko nežinoma klaida bandant užmegzti ryšį su Jūsų kompiuterio IP.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Jokio atsako nebuvo gauta iš Jūsų kliento.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Tai galėtų būti dėl įvairių priežasčių, tokių kaip:
Jūsų ugniasienė ar maršrutizatorius nebuvo tinkamai suderintas su „Shareaza“ programa; arba „Shareaza“ veikia
naudodama kitą prievadą; arba ji nėra prisijungusi prie tinklų.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Atsakas iš Jūsų kliento buvo gautas!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Tai reiškia, jog „Shareaza“ programa gali priimti
UDP protokolo paketus iš tinklų.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Sveikiname! Viskas yra tvarkoje, ir „Shareaza“ programa
turėtų veikti teisingai.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Mažų mažiausiai aptikta viena problema. Jums tikriausiai
reikės suderinti savo ugniasienę ar maršrutizatorių darbui su „Shareaza“ programa.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Atlikti ryšio patikros testą");

define("_MSG_FORM_TEXT", "Tam, kad šis testas būtų įvykdytas, Jūs privalote turėti veikiančią
„Shareaza“ programą. Jei ji šiuo metu neveikia, startuokite ją ir prisijunkite prie tinklų
(nesvarbu, ar ji sėkmingai prisijungs ar ne — užtenka to, kad ji mėgintų jungtis). Tada įveskite
„Shareaza“ programos prievado numerį į žemiau esantį laukelį ir paspauskite mygtuką „Testuoti“.");

define("_MSG_FORM_IP", "Jūsų IP yra %s.");
define("_MSG_FORM_PROXY", "Jūsų tarpinio serverio IP yra %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Prievadas:");
define("_MSG_FORM_LTO", "Ilgas laukimas");
/* the button */
define("_MSG_FORM_BUTTON", "Testuoti");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Iš kur sužinoti, kokį prievadą „Shareaza“ naudoja?");
define("_MSG_FORM_PORT_HOWTO", "Jei Jūs nežinote, kokį prievadą „Shareaza“ naudoja,
atverkite „Shareaza“ programą ir po to — nuostatų langą, pasirinkę iš meniu <b>„Įrankiai“</b> punktą
<b>„Shareaza“&nbsp;nuostatos</b>. Tada kairiajame skydelyje spragtelkite <b>Internetas&nbsp;&gt;&nbsp;Ryšys</b>
ir pažiūrėkite į laukelį <b>„Prievadas“</b>, esantį dešinėje pusėje. Tai ir bus „Shareaza“ programos
prievadas.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Pastaba apie tarpinius (proxy) serverius");
define("_MSG_FORM_PROXY_HOWTO", "Jei Jūs savo naršyklės nuostatose turite įgalinę tarpinį serverį,
tai, galimas dalykas, šis ryšio testas nepavyks, jei nebus galima nustatyti Jūsų kompiuterio IP.
Jei testuojant pavyks aptikti tarpinį serverį, bus testuojamas tarpinio serverio IP adresas, užuot Jūsų.
Šiuo atveju testas, be abejo, nepavyks. Atjunkite laikinai tarpinį serverį, kad galėtumėte testuoti.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Ar man reikėtų pažymėti „Ilgas laukimas“?");
define("_MSG_FORM_LTO_HOWTO", "Paprastai to nereikia daryti. Nepažymėjus šio laukelio,
tam, kad galima būtų pamatyti, ar viskas dirba, testas lauks apie 5 sekundes, tikrindamas tiek TCP,
tiek UDP susijungimus. Paprastai tiek laiko pakanka, bet jei Jūsų kompiuteris yra
labai apkrautas, užimtas arba jei Jūs turite nepatikimą interneto ryšį, tai tada galbūt
reikėtų daugiau laiko, laukiant „Shareaza“ programos atsako į testą, ir tada 5 sekundžių
gali nepakakti. Jei Jūs įgalinsite ilgąjį laukimą, testas lauks 10 sekundžių. Akivaizdu, pats
testas vyks irgi ilgiau.");

/* ======== end of file */

?>