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
  char  *cp, *stripadr, *strprt, *dsmccfl[DSMCCFILES], *strpid[DSMCCFILES], *strintvl[DSMCCFILES], *updtf;
  int   ix, prtnm, nsprtnm, pid[DSMCCFILES];
  struct timeval  dsmccalwble[DSMCCFILES];
  DSTRBINFO *dtrbp;

  lnbf = linep;
  while (isblank(*lnbf)){lnbf++;}
  if (*lnbf == '#')
    return; /* '#'始まりはコメント*/

  stripadr = NULL;
  updtf = NULL;

  strprt = NULL;
  nsprtnm = 0;

  memset(dsmccfl, 0, sizeof(dsmccfl));
  memset(strpid, 0, sizeof(strpid));
  memset(pid, 0, sizeof(pid));
  memset(strintvl, 0, sizeof(strintvl));

  dtrbp = NULL;

  cp = strstr(lnbf, "ipadr=");
  if (cp)
  {
    stripadr = cp + strlen("ipadr=");
  }
  cp = strstr(lnbf, "port=");
  if (cp)
  {
    strprt = cp + strlen("port=");
  }

  cp = strstr(lnbf, "update=");
  if (cp)
  {
    updtf = cp + strlen("update=");
  }
  for (ix=0; ix<DSMCCFILES; ix++)
  {
    cp = strstr(lnbf, dsmccprms[ix]);
    if (cp)
    {
      dsmccfl[ix] = cp + strlen(dsmccprms[ix]);
    }
    cp = strstr(lnbf, pidprms[ix]);
    if (cp)
    {
      strpid[ix] = cp + strlen(pidprms[ix]);
    }

    dsmccalwble[ix] = transinfo.dsmcc_intrvl;
    cp = strstr(lnbf, sintrvl[ix]);
    if (cp)
    {
      strintvl[ix] = cp + strlen(sintrvl[ix]);
    }
  }

  if (stripadr)
  {
    trim_line_end(stripadr);
  }
  if (updtf)
  {
    trim_line_end(updtf);
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
    if (dsmccfl[ix])
    {
      trim_line_end(dsmccfl[ix]);
    }
    if (strpid[ix])
    {
      int lwc;
      trim_line_end(strpid[ix]);
      cp = strpid[ix];
      while (*cp)
      {
        lwc = tolower(*cp);
        if (('0' <= lwc) && (lwc <= '9'))
          {pid[ix] = ((pid[ix] << 4)|(lwc - '0'));}
        else
          {pid[ix] = ((pid[ix] << 4)|((lwc - 'a')+10));}
        cp++;
      }
    }
    if (strintvl[ix])
    {
      trim_line_end(strintvl[ix]);
      dsmccalwble[ix].tv_usec = atoi(strintvl[ix]) * 1000;  // msec to usec
      dsmccalwble[ix].tv_sec = dsmccalwble[ix].tv_usec / 1000000;
      dsmccalwble[ix].tv_usec %= 1000000;
      if (timercmp(&dsmccalwble[ix], &transinfo.dsmcc_intrvl, <))
      {
        dsmccalwble[ix] = transinfo.dsmcc_intrvl;
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
          dtrbp->dsmcc[ix]._pid = pid[ix];
          dtrbp->dsmcc[ix]._fnm = dsmccfl[ix];
          dtrbp->dsmcc[ix]._alwble_time = dsmccalwble[ix]; /* DSM-CC 次回送出間隔時間 */
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
