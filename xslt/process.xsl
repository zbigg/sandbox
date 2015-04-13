<?xml version="1.0"?>
<xsl:transform version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

    <xsl:output method="xml" indent="yes"/>

    <xsl:template match="import" mode="traverse">
        <xsl:param name="file" />
        <xsl:variable name="filedoc" select="document(@file)/module"/>
        <xsl:apply-templates select="$filedoc/import" mode="traverse"/>
        <xsl:copy-of select="$filedoc/variant"/>
    </xsl:template>

    <xsl:template match="variant" mode="traverse">
        <xsl:param name="name"/>
        <xsl:copy-of select="."/>
    </xsl:template>

    <xsl:variable name="all-roots">
       <xsl:apply-templates select="/app/import" mode="traverse"/>
       <xsl:apply-templates select="/app/variant"  mode="traverse"/>
    </xsl:variable>

    <!--
        aggregated nodes
    -->
    <xsl:template match="cpp-sources" mode="merge">
      <xsl:param name="current_variant"/>
      <cpp-sources>
         <xsl:copy-of select="$all-roots/variant[@name=$current_variant]/cpp-sources/*"/>
      </cpp-sources>
    </xsl:template>
    <xsl:template match="java-sources" mode="merge">
      <xsl:param name="current_variant"/>
      <java-sources>
         <xsl:copy-of select="$all-roots/variant[@name=$current_variant]/java-sources/*"/>
      </java-sources>
    </xsl:template>
    <xsl:template match="res" mode="merge">
      <xsl:param name="current_variant"/>
      <res>
         <xsl:copy-of select="$all-roots/variant[@name=$current_variant]/res/*"/>
      </res>
    </xsl:template>

    <xsl:template match="node()|@*" mode="merge">
       <!--<xsl:message>3 processing element <xsl:value-of select="name(.)"/> with relocation "<xsl:value-of select="$priv-path-prefix"/>"</xsl:message>-->
       <xsl:copy>
           <xsl:apply-templates select="@*"/>
           <xsl:apply-templates/>
       </xsl:copy>
    </xsl:template>

    <xsl:template match="/app/variant">
        <xsl:param name="name"/>
        <xsl:message>processing root variant <xsl:value-of select="@name"/></xsl:message>
        <xsl:variable name="namex" select="@name"/>
        <variant>
           <xsl:apply-templates select="@*"/>

           <xsl:apply-templates select="*" mode="merge">
               <xsl:with-param name="current_variant" select="$namex"/>
           </xsl:apply-templates>
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