<?php
/*
   Copyright (c) Help, 2004 - 2008
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
   Language file for: Español (Spanish)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula." (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Traducción Al Español Por Help");

/* the title (used in various places) */
define("_MSG_TITLE", "Prueba de conexión Shareaza");
define("_MSG_TITLE_WITH_LINK", "Prueba de conexión <a href='http://shareaza.sourceforge.net/'>Shareaza</a>");

define("_MSG_LANGUAGES", "Esta prueba de conexión esta disponible en:");
define("_MSG_FOOTER", "Preguntas y comentarios sobre esta prueba son bienvenidas en
<a href='http://shareaza.sourceforge.net/phpbb/'>Foro de Shareaza</a>.");

/* stats line */
define("_MSG_STATS", "Esta prueba ha sido realizada %d&nbsp;veces desde
el&nbsp;%s. Porcentaje de suceso: Las 2 pruebas:&nbsp;%d%%, solamente
TCP:&nbsp;%d%%, solamente UDP:&nbsp;%d%%, ninguno:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Error");

define("_MSG_ERROR_PORT_INVALID", "El número de puerto que entraste no es un puerto valido, por favor da uno entre 1&nbsp;-&nbsp;65535.");

define("_MSG_ERROR_PORT_ZERO", "El puerto que diste no es correcto.  Si la configuración del puerto
en Shareaza dice '0', eso significa que esta al azar.  Si tienes un firewall o un router, no puedes usar puertos al azar.  En
ese caso deshabilita la opción <b>Azar</b> y da un número de puerto, en la cual debes
configurar en tu cortajuegos o router.  El puerto por defecto es 6346, pero cualquier puerto
funciona.  Si tu no tienes un firewall o un router, entonces puedes usar puertos
al azar que deseas, pero esta prueba solo puede ser hecha si tu sabes como encontrar
el puerto que Shareaza esta usando actualmente.  <i>Nota: Cuando cambias el número de puerto en la configuracíon,
te debes desconectar y conectar para que los cambios tomen efecto.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "La prueba de conexión no pudo encontrar tu IP.  Por favor reporta este problema.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Si necesitas ayuda configurando tu firewall o router, visita este sitio en el
wiki: <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters/es'>FAQ:&nbsp;Firewall/Router</a>.");

/* progress box */
define("_MSG_PROGRESS", "La prueba de conexión es en proceso; pero puede tomar unos segundos, por favor espera...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Log de detalles");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "El log de detalles esta escondido, haz click aquí para mostrarlo.");
define("_MSG_DETAIL_HIDE", "Esconder el log de detalles.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Resultados");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Esta prueba no pudo ser hecha.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Por favor reporta este problema con todos los detalles que dice el log.");

/* internal error */
define("_MSG_RESULTS_IE", "Esta prueba no pudo ser hecha, hay un error interno.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "La conexión de TCP dió timeout.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Esto puede ver sigo causado por un firewall o router no configurado correctamente para usar
Shareaza.");

define("_MSG_RESULTS_TCP_REFUSED_1", "La conexión TCP fue rechazado, el puerto esta cerrado.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Ningún router o firewall ha sido
configuradoen este puerto, o ninguna aplicación lo esta usando.  Por favor asegurate que tu firewall o router esta confugurado correctamente para usar este puerto, o que
Shareaza lo esta usando.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "La conexión TCP ha sido aceptada en la computadora.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Pero no hubo respuesta.
Probablemente esto es correcto. Pero asegurate que Shareaza esta usando este puerto,
no otra aplicación.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "La conexión TCP ha sido aceptada en tu computadora
y se le ha dado respuesta.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Esto significa que clientes en otras redes se pueden conectar correctamente a ti.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "Ocurrio un error no conocido al tratar de conectar.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "No se ha recibido respuesta de tu cliente.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Esto puede ser por varias razones, como que
tu firewall o router no esta correctamente configurado para Shareaza, o que Shareaza no esta conectado en este puerto, o no esta conectado a otras redes.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Una respuesta ha sido recibida de tu cliente!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Esto significa que puede recibir paquetes TCP y UDP sin problemas.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Enhorabuena!, todo parece que esta bien configurado y Shareaza debe funcionar correctamente.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Por lo menos un problema ha sido detectado y probablemente
debes configurar tu firewall o router.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Hacer una prueba de conexión");

define("_MSG_FORM_TEXT", "Para que esta prueba de conexión funcione, debes tener
Shareaza ejecutandose.  Si todavia no se esta ejecutando, ejecutalo y has que se conecte a las
redes.  (No importa si se conecta correctamente o no, es suficiente si esta tratando.)  Después ingresa el puerto que Shareaza esta utilizando en la caja de abajo
y haz click en 'Probar'.");

define("_MSG_FORM_IP", "Tu ip es %s.");
define("_MSG_FORM_PROXY", "Tu proxy es %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Puerto:");
define("_MSG_FORM_LTO", "Timeout largo");
/* the button */
define("_MSG_FORM_BUTTON", "Probar");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "¿Como puedo saber que puerto usa Shareaza?");
define("_MSG_FORM_PORT_HOWTO", "Si no sabes que puerto Shareaza usa,
abre Shareaza y vete al dialogo de configuración seleccionando <b>Configuración&nbsp;Shareaza</b> del menu <b>Herramientas</b>.  Luego en el panel izquiero
haz click en<b>Conexión de&nbsp;&gt;&nbsp;Internet</b> y busca el
campo de <b>Puerto</b> en la derecha.  Este es el puerto que Shareaza usa.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Una nota sobre servidores proxy");
define("_MSG_FORM_PROXY_HOWTO", "Si usted tiene un servidor proxy habilitado en los ajustes de su browser,
es posible que esta prueba de la conexión no podrá descubrir el IP de su computadora.
Si no puede detectar un proxy, probará el IP del proxy, en vez del tuyo, que por supuesto no trabajará.
Inhabilite su proxy temporalmente para hacer esta prueba.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "¿Deberia de habilitar 'Timeout largo'?");
define("_MSG_FORM_LTO_HOWTO", "Usualmente, esto no es necesario.  Por defecto,
esta prueba va esperar 5 segundos en las pruebas TCP y UDP, para ver si todo funciona
como se esperaba. Usualmente, 5 segundos es suficiente, pero si tu computadora esta muy ocupada o si tienes una conexión a internet no fiable, puede tardar
más tiempo para contestar correctamente a la prueba, en ese caso 5
segundos no es suficiente.  Si habilitas timeout largo, la prueba va esperar
10 segundos.  Obviamente, la prueba va tardar más en terminar.");

/* ======== end of file */

?>