<?xml version="1.0"?>
<xsl:transform version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

    <xsl:output method="xml" indent="yes"/>

    <xsl:template match="import" mode="traverse">
        <xsl:param name="file" />
        <xsl:message>import <xsl:value-of select="@file"/> (f=<xsl:value-of select="@file"/>)</xsl:message>
        <xsl:comment>begin of <xsl:value-of select="@file"/></xsl:comment>
        <xsl:variable name="filedoc" select="document(@file)/module"/>
        <xsl:apply-templates select="$filedoc/import" mode="traverse"/>
        <xsl:copy-of select="$filedoc/*"/>
        <xsl:message>end of import</xsl:message>
        <xsl:comment> end of <xsl:value-of select="@file"/></xsl:comment>
    </xsl:template>

    <xsl:template match="variant" mode="traverse">
        <xsl:param name="name"/>
        <xsl:message>processing root variant <xsl:value-of select="@name"/></xsl:message>
        <xsl:copy-of select="."/>
    </xsl:template>

    <xsl:variable name="all-roots">
       <xsl:apply-templates select="/app/import" mode="traverse"/>
       <xsl:apply-templates select="/app/variant"  mode="traverse"/>
    </xsl:variable>

    <xsl:template match="/app/variant">
        <xsl:variable name="namex" select="@name"/>
        <variant>
           <xsl:attribute name="name" select="@name"/>
           <cpp-sources>
               <xsl:copy-of select="$all-roots/variant[@name=$namex]/cpp-sources/*"/>
           </cpp-sources>
           <java-sources>
               <xsl:copy-of select="$all-roots/variant[@name=$namex]/java-sources/*"/>
           </java-sources>
           <res>
               <xsl:copy-of select="$all-roots/variant[@name=$namex]/res/*"/>
           </res>
        </variant>
    </xsl:template>

    <xsl:template match="node()|@*">
       <!--<xsl:message>3 processing element <xsl:value-of select="name(.)"/> with relocation "<xsl:value-of select="$priv-path-prefix"/>"</xsl:message>-->
       <xsl:copy>
           <xsl:apply-templates select="@*"/>
           <xsl:apply-templates/>
       </xsl:copy>
    </xsl:template>

</xsl:transform>