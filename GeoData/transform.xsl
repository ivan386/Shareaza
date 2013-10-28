<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:output method="xml" version="1.0" encoding="UTF-8" indent="yes"/> 

<xsl:template match="/">
<world>
	<xsl:for-each select="//WorldGPS">

<!-- Add a carriage return and tab -->
<xsl:text>	
</xsl:text>

		<xsl:variable name="currcountry">
			<xsl:value-of select="Code"/>
		</xsl:variable>
		<xsl:variable name="prevcountry">
			<xsl:value-of select="preceding-sibling::WorldGPS[1]/Code"/>
		</xsl:variable>
		<xsl:variable name="nextcountry">
			<xsl:value-of select="following-sibling::WorldGPS[1]/Code"/>
		</xsl:variable>
  		<xsl:if test="$prevcountry!=$currcountry">
  			<xsl:text disable-output-escaping="yes">&lt;country code="</xsl:text>
  			<xsl:copy-of select="$currcountry"/>
  			<xsl:text disable-output-escaping="yes">" name="</xsl:text>
  			<xsl:value-of select="Country"/>
  			<xsl:text disable-output-escaping="yes">"&gt;</xsl:text>
<!-- Add a carriage return -->
<xsl:text>
</xsl:text>
			<city>
			<xsl:attribute name="name">
				<xsl:if test="IsState = 0">
					<xsl:value-of select="PlaceName"/>
				</xsl:if>
			</xsl:attribute>
			<xsl:attribute name="state">
				<xsl:if test="IsState = 0">
					<xsl:value-of select="District"/>
				</xsl:if>
				<xsl:if test="IsState = 1">
					<xsl:value-of select="PlaceName"/>
				</xsl:if>
			</xsl:attribute>
			<xsl:attribute name="latitude">
				<xsl:value-of select="Latitude"/>
			</xsl:attribute>
			<xsl:attribute name="longitude">
				<xsl:value-of select="Longitude"/>
			</xsl:attribute>
			</city>
  		</xsl:if>
  		<xsl:if test="$prevcountry=$currcountry">
<!-- Add a carriage return and tab -->
<xsl:text>	
</xsl:text>
			<city>
			<xsl:attribute name="name">
				<xsl:if test="IsState = 0">
					<xsl:value-of select="PlaceName"/>
				</xsl:if>
			</xsl:attribute>
			<xsl:attribute name="state">
				<xsl:if test="IsState = 0">
					<xsl:value-of select="District"/>
				</xsl:if>
				<xsl:if test="IsState = 1">
					<xsl:value-of select="PlaceName"/>
				</xsl:if>
			</xsl:attribute>
			<xsl:attribute name="latitude">
				<xsl:value-of select="Latitude"/>
			</xsl:attribute>
			<xsl:attribute name="longitude">
				<xsl:value-of select="Longitude"/>
			</xsl:attribute>
			</city>
<!-- Add a carriage return -->
<xsl:text>
</xsl:text>
  		</xsl:if>

		<xsl:if test="$nextcountry!=$currcountry">
<!-- Add a carriage return -->
<xsl:text>
</xsl:text>
			<xsl:text disable-output-escaping="yes">&lt;/country&gt;</xsl:text>
<!-- Add a carriage return -->
<xsl:text>
</xsl:text>
  		</xsl:if>
	</xsl:for-each>

<!-- Add a carriage return -->
<xsl:text>
</xsl:text>
</world>
</xsl:template>

</xsl:stylesheet>