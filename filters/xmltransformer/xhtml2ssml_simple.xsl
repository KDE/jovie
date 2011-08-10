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
<xsl:template match="*[local-name()='html' or local-name()='HTML']">
    <xsl:apply-templates/>
</xsl:template>

<!-- Ignore header, speak the body of xhtml document. -->
<xsl:template match="*[local-name()='head' or local-name()='HEAD']"/>
<xsl:template match="*[local-name()='body' or local-name()='BODY']">
    <xsl:element name="speak">
        <xsl:copy-of select="/html/@lang"/>
        <xsl:copy-of select="/HTML/@lang"/>
        <xsl:apply-templates/>
    </xsl:element>
</xsl:template>

<!-- Paragraph -->
<xsl:template match="*[local-name()='p' or local-name()='P']">
    <xsl:apply-templates/>
</xsl:template>

<!--  H1, H2, H3, H4, H5, H6:  ignore tag, speak content as sentence. -->
<xsl:template match="*[contains('h1|h2|h3|h4|h5|h6|H1|H2|H3|H4|H5|H6|',concat(local-name(),'|'))]">
    <xsl:apply-templates/>
</xsl:template>

<!-- DFN, LI, DD, DT:  ignore tag, speak content. -->
<xsl:template match="*[contains('dfn|li|dd|dt|DFN|LI|DD|DT|',concat(local-name(),'|'))]">
    <xsl:apply-templates/>
</xsl:template>

<!-- PRE, CODE, TT;  ignore tag, speak content. -->
<xsl:template match="*[contains('pre|code|tt|PRE|CODE|TT|',concat(local-name(),'|'))]">
    <xsl:apply-templates/>
</xsl:template>

<!-- EM, STRONG, I, B, S, STRIKE, U:  speak emphasized.  -->
<xsl:template match="*[contains('em|strong|i|b|s|strike|EM|STRONG|I|B|S|STRIKE|',concat(local-name(),'|'))]">
    <emphasis level="strong">
        <xsl:apply-templates/>
    </emphasis>
</xsl:template>

<!-- A: speak hyperlink emphasized, address fast. -->
<xsl:template match="*[local-name()='a' or local-name()='A']">
    <xsl:if test="@href">
        <emphasis level="moderate">
            <xsl:apply-templates/>
        </emphasis>
        <prosody rate="fast" volume="soft">
            <xsl:value-of select="'Link to. '"/>
            <xsl:value-of select="@href"/>
        </prosody>
    </xsl:if>
</xsl:template>

<!-- Ignore scripts. -->
<xsl:template match="*[local-name()='script' or local-name()='SCRIPT']"/>

<!-- Ignore styles. -->
<xsl:template match="*[local-name()='style' or local-name()='STYLE']"/>

</xsl:stylesheet>
