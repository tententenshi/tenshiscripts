<?xml version="1.0" encoding="Shift_JIS"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- 1�Ԗڂ� keyw �� �c ��������\�� -->
  <xsl:param name="Expire" />
  <xsl:param name="Category" />
  <xsl:param name="SearchQuery" />

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="�������">
<!--
    <html><body>
-->
      <table border="0" cellpadding="0" cellspacing="0" width="100%">
        <tbody>
          <xsl:apply-templates select="����" />
        </tbody>
      </table>
<!--
    </body></html>
-->
  </xsl:template>

  <xsl:template match="����">
<!-- 1�Ԗڂ� keyw �� �c ��������\�� -->
  <xsl:if test="(($Category='')and($SearchQuery='')) or
                (($SearchQuery!='')and((keyw=$SearchQuery))) or
                (keyw[1]=$Category)">

    <xsl:if test="translate(./�o���, '-', '')&lt;$Expire">
      <tr style="background-color: #40846A;"><td>
      <div style="color: rgb(255, 255, 255); font-weight: bold; margin: 0.5em;">���Ԗ���</div>
      </td></tr>
    </xsl:if>

    <tr>
      <td style="border: 1px solid gray;">
      <table border="0" cellpadding="1" cellspacing="1" width="100%">
        <tbody>

          <tr>
            <xsl:choose>
              <xsl:when test="@notice!=''">
                <xsl:choose>
                <xsl:when test="translate(./�o���, '-', '')&lt;$Expire">
                  <xsl:attribute name="style">background-color: #B74545;</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:attribute name="style">background-color: red;</xsl:attribute>
                </xsl:otherwise>
                </xsl:choose>
              </xsl:when>
            <xsl:otherwise>
              <xsl:if test="translate(./�o���, '-', '')&lt;$Expire">
                <xsl:attribute name="style">background-color: #99CCB8;</xsl:attribute>
              </xsl:if>
            </xsl:otherwise>
            </xsl:choose>

            <td width="7%"></td>
            <td width="15%"><center><small>�o��</small></center></td>
            <td width="15%"><center><small>���J</small></center></td>
            <td width="15%"><center><small>����</small></center></td>
            <td width="15%"><center><small>�o�^</small></center></td>
            <td></td>
          </tr>

          <tr bgcolor="#E6F3EE">
            <td><center>no.</center></td>
            <td><center>
              <xsl:choose>
              <xsl:when test="./URL!=''">
                <a>
                <xsl:attribute name="href">
                  <xsl:value-of select="./URL" />
                </xsl:attribute>
                <xsl:value-of select="./�o��ԍ�"/>
                </a>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="./�o��ԍ�"/>
              </xsl:otherwise>
            </xsl:choose>
            </center></td>
            <td><center>
              <xsl:choose>
              <xsl:when test="./���JURL!=''">
                <a>
                <xsl:attribute name="href">
                  <xsl:value-of select="./���JURL" />
                </xsl:attribute>
                <xsl:value-of select="./���J�ԍ�"/>
                </a>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="./���J�ԍ�"/>
              </xsl:otherwise>
            </xsl:choose>
            </center></td>
            <td><center>
              <xsl:choose>
              <xsl:when test="./����URL!=''">
                <a>
                <xsl:attribute name="href">
                  <xsl:value-of select="./����URL" />
                </xsl:attribute>
                <xsl:value-of select="./�����ԍ�"/>
                </a>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="./�����ԍ�"/>
              </xsl:otherwise>
            </xsl:choose>
            </center></td>
            <td><center>
              <xsl:choose>
                <xsl:when test="./@status=''">
                  <xsl:attribute name="style">background-color: #BCD6CB;</xsl:attribute>
                    <xsl:choose>
                    <xsl:when test="./�o�^URL!=''">
                      <a>
                      <xsl:attribute name="href">
                        <xsl:value-of select="./�o�^URL" />
                      </xsl:attribute>
                      <xsl:value-of select="./�o�^�ԍ�"/>
                      </a>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="./�o�^�ԍ�"/>
                    </xsl:otherwise>
                    </xsl:choose>
                  <xsl:text>pending</xsl:text>
                </xsl:when>
                <xsl:when test="./@status='�扺��'">
                  <xsl:attribute name="style">background-color: #40846A;</xsl:attribute>
                  <span style="color: rgb(255, 255, 255); font-weight: bold;">�扺��</span>
                </xsl:when>
                <xsl:when test="./@status='����'">
                  <xsl:attribute name="style">background-color: #40846A;</xsl:attribute>
                  <span style="color: rgb(255, 255, 255); font-weight: bold;">����</span>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:choose>
                    <xsl:when test="./@status='�N���s�['">
                      <xsl:attribute name="style">background-color: #40846A;</xsl:attribute>
                    </xsl:when>
                  </xsl:choose>
                  <span>
                    <xsl:choose>
                      <xsl:when test="./@status='�N���s�['">
                        <span style="color: rgb(255, 255, 255); font-weight: bold;">
                          �N���s�[ - 
                        </span>
                      </xsl:when>
                    </xsl:choose>
                    <xsl:choose>
                    <xsl:when test="./�o�^URL!=''">
                      <a>
                      <xsl:attribute name="href">
                        <xsl:value-of select="./�o�^URL" />
                      </xsl:attribute>
                      <xsl:value-of select="./�o�^�ԍ�"/>
                      </a>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="./�o�^�ԍ�"/>
                    </xsl:otherwise>
                    </xsl:choose>
                  </span>
                </xsl:otherwise>
              </xsl:choose>
            </center></td>
            <td></td>
          </tr>
          <tr bgcolor="#ececdd">
            <td><center>����</center></td>
            <td><center><xsl:value-of select="./�o���"/></center></td>
            <td><center><xsl:value-of select="./���J��"/></center></td>
            <td><center><xsl:value-of select="./������"/></center></td>
            <td><center><xsl:value-of select="./�o�^��"/></center></td>
            <td></td>
          </tr>

          <tr>
            <td></td>
            <td colspan="5">
            <div style="margin-left:2em;">
            <table width="70%"><tbody>
            <tr>
              <td width="44%" valign="top" style="border: 1px solid seagreen;">
                <table width="100%"><tbody>
                  <tr><td style="width: 2.5em;"><small>�o��l</small></td>
                  <td bgcolor="#efffe1"><div style="font-weight:bold;"><xsl:value-of select="�o��l"/></div></td></tr>
                </tbody></table>
              </td>
              <td valign="top" style="border: 1px solid seagreen;">
                <table width="100%"><tbody>
                  <tr><td style="width: 2.5em;"><small>������</small></td>
                  <td bgcolor="#efffe1"><div>
                    <xsl:for-each select="������">
                      <xsl:value-of select="." />
                      <xsl:if test="position()!=last()">�A</xsl:if>
                    </xsl:for-each>
                  </div></td></tr>
                </tbody></table>
              </td>
            </tr>
            </tbody></table>
            </div>
            </td>
          </tr>
          <tr bgcolor="#ffffc6">
            <td><center>�T�v</center></td>
            <td colspan="5"><xsl:copy-of select="�T�v/*|�T�v/text()"/></td>
          </tr>
          <tr bgcolor="#ededed">
            <td><center>keyw</center></td>
            <td colspan="5">
              <xsl:for-each select="keyw">
                <xsl:value-of select="." />
                <xsl:if test="position()!=last()">�A</xsl:if>
              </xsl:for-each>
            </td>
          </tr>

          <xsl:apply-templates select="�R�����g"/>

        </tbody>
      </table>
      </td>
    </tr>
    <tr>
      <td><font color="#ffffff">.</font></td>
    </tr>
    </xsl:if>
  </xsl:template>

  <xsl:template match="�R�����g">
    <tr style="background-color: #FCE5FC;">
      <td><center>�R�����g</center></td>
      <td colspan="5"><span style="color: rgb(204, 51, 204);"><xsl:copy-of select="*|text()"/></span></td>
    </tr>
  </xsl:template>


</xsl:stylesheet> 