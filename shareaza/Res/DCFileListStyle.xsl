<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="/">
		<html>
			<head>
				<style>
				body
				{
					font-family:verdana;
				}
				</style>
			</head>
			<body>
				<ul>
				<xsl:apply-templates/>
				</ul>
			</body>
		</html>
	</xsl:template>
	<xsl:template match="Directory">
		<li>
			<xsl:value-of select="@Name"/>
			<ul><xsl:apply-templates/></ul>
		</li>
		
		
	</xsl:template>
	<xsl:template match="File">
		<li>
			<a>
				<xsl:attribute name="href">
					
					magnet:?xl=<xsl:value-of select="@Size"/>&amp;xs=urn:tree:tiger:<xsl:value-of select="@TTH"/><xsl:apply-templates select="@BR"/>&amp;dn=<xsl:value-of select="@Name"/>
				</xsl:attribute>
				<xsl:value-of select="@Name"/>
			</a>
		</li>
	</xsl:template>
	<xsl:template match="@BR">
		&amp;br=<xsl:value-of select="."/>000
	</xsl:template>
</xsl:stylesheet>