<?xml version="1.0" encoding="UTF-8"?>

<!-- ***********************************************************************
  Stylesheet for transforming XHTML into SSSML markup.
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
 The rendering is roughly based on the stylesheet from Appendix A of the
 CSS2 specification, http://www.w3.org/TR/REC-CSS2/sample.html
 
 @media speech {
 H1, H2, H3, 
 H4, H5, H6    { voice-family: paul, male; stress: 20; richness: 90 }
 H1            { pitch: x-low; pitch-range: 90 }
 H2            { pitch: x-low; pitch-range: 80 }
 H3            { pitch: low; pitch-range: 70 }
 H4            { pitch: medium; pitch-range: 60 }
 H5            { pitch: medium; pitch-range: 50 }
 H6            { pitch: medium; pitch-range: 40 }
 LI, DT, DD    { pitch: medium; richness: 60 }
 DT            { stress: 80 }
 PRE, CODE, TT { pitch: medium; pitch-range: 0; stress: 0; richness: 80 }
 EM            { pitch: medium; pitch-range: 60; stress: 60; richness: 50 }
 STRONG        { pitch: medium; pitch-range: 60; stress: 90; richness: 90 }
 DFN           { pitch: high; pitch-range: 60; stress: 60 }
 S, STRIKE     { richness: 0 }
 I             { pitch: medium; pitch-range: 60; stress: 60; richness: 50 }
 B             { pitch: medium; pitch-range: 60; stress: 90; richness: 90 }
 U             { richness: 0 }
 A:link        { voice-family: harry, male }
 A:visited     { voice-family: betty, female }
 A:active      { voice-family: betty, female; pitch-range: 80; pitch: x-high }
}

As SSML does not seem to offer an equivalent for "stress" and "richness".
They are mapped to rate and volume respectively.

 H1            { male; pitch: x-low;  range: x-high;  rate: slow;  volume: x-loud}
 H2            { male; pitch: x-low;  range: high;    rate: slow;  volume: x-loud }
 H3            { male; pitch: low;    range: high;    rate: slow;  volume: x-loud }
 H4            { male; pitch: medium; range: medium;  rate: slow;  volume: x-loud }
 H5            { male; pitch: medium; range: low;     rate: slow;  volume: x-loud }
 H6            { male; pitch: medium; range: x-low;   rate: slow;  volume: x-loud }
 LI, DD        { pitch: medium; }
 DT            { pitch: medium; rate: x-fast }
 PRE, CODE, TT { pitch: medium; range: x-low;  rate: slow;   volume: loud }
 EM            { pitch: medium; range: medium; rate: medium; volume: loud }
 STRONG        { pitch: medium; range: medium; rate: x-fast; volume: x-loud }
 DFN           { pitch: high;   range: medium; rate: medium }
 S, STRIKE     { volume: x-soft }
 I             { pitch: high; range: medium; rate: fast; volume: medium }
 B             { pitch: high; range: medium; rate: x-fast; volume: x-loud }
 U             { volume: medium }
 A             { female }

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
    <p><xsl:apply-templates/></p>
</xsl:template>

<!--  H1            { pitch: x-low; range: x-high;  rate: slow;  volume: x-loud } -->
<xsl:template match="*[local-name()='h1']">
    <voice gender="male"><prosody pitch="x-low" range="x-high" rate="slow" volume="x-loud">
        <xsl:apply-templates/>
    </prosody></voice>
</xsl:template>

<!-- H2            { male; pitch: x-low;  range: high;  rate: slow;  volume: x-loud } -->
<xsl:template match="*[local-name()='h2']">
    <voice gender="male"><prosody pitch="x-low" range="high" rate="slow" volume="x-loud">
        <xsl:apply-templates/>
    </prosody></voice>
</xsl:template>

<!-- H3            { male; pitch: low;    range: high;  rate: slow;  volume: x-loud } -->
<xsl:template match="*[local-name()='h3']">
    <voice gender="male"><prosody pitch="low" range="high" rate="slow" volume="x-loud">
        <xsl:apply-templates/>
    </prosody></voice>
