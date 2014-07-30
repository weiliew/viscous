<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:template match="/ | @* | node()">
    <xsl:copy>
        <xsl:apply-templates select="@* | node()"/>
    </xsl:copy>
</xsl:template>

<!-- TODO - not sure how to remove the component tag as yet - for now, just grep them away at cmd prompt level -->
<xsl:template match="component">
<xsl:variable name="componentname"><xsl:value-of select="@name"/></xsl:variable>
    <xsl:copy> 
    <xsl:apply-templates select="//components/component[@name = $componentname]/node()"/>
    </xsl:copy>
</xsl:template>

<xsl:template match="components"/>

</xsl:stylesheet> 


