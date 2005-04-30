<?xml version="1.0" encoding="UTF-8"?>

<!-- ***********************************************************************
  SSMLtoSable.xsl
  Stylesheet for transforming SSML into SABLE markup.
  ============
  Copyright : (C) 2004 by Paul Giannaros
  ============
  Original author: Paul Giannaros <ceruleanblaze@gmail.com>
 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *************************************************************************** -->
<!-- @todo create a doc detailing which parts of SSML this sheet can handle -->

<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" indent="no"/>

<!-- speak: Indicates SSML markup. -->
<xsl:template match="/speak">
    <SABLE><xsl:apply-templates/></SABLE>
</xsl:template>

<!-- sub: The word that the text sounds like as abbreviations 
       can be pronounced differently. For example,
       <sub alias="doctor">Dr.</sub> smith lives at 32 johnson <sub alias="drive">dr.</sub> -->
<xsl:template match="//sub">
    <xsl:choose>
        <xsl:when test="@alias">
            <xsl:value-of select="@alias"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="."/>
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>

<!-- p: Indicate a paragraph of text -->
<xsl:template match="//p">
    <DIV TYPE="paragraph"><xsl:apply-templates/></DIV>
</xsl:template>
<!-- s: Forceefully indicate a sentence (Does not need to be used
       if full stops are present) -->
<xsl:template match="//s">
    <DIV TYPE="sentence"><xsl:apply-templates/></DIV>
</xsl:template>
   
<!-- emphasis: Emphasize a word or group of words. -->
<xsl:template match="//emphasis">
   <!-- SSML and SABLE both take the same values for their attributes -
   strong, moderate, none, reduced -->
   <EM TYPE="{@level}"><xsl:apply-templates/></EM>
</xsl:template>

<xsl:template match="//voice">
    <!--  This is turned off because if Festival lacks an installed voice to match a gender
          or voice name, it bombs out.  argh!
    <xsl:call-template name="voice"><xsl:with-param name="a" select="@*" /></xsl:call-template> -->
    <xsl:apply-templates/>
</xsl:template>

<xsl:template name="voice">
    <xsl:param name="a" />
    <!-- Get the name of the tag we're creating and convert to a SABLE tag. -->
    <xsl:variable name="tag">
        <xsl:choose>
            <xsl:when test="name($a[1])='gender'">SPEAKER</xsl:when>
            <xsl:when test="name($a[1])='age'">SPEAKER</xsl:when>
            <xsl:when test="name($a[1])='name'">SPEAKER</xsl:when>
            <xsl:otherwise><xsl:value-of select="$a[1]"/></xsl:otherwise>
        </xsl:choose>
    </xsl:variable>

    <xsl:element name="{$tag}">
        <!-- Create the right attribute to go with element $tag. -->

        <xsl:choose>
            <!-- gender:
                 The gender of the voice.
                 Values such as male, female, and neutral are supported.  -->
            <xsl:when test="name($a[1])='gender' and $a[1]='male'">
                <xsl:attribute name="GENDER">male1</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='gender' and $a[1]='female'">
                <xsl:attribute name="GENDER">female1</xsl:attribute>
            </xsl:when>
            <!-- If none of the above match, take the users selected value. -->
            <xsl:when test="name($a[1])='gender'">
                <xsl:attribute name="GENDER"><xsl:value-of select=" $a[1]"/></xsl:attribute>
            </xsl:when>

            <!-- age:
                The age of the voice.
                Positive integer values are supported. -->
            <xsl:when test="name($a[1])='age' and number($a[1]) &lt; 10">
                <xsl:attribute name="AGE">child</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='age' and number($a[1]) &lt; 20">
                <xsl:attribute name="AGE">teen</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='age' and number($a[1]) &lt; 30">
                <xsl:attribute name="AGE">younger</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='age' and number($a[1]) &lt; 50">
                <xsl:attribute name="AGE">middle</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='age'">
                <xsl:attribute name="AGE">older</xsl:attribute>
            </xsl:when>

            <!-- name:
                Voice name.  Synth dependent. -->
            <xsl:when test="name($a[1])='name'">
                <xsl:attribute name="NAME"><xsl:value-of select=" $a[1]"/></xsl:attribute>
            </xsl:when>
        </xsl:choose>

        <!-- Recursively call ourself. -->
        <xsl:choose>
            <xsl:when test="$a[2]">
                <xsl:call-template name="voice"><xsl:with-param name="a" select="$a[position()>1]" /></xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates/>
            </xsl:otherwise>
        </xsl:choose>

    </xsl:element>
</xsl:template>

<xsl:template match="prosody">
    <!-- contour and duration not supported. -->
    <xsl:call-template name="prosody"><xsl:with-param name="a"
        select="@pitch|@rate|@volume|@range" />
    </xsl:call-template>
</xsl:template>