</xsl:template>

<!-- H4            { male; pitch: medium; range: medium;  rate: slow;  volume: x-loud } -->
<xsl:template match="*[local-name()='h4']">
    <voice gender="male"><prosody pitch="medium" range="medium" rate="slow" volume="x-loud">
        <xsl:apply-templates/>
    </prosody></voice>
</xsl:template>

<!-- H5            { male; pitch: medium; range: low;  rate: slow;  volume: x-loud } -->
<xsl:template match="*[local-name()='h5']">
    <voice gender="male"><prosody pitch="low" range="low" rate="slow" volume="x-loud">
        <xsl:apply-templates/>
    </prosody></voice>
</xsl:template>

<!-- H6            { male; pitch: medium; range: x-low;  rate: slow;  volume: x-loud } -->
<xsl:template match="*[local-name()='h6']">
    <voice gender="male"><prosody pitch="medium" range="x-low" rate="slow" volume="x-loud">
        <xsl:apply-templates/>
    </prosody></voice>
</xsl:template>

<!-- LI, DD        { pitch: medium; } -->
<xsl:template match="*[local-name()='li']">
    <prosody pitch="medium">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>
<xsl:template match="*[local-name()='dd']">
    <prosody pitch="medium">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- DT            { pitch: medium; rate: x-fast } -->
<xsl:template match="*[local-name()='dt']">
    <prosody pitch="medium" rate="x-fast">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- PRE, CODE, TT { pitch: medium; range: x-low;  rate: slow;   volume: loud } -->
<xsl:template match="*[local-name()='pre']">
    <prosody pitch="medium" range="x-low" rate="slow" volume="loud">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>
<xsl:template match="*[local-name()='code']">
    <prosody pitch="medium" range="x-low" rate="slow" volume="loud">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>
<xsl:template match="*[local-name()='tt']">
    <prosody pitch="medium" range="x-low" rate="slow" volume="loud">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- EM            { pitch: medium; range: medium; rate: medium; volume: loud } -->
<xsl:template match="*[local-name()='em']">
    <prosody pitch="medium" range="medium" rate="medium" volume="loud">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- STRONG        { pitch: medium; range: medium; rate: x-fast; volume: x-loud } -->
<xsl:template match="*[local-name()='strong']">
    <prosody pitch="medium" range="medium" rate="x-fast" volume="x-loud">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- DFN           { pitch: high;   range: medium; rate: medium } -->
<xsl:template match="*[local-name()='pre']">
    <prosody pitch="high" range="medium" rate="medium">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- S, STRIKE     { volume: x-soft } -->
<xsl:template match="*[local-name()='s']">
    <prosody volume="x-soft">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>
<xsl:template match="*[local-name()='strike']">
    <prosody volume="x-soft">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- I             { pitch: high; range: medium; rate: fast; volume: medium } -->
<xsl:template match="*[local-name()='i']">
    <prosody pitch="high" range="medium" rate="fast" volume="medium">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- B             { pitch: high; range: medium; rate: x-fast; volume: x-loud } -->
<xsl:template match="*[local-name()='b']">
    <prosody pitch="high" range="medium" rate="x-fast" volume="x-loud">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- U             { volume: medium } -->
<xsl:template match="*[local-name()='u']">
    <prosody pitch="medium">
        <xsl:apply-templates/>
    </prosody>
</xsl:template>

<!-- A            { female } -->
<xsl:template match="*[local-name()='a']">
    <voice gender="female">
        <xsl:apply-templates/>
    </voice>
</xsl:template>

<!-- HREF attribute -->
<xsl:template match="@href">
    <prosody volume="soft">Address</prosody>
    <prosody rate="x-fast">
        <xsl:value-of select="."/>
    </prosody>
</xsl:template>

<!-- Ignore scripts. -->
<xsl:template match="*[local-name()='script']"/>

<!-- Ignore styles. -->
<xsl:template match="*[local-name()='style']"/>

</xsl:stylesheet>
