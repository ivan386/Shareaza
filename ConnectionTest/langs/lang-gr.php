<?php
/*
   Copyright (c) jlh, erimitis, 2004 - 2008
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
   Language file for: Greek
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Ελληνική μετάφραση από erimitis");

/* the title (used in various places) */
define("_MSG_TITLE", "Δοκιμή Σύνδεσης του Shareaza");
define("_MSG_TITLE_WITH_LINK", "Δοκιμή Σύνδεσης του <a href='http://shareaza.sourceforge.net/'>Shareaza</a>");

define("_MSG_LANGUAGES", "Αυτή η δοκιμή σύνδεσης είναι επίσης διαθέσιμη στα:");
define("_MSG_FOOTER", "Ερωτήσεις και σχόλια σχετικά με αυτήν τη δοκιμή είναι ευπρόσδεκτα στα
<a href='http://shareaza.sourceforge.net/phpbb/'>Shareaza Forums</a>.");

/* stats line */
define("_MSG_STATS", "Αυτή η δοκιμή έχει γίνει %d&nbsp;φορές από %s.  Δείκτης
επιτυχίας: Αμφότερες&nbsp;δοκιμές:&nbsp;%d%%, TCP&nbsp;μόνο:&nbsp;%d%%,
UDP&nbsp;μόνο:&nbsp;%d%%, καμία:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Σφάλμα");

define("_MSG_ERROR_PORT_INVALID", "Ο αριθμός θύρας που δώσατε δεν είναι έγκυρος.
Παρακαλώ εισάγετε έναν έγκυρο από το εύρος 1&nbsp;-&nbsp;65535.
Αν δεν γνωρίζετε ποια θύρα να εισάγετε, διαβάστε παρακάτω στο
<i>'Εύρεση της θύρας που χρησιμοποιεί το Shareaza'</i>.");

define("_MSG_ERROR_PORT_ZERO", "Ο αριθμός θύρας που δώσατε δεν είναι έγκυρος.
Αν η ρύθμιση θύρας στο Shareaza είναι '0', σημαίνει ότι χρησιμοποιείται μια τυχαία θύρα.
Αν έχετε έναν firewall ή router, τότε δεν μπορείτε να χρησιμοποιήσετε τυχαία θύρα.
Σε αυτήν την περίπτωση αποεπιλέξτε τη ρύθμιση <b>Τυχαία</b>
και εισάγετε έναν αριθμό θύρας τον οποίο θα δηλώσετε στον firewall ή router σας.
Η προεπιλεγμένη θύρα είναι η 6346, αλλά κάθε θύρα κάνει.
Αν δεν έχετε firewall ή router, τότε μπορείτε να χρησιμοποιήσετε τυχαία θύρα,
αλλά αυτή η δοκιμή μπορεί να γίνει μόνο αν ξέρετε πώς να βρείτε την τρέχουσα θύρα του Shareaza.
<i>Σημείωση: Όταν αλλάζετε τον αριθμό θύρας στις ρυθμίσεις,
πρέπει να αποσυνδεθείτε και να επανασυνδεθείτε προκειμένου η αλλαγή να ισχύσει.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "Η δοκιμή σύνδεσης δεν μπόρεσε να βρει την IP διεύθυνσή σας.
Παρακαλώ αναφέρετε αυτό το πρόβλημα.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Αν χρειαστείτε βοήθεια στην παραμετροποίηση του firewall ή router σας, επισκεφθείτε την εξής σελίδα στο wiki:
<a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters'>FAQ:&nbsp;Routers/Firewalls</a>.");

/* progress box */
define("_MSG_PROGRESS", "Η δοκιμή σύνδεσης εξελίσσεται αυτήν τη στιγμή και ενδέχεται να διαρκέσει λίγα δευτερόλεπτα. Παρακαλώ περιμένετε...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Καταγραφή λεπτομερειών");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Η καταγραφή λεπτομερειών είναι κρυμμένη. Κλικ εδώ για εμφάνιση.");
define("_MSG_DETAIL_HIDE", "Απόκρυψη καταγραφής λεπτομερειών.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Αποτελέσματα");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Αυτή η δοκιμή δεν εκτελέστηκε.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Παρακαλώ αναφέρετε αυτό το πρόβλημα με ένα αντίγραφο της πλήρης καταγραφής λεπτομερειών.");

/* internal error */
define("_MSG_RESULTS_IE", "Αυτή η δοκιμή δεν μπόρεσε να εκτελεστεί, λόγω εσωτερικού σφάλματος.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "Η σύνδεση TCP έληξε.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Αυτό μπορεί να οφείλεται σε αθόρυβο firewall ή
router που δεν έχει ρυθμιστεί κατάλληλα για το Shareaza.");

define("_MSG_RESULTS_TCP_REFUSED_1", "Η σύνδεση TCP απορρίφθηκε. Η θύρα είναι κλειστή.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Ένας firewall ή router δεν έχει ρυθμιστεί
σε αυτήν τη θύρα ή καμιά εφαρμογή δεν τη χρησιμοποιεί αυτήν τη στιγμή.  Επιβεβαιώστε
ότι ο firewall ή router σας είναι ρυθμισμένος κατάλληλα ώστε να μην αποκλείει ή προωθεί αυτήν τη
θύρα και ότι το Shareaza εκτελείται ρυθμισμένο να χρησιμοποιεί αυτήν τη θύρα.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "Ο υπολογιστής σας δέχτηκε τη σύνδεση TCP, όμως δεν απάντησε στην αίτηση.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Αυτό ενδέχεται να είναι φυσιολογικό,
αρκεί να επιβεβαιώσετε ότι αυτή η θύρα χρησιμοποιείται από το Shareaza
και όχι από κάποια άλλη εφαρμογή.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "Ο υπολογιστής σας δέχτηκε τη σύνδεση TCP
και απάντησε στην αίτηση.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Αυτό σημαίνει ότι άλλα προγράμματα από το δίκτυο
μπορούν να συνδεθούν σε εσάς κανονικά.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "Ένα άγνωστο σφάλμα συνέβη κατά τη σύνδεση στη IP διεύθυνσή σας.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Δεν ελήφθη απάντηση από το πρόγραμμά σας.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Αυτό μπορεί να οφείλεται σε διάφορες αιτίες,
όπως ότι ο firewall ή router σας δεν είναι κατάλληλα ρυθμισμένος για το Shareaza
ή ότι το Shareaza δεν εκτελείται σε αυτήν τη θύρα ή δεν συνδέεται στα δίκτυα.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Ελήφθη απάντηση από το πρόγραμμά σας!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Αυτό σημαίνει ότι το Shareaza είσαι σε θέση
να λάβει πακέτα UDP από το δίκτυο.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Συγχαρητήρια! Όλα φαίνονται ορθά
και το Shareaza πρέπει να λειτουργεί κανονικά.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Εντοπίστηκε τουλάχιστον ένα πρόβλημα και
πιθανότατα πρέπει να ρυθμίσετε τον firewall ή router σας για το Shareaza.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Εκτέλεση της δοκιμής σύνδεσης");

define("_MSG_FORM_TEXT", "Για να λειτουργήσει αυτή η δοκιμή σύνδεσης, το Shareaza πρέπει να εκτελείται.
Αν δεν εκτελείται ήδη, εκκινήστε το και ζητήστε του να συνδεθεί στα δίκτυα.
(Δεν παίζει ρόλο αν καταφέρνει να συνδεθεί ή όχι, αρκεί να προσπαθεί.)
Στη συνέχεια εισάγετε τον αριθμό θύρας του Shareaza στο παρακάτω πεδίο
και κάντε κλικ στο 'Δοκιμή'.");

define("_MSG_FORM_IP", "Η IP διεύθυνσή σας είναι %s.");
define("_MSG_FORM_PROXY", "Ο proxy σας είναι %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Θύρα:");
define("_MSG_FORM_LTO", "Μεγάλη προθεσμία");
/* the button */
define("_MSG_FORM_BUTTON", "Δοκιμή");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Εύρεση της θύρας που χρησιμοποιεί το Shareaza");
define("_MSG_FORM_PORT_HOWTO", "Αν δεν γνωρίζετε ποια θύρα χρησιμοποιεί το Shareaza,
τότε πηγαίντε στο Shareaza και ανοίξτε τον διάλογο ρυθμίσεων επιλέγοντας
<b>Ρυθμίσεις&nbsp;Shareaza</b> από το μενού <b>Εργαλεία</b>.  Μετά κάντε κλικ
στο αριστερό πεδίο στο <b>Διαδίκτυο&nbsp;&gt;&nbsp;Σύνδεση</b> και βρείτε το πεδίο
<b>Θύρα</b> στα δεξιά.  Αυτός είναι ο αριθμός θύρας που χρησιμοποιεί το Shareaza.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Σημείωση σχετικά με τους proxy servers");
define("_MSG_FORM_PROXY_HOWTO", "Αν έχετε κάποιον proxy server ενεργοποιημένο
στις ρυθμίσεις του περιηγητή σας, τότε είναι πιθανό αυτή η δοκιμή σύνδεσης να μην
μπορέσει να βρει τη IP διεύθυνση του υπολογιστή σας.  Αν αποτύχει να εντοπίσει κάποιον proxy,
θα δοκιμάσει τη IP διεύθυνση του proxy αντί για τη δική σας, με αποτέλεσμα να μη λειτουργήσει.
Για να λειτουργήσει, απενεργοποιήστε προσωρινά τον proxy σας.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Πρέπει να ενεργοποιήσω τη 'Μεγάλη προθεσμία';");
define("_MSG_FORM_LTO_HOWTO", "Συνήθως, δεν χρειάζεται.  Προεπιλεγμένα,
αυτή η δοκιμή θα περιμένει 5 δευτερόλεπτα για τις δοκιμές TCP και UDP, για να δει αν τα πράγματα
λειτουργούν όπως αναμένεται.  Συνήθως, αυτό είναι αρκετό, αλλά αν ο υπολογιστής σας είναι
πολύ φορτωμένος, πολύ απασχολημένος ή αν έχετε αναξιόπιστη σύνδεση στο διαδίκτυο,
μπορεί να πάρει στο Shareaza περισσότερο χρόνο από 5 δευτερόλεπτα για να απαντήσει στη δοκιμή.
Αν ενεργοποιήσετε τη μεγάλη προθεσμία, η δοκιμή θα περιμένει 10 δευτερόλεπτα.
Τότε προφανώς η δοκιμή θα πάρει περισσότερο μέχρι να ολοκληρωθεί.");

/* ======== end of file */

?>