<?xml version="1.0" ?>
<!-- 
     File : test5.xsl     
     Author: Keith Isdale <keithisdale@gmail.com>
     Description: stylesheet for test "test5"
     Copyright Reserved Under GPL     
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

  <!-- Test xsl:value-of , xsl:variable, xsl:number 
       for test of xsl:param, xsl:with-param see "test2"
   -->
  <xsl:variable name="fred" select="'man'"/>

  <xsl:template match="/">
    <xsl:variable name="wilma" select="'wilma'"/>
    <xsl:variable name="barney" select="'barney'"/>
    <xsl:variable name="fred">Fred</xsl:variable>    
    <output>Here</output>
  </xsl:template>

</xsl:stylesheet>