<xsl:template name="prosody">
    <xsl:param name="a" />
    <!-- Get the name of the tag we're creating and convert to a SABLE tag. -->
    <xsl:variable name="tag">
        <xsl:choose>
            <xsl:when test="name($a[1])='pitch'">PITCH</xsl:when>
            <xsl:when test="name($a[1])='rate'">RATE</xsl:when>
            <xsl:when test="name($a[1])='volume'">VOLUME</xsl:when>
            <xsl:when test="name($a[1])='range'">PITCH</xsl:when>
            <xsl:otherwise><xsl:value-of select="$a[1]"/></xsl:otherwise>
        </xsl:choose>
    </xsl:variable>

    <xsl:element name="{$tag}">
        <!-- Create the right attribute to go with element $tag. -->

        <xsl:choose>
            <!-- pitch:
            The pitch with which the text is spoken.
            Values such as x-high, high, low, etc. and percentages (+ or -) 
            are supported. -->
            <xsl:when test="name($a[1])='pitch' and $a[1]='x-high'">
                <xsl:attribute name="BASE">70%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='pitch' and $a[1]='high'">
                <xsl:attribute name="BASE">40%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='pitch' and $a[1]='medium'">
                <xsl:attribute name="BASE">0%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='pitch' and $a[1]='low'">
                <xsl:attribute name="BASE">-40%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='pitch' and $a[1]='x-low'">
                <xsl:attribute name="BASE">-70%</xsl:attribute>
            </xsl:when>
            <!-- If none of the above match, take the users selected value. -->
            <xsl:when test="name($a[1])='pitch'">
                <xsl:attribute name="BASE"><xsl:value-of select=" $a[1]"/></xsl:attribute>
            </xsl:when>

            <!-- rate:
            The speed at which the text is spoken.
            Values such as x-fast, fast, slow, etc. and percentages (+ or -) 
            are supported. -->
            <xsl:when test="name($a[1])='rate' and $a[1]='x-fast'">
                <xsl:attribute name="SPEED">70%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='rate' and $a[1]='fast'">
                <xsl:attribute name="SPEED">40%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='rate' and $a[1]='medium'">
                <xsl:attribute name="SPEED">0%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='rate' and $a[1]='slow'">
                <xsl:attribute name="SPEED">-40%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='rate' and $a[1]='x-slow'">
                <xsl:attribute name="SPEED">-70%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='rate'">
                <xsl:attribute name="SPEED"><xsl:value-of select=" $a[1]"/></xsl:attribute>
            </xsl:when>       

            <!-- volume:
            The volume at which the text is spoken.
            Values such as x-loud, loud, quiet, etc. and percentages (+ or -) 
            are supported. -->
            <xsl:when test="name($a[1])='volume' and $a[1]='x-loud'">
                <xsl:attribute name="LEVEL">70%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='volume' and $a[1]='loud'">
                <xsl:attribute name="LEVEL">50%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='volume' and $a[1]='medium'">
                <xsl:attribute name="LEVEL">0%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='volume' and $a[1]='soft'">
                <xsl:attribute name="LEVEL">-50%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='volume' and $a[1]='x-soft'">
                <xsl:attribute name="LEVEL">-70%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='volume' and $a[1]='silent'">
                <xsl:attribute name="LEVEL">-100%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='volume'">
                <xsl:attribute name="LEVEL"><xsl:value-of select=" $a[1]"/></xsl:attribute>
            </xsl:when>       

            <!-- range:
                 The volume at which the text is spoken.
                 Values such as x-high, high, medium, low, x-low, etc. and percentages (+ or -) 
                 are supported. -->
            <xsl:when test="name($a[1])='range' and $a[1]='x-high'">
                <xsl:attribute name="RANGE">70%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='range' and $a[1]='high'">
                <xsl:attribute name="RANGE">40%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='range' and $a[1]='medium'">
                <xsl:attribute name="RANGE">0%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='range' and $a[1]='low'">
                <xsl:attribute name="RANGE">-40%</xsl:attribute>
            </xsl:when>
            <xsl:when test="name($a[1])='range' and $a[1]='x-low'">
                <xsl:attribute name="RANGE">-70%</xsl:attribute>
            </xsl:when>
            <!-- If none of the above match, take the users selected value. -->
            <xsl:when test="name($a[1])='range'">
                <xsl:attribute name="RANGE"><xsl:value-of select=" $a[1]"/></xsl:attribute>
            </xsl:when>

        </xsl:choose>

        <!-- Recursively call ourself. -->
        <xsl:choose>
            <xsl:when test="$a[2]">
                <xsl:call-template name="prosody"><xsl:with-param name="a" select="$a[position()>1]" /></xsl:call-template>
            </xsl:when>
            <xsl:otherwise>
                <xsl:apply-templates/>
            </xsl:otherwise>
        </xsl:choose>
    </xsl:element>
</xsl:template>

</xsl:stylesheet>

