#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "procdef.h"
#include "glovaldef.h"

int presndcnt = 0, mxsndcnt = 0;
void
dstrb_proc()
{
  DSTRBINFO *dstrinfp;
  int sndcnt, dmsccsndcnt, ix, dsmccalv;
  ssize_t sndact;
  struct timeval  nwtm, psttm, dsmcctm;

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
    dsmccalv = 0;
    for (ix=0; ix<DSMCCFILES; ix++)
    {
      if (dstrinfp->dsmcc[ix]._data)
      {
        dsmccalv = 1;
        break;
      }
    }
    if (dsmccalv)
    {
      timersub(&nwtm, &(dstrinfp->dsmcc_pretv), &dsmcctm); /* DSMCC 送出からの経過時間 */
      if (timercmp(&transinfo.dsmcc_intrvl, &dsmcctm, <=))
      {/* DSMCC送出インターバルタイムアウト */
        QUE   *dsmcc_root;
        TSPACKLST *tsplp;

        for (ix=0; ix<DSMCCFILES; ix++)
        {
          dsmcc_root = dstrinfp->dsmcc[ix]._data;

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
        }
        dstrinfp->dsmcc_pretv = nwtm;
        dmsccsndcnt++;
      }
    }
    /*====*/

    dstrinfp = (DSTRBINFO *)NEXT(dstrinfp);
  }
}
