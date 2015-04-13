<?xml version="1.0"?>
<xsl:transform version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

    <xsl:output method="xml" indent="yes"/>

    <xsl:param name="priv-variant-name"/>
    <xsl:param name="priv-variant-target"/>
    <xsl:param name="priv-variant-file"/>

    <xsl:template match="include-file">
        <xsl:param name="file" />
        <xsl:param name="path-prefix">.</xsl:param>
        <xsl:param name="priv-path-prefix">.</xsl:param>
        <xsl:message>include-file <xsl:value-of select="@file"/> (pp=<xsl:value-of select="concat($priv-path-prefix,'/',@path-prefix)"/>)</xsl:message>
        <xsl:comment>begin of <xsl:value-of select="@file"/></xsl:comment>
        <xsl:apply-templates select="document(@file)/priv-module/variant">
            <xsl:with-param name="priv-path-prefix" select="concat($priv-path-prefix,'/',@path-prefix)"/>
            <xsl:with-param name="priv-variant-name" select="$priv-variant-name"/>
            <xsl:with-param name="priv-variant-target" select="$priv-variant-target"/>
            <xsl:with-param name="priv-variant-file" select="@file"/>
        </xsl:apply-templates>
        <xsl:message>include-file END </xsl:message>
        <xsl:comment> end of <xsl:value-of select="@file"/></xsl:comment>
    </xsl:template>

    <xsl:template match="variant">
        <xsl:param name="name"/>
        <xsl:param name="target"/>
        <xsl:message>variant <xsl:value-of select="@file"/> (n=<xsl:value-of select="@name"/> t=<xsl:value-of select="@target"/> f=<xsl:value-of select="$priv-variant-file"/>)</xsl:message>
        <xsl:choose>
                <xsl:when test="$priv-variant-name = ''">
                    <xsl:message>selected! first variant <xsl:value-of select="@file"/></xsl:message>
                    <xsl:apply-templates select="*">
                        <xsl:with-param name="priv-variant-name" select="@name"/>
                        <xsl:with-param name="priv-variant-target" select="@target"/>
                    </xsl:apply-templates>
                </xsl:when>
                <xsl:when test="$priv-variant-name = @name">
                    <xsl:message>selected! sub variant <xsl:value-of select="@file"/></xsl:message>
                    <xsl:apply-templates select="*">
                        <xsl:with-param name="priv-variant-name" select="$priv-variant-name"/>
                        <xsl:with-param name="priv-variant-target" select="$priv-variant-target"/>
                    </xsl:apply-templates>
                </xsl:when>
        </xsl:choose>
    </xsl:template>
    <!--
        variant content content processing
    -->
    <xsl:template match="include-path">
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