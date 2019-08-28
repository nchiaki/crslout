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

void
make_destrib_info(char *lnbf)
{
  char  *cp, *stripadr, *strprt, *dsmccfl[DSMCCFILES], *strpid[DSMCCFILES], *updtf;
  int   ix, prtnm, nsprtnm, pid[DSMCCFILES];
  DSTRBINFO *dtrbp;

  stripadr = NULL;
  updtf = NULL;

  strprt = NULL;
  nsprtnm = 0;

  memset(dsmccfl, 0, sizeof(dsmccfl));
  memset(strpid, 0, sizeof(strpid));
  memset(pid, 0, sizeof(pid));

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
    {
      dsmccfl[ix] = cp + strlen(dsmccprms[ix]);
    }
    cp = strstr(lnbf, pidprms[ix]);
    {
      strpid[ix] = cp + strlen(pidprms[ix]);
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
        }
        dtrbp->updtfnm = updtf;
        ts_dsmcc_section_preparation(dtrbp);

        dtrbp->dsmcc_pretv.tv_sec = 0;
        dtrbp->dsmcc_pretv.tv_usec = 0;

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
