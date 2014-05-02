<?php
/*
   Copyright (c) jlh, Wieldar, 2004 - 2008
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
   Language file for: Russian
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Перевод на русский -- Wieldar");

/* the title (used in various places) */
define("_MSG_TITLE", "Тест соединения Shareaza");
define("_MSG_TITLE_WITH_LINK", "Тест соединения <a href='http://shareaza.sourceforge.net/'>Shareaza</a>");

define("_MSG_LANGUAGES", "Тест соединения также доступен на других языках:");
define("_MSG_FOOTER", "Вопросы и комментарии приветствуются на
<a href='http://shareaza.sourceforge.net/phpbb/'>форуме Shareaza</a>.");

/* stats line */
define("_MSG_STATS", "Этот тест пройден %d&nbsp;раз с %s.  Частота успешного тестирования:
Оба&nbsp;теста:&nbsp;%d%%, только&nbsp;TCP:&nbsp;%d%%,
только&nbsp;UDP:&nbsp;%d%%, не&nbsp;пройдено:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Ошибка");

define("_MSG_ERROR_PORT_INVALID", "Вы ввели неверный порт. Пожалуйста, введите значение и диапазона
1&nbsp;-&nbsp;65535.  Если вы не знаете какой порт вводить, обратитесь к разделу <i>'Как определить порт,
используемый в Shareaza'</i>.");

define("_MSG_ERROR_PORT_ZERO", "Вы ввели неверный порт.  Если в Shareaza установлен порт 0, это
означает, что используется случайный порт. Если вы подключены к интернету через фаерволл
 или роутер (маршрутизатор), то случайный порт использовать нельзя.   В таком случае отключите использование
 случайных портов и введите значение порта, которое вы установите в фаевролле или роутере.
 По умолчанию используется порт 6346, но можно использовать и другие значения. Если вы подключены к интернету
 не через фаерволл или роутер, то можно использовать случайные порты, но для тестирования соединения
 вы должны знать порт, который использует Shareaza в данный момент.  <i>На заметку: При изменении
 значения порта в Shareaza вы должны отключиться и повторно соединиться с сетью (или перезапустить программу).
 </i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "Не удается определить ваш IP-адрес.  Пожалуйста, сообщите об этой ошибке.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Если вам нужна помощь по настройке фаерволла или роутера,
обратитесь к вики:
<a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters/ru'>FAQ:&nbsp;Routers/Firewalls</a>.");

/* progress box */
define("_MSG_PROGRESS", "Начато тестирование соединения, это может занять несколько секунд.");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Отчет");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "Отчет скрыт, щелкните сюда чтобы показать.");
define("_MSG_DETAIL_HIDE", "Скрыть отчет.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Результаты");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Тестирование не проводилось.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Пожалуйста, сообщите об этой ошибке, приложив полный отчет.");

/* internal error */
define("_MSG_RESULTS_IE", "Не удалось провести тестирование из-за внутренней ошибки.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "Вышло время TCP-соединения.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Это возможно из-за неверной настройки фаерволла или роутера.");

define("_MSG_RESULTS_TCP_REFUSED_1", "Невозмжно установить TCP-соединение. Порт закрыт.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Возможно, фаерволл или роутер настроены неправильно или
указанный порт не используется приложением.  Убедитесь, что фаерволл или роутер настроены таким образом
чтобы он не блокировал (или перенаправлял) данные, приходящие на указанный порт. Также, убедитесь, что
Shareaza запущена, и в настройках установлен именно этот порт.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "TCP-соединение принято вашим компьютером.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Но на запрос не было получено никакого ответа.
Скорее всего, это нормально, только убедитесь, что именно Shareaza использует указанный порт, а не
какое-либо другое приожение.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "TCP-соединение принято вашим компьютером и на запрос
получен верный ответ.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Это означает, что другие клиенты сети могут устанавливать с
вами соединение.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "При соединении с вашим IP-адресом произошла неизвестная ошибка.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Ответ от вашего клиента не получен.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Возможны различные причины. Например -- ваш фаерволл или роутер
неверно настроены или Shareaza не использует данный порт, или же она не соединена с сетью.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Ответ от вашего клиента получен!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Это означает, что Shareaza может принимать UDP-пакеты.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Поздравляем! Похоже, что все нормально и Shareaza должна
работать правильно.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Обнаружена по крайней мере одна проблема в соединении. Возможно,
вам следует настроить ваш фаерволл или роутер для Shareaza.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Пройти тест соединения");

define("_MSG_FORM_TEXT", "Чтобы пройти тест соединения вы должны запустить Shareaza.
Если она еще не запущена, запустите ее и подключитесь к сети (не важно -- удастся ей подключиться или
 нет, достаточно чтобы просто пыталась). Затем, введите номер порта, используемого в Shareaza
и нажмите 'Тест'.");

define("_MSG_FORM_IP", "Ваш IP: %s.");
define("_MSG_FORM_PROXY", "Ваш прокси: %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Порт:");
define("_MSG_FORM_LTO", "Длительное ожидание");
/* the button */
define("_MSG_FORM_BUTTON", "Тест");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Как определить порт, используемый в Shareaza?");
define("_MSG_FORM_PORT_HOWTO", "Если вы не знаете какой порт использет Shareaza, откройте меню
<b>Инструменты</b>, далее -- <b>Настройки&nbsp;Shareaza</b>. В левой панели щелкните на
<b>Интернет&nbsp;&gt;&nbsp;Соединение</b> и найдите поле <b>Порт</b> справа. Это и есть номер порта,
используемого в Shareaza.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "О прокси-серверах");
define("_MSG_FORM_PROXY_HOWTO", "Если в настройках вашего браузера указан прокси-сервер, скорее всего
определить ваш IP не удастся. Если прокси обнаружить не получится, то тестироваться будет
соединение с прокси-сервером, а не с вашим компьютером, вследствие чего результат тестирования будет
неверным. Отключите прокси на время тестирования.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Нужно ли включать 'Длительное ожидание'?");
define("_MSG_FORM_LTO_HOWTO", "Обычно -- нет.  По умолчанию, попытки TCP и UDP-соединений
длятся не более 5 секунд.  Обычно, этого вполне достаточно, но если ваш компьютер в момент тестирования
 перегружен или в вашем интернет-соединении частые помехи, то на ответ Shareaza
 может потребоваться больше 5 секунд. Если вы включите опцию 'Длительное ожидание', то
 установка подключения будет ожидаться не более 10 секунд. Очевидно, тестирование займет больше
 времени.");

/* ======== end of file */

?>