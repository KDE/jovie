<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" encoding="ISO-8859-1" indent="no"/>

<xsl:template match="speak">
&lt;SABLE&gt;
    <xsl:apply-templates/>
&lt;/SABLE&gt;
</xsl:template>

<xsl:template match="//prosody">
        <xsl:if test="@pitch='high'">
&lt;PITCH BASE="50%"&gt;<xsl:value-of select="."/>&lt;/PITCH&gt;
        </xsl:if>
        <xsl:if test="@pitch='medium'">
<xsl:value-of select="."/>
        </xsl:if>
        <xsl:if test="@pitch='low'">
&lt;PITCH BASE="-50%"&gt;<xsl:value-of select="."/>&lt;/PITCH&gt;
        </xsl:if>
</xsl:template>

</xsl:stylesheet>
