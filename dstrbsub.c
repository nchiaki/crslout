#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include  <stdio.h>
#include  <string.h>
#include <stdlib.h>
#include <errno.h>

#include  "procdef.h"
#include  "glovaldef.h"
#include  "funcdef.h"

void
trim_line_end(char *linep)
{
  char  *cp;

  cp = linep;
  while (*cp)
  {
    if ((*cp == ' ') ||
        (*cp == '\t') ||
        (*cp == '\r') ||
        (*cp == '\n'))
    {
      *cp = '\0';
      break;
    }
    cp++;
  }
}

char  *dsmccprms[] = {"dsmcc1=","dsmcc2=","dsmcc3="};
char  *pidprms[] = {"pid1=","pid2=","pid3="};
char  *sintrvl[] = {"sintrvl1=", "sintrvl2=","sintrvl3="};
void
make_destrib_info(char *linep)
{
  char *lnbf;
  char  *srhp, *cp, *stripadr, *strprt, *updtf;
  int   ix, prtnm, nsprtnm;
  DSTRBINFO *dtrbp;
  struct dsmcc_conf_parame {
    char  *dsmccfl;
    char  *strpid;
    char  *strintvl;
    int   pid;
    struct timeval  dsmccalwble;
  } dsmcc[DSMCCFILES];

  lnbf = linep;
  while (isblank(*lnbf)){lnbf++;}
  if (*lnbf == '#')
    return; /* '#'始まりはコメント*/

  stripadr = NULL;
  updtf = NULL;

  strprt = NULL;
  nsprtnm = 0;

  memset(dsmcc, 0, sizeof(dsmcc));

  dtrbp = NULL;

  srhp = strstr(lnbf, "#"); // 設定行途中の#以降はコメント扱い

  cp = strstr(lnbf, "ipadr=");
  if (cp)
  {
    if (!srhp || (cp < srhp))
      stripadr = cp + strlen("ipadr=");
  }
  cp = strstr(lnbf, "port=");
  if (cp)
  {
    if (!srhp || (cp < srhp))
      strprt = cp + strlen("port=");
  }

  cp = strstr(lnbf, "update=");
  if (cp)
  {
    if (!srhp || (cp < srhp))
      updtf = cp + strlen("update=");
  }
  for (ix=0; ix<DSMCCFILES; ix++)
  {
    cp = strstr(lnbf, dsmccprms[ix]);
    if (cp)
    {
      if (!srhp || (cp < srhp))
        dsmcc[ix].dsmccfl = cp + strlen(dsmccprms[ix]);
    }
    cp = strstr(lnbf, pidprms[ix]);
    if (cp)
    {
      if (!srhp || (cp < srhp))
        dsmcc[ix].strpid = cp + strlen(pidprms[ix]);
    }

    dsmcc[ix].dsmccalwble = transinfo.dsmcc_intrvl;
    cp = strstr(lnbf, sintrvl[ix]);
    if (cp)
    {
      if (!srhp || (cp < srhp))
        dsmcc[ix].strintvl = cp + strlen(sintrvl[ix]);
    }
  }

  if (stripadr)
  {
    trim_line_end(stripadr);
  }
  if (updtf)
  {
    trim_line_end(updtf);
    cp = malloc(strlen(updtf)+1);
    if (cp)
    {
        strcpy(cp, updtf);
        updtf = cp;
    }
  }

  if (strprt)
  {
    trim_line_end(strprt);
    prtnm = atoi(strprt);
    nsprtnm = htons(prtnm);
  }
  else
  {
    nsprtnm = rcvinfo.src_addr.sin_port;
  }
  for (ix=0; ix<DSMCCFILES; ix++)
  {
    if (dsmcc[ix].dsmccfl)
    {
      trim_line_end(dsmcc[ix].dsmccfl);
      cp = malloc(strlen(dsmcc[ix].dsmccfl)+1);
      if (cp)
      {
          strcpy(cp, dsmcc[ix].dsmccfl);
          dsmcc[ix].dsmccfl = cp;
      }
    }
    if (dsmcc[ix].strpid)
    {
      int lwc;
      trim_line_end(dsmcc[ix].strpid);
      cp = dsmcc[ix].strpid;
      while (*cp)
      {
        lwc = tolower(*cp);
        if (('0' <= lwc) && (lwc <= '9'))
          {dsmcc[ix].pid = ((dsmcc[ix].pid << 4)|(lwc - '0'));}
        else
          {dsmcc[ix].pid = ((dsmcc[ix].pid << 4)|((lwc - 'a')+10));}
        cp++;
      }
    }
    if (dsmcc[ix].strintvl)
    {
      trim_line_end(dsmcc[ix].strintvl);
      dsmcc[ix].dsmccalwble.tv_usec = atoi(dsmcc[ix].strintvl) * 1000;  // msec to usec
      dsmcc[ix].dsmccalwble.tv_sec = dsmcc[ix].dsmccalwble.tv_usec / 1000000;
      dsmcc[ix].dsmccalwble.tv_usec %= 1000000;
      if (timercmp(&dsmcc[ix].dsmccalwble, &transinfo.dsmcc_intrvl, <))
      {
        dsmcc[ix].dsmccalwble = transinfo.dsmcc_intrvl;
      }
    }
  }

  /*====*/

  if (stripadr)
  {
    dtrbp = (DSTRBINFO *)malloc(sizeof(DSTRBINFO));
    if (dtrbp)
    {
      QINIT(dtrbp);
      dtrbp->dst_addr.sin_family = AF_INET;
      dtrbp->dst_addr.sin_port = nsprtnm;
      dtrbp->dst_addr.sin_addr.s_addr = inet_addr(stripadr);

      dtrbp->sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (dtrbp->sock < 0)
      {
        fprintf(stderr, "%s socket : %s\n", stripadr, strerror(errno));
      }
      else
      {
#if 0
        int rtn;
        rtn = bind(dtrbp->sock, (struct sockaddr *)&(dtrbp->dst_addr), sizeof(dtrbp->dst_addr));
        if (rtn < 0)
        {
          fprintf(stderr, "%s bind : %s\n", stripadr, strerror(errno));
          exit(1);
        }
#endif
        for (ix=0; ix<DSMCCFILES; ix++)
        {
          dtrbp->dsmcc[ix]._pid = dsmcc[ix].pid;
          dtrbp->dsmcc[ix]._fnm = dsmcc[ix].dsmccfl;
          dtrbp->dsmcc[ix]._alwble_time = dsmcc[ix].dsmccalwble; /* DSM-CC 次回送出間隔時間 */
          dtrbp->dsmcc[ix]._next_time.tv_sec = 0; /* DSM-CC 次回送出時刻 */
          dtrbp->dsmcc[ix]._next_time.tv_usec = 0;
        }
        dtrbp->st_mtim.tv_sec = 0;
        dtrbp->st_mtim.tv_nsec = 0;
        dtrbp->updtfnm = updtf;
        ts_dsmcc_section_preparation(dtrbp);
#if 0
        dtrbp->dsmcc_pretv.tv_sec = 0;
        dtrbp->dsmcc_pretv.tv_usec = 0;
#endif
        ENQUEUE(&dstrbinfo_root, dtrbp);
        dstrbinfo_cnt++;
      }
    }
    else
    {
      fprintf(stderr, "%s:Can't make destribution data: %s\n", stripadr, strerror(errno));
    }
  }
}
