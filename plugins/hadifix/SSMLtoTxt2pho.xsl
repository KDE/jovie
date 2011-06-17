<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" encoding="ISO-8859-1" indent="no"/>

<!-- XSLT stylesheet to convert SSML into a format that can be processed by the
     Hadifix txt2pho processor.

     (c) 2004 by Gary Cramblitt

     Original author: Gary Cramblitt <garycramblitt@comcast.net>
     Current Maintainer: Gary Cramblitt <garycramblitt@comcast.net>

     This program is free software; you can redistribute it and/or modify  *
     it under the terms of the GNU General Public License as published by  *
     the Free Software Foundation; either version 2 of the License, or     *
     (at your option) any later version.                                   *

     The txt2pho processor permits special markup to be embedded in text to control
     speech attributes such as speed (duration), pitch, etc.
     The markup must be inside curly braces and separated from other text by a space.
     Spaces within the markup are not permitted.
     See the txt2pho README file for details.

     Something the README does not say is that {Pitch} markup applies only to one
     sentence and reverts back to normal when the sentence is completed.
     It means that we must repeatedly ouput {Pitch} markup
     for each sentence in the text.  For example, the SSML

         <prosody pitch="high">Sentence one.  Sentence two.</prosody> Sentence three.

     must be converted to

         {Pitch:300} Sentence one. {Pitch:300} Sentence two. Sentence three.

     {Duration} markup, on the other hand, is addative and stays in effect until
     changed.  For example, the SSML

         <prosody rate="fast">Sentence one. Sentence two.</prosody> Sentence three.

     must be converted to

         {Duration:-0.5} Sentence one.  Sentence two. {Duration:0.5} Sentence three.

     txt2pho will also stop processing when it sees a newline.  Therefore, we must take
     care to strip all newlines and avoid inserting any newlines.
     -->

<!-- Strip all elements and attributes from output. -->
<xsl:strip-space elements="*" />

<xsl:template match="speak">
    <xsl:apply-templates/>
</xsl:template>

<!-- Handle markup that maintains state. -->
<xsl:template match="prosody">
    <!-- Rate (speed),  Rates are addative and stay in effect until changed. -->
    <!-- TODO: SSML permits nesting of prosody elements.  As coded below,
         <prosody rate="slow">One<prosody rate="slow">Two</prosody</prosody>
         will pronounce "Two" doubly slow, which it should not do. -->
    <xsl:choose>
        <xsl:when test="@rate='fast'">
            <!-- Increase speed. -->
            <xsl:value-of select="'{Duration:-0.5} '"/>
            <!-- Continue processing. -->
            <xsl:apply-templates/>
            <!-- Decrease speed. -->
            <xsl:value-of select="'{Duration:0.5} '"/>
        </xsl:when>
        <xsl:when test="@rate='slow'">
            <!-- Decrease speed. -->
            <xsl:value-of select="'{Duration:0.5} '"/>
            <!-- Continue processing. -->
            <xsl:apply-templates/>
            <!-- Increase speed. -->
            <xsl:value-of select="'{Duration:-0.5} '"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:apply-templates/>
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>

<!-- Outputs a sentence with txt2pho markup.
     Called in the context of a text node.  Obtain markup by
     extracting ancestor node attributes.
     @param sentence            The sentence to mark up.
     @return                    Sentence preceeded by txt2pho markup.
     -->
<xsl:template name="output-sentence">
    <xsl:param name="sentence"/>
    <!-- Pitch -->
    <xsl:if test="ancestor::prosody/@pitch='high'">
        <xsl:value-of select="'{Pitch:300} '"/>
    </xsl:if>
    <xsl:if test="ancestor::prosody/@pitch='low'">
        <xsl:value-of select="'{Pitch:-40} '"/>
    </xsl:if>
    <!-- Output the sentence itself. -->
    <xsl:value-of select="$sentence"/>
</xsl:template>

<!-- Return the first sentence of argument.
     Sentence delimiters are '.:;!?' 
     @param paragraph           Paragraph from which to extract first sentence.
     @return                    The first sentence, or if no such sentence, the paragraph.
     -->
<xsl:template name="parse-sentence">
    <xsl:param name="paragraph"/>
    <!-- Copy paragraph, replacing all delimeters with period. -->
    <xsl:variable name="tmp">
        <xsl:value-of select="translate($paragraph,':;!?','....')"/>
    </xsl:variable>
    <!-- Look for first period and space and extract corresponding substring from original. -->
    <xsl:choose>
        <xsl:when test="tqcontains($tmp, '. ')">
            <xsl:value-of select="substring($paragraph, 1, string-length(substring-before($tmp, '. '))+2)"/>
        </xsl:when>
        <xsl:otherwise>
            <xsl:value-of select="$paragraph"/>
        </xsl:otherwise>
    </xsl:choose>
</xsl:template>

<!-- Process a paragraph, outputting each sentence with txt2pho markup.
     @param paragraph           The paragraph to process.
     @return                    The paragraph with each sentence preceeded by txt2pho markup.
     -->
<xsl:template name="output-paragraph">
    <xsl:param name="paragraph" />
    <!-- Stop when no more sentences to output. -->
    <xsl:choose>
        <xsl:when test="normalize-space($paragraph)!=''">
            <!-- Split the paragraph into first sentence and rest of paragraph (if any). -->
            <xsl:variable name="sentence">
                <xsl:call-template name="parse-sentence">
                    <xsl:with-param name="paragraph" select="$paragraph"/>
                </xsl:call-template>
            </xsl:variable>
            <!-- debug: <xsl:value-of select="concat('[sentence: ',$sentence,']')"/> -->
            <xsl:variable name="rest">
                <xsl:value-of select="substring-after($paragraph,$sentence)" />
            </xsl:variable>
            <!-- Output the sentence with markup. -->
            <xsl:call-template name="output-sentence" >
                <xsl:with-param name="sentence" select="$sentence" />
            </xsl:call-template>
            <!-- Recursively process the rest of the paragraph. -->
            <xsl:call-template name="output-paragraph">
                <xsl:with-param name="paragraph" select="$rest" />
            </xsl:call-template>
        </xsl:when>
    </xsl:choose>
</xsl:template>

<!-- Process each text node. -->
<xsl:template match="text()">
    <!-- debug: <xsl:value-of select="concat('[paragraph: ',normalize-space(.),']')"/> -->
    <xsl:call-template name="output-paragraph">
        <xsl:with-param name="paragraph" select="concat(normalize-space(.),' ')"/>
    </xsl:call-template>
</xsl:template>

</xsl:stylesheet>
