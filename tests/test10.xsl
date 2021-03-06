<?xml version="1.0" ?>
<!-- 
     File : test10.xsl     
     Author: Keith Isdale <keithisdale@gmail.com>
     Description: stylesheet for test "test10"
     Copyright Reserved Under GPL     
-->
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

  <!-- Test the ability to set break points at XML file entities 
       and changing variables -->
  <xsl:template match="/">
    <output>
        <xsl:call-template name="test_set_variable">
          <xsl:with-param name="item" select="'1234'"/>
        </xsl:call-template>
    </output>
    <xsl:apply-templates />
  </xsl:template>

  <xsl:template name="test_set_variable">
    <xsl:param name="item" select="'default-value'"/>
    <xsl:value-of select="$item"/>
  </xsl:template>

  <xsl:template match="result">
        <xsl:apply-templates />
  </xsl:template>

</xsl:stylesheet>
