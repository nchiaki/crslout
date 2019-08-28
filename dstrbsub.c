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

void
make_destrib_info(char *lnbf)
{
  char  *cp, *stripadr, *strprt, *dsmccfl, *strpid;
  int prtnm, nsprtnm, pid;
  DSTRBINFO *dtrbp;

  stripadr = NULL;
  dsmccfl = NULL;

  strprt = NULL;
  nsprtnm = 0;

  strpid = NULL;
  pid = 0;

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
  cp = strstr(lnbf, "dsmccfile=");
  {
    dsmccfl = cp + strlen("dsmccfile=");
  }
  cp = strstr(lnbf, "pid=");
  {
    strpid = cp + strlen("pid=");
  }

  if (stripadr)
  {
    trim_line_end(stripadr);
  }
  if (dsmccfl)
  {
    trim_line_end(dsmccfl);
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
  if (strpid)
  {
    int lwc;
    trim_line_end(strpid);
    cp = strpid;
    while (*cp)
    {
      lwc = tolower(*cp);
      if (('0' <= lwc) && (lwc <= '9'))
        {pid = ((pid << 4)|(lwc - '0'));}
      else
        {pid = ((pid << 4)|((lwc - 'a')+10));}
      cp++;
    }
  }

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
        dtrbp->pid = pid;
        dtrbp->dsmccfnm = dsmccfl;
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
