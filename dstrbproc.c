#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "procdef.h"
#include "glovaldef.h"
#include "funcdef.h"

int presndcnt = 0, mxsndcnt = 0;
void
dstrb_proc()
{
  DSTRBINFO *dstrinfp;
  int sndcnt, dmsccsndcnt, ix;
  ssize_t sndact;
  struct timeval  nwtm;
  struct dstrb_flag_info {
    int dsmccalv:1;
    int update:1;
  } flg;

  sndcnt = 0;
  dmsccsndcnt = 0;

  dstrinfp = (DSTRBINFO *)NEXT(&dstrbinfo_root);
  while (dstrinfp !=  (DSTRBINFO *)&dstrbinfo_root)
  {
    WDSEQ_SET2BYTE(&rcvinfo.rcv_data[2], dstrinfp->oseqnum);
    dstrinfp->oseqnum++;
    sndact = sendto(dstrinfp->sock, rcvinfo.rcv_data, rcvinfo.rcv_actlen, 0, (struct sockaddr *)&(dstrinfp->dst_addr), sizeof(dstrinfp->dst_addr));
    if (sndact < 0)
    {
      fprintf(stderr, "sendto: %s\n", strerror(errno));
    }
    else
    {
      sndcnt++;
    }
    gettimeofday(&nwtm, NULL);

    if (timercmp(&transinfo.next_trnstime, &nwtm, <=))
    {/* 分配時間タイムアウト */
        if (presndcnt != sndcnt)
        {
          if (mxsndcnt < sndcnt)
          {
            printf("Send TMO %d/%d\n", sndcnt, dstrbinfo_cnt);
            mxsndcnt = sndcnt;
          }
          presndcnt = sndcnt;
        }
        break;
    }
    else
      mxsndcnt = 0;

    /*== DSM-CC データの存在チェックと送出 ==*/
    flg.dsmccalv = 0;
    for (ix=0; ix<DSMCCFILES; ix++)
    {
      if (dstrinfp->dsmcc[ix]._data)
      {
        flg.dsmccalv = 1;
        break;
      }
    }
    //if (flg.dsmccalv)
    {/* DSMCC送出インターバルタイムアウト */
      QUE   *dsmcc_root;
      TSPACKLST *tsplp;
      struct stat sttbf;
      int   rtn;

      flg.update = 0;
      if (dstrinfp->updtfnm)
      {  /* 総合 update チェック */
          rtn = stat(dstrinfp->updtfnm, &sttbf);
          if (0 <= rtn)
          {
              if (memcmp(&(dstrinfp->st_mtim), &sttbf.st_mtim, sizeof(sttbf.st_mtim)))
              {/* いずれかのDSM-CCファイルにアップデートあり */
                  dstrinfp->st_mtim = sttbf.st_mtim;
                  flg.update = 1;
              }
          }
      }

      for (ix=0; ix<DSMCCFILES; ix++)
      {
        if (flg.update)
        {/* ファイル別updateチェック */
          rtn = stat(dstrinfp->dsmcc[ix]._fnm, &sttbf);
          if (0 <= rtn)
          {
            if (memcmp(&(dstrinfp->dsmcc[ix]._st_mtim), &sttbf.st_mtim, sizeof(sttbf.st_mtim)))
            {/* DSM-CCファイルにアップデートあり */
              dsmcc_root = dstrinfp->dsmcc[ix]._data;
              if (dsmcc_root)
              {/* 現在保持しているデータを開放してから、updateデータを取得しなおす */
                DEQUEUE(tsplp, (TSPACKLST *), dsmcc_root);
                while (tsplp != (TSPACKLST *)dsmcc_root)
                {
                  free(tsplp);
                  DEQUEUE(tsplp, (TSPACKLST *), dsmcc_root);
                }
                free(dsmcc_root);
                dstrinfp->dsmcc[ix]._data = NULL;
#ifdef        DSMCCUPDTDBG
                {
                  struct timeval  nwtm;
                  gettimeofday(&nwtm, NULL);
                  printf("%ld.%06ld %s Updata[%d]\n", nwtm.tv_sec, nwtm.tv_usec, inet_ntoa(dstrinfp->dst_addr.sin_addr), ix);
                }
#endif
                /* updateされたデータの取得*/
                dsmcc_preparation(&dstrinfp->dsmcc[ix]);
                dstrinfp->dsmcc[ix]._st_mtim = sttbf.st_mtim;
              }
            }
          }
        }

        dsmcc_root = dstrinfp->dsmcc[ix]._data;
        if (dsmcc_root)
        {/* DSM-CCデータ送出 */
          gettimeofday(&nwtm, NULL);
          if (timercmp(&dstrinfp->dsmcc[ix]._next_time, &nwtm, <=))
          {
              TSPACKSNDBLK  *tspcksdbkp;
              tspcksdbkp = malloc(sizeof(TSPACKSNDBLK));
              if (tspcksdbkp)
              {/* DSM-CCの送出は、UDP受信待ちの空き時間で行う */
                //dstrinfp->dsmcc[ix]._fire_time = nwtm;

                tspcksdbkp->dstrbinfo_home = dstrinfp;
                tspcksdbkp->dsmcc_idx = ix;
                dstrinfp->dsmcc[ix]._data = NULL;
                tspcksdbkp->dsmcc_data = dsmcc_root;
                tspcksdbkp->next = NULL;  // tspcksdbkp->dsmcc_dataを辿る為のアイテム
                ENQUEUE(&ts_pack_send_wroot, tspcksdbkp);
              }
              else
              {
                fprintf(stderr, "No more DSM-CC TSPACKSNDBLK %s[%d]: %s\n", inet_ntoa(dstrinfp->dst_addr.sin_addr), ix, strerror(errno));
              }
            }
        }
      } //for (ix=0; ix<DSMCCFILES; ix++)
      dmsccsndcnt++;
    }
    /*====*/

    dstrinfp = (DSTRBINFO *)NEXT(dstrinfp);
  }
}
