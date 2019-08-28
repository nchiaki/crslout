#include  <stdio.h>
#include  "procdef.h"

int seqchkf = 0;

char  *destribcnffname = NULL;
int destribcnffd = -1;
QUE dstrbinfo_root = {&dstrbinfo_root, &dstrbinfo_root};
int dstrbinfo_cnt = 0;

TRNSINFO  transinfo;
RCVINFO rcvinfo;
