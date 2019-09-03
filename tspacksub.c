
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <netdb.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include  "procdef.h"
#include  "glovaldef.h"
#include  "funcdef.h"

void *
dsmcc_ts_segmentation(int pid, void *dsmcc, off_t dsmcclen)
{
  TSPACKLST *tspcklstp;
  char  *tmpdsmccdiisecp, *tmptsp;
  int   diiseclen, tmpdiiseclen, nts, usrnts, cc, ntsmtuc;
  QUE   *dsmcc_ts_root;

  dsmcc_ts_root = NULL;

  if (dsmcc)
  {
    diiseclen = dsmcclen;

    /****/
    /* DSM-CC セクションデータをTSパケットにフラグメントした時のパケット数を得る
    */
		nts = (diiseclen/(188-TSHDRZ)) + (diiseclen%(188-TSHDRZ)?1:0);	/*TSヘッダーは4bytes*/
		usrnts = nts;

    dsmcc_ts_root = malloc(sizeof(QUE));
		if (dsmcc_ts_root)
		{
			cc = 0;
			tmpdsmccdiisecp = dsmcc;
			QINIT(dsmcc_ts_root);
			while (nts)
			{
        ntsmtuc = 0;
        tspcklstp = (TSPACKLST *)malloc(sizeof(TSPACKLST));
        if (!tspcklstp)
        {
          fprintf(stderr, "No more TS packect list data: %s", strerror(errno));
          if (!QisEMPTY(dsmcc_ts_root))
          {
            DEQUEUE(tspcklstp, (TSPACKLST *), dsmcc_ts_root);
            while (tspcklstp != (TSPACKLST *)dsmcc_ts_root)
            {
              free(tspcklstp);
              DEQUEUE(tspcklstp, (TSPACKLST *), dsmcc_ts_root);
            }
            free(dsmcc_ts_root);
          }
          return NULL;
        }
        tspcklstp->tspacklen = 0;
        while (nts && (ntsmtuc < MAXTS_IN_PACK))
        {
          if (!ntsmtuc)
          {/* TS over RTP の先頭には RTP ヘッダーを印加 */
            tmptsp = &(tspcklstp->tspack[RTP_HDRS]);
            tspcklstp->tspacklen += RTP_HDRS;
          }

  				tmptsp[0] = 0x47;
  				tmptsp[1] = 0;
  				if (nts == usrnts)
  				{
  					tmptsp[1] |= 1 << 6;	/*payload unit start indicator*/
  				}
  				tmptsp[1] |= GET_Maskedvalue(pid, 0x01f, 8);
  				tmptsp[2] = pid;

  				tmptsp[3] = 0;
  				tmptsp[3] |= 1 << 4;		/*adaptation  field  control (1 == Only payload)*/
  				tmptsp[3] |= cc & 0x0f;		/*Continuty counter*/
  				cc++;

          /* DSM-CC Section データから引っ張ってくるサイズを求める
          */
  				if ((188-TSHDRZ) < diiseclen)
  					tmpdiiseclen = (188-TSHDRZ);
  				else
  					tmpdiiseclen = diiseclen;
  				diiseclen -= tmpdiiseclen;

  				memcpy(&tmptsp[TSHDRZ], tmpdsmccdiisecp, tmpdiiseclen);
          tmptsp += (TSHDRZ+tmpdiiseclen);
          tspcklstp->tspacklen += (TSHDRZ+tmpdiiseclen);
          tmpdsmccdiisecp += tmpdiiseclen;

          ntsmtuc++;
  				--nts;
        }
        ENQUEUE(dsmcc_ts_root, tspcklstp);

			}
		}
	}
  return (void *)dsmcc_ts_root;
}

void
dsmcc_preparation(DSMCCINF *dsmccp)
{
  char  *dsmccfnm;
  int   rtn, fd;
  ssize_t act;
  void    *dtbf;
  struct stat sttbf;

  dsmccp->_data = NULL;
  dsmccp->_len = 0;

  dsmccfnm = dsmccp->_fnm;

  if (!dsmccfnm)
  {
    return ;
  }
  fd = open(dsmccfnm, O_RDONLY);
  if (fd < 0)
  {
    fprintf(stderr, "open error %s: %s\n", dsmccfnm, strerror(errno));
    return ;
  }
  rtn = fstat(fd, &sttbf);
  if (rtn < 0)
  {
    close(fd);
    fprintf(stderr, "stat error %s: %s\n", dsmccfnm, strerror(errno));
    return ;
  }
  dtbf = malloc(sttbf.st_size);
  if (!dtbf)
  {
    close(fd);
    fprintf(stderr, "malloc error %s: %s\n", dsmccfnm, strerror(errno));
    return ;
  }
  act = read(fd, dtbf, sttbf.st_size);
  if (act != sttbf.st_size)
  {
    close(fd);
    free(dtbf);
    if (act < 0)
    {
      fprintf(stderr, "read error %s: %s\n", dsmccfnm, strerror(errno));
    }
    else
    {
      fprintf(stderr, "read unmatch size %s: %ld/%ld\n", dsmccfnm, act, sttbf.st_size);
    }
    return ;
  }

  dsmccp->_data = dsmcc_ts_segmentation(dsmccp->_pid, dtbf, sttbf.st_size);
  if (dsmccp->_data)
  {
      dsmccp->_len = sttbf.st_size;
      dsmccp->_st_mtim = sttbf.st_mtim;
  }
  close(fd);
  free(dtbf);

  return;
}

void
ts_dsmcc_section_preparation(DSTRBINFO *dtrbp)
{
  int   rtn, ix;
  struct stat sttbf;

  /* updateファイル情報 */
  if (dtrbp->updtfnm)
  {
    rtn = stat(dtrbp->updtfnm, &sttbf);
    if (0 <= rtn)
      dtrbp->st_mtim = sttbf.st_mtim;
  }

  /* DSM-CCファイル情報 */
  for (ix=0; ix<DSMCCFILES; ix++)
  {
    dsmcc_preparation(&dtrbp->dsmcc[ix]);
  }
  return ;
}
