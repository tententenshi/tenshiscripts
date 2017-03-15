<?xml version="1.0" encoding="euc-jp"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- 1���ܤ� keyw �� �� ���ä���ɽ�� -->
  <xsl:param name="Expire" />
  <xsl:param name="Category" />
  <xsl:param name="SearchQuery" />

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="�õ�����">
<!--
    <html><body>
-->
      <table border="0" cellpadding="0" cellspacing="0" width="100%">
        <tbody>
          <xsl:apply-templates select="�õ�" />
        </tbody>
      </table>
<!--
    </body></html>
-->
  </xsl:template>

  <xsl:template match="�õ�">
<!-- 1���ܤ� keyw �� �� ���ä���ɽ�� -->
  <xsl:if test="(($Category='')and($SearchQuery='')) or
                (($SearchQuery!='')and((keyw=$SearchQuery))) or
                (keyw[1]=$Category)">

    <xsl:if test="translate(./�д���, '-', '')&lt;$Expire">
      <tr style="background-color: #40846A;"><td>
      <div style="color: rgb(255, 255, 255); font-weight: bold; margin: 0.5em;">������λ</div>
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
                <xsl:when test="translate(./�д���, '-', '')&lt;$Expire">
                  <xsl:attribute name="style">background-color: #B74545;</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:attribute name="style">background-color: red;</xsl:attribute>
                </xsl:otherwise>
                </xsl:choose>
              </xsl:when>
            <xsl:otherwise>
              <xsl:if test="translate(./�д���, '-', '')&lt;$Expire">
                <xsl:attribute name="style">background-color: #99CCB8;</xsl:attribute>
              </xsl:if>
            </xsl:otherwise>
            </xsl:choose>

            <td width="7%"></td>
            <td width="15%"><center><small>�д�</small></center></td>
            <td width="15%"><center><small>����</small></center></td>
            <td width="15%"><center><small>����</small></center></td>
            <td width="15%"><center><small>��Ͽ</small></center></td>
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
                <xsl:value-of select="./�д��ֹ�"/>
                </a>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="./�д��ֹ�"/>
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
                <xsl:value-of select="./�����ֹ�"/>
                </a>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="./�����ֹ�"/>
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
                <xsl:value-of select="./�����ֹ�"/>
                </a>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="./�����ֹ�"/>
              </xsl:otherwise>
            </xsl:choose>
            </center></td>
            <td><center>
              <xsl:choose>
                <xsl:when test="./@status=''">
                  <xsl:attribute name="style">background-color: #BCD6CB;</xsl:attribute>
                    <xsl:choose>
                    <xsl:when test="./��ϿURL!=''">
                      <a>
                      <xsl:attribute name="href">
                        <xsl:value-of select="./��ϿURL" />
                      </xsl:attribute>
                      <xsl:value-of select="./��Ͽ�ֹ�"/>
                      </a>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="./��Ͽ�ֹ�"/>
                    </xsl:otherwise>
                    </xsl:choose>
                  <xsl:text>pending</xsl:text>
                </xsl:when>
                <xsl:when test="./@status='�貼��'">
                  <xsl:attribute name="style">background-color: #40846A;</xsl:attribute>
                  <span style="color: rgb(255, 255, 255); font-weight: bold;">�貼��</span>
                </xsl:when>
                <xsl:when test="./@status='����'">
                  <xsl:attribute name="style">background-color: #40846A;</xsl:attribute>
                  <span style="color: rgb(255, 255, 255); font-weight: bold;">����</span>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:choose>
                    <xsl:when test="./@status='ǯ����Ǽ'">
                      <xsl:attribute name="style">background-color: #40846A;</xsl:attribute>
                    </xsl:when>
                  </xsl:choose>
                  <span>
                    <xsl:choose>
                      <xsl:when test="./@status='ǯ����Ǽ'">
                        <span style="color: rgb(255, 255, 255); font-weight: bold;">
                          ǯ����Ǽ - 
                        </span>
                      </xsl:when>
                    </xsl:choose>
                    <xsl:choose>
                    <xsl:when test="./��ϿURL!=''">
                      <a>
                      <xsl:attribute name="href">
                        <xsl:value-of select="./��ϿURL" />
                      </xsl:attribute>
                      <xsl:value-of select="./��Ͽ�ֹ�"/>
                      </a>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="./��Ͽ�ֹ�"/>
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
            <td><center><xsl:value-of select="./�д���"/></center></td>
            <td><center><xsl:value-of select="./������"/></center></td>
            <td><center><xsl:value-of select="./������"/></center></td>
            <td><center><xsl:value-of select="./��Ͽ��"/></center></td>
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
                  <tr><td style="width: 2.5em;"><small>�д��</small></td>
                  <td bgcolor="#efffe1"><div style="font-weight:bold;"><xsl:value-of select="�д��"/></div></td></tr>
                </tbody></table>
              </td>
              <td valign="top" style="border: 1px solid seagreen;">
                <table width="100%"><tbody>
                  <tr><td style="width: 2.5em;"><small>ȯ����</small></td>
                  <td bgcolor="#efffe1"><div>
                    <xsl:for-each select="ȯ����">
                      <xsl:value-of select="." />
                      <xsl:if test="position()!=last()">��</xsl:if>
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
            <td><center>����</center></td>
            <td colspan="5">
              <xsl:choose>
                <xsl:when test="./̾��!=''">
                  <span style="font-style:italic">
                    ̾�Ρ�
                    <span style="font-weight:bold;"><xsl:copy-of select="̾��/*|̾��/text()"/></span>
                    <br/>
                  </span>
                </xsl:when>
              </xsl:choose>
              <xsl:copy-of select="����/*|����/text()"/>
            </td>
          </tr>
          <tr bgcolor="#ededed">
            <td><center>keyw</center></td>
            <td colspan="5">
              <xsl:for-each select="keyw">
                <xsl:value-of select="." />
                <xsl:if test="position()!=last()">��</xsl:if>
              </xsl:for-each>
            </td>
          </tr>

          <xsl:apply-templates select="������"/>

        </tbody>
      </table>
      </td>
    </tr>
    <tr>
      <td><font color="#ffffff">.</font></td>
    </tr>
    </xsl:if>
  </xsl:template>

  <xsl:template match="������">
    <tr style="background-color: #FCE5FC;">
      <td><center>������</center></td>
      <td colspan="5"><span style="color: rgb(204, 51, 204);"><xsl:copy-of select="*|text()"/></span></td>
    </tr>
  </xsl:template>


</xsl:stylesheet> 