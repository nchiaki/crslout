#include  <stdio.h>
#include  "procdef.h"
#include  "glovaldef.h"

char  pre_rcv_data_byte;
int   sqchks = 0;
int   sqchers = 0;
void
recv_proc(void)
{
  do {
    rcvinfo.rcv_actlen = recv(rcvinfo.sock, rcvinfo.rcv_data, RCVDTARAZ, 0);
  } while (rcvinfo.rcv_actlen <= 0);
  gettimeofday(&rcvinfo.rcv_time, NULL);

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
