<?php
/*
   Copyright (c) zigotozor, 2008
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
   Language file for: Turkish
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Turkish translation by zigotozor");

/* the title (used in various places) */
define("_MSG_TITLE", "Shareaza Bağlantı Testi");
define("_MSG_TITLE_WITH_LINK", "<a href='http://shareaza.sourceforge.net/'>Shareaza</a> Bağlantı Testi");

define("_MSG_LANGUAGES", "Bu bağlantı testini şu dillerde de kullanabilirsiniz:");
define("_MSG_FOOTER", "Bu test hakkında sorularınız ve yorumlarınız için en uygun yer
<a href='http://shareaza.sourceforge.net/phpbb/'>Shareaza Forumlarıdır</a>.");

/* stats line */
define("_MSG_STATS", "Bu testin yapılış sayısı %d&nbsp;kez başlangıç tarihi %s.  Başarı yüzdesi: heriki&nbsp;test:&nbsp;%d%%, sadece&nbsp;TCP:&nbsp;%d%%,
sadece&nbsp;UDP:&nbsp;%d%%, hiçbiri:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Hata");

define("_MSG_ERROR_PORT_INVALID", "Girdiğiniz port numarası gerçerli
bir port değildir, lütfen 1&nbsp;-&nbsp;65535 arası bir port giriniz. Eğer hangi port
numarasını kullanmanız gerektiğini bilmiyorsanız, aşağıdaki
<i>'Shareaza istemcisinin hangi portu kullandığını nasıl bilebilirim?'</i> paragrafını okuyunuz.");

define("_MSG_ERROR_PORT_ZERO", "Girdiğiniz port numarası gerçerli
bir port değildir. Eğer Shareaza istemcisinin port ayarı '0' gösteriyorsa, bu rastgele
seçilen bir portun kullanıldığı anlamına gelir. Bir güvenlik duvarınız ya da yönlendiriciniz
varsa rastgele seçilen bir port kullanamazsınız. Bu durumda <b>Rastgele</b> seçeneğini
seçmeyip güvenlik duvarınızda ya da yönlendiricinizde kuracağınız bir port numarası
girmeniz gerekmektedir. Temel olarak 6346 numaralı port kullanılır, ama istediğiniz herhangi
bir (saklı olmayan) port numarasını girebilirsiniz. Eğer güvenlik duvarınız ya da yönlendiriciniz yoksa
rastgele port kullanabilirsiniz, ama bu test sadece Shareaza istemcisinin şu anda kullandığı
portu biliyorsanız yapılabilir.  <i>Not: Ayarlarda port numarasını değiştirirseniz, bunun
etkinleşmesi için Shareaza istemcisinin bağlantısını kesip onu tekrar çevrimiçine almanız gerekecektir.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "Bağlantı testi IP adresinizi bulamadı. Bu sorunu Shareaza forumlarında belirtiniz.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Eğer güvenlik duvarı ya da yönlendiricinizin ayarları için yardıma ihtiyacınız varsa, wiki'deki şu sayfayı okumanız önerilir: <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters'>FAQ:&nbsp;Routers/Firewalls</a>.");

/* progress box */
define("_MSG_PROGRESS", "Bağlantı testi yapılmaktadır, bu birkaç saniye alabilir, bekleyiniz...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Ayrıntılı kütük");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Ayrıntılı kütük saklanmıştır, gösterilmesi için buraya tıklayınız.");
define("_MSG_DETAIL_HIDE", "Ayrıntılı kütüğü sakla.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Sonuç");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Bu test yapılamamıştır.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Bu sorunu ayrıntılı kütüğün tümünün kopyası ile programcılara iletiniz.");

/* internal error */
define("_MSG_RESULTS_IE", "Sunucu hatası nedeniyle bu test yapılamamıştır.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "TCP bağlantısı zaman aşımına uğramıştır.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Bunun sebebi Shareaza istemcisi için gerektiği gibi ayarlanmamış bir güvenlik duvarı ya da yönlendirici olabilir.");

define("_MSG_RESULTS_TCP_REFUSED_1", "TCP bağlantısı reddedilmiştir, bu port kapalıdır.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Ya bir güvenlik duvarı ya bir yönlendirici bu port için ayarlanmamıştır, ya da hiç bir uygulama bu portu kullanmamaktadır. Güvenlik duvarınızın veya yönlediricinizin bu portu kapatmamak üzere ayarlandığından, bu portu açtığınızdan ve Shareaza istemcisinin etkin ve bu portu kullanıyor olduğundan emin olunuz.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "Bilgisayarınız TCP bağlantısını kabul etmiştir.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Ama soruya cevap vermemiştir. Bu muhtemelen her şeyin yolunda olduğu anlamına gelir. Sadece bu portun başka bir uygulama tarafından değil, Shareaza istemcisi tarafından kullanıldığından emin olunuz.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "Bilgisayarınız TCP bağlantısını kabul etmiş ve soruya cevap vermiştir.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Bu diğer istemcilerin size doğru bir şekilde bağlanabileceklerini gösterir.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "IP adresinize bağlanılırken bilinmeyen bir hata meydana gelmiştir.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "İstemcinizden cevap alınamamıştır.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Bunun güvenlik duvarınızın ya da yönlendiricinizin gerektiği gibi ayarlanmamış olması, Shareaza istemcisinin bu portu kullanmıyor olması, çevrimdışı olması veya ağlara bağlanmayı denemiyor olması gibi çeşitli sebepleri olabilir.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "İstemcinizden cevap alınmıştır!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Bu Shareaza istemcisinin ağdan UDP paketlerini alabildiğini göstermektedir.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Tebrikler, her şey yolunda görünmektedir ve Shareaza normal bir şekilde çalışabilmelidir.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "En azından bir sorun bulunmuştur ve muhtemelen güvenlik duvarınızı ya da yönlendiricinizi ayarlamanız gerekmektedir.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Bağlantı testini yap");

define("_MSG_FORM_TEXT", "Bu testin yapılabilmesi için Shareaza istemcisinin etkin olması gerekmektedir. Eğer Shareaza etkin değilse onu başlatıp bir ağa bağlanmasını sağlayınız. (Başarılı bir şekilde bir ağa bağlanıp bağlanamaması önemli değildir, önemli olan bağlanmayı denemesidir.)  Bunu yaptıktan sonra Shareaza istemcisinin port numarasını aşağıdaki kutuya girip 'Test' butonuna tıklayınız.");

define("_MSG_FORM_IP", "IP adresiniz %s.");
define("_MSG_FORM_PROXY", "Proxy sunucunuz %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Port:");
define("_MSG_FORM_LTO", "Uzun zaman aşımı");
/* the button */
define("_MSG_FORM_BUTTON", "Test");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Shareaza istemcisinin hangi portu kullandığını nasıl bilebilirim?");
define("_MSG_FORM_PORT_HOWTO", "Eğer hangi portu kullanmanız gerektiğini bilmiyorsanız, Shareaza istemcisinin ayarlar penceresini <b>Araçlar</b> menüsünden <b>Shareaza&nbsp;Ayarları</b>nı seçerek  açınız. Bu pencerenin sol kenarında <b>İnternet&nbsp;&gt;&nbsp;Bağlantı</b>ya tıklayınız ve sağdaki Port alanına bakınız. Bu Shareaza istemcisinin kullandığı port numarasıdır.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Proxy sunucular hakkında bir not");
define("_MSG_FORM_PROXY_HOWTO", "Eğer <u>tarayıcınızın</u> ayarlarında proxy sunucusu seçeneğini etkinleştirdiyseniz, bu bu testin bilgisayarınızın IP adresini bulamamasına yol açabilir. Test proxy sunucusunu tespit edemezse sizin IP adresiniz yerine proxy sunucusununkini deneyecek ve doğal olarak bu bir işe yaramayacaktır. Bu test için geçici olarak proxy sunucusu kullanmayınız.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "'Uzun zaman aşımı'nı kullanmalı mıyım?");
define("_MSG_FORM_LTO_HOWTO", "Genelde buna gerek yoktur. Temel olarak, her şeyin yolunda olup olmadığını göstermek  için bu test TCP ve UDP testleri için 5 saniye bekleyecektir.
Genelde bu yeterli bir süredir, ama bilgisayarınız çok fazla iş yapıyorsa ya da İnternet bağlantınızın kalitesi kötüyse, Shareaza istemcisinin teste cevap vermek için daha fazla zamana ihtiyacı olabilir ki, bu durumda 5 saniye yeterli olmayabilir. Uzun zaman aşımını seçerseniz test 10 saniye bekleyecektir. Doğal olarak bu durumda testin tamamlanması daha uzun sürecektir.");

/* ======== end of file */

?>