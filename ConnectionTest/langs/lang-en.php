<?php
/*
   Copyright (c) jlh, Brov, 2004 - 2008
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
   Language file for: English
*/

/*
   notes for translators:
     * strings may contain new lines in them to break long lines.
     * only translate the strings, leave those _MSG_BLAHBLAH as they are
     * take care to have the same punctuations in your translations,
       i.e. if the english string has a period/colon/anything at the
       end, so should have the translation.  same for capitalization.
     * for those that don't know: the sequence "&nbsp;" stands for a
       non-breakable space, and is handy for places where it would look
       ugly if it would be broken there.  also escape other characters,
       like ">" becomes "&gt;".  use HTML tags only where they are used
       in the english version, too.
     * contact me in case of problems/comments/suggestions/questions.
     * NEW: All language files must now use UTF-8 encoding
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "");

/* the title (used in various places) */
define("_MSG_TITLE", "Shareaza Connection Test");
define("_MSG_TITLE_WITH_LINK", "<a href='http://shareaza.sourceforge.net/'>Shareaza</a> Connection Test");

define("_MSG_LANGUAGES", "This connection test is also available in:");
define("_MSG_FOOTER", "Questions and comments about this test are welcome on the
<a href='http://shareaza.sourceforge.net/phpbb/'>Shareaza Forums</a>.");

/* stats line */
define("_MSG_STATS", "This test has been done %d&nbsp;times since %s.  Success
rate: Both&nbsp;tests:&nbsp;%d%%, TCP&nbsp;only:&nbsp;%d%%,
UDP&nbsp;only:&nbsp;%d%%, none:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Error");

define("_MSG_ERROR_PORT_INVALID", "The port number you entered is not a valid
port, please enter a valid one from the range 1&nbsp;-&nbsp;65535.  If you
don't know which port to enter, read below under <i>'How to tell what port
Shareaza uses'</i>.");

define("_MSG_ERROR_PORT_ZERO", "The port you entered is not a valid port.  If
the port setting in Shareaza says '0', then it means that a random port is being
used.  If you have a firewall or a router, then you cannot use random ports.  In
that case uncheck the <b>Random</b> setting and enter a port number, which
you'll configure on your firewall or router.  The default port is 6346, but any
port will do.  If you don't have a firewall or router, then you can use random
ports if you want, but this test can only be done if you know how to find out
what port Shareaza currently uses.  <i>Note: When changing the port number in
the settings, you must disconnect and reconnect for the change to take
effect.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "The connection test could not find out what your IP
is.  Please report this problem.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "If you need help with configuring your firewall or router, visit this page on the
wiki: <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters'>FAQ:&nbsp;Routers/Firewalls</a>.");

/* progress box */
define("_MSG_PROGRESS", "Connection test now in progress; it might take a few seconds, please stand by...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Detail log");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Detail log is hidden, click here to unhide.");
define("_MSG_DETAIL_HIDE", "Hide detail log.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Results");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "This test has not been performed.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Please report this problem with a copy of the full detail log.");

/* internal error */
define("_MSG_RESULTS_IE", "This test could not be performed, there was an internal error.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "The TCP connection has timed out.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "This can be due to a stealth firewall or
router that has not been configured properly for Shareaza.");

define("_MSG_RESULTS_TCP_REFUSED_1", "The TCP connection has been refused, the port is closed.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Either a firewall or a router has not been
configured on this port, or no application is currently using it.  Make sure
your firewall or router is properly configured to not block or forward this
port and that Shareaza is running and configured to use this port.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "The TCP connection has been accepted by your computer.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "But no answer was given to the request.
This is probably all right.  Just make sure that it's Shareaza using this port,
not another application.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "The TCP connection has been accepted by
your computer and the request has been answered.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "This means that other clients from the
networks can properly connect to you.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "There has been an unknown error while connecting to your IP.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "No answer has been received from your client.");
define("_MSG_RESULTS_UDP_NOTHING_2", "This can have various reasons, like that
your firewall or router isn't properly configured for Shareaza, or Shareaza is
not running on this port, or not connecting to the networks.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "An answer has been received from your client!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "This means that Shareaza is able to
receive UDP packets from the network.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Congratulations, everything seems to be fine
and Shareaza should work properly.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "At least one problem has been detected and
you probably have to configure your firewall or router for Shareaza.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Make the connection test");

define("_MSG_FORM_TEXT", "For this connection test to work, you must have
Shareaza running.  If it's not running already, start it and make it connect to
the networks.  (It doesn't matter if it connects successfully or not, it's
enough if it's trying.)  Then enter Shareaza's port number in the box below and
click on 'Test'.");

define("_MSG_FORM_IP", "Your IP is %s.");
define("_MSG_FORM_PROXY", "Your proxy is %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Port:");
define("_MSG_FORM_LTO", "Long timeout");
/* the button */
define("_MSG_FORM_BUTTON", "Test");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "How to tell what port Shareaza uses?");
define("_MSG_FORM_PORT_HOWTO", "If you don't know which port Shareaza uses,
then switch to Shareaza and open the settings dialog by selecting
<b>Shareaza&nbsp;Settings</b> from the menu <b>Tools</b>.  Then on the left
panel click on <b>Internet&nbsp;&gt;&nbsp;Connection</b> and look at the
<b>Port</b> field on the right.  This is the port number that Shareaza uses.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "A note about proxy servers");
define("_MSG_FORM_PROXY_HOWTO", "If you have a proxy server enabled in your
browser's settings, then it's possible that this connection test will not be
able to find out the IP of your computer.  If it fails to detect a proxy, it
will test the proxy's IP, instead of yours, which of course will not work.
Disable your proxy temporarily to make this test.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Should I enable 'Long timeout'?");
define("_MSG_FORM_LTO_HOWTO", "Usually, this is not necessary.  By default,
this test will wait for 5 seconds for the TCP and UDP tests, to see if things
work as expected.  Usually, this should be enough, but if your computer is very
stressed, very busy or if you have an unreliable internet connection, it might
take more time for Shareaza to properly respond to the test, in which case 5
seconds may not be enough.  If you enable the long timeout, the test will wait
for 10 seconds.  Obviously, the test will then take longer to complete.");

/* ======== end of file */

?>