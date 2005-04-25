<?xml version="1.0" encoding="UTF-8"?>

<!-- ***********************************************************************
  Stylesheet for transforming XHTML into SSML markup.
  ============
  Copyright : (C) 2005 by Gary Cramblitt
  ============
  Original author: Gary Cramblitt <garycramblitt@comcast.net>
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *************************************************************************** -->

<!-- ***********************************************************************
  The rendering takes a minimalist approach, mapping <b>, <em>, <i>
  etc. to louder voices.  Everything else is pretty much mapped to just
  paragraph and sentence tags.  Hyperlink addresses are spoken fast.
  *********************************************************************** -->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="xml" indent="no"/>

<!-- root -->
<xsl:template match="/">
    <xsl:apply-templates/>
</xsl:template>

<!-- html -->
<!-- local-name() must be used in order to ignore namespaces. -->
<xsl:template match="*[local-name()='html']">
    <xsl:element name="speak">
        <xsl:copy-of select="@lang"/>
        <xsl:apply-templates/>
    </xsl:element>
</xsl:template>

<!-- Ignore header, speak the body of xhtml document. -->
<xsl:template match="*[local-name()='head']"/>
<xsl:template match="*[local-name()='body']">
    <xsl:apply-templates/>
</xsl:template>

<!-- Paragraph -->
<xsl:template match="*[local-name()='p']">
    <xsl:apply-templates/>
</xsl:template>

<!--  H1, H2, H3, H4, H5, H6:  ignore tag, speak content as sentence. -->
<xsl:template match="*[local-name()='h1|h2|h3|h4|h5|h6']">
    <xsl:apply-templates/>
</xsl:template>

<!-- DFN, LI, DD, DT:  ignore tag, speak content. -->
<xsl:template match="*[local-name()='dfn|li|dd|dt']">
    <xsl:apply-templates/>
</xsl:template>

<!-- PRE, CODE, TT;  ignore tag, speak content. -->
<xsl:template match="*[local-name()='pre|code|tt']">
    <xsl:apply-templates/>
</xsl:template>

<!-- EM, STRONG, I, B, S, STRIKE, U:  speak emphasized.  -->
<xsl:template match="*[local-name()='em|strong|i|b|s|strike']">
    <emphasis level="moderate">
        <xsl:apply-templates/>
    </emphasis>
</xsl:template>

<!-- A: speak hyperlink emphasized, address fast. -->
<xsl:template match="*[local-name()='a']">
    <xsl:if test="@href">
        <emphasis level="moderate">
            <xsl:apply-templates/>
        </emphasis>
        <prosody rate="fast">
            <xsl:value-of select="'Link to. '"/>
            <xsl:value-of select="@href"/>
        </prosody>
    </xsl:if>
</xsl:template>

<!-- Ignore scripts. -->
<xsl:template match="*[local-name()='script']"/>

<!-- Ignore styles. -->
<xsl:template match="*[local-name()='style']"/>

</xsl:stylesheet>
