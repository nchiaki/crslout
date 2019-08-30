#ifndef _procdef_h
#define _procdef_h

#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "queuedef.h"
/**
##　データ構成
以下のデータを構成する

* 伝送情報（転送レート、転送データサイズ）(起動パラメータ or 保存情報)
* 転送許容経過時間
* 受信パケット情報（送信元アドレス、宛先ポート番号、データ本体）
* パケット受信時刻
* 転送先情報（宛先アドレス、宛先ポート番号）(N件、保存情報)
**/
#define MAC_HDRS  14
#define IPV4_HDRS 20
#define UDP_HDRS  8
#define RTP_HDRS  12

typedef struct trans_info {
  long  trnsrate;
  int   trnsdtsz;
  struct timeval  alwble_trnstime;  // 指定された転送レートを時間に換算
  struct timeval  next_trnstime;  // 次のUDPパケットが入力される予想時刻
  struct timeval  dsmcc_intrvl; // DSM-CCのデフォルト送出間隔時間
} TRNSINFO;

#define TRNS_HDRS (MAC_HDRS+IPV4_HDRS+UDP_HDRS+RTP_HDRS)
#define CALC_PACK_INTVL_USEC(r, d) (1000000/((r/8)/(TRNS_HDRS+d)))

#define DSMCC_INTRVL_SEC  0
#define DSMCC_INTRVL_USEC 100000  //  tr-b15 1/4 5.3.2 DII の最小感覚 100msec

#define RCVDTARAZ 1500
typedef struct recv_info {
  int             sock;
  struct sockaddr_in src_addr;
  socklen_t       addrlen;
  int             dst_port;
  char            rcv_data[RCVDTARAZ];
  int             rcv_actlen;
  struct timeval  rcv_time; // UDPパケット受信時刻
} RCVINFO;

#define DSMCCFILES  3
typedef struct dsmcc_file_info {  /* DSM-CCファイル毎に用意 */
  int             _pid;
  char            *_fnm;
  void            *_data;
  int             _len;
  struct timeval  _alwble_time; /* DSM-CC 次回送出間隔時間 */
  struct timeval  _next_time; /* DSM-CC 次回送出時刻 */
  struct timeval  _fire_time; /* DSM-CC 送信確定時間 */
  struct timespec _st_mtim;  /* 最終修正時刻 */
} DSMCCINF;

typedef struct destribution_info {/* 転送先毎に用意 */
  QUE             queue;
  struct sockaddr_in dst_addr;
  socklen_t       addrlen;
  int             dst_port;
  int             sock;
  DSMCCINF        dsmcc[DSMCCFILES];
  char            *updtfnm;
  struct timespec st_mtim;  /* 最終修正時刻 */
  //struct timeval  dsmcc_pretv;
} DSTRBINFO;

typedef struct  ts_packet_list  { /* TSパケット毎に用意 */
  QUE     queue;
  char    tspack[188];
} TSPACKLST;

/**
dstrbinfo_root
 |
 +-->  DSTRBINFO -----  --->  DSTRBINFO ---> DSTRBINFO ........
        |
        |  DSMCCINF -----
        |   |        ---> root ----> TSPACKLST ---> TSPACKLST ---> TSPACKLST .....
        |   +------------
        |   |        ---> root .... or Null
        |   +------------
        |   |        ---> root .... or Null
        |   +------------
        |
        +------------
**/


typedef struct ts_packet_send_wait_data {/* 送信待ち状態にあるDSM-CCデータ毎に*/
  QUE       queue;
  DSTRBINFO *dstrbinfo_home;
  int       dsmcc_idx;
  void      *dsmcc_data;
  void      *next;
} TSPACKSNDBLK;

/**
ts_pack_send_wroot
  |
  V
TSPACKSNDBLK
  |
  V                              +---------------------------
  TSPACKSNDBLK ----             |           +----+----+----+
   |dstrbinfo_home ----> DSTRBINFO DSMCCINF |     |Null|    |
   |                           |          +-~---+----+----+
   |                           |            A     A    A  (Indicates any one)
   |                           +------------|-----|----|--
   |dsmcc_idx    ---------------------------+-----+----+
   |
   |dsmcc_data ----> root ----> TSPACKLST ---> TSPACKLST ---> TSPACKLST .....
   |                 A           A               A             A
   |next             |           |               |             |
   |  Null or -------+-----------+---------------+-------------+
   +----------------
**/

#define TSHDRZ  4

/**
*** データ加工マクロ
**/
#define	GET_Maskedvalue(v,m,b)	(((v) & ((m)<<(b))) >> (b))
#define	SET_4BYTES(dstp, src)	{uchar *b=dstp; *(b++)=GET_Maskedvalue(src,0x0ff,24); *(b++)=GET_Maskedvalue(src,0x0ff,16); *(b++)=GET_Maskedvalue(src,0x0ff,8); *(b++)=GET_Maskedvalue(src,0x0ff,0);}
#define	SET_2BYTES(dstp, src)	{uchar *b=dstp; *(b++)=GET_Maskedvalue(src,0x0ff,8); *(b++)=GET_Maskedvalue(src,0x0ff,0);}

#endif
