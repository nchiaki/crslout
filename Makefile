CC=gcc
LIB=
#LIBDIR=-lpthread -Isypcieif -L/usr/local/lib
LIBDIR=-lpthread -L/usr/local/lib
INCDIR=-I/usr/local/include
CFLAGS=-ggdb -Wall  $(DBGFLG)
#CFLAGS=-O3 -Wall  $(DBGFLG)
#LDFLAGS=-Xlinker --cref
TARGETS=crslout

SRCS=main.c gloval.c initproc.c cmdproc.c recvproc.c dstrbproc.c dstrbsub.c tspacksub.c

OBJS=$(SRCS:.c=.o)

all:	$(TARGETS) $(OBJS)

$(TARGETS):	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(INCDIR) $(LIBDIR) $(LIB)

.c.o:
	$(CC) $(CFLAGS) -c $< $(INCDIR) $(LIBDIR) $(LIB)

.PHONY:	clean
clean:
	$(RM) *~ $(TARGETS) $(OBJS)

DBGFLG=
#-DRSVNLLDBG -DDSMCCSNDDBG -DDSMCCUPDTDBG
