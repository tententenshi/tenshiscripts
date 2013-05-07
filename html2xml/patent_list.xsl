<?xml version="1.0" encoding="Shift_JIS"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

  <!-- 1番目の keyw が … だったら表示 -->
  <xsl:param name="Expire" />
  <xsl:param name="Category" />
  <xsl:param name="SearchQuery" />

  <xsl:template match="/">
    <xsl:apply-templates/>
  </xsl:template>

  <xsl:template match="特許情報">
<!--
    <html><body>
-->
      <table border="0" cellpadding="0" cellspacing="0" width="100%">
        <tbody>
          <xsl:apply-templates select="特許" />
        </tbody>
      </table>
<!--
    </body></html>
-->
  </xsl:template>

  <xsl:template match="特許">
<!-- 1番目の keyw が … だったら表示 -->
  <xsl:if test="(($Category='')and($SearchQuery='')) or
                (($SearchQuery!='')and((keyw=$SearchQuery))) or
                (keyw[1]=$Category)">

    <xsl:if test="translate(./出願日, '-', '')&lt;$Expire">
      <tr style="background-color: #40846A;"><td>
      <div style="color: rgb(255, 255, 255); font-weight: bold; margin: 0.5em;">期間満了</div>
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
                <xsl:when test="translate(./出願日, '-', '')&lt;$Expire">
                  <xsl:attribute name="style">background-color: #B74545;</xsl:attribute>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:attribute name="style">background-color: red;</xsl:attribute>
                </xsl:otherwise>
                </xsl:choose>
              </xsl:when>
            <xsl:otherwise>
              <xsl:if test="translate(./出願日, '-', '')&lt;$Expire">
                <xsl:attribute name="style">background-color: #99CCB8;</xsl:attribute>
              </xsl:if>
            </xsl:otherwise>
            </xsl:choose>

            <td width="7%"></td>
            <td width="15%"><center><small>出願</small></center></td>
            <td width="15%"><center><small>公開</small></center></td>
            <td width="15%"><center><small>公告</small></center></td>
            <td width="15%"><center><small>登録</small></center></td>
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
                <xsl:value-of select="./出願番号"/>
                </a>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="./出願番号"/>
              </xsl:otherwise>
            </xsl:choose>
            </center></td>
            <td><center>
              <xsl:choose>
              <xsl:when test="./公開URL!=''">
                <a>
                <xsl:attribute name="href">
                  <xsl:value-of select="./公開URL" />
                </xsl:attribute>
                <xsl:value-of select="./公開番号"/>
                </a>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="./公開番号"/>
              </xsl:otherwise>
            </xsl:choose>
            </center></td>
            <td><center>
              <xsl:choose>
              <xsl:when test="./公告URL!=''">
                <a>
                <xsl:attribute name="href">
                  <xsl:value-of select="./公告URL" />
                </xsl:attribute>
                <xsl:value-of select="./公告番号"/>
                </a>
              </xsl:when>
              <xsl:otherwise>
                <xsl:value-of select="./公告番号"/>
              </xsl:otherwise>
            </xsl:choose>
            </center></td>
            <td><center>
              <xsl:choose>
                <xsl:when test="./@status=''">
                  <xsl:attribute name="style">background-color: #BCD6CB;</xsl:attribute>
                    <xsl:choose>
                    <xsl:when test="./登録URL!=''">
                      <a>
                      <xsl:attribute name="href">
                        <xsl:value-of select="./登録URL" />
                      </xsl:attribute>
                      <xsl:value-of select="./登録番号"/>
                      </a>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="./登録番号"/>
                    </xsl:otherwise>
                    </xsl:choose>
                  <xsl:text>pending</xsl:text>
                </xsl:when>
                <xsl:when test="./@status='取下げ'">
                  <xsl:attribute name="style">background-color: #40846A;</xsl:attribute>
                  <span style="color: rgb(255, 255, 255); font-weight: bold;">取下げ</span>
                </xsl:when>
                <xsl:when test="./@status='拒絶'">
                  <xsl:attribute name="style">background-color: #40846A;</xsl:attribute>
                  <span style="color: rgb(255, 255, 255); font-weight: bold;">拒絶</span>
                </xsl:when>
                <xsl:otherwise>
                  <xsl:choose>
                    <xsl:when test="./@status='年金不納'">
                      <xsl:attribute name="style">background-color: #40846A;</xsl:attribute>
                    </xsl:when>
                  </xsl:choose>
                  <span>
                    <xsl:choose>
                      <xsl:when test="./@status='年金不納'">
                        <span style="color: rgb(255, 255, 255); font-weight: bold;">
                          年金不納 - 
                        </span>
                      </xsl:when>
                    </xsl:choose>
                    <xsl:choose>
                    <xsl:when test="./登録URL!=''">
                      <a>
                      <xsl:attribute name="href">
                        <xsl:value-of select="./登録URL" />
                      </xsl:attribute>
                      <xsl:value-of select="./登録番号"/>
                      </a>
                    </xsl:when>
                    <xsl:otherwise>
                      <xsl:value-of select="./登録番号"/>
                    </xsl:otherwise>
                    </xsl:choose>
                  </span>
                </xsl:otherwise>
              </xsl:choose>
            </center></td>
            <td></td>
          </tr>
          <tr bgcolor="#ececdd">
            <td><center>西暦</center></td>
            <td><center><xsl:value-of select="./出願日"/></center></td>
            <td><center><xsl:value-of select="./公開日"/></center></td>
            <td><center><xsl:value-of select="./公告日"/></center></td>
            <td><center><xsl:value-of select="./登録日"/></center></td>
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
                  <tr><td style="width: 2.5em;"><small>出願人</small></td>
                  <td bgcolor="#efffe1"><div style="font-weight:bold;"><xsl:value-of select="出願人"/></div></td></tr>
                </tbody></table>
              </td>
              <td valign="top" style="border: 1px solid seagreen;">
                <table width="100%"><tbody>
                  <tr><td style="width: 2.5em;"><small>発明者</small></td>
                  <td bgcolor="#efffe1"><div>
                    <xsl:for-each select="発明者">
                      <xsl:value-of select="." />
                      <xsl:if test="position()!=last()">、</xsl:if>
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
            <td><center>概要</center></td>
            <td colspan="5"><xsl:copy-of select="概要/*|概要/text()"/></td>
          </tr>
          <tr bgcolor="#ededed">
            <td><center>keyw</center></td>
            <td colspan="5">
              <xsl:for-each select="keyw">
                <xsl:value-of select="." />
                <xsl:if test="position()!=last()">、</xsl:if>
              </xsl:for-each>
            </td>
          </tr>

          <xsl:apply-templates select="コメント"/>

        </tbody>
      </table>
      </td>
    </tr>
    <tr>
      <td><font color="#ffffff">.</font></td>
    </tr>
    </xsl:if>
  </xsl:template>

  <xsl:template match="コメント">
    <tr style="background-color: #FCE5FC;">
      <td><center>コメント</center></td>
      <td colspan="5"><span style="color: rgb(204, 51, 204);"><xsl:copy-of select="*|text()"/></span></td>
    </tr>
  </xsl:template>


</xsl:stylesheet> 