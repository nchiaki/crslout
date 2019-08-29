#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/time.h>
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
  struct timeval  nwtm, psttm, dsmcctm;
  struct dstrb_flag_info {
    int dsmccalv:1;
    int update:1;
  } flg;

  sndcnt = 0;
  dmsccsndcnt = 0;

  dstrinfp = (DSTRBINFO *)NEXT(&dstrbinfo_root);
  while (dstrinfp !=  (DSTRBINFO *)&dstrbinfo_root)
  {
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
    timersub(&nwtm, &rcvinfo.rcv_time, &psttm); /* UDP受信からの（分配中の）経過時間 */

    if (timercmp(&transinfo.alwble_trnstime, &psttm, <=))
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
    if (flg.dsmccalv)
    {
      timersub(&nwtm, &(dstrinfp->dsmcc_pretv), &dsmcctm); /* DSMCC 送出からの経過時間 */
      if (timercmp(&transinfo.dsmcc_intrvl, &dsmcctm, <=))
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
                {
                  /* 現在保持しているデータを開放する */
                  DEQUEUE(tsplp, (TSPACKLST *), dsmcc_root);
                  while (tsplp != (TSPACKLST *)dsmcc_root)
                  {
                    free(tsplp);
                    DEQUEUE(tsplp, (TSPACKLST *), dsmcc_root);
                  }
                  free(dsmcc_root);
                  dstrinfp->dsmcc[ix]._data = NULL;

                  /* updateされたデータの取得*/
                  dsmcc_preparation(&dstrinfp->dsmcc[ix]);
                  dstrinfp->dsmcc[ix]._st_mtim = sttbf.st_mtim;
                }
              }
            }
          }
          /* DSM-CCデータ送出 */
          dsmcc_root = dstrinfp->dsmcc[ix]._data;
          if (dsmcc_root)
          {
            TSPACKSNDBLK  *tspcksdbkp;
            tspcksdbkp = malloc(sizeof(TSPACKSNDBLK));
            if (tspcksdbkp)
            {/* DSM-CCの送出は、UDP受信待ちの空き時間で行う */
              tspcksdbkp->dstrbinfo_home = dstrinfp;
              tspcksdbkp->dsmcc_idx = ix;
              dstrinfp->dsmcc[ix]._data = NULL;
              tspcksdbkp->dsmcc_data = dsmcc_root;
              tspcksdbkp->next = NULL;
              ENQUEUE(&ts_pack_send_wroot, tspcksdbkp);
            }
            else
            {
              fprintf(stderr, "No more TSPACKSNDBLK: %s\n", strerror(errno));
            }
#if 0
            tsplp = (TSPACKLST *)NEXT(dsmcc_root);
            while (tsplp != (TSPACKLST *)dsmcc_root)
            {
              sndact = sendto(dstrinfp->sock, tsplp->tspack, sizeof(tsplp->tspack), 0, (struct sockaddr *)&(dstrinfp->dst_addr), sizeof(dstrinfp->dst_addr));
              if (sndact < 0)
              {
                fprintf(stderr, "sendto(dmscc%d): %s\n", ix, strerror(errno));
              }
              tsplp = (TSPACKLST *)NEXT(tsplp);
            }
#endif
          }
        }
        dstrinfp->dsmcc_pretv = nwtm;
        dmsccsndcnt++;
      }
    }
    /*====*/

    dstrinfp = (DSTRBINFO *)NEXT(dstrinfp);
  }
}
