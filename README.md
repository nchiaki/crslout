# crslout

## コマンドusage

udpdstrbtn rate <転送レート> dtsz <データサイズ> dstcnf <転送先情報ファイル>

## 分配先定義ファイルデータフォーマット

1行、1宛先：

ipadr=＜IPアドレス＞ [port=<ポート番号>]　[update=<更新通知ファイル> [pid1=<PID\> dsmcc1=<DSM-CCデータファイル>] [pid2=<PID\> dsmc2=<DSM-CCデータファイル>] [pid2=<PID\> dsmc2=<DSM-CCデータファイル>]]

<PID\> ::= 4桁迄の16進

<更新通知ファイル>　::= このファイルが更新されると、指定されたdsmccファイルの内、更新された内容をリロードします。



## メモ

* サービスIDは、なんらかで指定（作成）されたものを適用する
* サービスIDは、以下項目と紐づけられている

<pre>
typedef struct datainfob_es_into {
  int   es_valid; /*==0:無効ESデータ*/
  int   es_cmptag;
  int   es_pid;
  int   es_streamtype;
  int   es_datacmp;
  int   es_port;
} DTINFES;

typedef struct datainfob_service_info {
  int   dtb_servno;
  int   dtb_servtype;
  int   dtb_pcrpid;
  int   dtb_resolution;
  DTINFES dtb_es[MAX_DATA_PID_NUM];
} DTINFBSRV;
</pre>

* コンポーネントタグは 0x40(データ放送)

* 設定済み ESデータから以下を作成

<pre>
#define	AVRESTRICT_SRVIDZ	8
#define	AVRESTRICT_OTHERSRVIDZ	4
#define	AVRESTRICT_D4IDMAXCONT	8
#define	AVRESTRICT_MSGBANTXTMAX	128
#define	AVRESTRICT_MSGIDINPUTTXTMAX	128
#define	AVRESTRICT_TICKETNAMEMAX	64
typedef struct avrestriction_baseconf_xmlconf {
	char	abx_avrestrict_mode;
	char	abx_simultaneous;
	int		abx_srvid_own;
	int		abx_dtb_resolution;
	int		abx_srvid_other[AVRESTRICT_OTHERSRVIDZ];
	char	abx_tvpwroff_mode;
	char	abx_d8date_accept_MM;
	char	abx_d8date_accept_DD;
	char	abx_d8date_accept_hh;
	char	abx_d8date_accept_mm;
	char	abx_d8date_term_MM;
	char	abx_d8date_term_DD;
	char	abx_d8date_term_hh;
	char	abx_d8date_term_mm;
	int		abx_d4id[AVRESTRICT_D4IDMAXCONT];
	char	abx_msg4ban_text[AVRESTRICT_MSGBANTXTMAX+1];
	char	abx_msg4ban_bgcolor;
	char	abx_msg4ban_txcolor;
	char	abx_msg4idinput_text[AVRESTRICT_MSGIDINPUTTXTMAX+1];
	char	abx_msg4idinput_bgcolor;
	char	abx_msg4idinput_txcolor;
	char	abx_tv_distinction;
	char	abx_ticket_name1[AVRESTRICT_TICKETNAMEMAX+1];
	char	abx_ticket_name2[AVRESTRICT_TICKETNAMEMAX+1];
} AVREST_BSCNF_XML;
</pre>

* PAT, PMT から PID/table id を取得
