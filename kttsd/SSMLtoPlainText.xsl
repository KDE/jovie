<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text" encoding="ISO-8859-1" indent="no"/>

<xsl:template match="speak">
<xsl:value-of select="."/>
</xsl:template>

</xsl:stylesheet>
