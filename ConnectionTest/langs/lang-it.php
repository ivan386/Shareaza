<?php
/*
   Copyright (c) Jocker, 2005 - 2008
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
   Language file for: Italiano (Italian)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Traduzione italiana a cura di
<a href='http://shareazaitalia.no-ip.info'>Jocker</a>");

/* the title (used in various places) */
define("_MSG_TITLE", "Test della Connessione di Shareaza");
define("_MSG_TITLE_WITH_LINK", "Test della Connessione di <a href='http://shareaza.sourceforge.net/'>Shareaza</a>");

define("_MSG_LANGUAGES", "Questo test della connessione &egrave; disponibile anche in:");
define("_MSG_FOOTER", "Domande e commenti riguardo questo test sono i benvenuti nei
<a href='http://shareaza.sourceforge.net/phpbb/'>Forum di Shareaza</a>.");

/* stats line */
define("_MSG_STATS", "Questo test è stato eseguito %d&nbsp;volte da&nbsp;%s.
Percentuale di successo: Entrambi i test:&nbsp;%d%%, solo TCP:&nbsp;%d%%,
solo UDP:&nbsp;%d%%, nessuno:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Errore");

define("_MSG_ERROR_PORT_INVALID", "Il numero della porta che hai inserito non &egrave;
valido, per favore inseriscine uno valido compreso fra 1&nbsp;e&nbsp;65535. Se non conosci
quale numero inserire, leggi la sezione <i>'Come conoscere la porta che Shareaza usa?'</i>.");

define("_MSG_ERROR_PORT_ZERO", "Il numero della porta che hai inserito non &egrave;
valido. Se il campo Porta delle impostazioni di Shareaza dice '0', allora significa che si sta usando
una porta casuale. Se hai un firewall o router, allora non puoi usare porte casuali.
In questo caso deseleziona <b>Casuale</b> e inserisci un numero di porta. La porta predefinita &egrave; la 6346,
ma vanno bene tutte. Se non hai un firewall o un router, puoi usare una porta casuale, ma questo test
pu&ograve; funzionare solo se conosci la porta che Shareaza sta usando. <i>Nota: quando cambi il numero
della porta nelle impostazioni, devi disconnetterti e riconnetterti affinch&egrave; la modifica abbia effetto.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "Il test della connessione non riesce a capire quale sia il tuo indirizzo IP.
Per favore riporta questo problema.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Se hai bisogno a configurare il tuo firewall o router, visita questa pagina
del wiki <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters/it'>FAQ:&nbsp;Router/Firewall</a>.");

/* progress box */
define("_MSG_PROGRESS", "Test della connessione in corso; potrebbe impiegarci qualche secondo, per favore attendere...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Log dettagliato");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Il log dettagliato &egrave; nascosto, clicca qui per mostrarlo.");
define("_MSG_DETAIL_HIDE", "Nascondi log dettagliato.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Risultati");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Questo test non &egrave; stato eseguito.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Per favore riporta questo errore con una copia completa del log dettagliato.");

/* internal error */
define("_MSG_RESULTS_IE", "Non &egrave; stato possibile eseguire questo test a causa di un errore interno.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "La connessione TCP ha superato il limite di tempo.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Questo pu&ograve;  essere dovuto a un firewall o router che non
&egrave;  stato configurato correttamente.");

define("_MSG_RESULTS_TCP_REFUSED_1", "La connessione TCP &egrave; stata rifiutata, la porta &egrave; chiusa.");
define("_MSG_RESULTS_TCP_REFUSED_2", "O il firewall o il router non sono stati configurati su questa porta,
o nessuna applicazione la st&agrave; utilizzando attualmente. Assicurati che il firewall
o router sia configurato correttamente per non blccare o inoltrare questa porta e che
Shareaza sia in esecuzione e configurato per utilizzare questa porta.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "La connessione TCP &egrave; stata accettata dal tuo computer.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Ma nessuna risposta &egrave; stata data alla richiesta.
Questo probabilmente non &egrave; un problema. Assicurati che sia Shareaza ad utilizzare quella porta,
non un'altra applicazione.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "La connessione TCP &egrave; stata accettata dal tuo computer
e la richiesta ha avuto una risposta.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Questo significa che gli altri client della rete possono
connettersi a te con successo.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "C'&egrave; stato un erorre sconosciuto durante la connessione al tuo IP.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Nessuna risposta &egrave; stata ricevuta dal tuo client.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Questo pu&ograve; avere diversi motivi, fra i quali il tuo firewall
o router non &egrave; configurato correttamente per Shareaza, o Shareaza non &egrave; in esecuzione su questa porta, o non si sta
connettendo alle reti.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Una risposta &egrave; stata ricevuta dal tuo client!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Questo significa che Shareaza &egrave; in grado di ricevere
pacchetti UDP dalla rete.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Congratulazioni, sembra che tutto sia a posto e che Shareaza possa lavorare correttamente.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Almeno un problema &egrave; stato rilevato e
probabilmente devi configurare il tuo firewall o router per Shareaza.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Esegui il test della connessione");

define("_MSG_FORM_TEXT", "Per poter eseguire questo test, Shareaza deve essere in esecuzione.
Se non &egrave; gi&agrave; in esecuzione, avvialo e fallo connettere ad almeno una rete
(Non importa se si connette con successo o no, basta che ci provi).
Inserisci il numero della porta di Shareaza nel riquadro qui in basso e clicca su 'Test'.");

define("_MSG_FORM_IP", "Il tuo IP &egrave; %s.");
define("_MSG_FORM_PROXY", "Il tuo proxy &egrave; %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Porta:");
define("_MSG_FORM_LTO", "Timeout elevato");
/* the button */
define("_MSG_FORM_BUTTON", "Test");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Come conoscere la porta che Shareaza usa?");
define("_MSG_FORM_PORT_HOWTO", "Se non conosci la porta che Shareaza usa, allora passa a Shareaza e apri la
finestra delle impostazioni selezionando <b>Opzioni di Shareaza</b> dal menu <b>Strumenti</b>.
Quindi clicca su <b>Internet&nbsp;&gt;&nbsp;Connessione</b> dal menu a sinistra e cerca il campo <b>Porta</b> sulla destra:
questo &egrave; il numero della porta usata da Shareaza.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Una nota riguardo i server proxy");
define("_MSG_FORM_PROXY_HOWTO", "Se hai attivo un server proxy nelle impostazioni del tuo browser,
allora &egrave; possibile che questo test non sia in grado di trovare l'IP del tuo computer.
Se non riesce a rilevare il proxy, il test verr&agrave; eseguito sull'IP del proxy, al posto del tuo,
il che ovviamente non funzioner&agrave;. Disabilita temporaneamente il proxy per eseguire questo test.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Devo abilitare 'Timeout elevato'?");
define("_MSG_FORM_LTO_HOWTO", "Solitamente questo non &egrave; necessario.
Di default questo test attender&agrave; 5 secondi per i test TCP e UDP per vedere se tutto funziona come dovrebbe.
Solitamente questo tempo &egrave; abbastanza, ma se il tuo computer &egrave; molto impegnato
o se hai una connessione ad internet poco affidabile, allora Shareaza potrebbe impiegarci più tempo
a rispondere al test. Se abiliti 'Timeout elevato', il test attender&agrave; 10 secondi.
Ovviamente questo render&agrave; il test più lungo.");

/* ======== end of file */

?>