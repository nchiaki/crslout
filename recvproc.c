#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include  <stdio.h>
#include <stdlib.h>
#include  <errno.h>
#include  <string.h>

#include  "procdef.h"
#include  "glovaldef.h"

long  recv_maxnulls = 0;
unsigned short int  pre_rcv_data_seq;
int   sqchks = 0;
int   sqchers = 0;
#ifdef        DSMCCSNDDBG
struct timeval  pre_dsmcc_strttm[DSMCCFILES];
#endif
void
recv_proc(void)
{
  long  recvnulls;
  struct timeval  nwtm;

  recvnulls = -1;
  do {
    rcvinfo.rcv_actlen = recv(rcvinfo.sock, rcvinfo.rcv_data, RCVDTARAZ, MSG_DONTWAIT);
    if (rcvinfo.rcv_actlen < 0)
    {// UDP 受信無し
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
      {// 空読み状態
        recvnulls++;

        if (!QisEMPTY(&ts_pack_send_wroot))
        {/* DSM-CC送出待ちデータブロック有り */
          TSPACKSNDBLK  *tspcksdbkp;
          DSTRBINFO     *dstrinfp;

          tspcksdbkp = (TSPACKSNDBLK *)NEXT(&ts_pack_send_wroot);
          if (tspcksdbkp != (TSPACKSNDBLK *)&ts_pack_send_wroot)
          {
            dstrinfp = (DSTRBINFO *)tspcksdbkp->dstrbinfo_home; /* DSM-CC送出待ちデータが対象としている分配先のインフォメーション確保*/

            if (!tspcksdbkp->next)
            {
              gettimeofday(&nwtm, NULL);
#ifdef        DSMCCSNDDBG
              struct timeval  tmptm;
              timersub(&nwtm, &pre_dsmcc_strttm[tspcksdbkp->dsmcc_idx], &tmptm);
              printf("%ld.%06ld %s DSMCC[%d] %ld.%06ld\n", nwtm.tv_sec, nwtm.tv_usec, inet_ntoa(dstrinfp->dst_addr.sin_addr), tspcksdbkp->dsmcc_idx, tmptm.tv_sec, tmptm.tv_usec);
              pre_dsmcc_strttm[tspcksdbkp->dsmcc_idx] = nwtm;
#endif
              dstrinfp->dsmcc[tspcksdbkp->dsmcc_idx]._fire_time = nwtm;
              tspcksdbkp->next = NEXT(tspcksdbkp->dsmcc_data);  /* DSM-CC送出待ちデータブロックにおける先頭データ*/
            }
            else
              tspcksdbkp->next = NEXT(tspcksdbkp->next);  /* DSM-CC送出待ちデータブロックにおける継続データ*/

            if (tspcksdbkp->next != tspcksdbkp->dsmcc_data)
            {/* DSM-CC送出待ちデータブロック継続中 */
              TSPACKLST *tsplp;
              ssize_t sndact;

              tsplp = (TSPACKLST *)tspcksdbkp->next;

              memcpy(tsplp->tspack, rcvinfo.rcv_data, RTP_HDRS);
              WDSEQ_SET2BYTE(&tsplp->tspack[2], dstrinfp->oseqnum);
              dstrinfp->oseqnum++;
              sndact = sendto(dstrinfp->sock, tsplp->tspack, tsplp->tspacklen, 0, (struct sockaddr *)&(dstrinfp->dst_addr), sizeof(dstrinfp->dst_addr));
              if (sndact < 0)
              {
                fprintf(stderr, "sendto(dmscc%d): %s\n", tspcksdbkp->dsmcc_idx, strerror(errno));
              }
            }
            else
            {/* DSM-CC送出待ちデータブロック終了 */
              struct timeval  ovhtv, nxttv;
              /* 次回送出時刻を求める */
              gettimeofday(&nwtm, NULL);
              timersub(&nwtm, &dstrinfp->dsmcc[tspcksdbkp->dsmcc_idx]._fire_time, &ovhtv);
              timeradd(&nwtm, &dstrinfp->dsmcc[tspcksdbkp->dsmcc_idx]._alwble_time, &nxttv);
              timersub(&nxttv, &ovhtv, &dstrinfp->dsmcc[tspcksdbkp->dsmcc_idx]._next_time);

              /*DSM-CC送出データ用ルートを分配先のインフォメーションテーブルに戻す */
              dstrinfp->dsmcc[tspcksdbkp->dsmcc_idx]._data = tspcksdbkp->dsmcc_data;

              QREMOVE(tspcksdbkp);
              free(tspcksdbkp);
            }
          }
        }
      }
      else
      {
        fprintf(stderr, "recv : %s\n", strerror(errno));
        exit(1);
      }
    }
  } while (rcvinfo.rcv_actlen <= 0);
  gettimeofday(&rcvinfo.rcv_time, NULL);
  timeradd(&rcvinfo.rcv_time, &transinfo.alwble_trnstime, &transinfo.next_trnstime);

#ifdef  RSVNLLDBG
  if (recv_maxnulls < recvnulls)
  {
    printf("Rcv MAX NULLs %ld\n", recvnulls);
    recv_maxnulls = recvnulls;
  }
#endif

  if (seqchkf)
  {
    unsigned short int rcvseq;
    rcvseq = (rcvinfo.rcv_data[2] << 8) | rcvinfo.rcv_data[3];
    //printf("%u\n",rcvseq);
    if ((pre_rcv_data_seq+1) != rcvseq)
    {
      if (1 < sqchks)
      {
        if (!((pre_rcv_data_seq == 0xffff) && (rcvseq == 0)))
        {
          printf("RD SQER %u -> %u\n", pre_rcv_data_seq, rcvseq);
          sqchers++;
        }
      }
    }
    pre_rcv_data_seq = rcvseq;
    sqchks++;
  }
}
