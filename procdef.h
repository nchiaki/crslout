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
  struct timeval  alwble_trnstime;
  struct timeval  dsmcc_intrvl;
} TRNSINFO;

#define TRNS_HDRS (MAC_HDRS+IPV4_HDRS+UDP_HDRS+RTP_HDRS)
#define CALC_PACK_INTVL_USEC(r, d) (1000000/((r/8)/(TRNS_HDRS+d)))

#define DSMCC_INTRVL_SEC  0
#define DSMCC_INTRVL_USEC 100000

#define RCVDTARAZ 1500
typedef struct recv_info {
  int             sock;
  struct sockaddr_in src_addr;
  socklen_t       addrlen;
  int             dst_port;
  char            rcv_data[RCVDTARAZ];
  int             rcv_actlen;
  struct timeval  rcv_time;
} RCVINFO;

typedef struct destribution_info {
  QUE             queue;
  struct sockaddr_in dst_addr;
  socklen_t       addrlen;
  int             dst_port;
  int             sock;
  int             pid;
  char            *dsmccfnm;
  void            *dsmcc;
  int             dsmcc_len;
  struct timeval  dsmcc_pretv;
  struct timespec st_mtim;  /* 最終修正時刻 */
} DSTRBINFO;

typedef struct  ts_packet_list  {
  QUE     queue;
  char    tspack[188];
} TSPACKLST;

#define TSHDRZ  4

/**
*** データ加工マクロ
**/
#define	GET_Maskedvalue(v,m,b)	(((v) & ((m)<<(b))) >> (b))
#define	SET_4BYTES(dstp, src)	{uchar *b=dstp; *(b++)=GET_Maskedvalue(src,0x0ff,24); *(b++)=GET_Maskedvalue(src,0x0ff,16); *(b++)=GET_Maskedvalue(src,0x0ff,8); *(b++)=GET_Maskedvalue(src,0x0ff,0);}
#define	SET_2BYTES(dstp, src)	{uchar *b=dstp; *(b++)=GET_Maskedvalue(src,0x0ff,8); *(b++)=GET_Maskedvalue(src,0x0ff,0);}

#endif
