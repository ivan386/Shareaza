<?php
/*
   Copyright (c) enigmabr, 2004 - 2008
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
   Language file for: Português (Portuguese)
*/

/* ======== general stuff */

/* set this to '1' if this is a RTL language */
define("_MSG_RTL", 0);

/* empty for English; for other languages do it like this: "Hawaiian
   translation by Hulahula" (in your language of course) */
define("_MSG_TRANSLATOR_STRING", "Traduzido para Português do Brasil por enigmabr");

/* the title (used in various places) */
define("_MSG_TITLE", "Teste de Conexão para o Shareaza");
define("_MSG_TITLE_WITH_LINK", "Teste de Conexão para o <a href='http://shareaza.sourceforge.net/'>Shareaza</a>");

define("_MSG_LANGUAGES", "Este teste também está disponível em:");
define("_MSG_FOOTER", "Problemas e comentários sobre este teste são bem-vindos no
<a href='http://shareaza.sourceforge.net/phpbb/'>Forum do Shareaza</a>.");

/* stats line */
define("_MSG_STATS", "Este teste foi feito %d&nbsp;vezes desde de %s. Sucesso
obtido: Ambos&nbsp;testes:&nbsp;%d%%, TCP&nbsp;somente:&nbsp;%d%%,
UDP&nbsp;somente:&nbsp;%d%%, Nenhum:&nbsp;%d%%.");

/* caption used for various sections containing an error */
define("_MSG_ERROR_CAPTION", "Erro");

define("_MSG_ERROR_PORT_INVALID", "O número da porta colocada não é de uma porta válida,
favor colocar uma porta válida (1&nbsp;à&nbsp;65535).  Se você não sabe qual porta colocar, leia
abaixo <i>'Como descobrir qual porta o Shareaza utiliza?'</i>.");

define("_MSG_ERROR_PORT_ZERO", "A porta colocada não é uma porta válida. Se a
porta configurada no Shareaza está '0', então talvez uma porta aleatória esteja sendo utilizada.
Se você possui um firewall ou router, então não pode usar portas aleatórias.
Nesse caso, desmarque a opção <b>Aleatória</b> e coloque o número de uma porta, que
deverá ser configurada em seu firewall ou router. O padrão é 6346, mas qualquer
porta deverá servir. Se você não possui um firewall ou router, então poderá utilizar uma porta
aleatória se quiser, mas este teste somente pode ser concluído se você souber como descobrir
qual a porta o Shareaza está utilizando. <i>Nota: Quando modificar o número da porta nas
configurações, você deve desconectar e conectar novamente para a modificação ter efeito.</i>");

/* when IP can't be found (this is very unlikely to happen) */
define("_MSG_ERROR_IP", "O teste de conexão não pôde descobrir qual é o seu IP.
Por favor, informe esse problema.");

/* the link to the wiki */
define("_MSG_WIKI_FR", "Se você necessita de ajuda para configurar o seu firewall ou router,
visite está página do wiki: <a href='http://shareaza.sourceforge.net/mediawiki/index.php?title=FAQ.FirewallsRouters/pt'>
FAQ:&nbsp;Routers/Firewalls</a>.");

/* progress box */
define("_MSG_PROGRESS", "Teste de conexão em progresso; isto poderá demorar alguns segundos.
Por favor, aguarde...");

/* ======== detail log */

/* caption of detail log */
define("_MSG_DETAIL_CAPTION", "Informações detalhadas");

/* the two switches for detail log */
define("_MSG_DETAIL_SHOW", "As informações detalhadas estão ocultas, clique aqui para mostrar.");
define("_MSG_DETAIL_HIDE", "Ocultar as informações detalhadas");

/* the detail log itself is not translated */

/* ======== result strings */

/* caption for result section */
define("_MSG_RESULTS_CAPTION", "Resultado");

/* -------- first the general ones */

/* this get's displayed on a test that has not been performed.  (probably never seen by the user) */
define("_MSG_RESULTS_NOT_TESTED", "Este teste não foi realizado.");

/* request to report a bug/problem */
define("_MSG_RESULTS_REPORT", "Por favor, informe este problema com uma cópia completa das
'informações detalhadas'.");

/* internal error */
define("_MSG_RESULTS_IE", "Este teste não pôde ser realizado, ocorreu um erro interno.");

/* the rest of the result strings are made of two parts: a _1 and a _2 part.
   the _1 part is displayed in color and contains a very short summary,
   while _2 gives possible reasons for the situation */

/* -------- TCP results */

define("_MSG_RESULTS_TCP_TIMEOUT_1", "O tempo para a conexão TCP esgotou.");
define("_MSG_RESULTS_TCP_TIMEOUT_2", "Isto pode ser devido ao firewall stealth ou
ao router, que não está configurado corretamente para Shareaza.");

define("_MSG_RESULTS_TCP_REFUSED_1", "A conexão TCP foi negada, a porta está fechada.");
define("_MSG_RESULTS_TCP_REFUSED_2", "Um ou outro; o firewall ou o router não está
configurado para esta porta, ou o aplicativo não está atualmente usando-a. Verifique se o
seu firewall ou router está corretamente configurado para não bloquear ou permitir essa
porta, e que, o Shareaza está rodando e configurado para utilizar esta porta.");

define("_MSG_RESULTS_TCP_CONNECTED_1", "A conexão TCP foi aceita pelo seu computador.");
define("_MSG_RESULTS_TCP_CONNECTED_2", "Mas, não responde ao que foi solicitado.
Provavelmente, está tudo certo. Apenas, verifique que o Shareaza está usando esta porta
e não outro aplicativo.");

define("_MSG_RESULTS_TCP_ANSWERED_1", "A conexão TCP foi aceita pelo seu computador e a
solicitação foi respondida.");
define("_MSG_RESULTS_TCP_ANSWERED_2", "Isto significa que outros clientes da rede
podem conectar corretamente com você.");

/* some error while connecting, but the exact nature is unknown */
define("_MSG_RESULTS_TCP_ERROR", "Ocorreu um erro desconhecido durante a conexão ao seu IP.");

/* -------- UDP results */

define("_MSG_RESULTS_UDP_NOTHING_1", "Sem resposta do seu cliente.");
define("_MSG_RESULTS_UDP_NOTHING_2", "Isto pode ter vários motivos; como que
seu firewall ou router não está corretamente configurado para o Shareaza, ou o Shareaza
não está rodando nesta porta, ou não está conectando na rede.");

define("_MSG_RESULTS_UDP_ANSWERED_1", "Uma resposta foi recebida do seu cliente!");
define("_MSG_RESULTS_UDP_ANSWERED_2", "Isto significa que o Shareaza está ativo para
receber pacotes UDP da rede.");

/* -------- results summary */

define("_MSG_RESULTS_SUMMARY_OK", "Parabéns, tudo parece estar bem e o Shareaza deverá
funcionar corretamente!");

define("_MSG_RESULTS_SUMMARY_NOT_OK", "Pelo menos, um problema foi encontrado e,
provavelmente, você terá que configurar o seu firewall ou router para o Shareaza.");

/* ======== the form */

/* caption of the test box */
define("_MSG_FORM_CAPTION", "Fazendo o teste de conexão");
define("_MSG_FORM_TEXT", "Para realizar este teste de conexão, você deve deixar o Shareaza rodando.
Caso ele não esteja rodando agora, inicie-o e faça a conexão com a rede. (Não importa se conectou com
sucesso ou não, pois ele estar tentando já é o suficiente.) Então, coloque o número da porta
do Shareaza dentro da caixa abaixo e clique em 'Testar'.");

define("_MSG_FORM_IP", "Seu IP é %s.");
define("_MSG_FORM_PROXY", "Seu proxy é %s.");
/* just before the port box */
define("_MSG_FORM_PORT", "Porta:");
define("_MSG_FORM_LTO", "Tempo de resposta longo");
/* the button */
define("_MSG_FORM_BUTTON", "Testar");

define("_MSG_FORM_PORT_HOWTO_CAPTION", "Como descobrir qual porta o Shareaza utiliza?");
define("_MSG_FORM_PORT_HOWTO", "Se você não sabe qual porta o Shareaza utiliza,
então vá para o Shareaza e: no menu <b>Ferramentas</b>, clique em <b>Configurações do
Shareaza...</b> e clique na aba <b>Conexão</b>. Então olhe o número no campo <b>Porta:</b>.
Este é o número da porta que o Shareza utiliza.");

define("_MSG_FORM_PROXY_HOWTO_CAPTION", "Nota sobre o proxy");
define("_MSG_FORM_PROXY_HOWTO", "Se você utiliza um servidor proxy nas configurações do seu navegador,
então possivelmente este teste de conexão não deverá encontrar o IP do seu computador.
Se a detecção de um servidor proxy falhar, deverá ser testado o IP do servidor proxy
ao invés do seu, que certamente fará o teste não funcionar corretamente.
Desative o seu proxy temporariamente para fazer o teste.");

define("_MSG_FORM_LTO_HOWTO_CAPTION", "Eu deverei ativar a opção 'Tempo de resposta longo'?");
define("_MSG_FORM_LTO_HOWTO", "Geralmente não é necessário. Por padrão, este teste deverá
aguardar 5 segundos para o teste das portas TCP e UDP, para ver se funciona como esperado.
Geralmente, isto é o suficiente, mas se o seu computador é muito 'estressado', muito ocupado
ou se você tem uma conexão não confiável, isto talvez faça com que o Shareaza demore mais
para responder corretamente ao teste, pois 5 segundos podem não ser o suficiente.  Se você ativar
a opção 'Tempo de resposta longo', o teste deverá aguardar por 10 segundos. Obviamente, o teste
deverá demorar mais tempo para terminar.");

/* ======== end of file */

?>