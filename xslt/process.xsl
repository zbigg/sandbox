<?xml version="1.0"?>
<xsl:transform version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

    <xsl:output method="xml" indent="yes"/>

    <xsl:template match="include-file">
        <xsl:param name="file" />
        <xsl:param name="path-prefix">.</xsl:param>
        <xsl:param name="priv-path-prefix">.</xsl:param>
        <xsl:message>include-file <xsl:value-of select="@file"/> (pp=<xsl:value-of select="concat($priv-path-prefix,'/',@path-prefix)"/>)</xsl:message>
        <xsl:comment>begin of <xsl:value-of select="@file"/></xsl:comment>
        <xsl:apply-templates select="document(@file)/priv-module/*">
            <xsl:with-param name="priv-path-prefix" select="concat($priv-path-prefix,'/',@path-prefix)"/>
        </xsl:apply-templates>
        <xsl:comment> end of <xsl:value-of select="@file"/></xsl:comment>
    </xsl:template>

    <xsl:template match="import">
        <xsl:param name="folder" />
        <xsl:param name="priv-path-prefix">.</xsl:param>
        <!--<xsl:message>1 processing element <xsl:value-of select="name(.)"/> with relocation "<xsl:value-of select="$priv-path-prefix"/>"</xsl:message>-->
        <!--<xsl:message>include-path <xsl:value-of select="@folder"/> into <xsl:value-of select="concat($priv-path-prefix,'/',@folder)"/></xsl:message>-->
        <include-path>
           <xsl:attribute name="folder">
               <xsl:value-of select="concat($priv-path-prefix, '/',@folder)"/>
           </xsl:attribute>
        </include-path>
    </xsl:template>

    <xsl:template match="folder">
        <xsl:param name="name" />
        <xsl:param name="priv-path-prefix">.</xsl:param>
        <!--<xsl:message>2 processing element <xsl:value-of select="name(.)"/> with relocation "<xsl:value-of select="$priv-path-prefix"/>"</xsl:message>-->
        <!--<xsl:message>folder <xsl:value-of select="@name"/> into <xsl:value-of select="concat($priv-path-prefix,'/',@name)"/></xsl:message>-->
        <folder>
            <xsl:attribute name="name">
               <xsl:value-of select="concat($priv-path-prefix,'/',@name)"/>
           </xsl:attribute>
           <xsl:apply-templates select="node()">
               <xsl:with-param name="priv-path-prefix" select="$priv-path-prefix"/>
            </xsl:apply-templates>
        </folder>
    </xsl:template>

    <xsl:template match="@*|node()">
        <xsl:param name="priv-path-prefix">.</xsl:param>
        <!--<xsl:message>3 processing element <xsl:value-of select="name(.)"/> with relocation "<xsl:value-of select="$priv-path-prefix"/>"</xsl:message>-->
        <xsl:copy>
            <xsl:apply-templates select="@*"/>
            <xsl:apply-templates select="node()">
                <xsl:with-param name="priv-path-prefix" select="$priv-path-prefix"/>
            </xsl:apply-templates>
        </xsl:copy>
    </xsl:template>
</xsl:transform>