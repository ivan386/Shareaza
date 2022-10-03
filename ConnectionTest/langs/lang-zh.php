<?php
/*
   Copyright (c) sanly, 2005 - 2008
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
   Language file for: 中文 (Chinese)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "简体中文   翻译者 Sanly");

/* the title (used in various places) */
define("_MSG_TITLE", "Shareaza 连接测试");
define("_MSG_TITLE_WITH_LINK", "<a href='http://shareaza.sourceforge.net/'>Shareaza</a> 连接测试");

define("_MSG_LANGUAGES", "此连接测试有以下其他语言版本可用:");
define("_MSG_FOOTER", "欢迎将有关此测试的问题和评论发送到
<a href='http://shareaza.sourceforge.net/phpbb/'>Shareaza 论坛</a>.");

/* stats line */
define("_MSG_STATS", "此测试已经进行了 %d&nbsp;次, 始于 %s. 成功
率: 全部成功:&nbsp;%d%%, 仅&nbsp;TCP:&nbsp;%d%%,
仅&nbsp;UDP:&nbsp;%d%%, 都未成功:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "错误");

define("_MSG_ERROR_PORT_INVALID", "您输入的端口号是一个无效的
端口, 请输入一个有效的数值, 范围为 1&nbsp;-&nbsp;65535. 如果您
不知道输入哪个端口号, 请阅读以下主题 <i>'如何得知 Shareaza 使用
的端口'</i>.");

define("_MSG_ERROR_PORT_ZERO", "您输入的端口不是一个有效的端
口. 如果在 Shareaza 中的端口设置为 '0', 则其表示正在使用的端口为
随机端口. 如果您的网络存在防火墙或路由器, 则您不能使用随机端
口. 在这种情况下请取消勾选 <b>随机</b> 选项并输入一个您将在
您的防火墙或路由器上设置的端口号. 默认的端口为 6346, 但同样可以
为任何端口. 如果您的网络不存在防火墙或路由器, 则您可以使用随机
端口, 但此测试只有在您知道如何找出 Shareaza 当前使用的端口的前提
下才能进行. <i>注意: 如果在设置中更改端口号, 您必须断开连接再
重新连接才能使变更生效.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "此连接测试无法找到您的 IP 地址. 请
报告这一问题.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "如果您需要帮助来设置您的防火墙或路由器, 请访问
wiki: <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters'>FAQ:&nbsp;路由器/防火墙</a>.");

/* progress box */
define("_MSG_PROGRESS", "连接测试正在进行中；可能需要几秒钟, 请稍候...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "详细日志");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "详细日志已隐藏, 单击此处显示.");
define("_MSG_DETAIL_HIDE", "隐藏详细日志.");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "测试结果");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "此测试未进行.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "请报告此问题, 同时附上一份完整的详细日志的副本.");

/* internal error */
define("_MSG_RESULTS_IE", "此测试无法进行, 存在一个内部错误.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "TCP 连接超时.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "可能是因为存在一个隐形防火墙或
路由器没有对 Shareaza 进行正确配置.");

define("_MSG_RESULTS_TCP_REFUSED_1", "TCP 连接被拒绝, 端口已关闭.");
define("_MSG_RESULTS_TCP_REFUSED_2", "可能是因为防火墙或路由器未对
此端口进行配置, 或者没有程序正在使用这一端口. 请确认您的防火墙或
路由器已经正确配置没有拦截或转发这一端口并且 Shareaza 正在运行且
设置为使用这一端口.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "TCP 连接已经被您的计算机接受.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "但没有对此请求作出回应.
这可能没有任何问题. 请确认 Shareaza 正在使用这一端口,
没有其他程序使用.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "TCP 连接已经被您的计算机接受
并且此请求已获得回应.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "这表示网络中的其他客户端可以
正常与您连接.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "在连接到您的 IP 地址时发生未知错误.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "未能从您的客户端接收到回应.");
define("_MSG_RESULTS_UDP_NOTHING_2", "这可能存在很多原因, 例如您的
防火墙或路由器没有对 Shareaza 进行正确配置, 或 Shareaza 未在此端口
上运行, 或没有连接到网络.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "已经从您的客户端接收到回应!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "这表示 Shareaza 可以从网络中
接收 UDP 数据包.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "祝贺, 一切正常并且 Shareaza 可以
正常工作.");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "至少发现一个问题并且
您可能需要为 Shareaza 配置您的防火墙或路由器.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "进行此连接测试");

define("_MSG_FORM_TEXT", "为了使此测试能正常工作, 您必须首先运行
Shareaza. 如果没有运行, 请运行 Shareaza 并使其连接到
网络.  (Shareaza 是否连接成功并不重要, 只要其正在尝试连接
即可.)  然后在以下输入框中输入 Shareaza 的端口号并
单击 '测试'.");

define("_MSG_FORM_IP", "您的 IP 地址为 %s.");
define("_MSG_FORM_PROXY", "您的代理为 %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "端口:");
define("_MSG_FORM_LTO", "延长超时");
/* the button */
define("_MSG_FORM_BUTTON", "测试");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "如何得知 Shareaza 使用的端口?");
define("_MSG_FORM_PORT_HOWTO", "如果您不知道 Shareaza 所使用的端口,
请切换到 Shareaza 并从菜单 <b>工具</b> 通过选择
<b>Shareaza&nbsp;设置</b> 打开设置对话框. 然后在左侧
面板单击 <b>互联&nbsp;&gt;&nbsp;连接</b> 并查看右侧的
<b>端口</b> 区域. 这就是 Shareaza 所使用的端口号.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "关于代理服务器");
define("_MSG_FORM_PROXY_HOWTO", "如果您在您的浏览器设置中
启用了一个代理服务器, 则此连接测试可能无法找到您计算机所
使用的 IP 地址. 如果其未能测出您是否使用代理, 它将测试
代理服务器的 IP 地址, 而不是您自身的 IP 地址, 这当然没有效果.
请暂时禁用您的代理服务器以进行此测试.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "我需要勾选 '延长超时' 吗?");
define("_MSG_FORM_LTO_HOWTO", "通常而言, 这没有必要. 在默认设置下,
此测试将等待 5 秒钟来测试 TCP 和 UDP, 以检测 Shareaza 是否能象预期的一样
工作. 通常, 这已经足够了, 但是如果您的计算机压力很大, 非常
繁忙或是您的互联网连接不可靠, 便会需要更多的时间
让 Shareaza 来正确地回应此测试, 在这种情况下 5 秒钟
可能是不够地. 如果您启用了延长超时, 此测试将会延长到
等待 10 秒钟. 显然, 此测试将会需要更长的时间来完成.");

/* ======== end of file */

?>