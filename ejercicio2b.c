#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>

void fTarea(int t, int c){

	char hora[100];
	time_t ahora = time(NULL);

	//El instante actual (hora de ahora)
	strcpy(hora, ctime(&ahora));
	printf("Empieza la tarea %d con tiempo de computo %d, en el instante %s\n", t, c, hora);
	fflush(stdout);
	sleep(c);
	ahora = time(NULL);
	strcpy(hora, ctime(&ahora)); //El instante actual (hora de ahora)
	printf("Termina la tarea %d a las %s\n", t, hora);
	fflush(stdout);
}

int main(){

	int ciclo = 0;
	sigset_t set;

	//Preparamos las señales
	if(sigemptyset(&set) != 0) error();
	if(sigaddset(&set, SIGALRM) !=0) error();

	//Bloqueamos la señal
	pthread_sigmask(SIG_BLOCK, &set, NULL);

	struct itimerspec required, old;
	struct timespec first, period;
	struct sigevent sig; 	//Informacion de la señal 

	int signum;	//Señal recibida
	timer_t timer;

	//Preparamos las señales
	if (sigemptyset(&set) != 0 ) error();
	if (sigaddset(&set, SIGALRM) != 0) error();

	//Asociamos señal y temporizador
	sig.sigev_notify = SIGEV_SIGNAL;
	sig.sigev_signo = SIGALRM;
	sig.sigev_value.sival_ptr = &timer;

	if(clock_gettime (CLOCK_MONOTONIC, &first) != 0) error(); 		//No necesario si usamos tiempos relativos
	if(timer_create(CLOCK_MONOTONIC, &sig, & timer) != 0) error(); 

	first.tv_sec = 10; 	//o first.tv_sec + 3 con tiempos absolutos 
	period.tv_sec = 10;
	period.tv_nsec = 0;
	required.it_value = first;
	required.it_interval = period;

	if(timer_settime(timer, 0, &required, &old) != 0)
	error();

	while(1){
		printf("Entramos en el while.\n");
		
		//Dependiendo del ciclo que estemos, vamos a pasar unas tareas u otras
		switch(ciclo){
			case 0:
				//Le pasamos la tarea y su tiempo de computo
				fTarea(1, 3);	//Cada 10 seg
				fTarea(2, 3);	//Cada 10 seg
				fTarea(3, 3);	//Cada 20 seg
				fTarea(4, 2);	//Cada 20 seg
				
				ciclo = 1;
			break;

			case 1:
				fTarea(1, 3);	//Cada 10 seg
				fTarea(2, 3);	//Cada 10 seg
				fTarea(5, 2);	//Cada 20 seg

				ciclo = 0;
			break;
		}

		if(sigwait(&set, &signum) != 0) error();	//Esperamos otros 10 seg del periodo
		//ciclo = ciclo % 2; 
	}

	return 0;
}

