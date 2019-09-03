# crslout

## コマンドusage

udpdstrbtn rate &lt;転送レート> dtsz &lt;データサイズ> dstcnf &lt;分配先情報ファイル>

<table>
<tr><th>&lt;転送レート></th><td>10進値（必須）</td></tr>
<tr><th>&lt;データサイズ></th><td>10進値（Def. 1370）</td></tr>
</table>

&lt;転送レート>と&lt;データサイズ>からパケット受信間隔時間を求め、
その間を分配時間に割り当てます。


## 「分配先定義ファイル」データフォーマット

1行、1宛先：

<pre>
ipadr=&lt;sIPアドレス> [port=&lt;ポート番号>]　
       [update=&lt;更新通知ファイル> [pid1=&lt;PID> dsmcc1=&lt;DSM-CCデータファイル> [sintrvl1=&lt;m秒>]]
                                [pid2=&lt;PID> dsmcc2=&lt;DSM-CCデータファイル> [sintrvl2=&lt;m秒>]]
                                [pid3=&lt;PID> dsmcc3=&lt;DSM-CCデータファイル> [sintrvl3=&lt;m秒>]]]
</pre>

<table>
<tr><th> &lt;PID> </th><td>16進(0x等の接頭語は不要。付けるとエラーになります)。</td></tr>
<tr><th>sndintrvl1..3</th><td>送出間隔（Def. 100msec）</td></tr>
<tr><th> &lt;m秒></th><td>100未満の指定は 100msec とします。</td></tr>
<tr><th> &lt;更新通知 ファイル></th><td>このファイルが更新されると、指定済みdsmccファイルの内、更新されたファイルの内容のみをリロードします。</td></tr>
<tr><th>&lt;DSM-CC データ ファイル></th><td>「DSM-CC Section」データとしてのテーブル識別子から始まるセクションデータが格納される。</td></tr>
</table>


## メモ
