#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <errno.h>

#include  "procdef.h"
#include  "funcdef.h"


pthread_t	udp_destribute_pthread;
int			udp_destribute_pthpolicy;
struct sched_param	udp_destribute_pthprm;

void *
udp_destribute_func(void *vp)
{
  int   rtn;
  rtn = pthread_getschedparam(udp_destribute_pthread, &udp_destribute_pthpolicy, &udp_destribute_pthprm);
	if (rtn == 0)
	{
#if 0
		int		minprio;
#define	FIO_ETHER_PRIO	40
		fio_ether_pthprio = sched_get_priority_max(SCHED_FIFO);
		minprio = sched_get_priority_min(SCHED_FIFO);
		if ((FIO_ETHER_PRIO < minprio) || (fio_ether_pthprio < FIO_ETHER_PRIO))
			fio_ether_pthprio = minprio + ((fio_ether_pthprio - minprio)/3);
		else
			fio_ether_pthprio = FIO_ETHER_PRIO;
		udp_destribute_pthprm.sched_priority = fio_ether_pthprio;
#endif
		udp_destribute_pthprm.sched_priority = sched_get_priority_max(SCHED_FIFO);
		rtn = pthread_setschedparam(udp_destribute_pthread, SCHED_FIFO, &udp_destribute_pthprm);
		if (rtn)
		{
			fprintf(stderr, "%s pthread_setschedparam SCHED_FIFO : %s\n", __func__, strerror(errno));
		}
	}
  while (1)
  {
    recv_proc();
    dstrb_proc();
  }
}

int
main(int ac, char *av[])
{
  cmdproc(ac, av);

  init_proc();

  pthread_create(&udp_destribute_pthread, NULL, udp_destribute_func, NULL);
	pthread_join(udp_destribute_pthread,NULL);

  return 0;
}
