#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include  "procdef.h"
#include  "glovaldef.h"
#include  "funcdef.h"

void
cre_sock(void)
{
  int rtn;
  int udpbfz = UDPRCVBUFSZ;

  rcvinfo.sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (rcvinfo.sock < 0)
  {
    fprintf(stderr, "r socket : %s\n", strerror(errno));
    exit(1);
  }

  rtn = setsockopt(rcvinfo.sock, SOL_SOCKET, SO_RCVBUF, &udpbfz, sizeof(udpbfz));
  if (rtn < 0)
  {
    fprintf(stderr, "setsockopt SO_RCVBUF %d : %s\n", udpbfz, strerror(errno));
    exit(1);
  }
  rtn = bind(rcvinfo.sock, (struct sockaddr *)&rcvinfo.src_addr, sizeof(struct sockaddr_in));
  if (rtn < 0)
  {
    fprintf(stderr, "bind : %s\n", strerror(errno));
    exit(1);
  }
}

void
init_proc(void)
{

  transinfo.alwble_trnstime.tv_sec = 0;
  transinfo.alwble_trnstime.tv_usec = CALC_PACK_INTVL_USEC(transinfo.trnsrate, transinfo.trnsdtsz);
  printf("Allowable transfer time: %ld.%06ld\n", transinfo.alwble_trnstime.tv_sec, transinfo.alwble_trnstime.tv_usec);

   transinfo.dsmcc_intrvl.tv_sec = DSMCC_INTRVL_SEC;
   transinfo.dsmcc_intrvl.tv_usec = DSMCC_INTRVL_USEC;

#if 0
  {
    struct protoent *prtent;
    prtent = getprotobyname("udp");
    if (prtent)
    {
      rcvinfo.sock = socket(AF_INET, SOCK_RAW, prtent->p_proto);
    }
  }
#endif

  cre_sock();
#if 0
  int rtn;
  rcvinfo.sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (rcvinfo.sock < 0)
  {
    fprintf(stderr, "r socket : %s\n", strerror(errno));
    exit(1);
  }
  rtn = bind(rcvinfo.sock, (struct sockaddr *)&rcvinfo.src_addr, sizeof(struct sockaddr_in));
  if (rtn < 0)
  {
    fprintf(stderr, "bind : %s\n", strerror(errno));
    exit(1);
  }
#endif

  {
    FILE  *cffl;
    char  *fgrtn, lnbf[256];
    //DSTRBINFO *dstrbp;

    cffl = fdopen(destribcnffd, "r");
    if (!cffl)
    {
      fprintf(stderr, "fdopen : %s\n", strerror(errno));
      exit(1);
    }

    fgrtn = fgets(lnbf, sizeof(lnbf)-1, cffl);
    while (fgrtn)
    {
#if 0
      dstrbp = make_destrib_info(lnbf);
      if (dstrbp)
      {
        ENQUEUE(&dstrbinfo_root, dstrbp);
        dstrbinfo_cnt++;
      }
#endif
      make_destrib_info(lnbf);
      fgrtn = fgets(lnbf, sizeof(lnbf)-1, cffl);
    }
    fclose(cffl);
  }

}
