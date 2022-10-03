<?php
/*
   Copyright (c) J-E L, jlh, 2004 - 2008
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
   Language file for: Français (French)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula." (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Traduction française de J-E L, mise à jour mineure par zigotozor");

/* the title (used in various places) */
define("_MSG_TITLE", "Test de connexion de Shareaza");
define("_MSG_TITLE_WITH_LINK", "Test de connexion de <a href='http://shareaza.sourceforge.net/'>Shareaza</a>");

define("_MSG_LANGUAGES", "Ce test de connexion est aussi disponible dans les langues suivantes&nbsp;:");
define("_MSG_FOOTER", "Questions et commentaires sont les bienvenus sur
<a href='http://shareaza.sourceforge.net/phpbb/'>les forums de Shareaza</a>.");

/* stats line */
define("_MSG_STATS", "Ce test a été effectué %d&nbsp;fois depuis le %s.
Statistiques de succès&nbsp;: les&nbsp;deux&nbsp;tests&nbsp;:&nbsp;%d%%, TCP&nbsp;seul&nbsp;:&nbsp;%d%%,
UDP&nbsp;seul&nbsp;:&nbsp;%d%%, aucun&nbsp;:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Erreur");

define("_MSG_ERROR_PORT_INVALID", "Le n&deg; de port entré n'est pas valide,
donnez un nombre compris entre 1&nbsp;et&nbsp;65535. Si vous ne
savez pas quel n&deg; de port il faut entrer, lisez ci-dessous <i>&laquo;&nbsp;Comment
savoir quel est le port utilisé par Shareaza&nbsp;?&raquo;</i>");

define("_MSG_ERROR_PORT_ZERO", "Le n&deg; de port entré n'est pas valide. Si
le n&deg; de port configuré dans les réglages de Shareaza est mis à &laquo;&nbsp;0&nbsp;&raquo;,
cela signifie qu'un port est choisi aléatoirement à chaque démarrage de Shareaza.
Si vous avez un pare-feu ou un routeur, vous ne pouvez pas utiliser un port aléatoire.
Dans ce cas, fixez un n&deg; de port autorisé et décochez la case <b>Aléatoire</b>.
Vous devez ensuite configurer votre pare-feu ou votre routeur pour le port choisi.
Le port par défaut de Shareaza est le 6346, mais n'importe quel n&deg; (non réservé) fait l'affaire.
Si vous ne passez pas par un pare-feu ou un routeur, vous pouvez utiliser le choix aléatoire si
vous le désirez, mais ce test ne peut être mené à son terme que si vous savez
trouver quel est le port utilisé par Shareaza au moment du test. <i>Note&nbsp;: Si
vous changez le port dans les réglages de Shareaza, il faudra le déconnecter et
reconnecter afin que les changements prennent effet.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "Le test de connexion n'arrive pas à trouver votre IP. Rendez-compte du problème SVP.");


/* the link to the wiki */
define("_MSG_WIKI_FR", "Si vous désirez de l'aide pour configurer votre pare-feu ou votre routeur, consultez cette page
du wiki&nbsp;: <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters/fr'>FAQ&nbsp;:&nbsp;Routeurs et pare-feu</a>.");

/* progress box */
define("_MSG_PROGRESS", "Test en cours, cela peut prendre quelques secondes, patientez SVP...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Journal détaillé");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Le journal détaillé est masqué, cliquez ici pour le visualiser.");
define("_MSG_DETAIL_HIDE", "Masquer le journal détaillé.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Résultats");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Ce test n'a pas pu être exécuté.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Rendez-compte du problème en joignant une copie du journal détaillé SVP.");

/* internal error */
define("_MSG_RESULTS_IE", "Test non effectué (erreur interne au programme).");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation
   La 1ère partie du test est affichée en couleur (Résumé très court),
   La 2ème partie donnant des raisons possibles/vraisemblables pour la situation.
*/

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "La connexion TCP a subi un &laquo;&nbsp;Timeout&nbsp;&raquo; (dépassement du temps alloué).");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Cela peut être dû à un pare-feu furtif ou à un routeur improprement configuré pour Shareaza.");

define("_MSG_RESULTS_TCP_REFUSED_1", "La connexion TCP a été refusée&nbsp;: &laquo;&nbsp;port fermé&nbsp;&raquo;.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Un pare-feu ou un routeur n'a pas été correctement
configuré pour ce port ou aucune application ne l'utilise actuellement.
Vérifiez que votre pare-feu (ou routeur) ne bloque pas (ou redirige correctement) ce
port, que Shareaza est lancé et qu'il est configuré pour utiliser ce port.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "La connexion TCP a bien été acceptée par votre ordinateur,");
define("_MSG_RESULTS_TCP_CONNECTED_2", "mais aucune réponse n'a été fournie à la requête.
Tout est probablement correct, mais vérifiez que c'est bien Shareaza qui utilise ce port, pas une autre application.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "La connexion TCP a été acceptée par votre ordinateur, et la requête a reçu une réponse.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Cela signifie que les autres clients du réseau peuvent correctement se connecter à vous.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "Erreur inconnue lors de la connexion à votre adresse IP.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Aucune réponse de votre logiciel client.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Cela peut avoir différentes causes&nbsp;:
pare-feu ou routeur mal configuré, Shareaza non paramétré sur ce n&deg; de port ou qui n'est ni connecté ni en train d'essayer de se connecter au réseau.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Une réponse a été fournie par votre logiciel client&nbsp;!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Cela signifie que Shareaza peut recevoir des paquets UDP en provenance du réseau.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Félicitations, tout semble fonctionner et Shareaza devrait travailler correctement.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Au moins un problème a été détecté&nbsp;:
vous devez probablement configurer votre pare-feu ou votre routeur pour Shareaza.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Test de connexion");

define("_MSG_FORM_TEXT", "Pour que ce test de connexion réussisse, Shareaza doit
être lancé et en fonctionnement. Si ce n'est pas déjà le cas, lancez-le et connectez-le
aux réseaux. (Il n'est pas important qu'il réussisse à se connecter,
il est suffisant qu'il essaye). Ensuite, entrez le n&deg; du port dans la case de saisie
et cliquez sur &laquo;&nbsp;Test&nbsp;&raquo;.");

define("_MSG_FORM_IP", "Votre adresse IP est&nbsp;: %s.");
define("_MSG_FORM_PROXY", "L'adresse de votre serveur proxy est&nbsp;: %s.");

/* just before the port box */
define("_MSG_FORM_PORT", "Port&nbsp;:");
define("_MSG_FORM_LTO", "Délai d'attente prolongé");
/* the button */
define("_MSG_FORM_BUTTON", "Test");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Comment savoir quel est le port utilisé par Shareaza&nbsp;?");
define("_MSG_FORM_PORT_HOWTO", "Dans Shareaza, ouvrez le menu déroulant <b>Outils</b>
et choisissez <b>Réglages&nbsp;de&nbsp;Shareaza</b>.
Sur le panneau de gauche de la fenêtre qui va s'ouvrir, cliquez sur <b>Internet > Connexion</b> et
vous trouverez le n&deg; de port utilisé à droite dans la case <b>Port</b>.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "A propos des serveurs proxy");
define("_MSG_FORM_PROXY_HOWTO", "Si vous utilisez un serveur proxy avec votre <u>navigateur</u> (cf. options Internet
de votre navigateur), il est possible que ce test ne parvienne pas à déterminer votre adresse IP
réelle. S'il ne détecte pas la présence du proxy, il utilisera l'adresse IP
de celui-ci au lieu de la votre pour faire le test, ce qui ne fonctionnera pas.
Désactivez temporairement le serveur proxy pour faire ce test.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Dois-je utiliser &laquo;&nbsp;Délai d'attente prolongé&nbsp;&raquo;?");
define("_MSG_FORM_LTO_HOWTO", "La plupart du temps, ce n'est pas nécessaire.
Ce test attend un maximum de 5 secondes pour les tests TCP et UDP.
Si votre ordinateur est très occupé ou si votre connexion internet est instable,
cela peut prendre plus de temps que prévu. &laquo;&nbsp;Délai d'attente prolongé&nbsp;&raquo; vous permet de
porter le délai à 10 secondes, mais le test prendra alors plus de temps pour
s'effectuer.");

/* ======== end of file */

?>