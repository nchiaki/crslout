#include <sys/types.h>
#include <sys/socket.h>
#include  <stdio.h>
#include <stdlib.h>
#include  <errno.h>
#include  <string.h>

#include  "procdef.h"
#include  "glovaldef.h"

long  recv_maxnulls = 0;
char  pre_rcv_data_byte;
int   sqchks = 0;
int   sqchers = 0;
void
recv_proc(void)
{
  long  recvnulls;

  recvnulls = -1;
  do {
    rcvinfo.rcv_actlen = recv(rcvinfo.sock, rcvinfo.rcv_data, RCVDTARAZ, MSG_DONTWAIT);
    if (rcvinfo.rcv_actlen < 0)
    {
      if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
      {
        recvnulls++;

        if (!QisEMPTY(&ts_pack_send_wroot))
        {/* DSM-CC送出待ちデータブロック有り */
          TSPACKSNDBLK  *tspcksdbkp;
          DSTRBINFO     *dstrinfp;

          tspcksdbkp = (TSPACKSNDBLK *)NEXT(&ts_pack_send_wroot);
          if (tspcksdbkp != (TSPACKSNDBLK *)&ts_pack_send_wroot)
          {
            if (!tspcksdbkp->next)
              tspcksdbkp->next = NEXT(tspcksdbkp->dsmcc_data);  /* DSM-CC送出待ちデータブロックにおける先頭データ*/
            else
              tspcksdbkp->next = NEXT(tspcksdbkp->next);  /* DSM-CC送出待ちデータブロックにおける継続データ*/

            dstrinfp = (DSTRBINFO *)tspcksdbkp->dstrbinfo_home; /* DSM-CC送出待ちデータが対象としている分配先のインフォメーション確保*/
            if (tspcksdbkp->next != tspcksdbkp->dsmcc_data)
            {/* DSM-CC送出待ちデータブロック継続中 */
              TSPACKLST *tsplp;
              ssize_t sndact;

              tsplp = (TSPACKLST *)tspcksdbkp->next;

              sndact = sendto(dstrinfp->sock, tsplp->tspack, sizeof(tsplp->tspack), 0, (struct sockaddr *)&(dstrinfp->dst_addr), sizeof(dstrinfp->dst_addr));
              if (sndact < 0)
              {
                fprintf(stderr, "sendto(dmscc%d): %s\n", tspcksdbkp->dsmcc_idx, strerror(errno));
              }
            }
            else
            {/* DSM-CC送出待ちデータブロック終了 */
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
  if (recv_maxnulls < recvnulls)
  {
    printf("Rcv MAX NULLs %ld\n", recvnulls);
    recv_maxnulls = recvnulls;
  }

  if (seqchkf)
  {
    if ((pre_rcv_data_byte+1) != rcvinfo.rcv_data[0])
    {
      if (1 < sqchks)
      {
        if (!(((unsigned char)pre_rcv_data_byte == 255) && ((unsigned char)rcvinfo.rcv_data[0] == 0)))
        {
          printf("RD SQER %d -> %d\n", pre_rcv_data_byte, rcvinfo.rcv_data[0]);
          sqchers++;
        }
      }
    }
    pre_rcv_data_byte =  rcvinfo.rcv_data[0];
    sqchks++;
  }
}
