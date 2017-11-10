<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/">
<html><head>
		<meta http-equiv="Content-Type" content="text/html; charset=UTF-8"/>
			<title>
				<xsl:value-of select="@title"/>
			</title>
				<style type="text/css">
					body  { margin: 0px; padding: 0px; background-color: #ffffff; color: #000000; font-family: <xsl:value-of select="@font"/>; font-size: <xsl:value-of select="@fontSize"/>px; }
					h1    { text-align: left; color: #ffffff; height: 64px; margin: 0px; padding: 20px; font-size: 10pt; font-weight: bold; background-image: url(res://shareaza.exe/#2/#221); }
					div { font-size: 8pt; width: 100%;}
					span    { background-color: #e0e8f0; padding: 4px; }
					.num  { width: 40px; text-align: center; }
					.url  { text-align: left; cursor: hand; }
					.size { width: 100px; text-align: center; }
				</style>
			</head>
			<body>
				<h1>
					<xsl:value-of select="@title"/>
				</h1>
					
				<xsl:apply-templates/>
				<script>
					<![CDATA[
							function a_onmouseout(e){
								if (e)
									target = (e.target || e.srcElement)
								else{
									e = window.event
									target = e.srcElement
								}
									
								if (target && target.href )
									window.external.hover('')
							}
							
							function a_onmouseover(e){
								if (e)
									target = (e.target || e.srcElement)
								else{
									e = window.event
									target = e.srcElement
								}
									
								if (target && target.href )
									window.external.hover(target.href);

							}
							
							function a_onclick(e){
								if (e)
									target = (e.target || e.srcElement)
								else{
									e = window.event
									target = e.srcElement
								}
									
								if (target && target.href )
									if ( ! window.external.open(target.href) )
										window.external.download(target.href);
								
								
								e.preventDefault ? e.preventDefault() : (e.returnValue = false);
							}

							
							var links = document.getElementsByTagName("a");
							for (var i = links.length - 1; i+1; i--){
								var link = links[i];
								link.onclick = a_onclick;
								link.onmouseover = a_onmouseover;
								link.onmouseout = a_onmouseout;
							}
					]]>
				</script>
			</body>
		</html>
	</xsl:template>
	<xsl:template match="File">
		<div>
			<span class="num">
				<xsl:value-of select="@index"/>
			</span>
			<span class="size"><xsl:value-of select="@size"/></span>
			<span class="url" >
				<a>
					<xsl:attribute name="href">
						<xsl:value-of select="@urn"/>
					</xsl:attribute>
					<xsl:value-of select="@path"/><xsl:value-of select="@name"/>
				</a>
			</span>
		</div>
	</xsl:template>
</xsl:stylesheet>