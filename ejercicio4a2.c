#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

#define _USE_UNIX98
#define TAM 3

pthread_mutex_t mutex;
pthread_mutexattr_t mattr;

int cnt = 0;			//Variable global

//Sin protocolo M bloquea a A y A bloquea a B?
//Con techo no coge el mutex y no se bloquea, sin el si se bloquea

void espera_activa(int tiempo){

	time_t ahora = time(NULL);
	while((ahora+tiempo) > time(NULL)); //Espera activa

}

void *tareaA(){
	printf("Entra en la tarea A.\n");
	fflush(stdout);
	while(1){
		//printf("Tarea A entra en while.\n");
		//fflush(stdout);
		sleep(3);
		printf("Tarea A quiere coger el mutex.\n");
		fflush(stdout);
		pthread_mutex_lock(&mutex);
		printf("*Tarea A coge el mutex, cnt = %d*.\n", cnt);
		fflush(stdout);
		cnt++;
		printf("\tTarea A va a dejar el mutex, cnt = %d.\n", cnt);
		fflush(stdout);
		pthread_mutex_unlock(&mutex);
		//printf("\tTarea A dejó el mutex, cnt = %d.\n", cnt);
		//fflush(stdout);
		}
	pthread_exit(NULL);
}

void *tareaM(){
	printf("Entra en la tarea M.\n");
	fflush(stdout);
	while(1){
		//printf("Tarea M entra en while duerme .\n");
		//fflush(stdout);
		sleep(5);
		printf("Tarea M entra en espera activa.\n");
		fflush(stdout);
		espera_activa(5);
		printf("Tarea M sale de espera activa.\n");
		fflush(stdout);
	}
	pthread_exit(NULL);
}

void *tareaB(){
	printf("Entra en la tarea B.\n");
	fflush(stdout);
	while(1){
		//printf("Tarea B entra en while.\n");
		//fflush(stdout);
		sleep(1);
		printf("Tarea B quiere coger el mutex.\n");
		fflush(stdout);
		pthread_mutex_lock(&mutex);
		printf("*Tarea B entra en el mutex, cnt = %d*.\n", cnt);
		fflush(stdout);
		espera_activa(5);
		cnt++;
		printf("\tTarea B va a dejar el mutex, cnt = %d.\n", cnt);
		fflush(stdout);
		pthread_mutex_unlock(&mutex);
		//printf("\tTarea B sale en el mutex, cnt = %d.\n", cnt);
		//fflush(stdout);
	}
	pthread_exit(NULL);
}

int main(){

	pthread_t th[TAM];
	pthread_attr_t attr[TAM];
	struct sched_param param;
	int i, rc, error;
	int prioceiling = 2;

	for(i=0; i<TAM; i++)
		pthread_attr_init(&attr[i]);

	//Atributos del mutex
	pthread_mutex_init(&mutex, &mattr);

	//Herencia de prioridad
	param.sched_priority = 4; 		//Prioridad del main
	pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
	
	//Prioridades de las hebras
	param.sched_priority = 3; 
	pthread_attr_setschedparam(&attr[0], &param);
	param.sched_priority = 2; 
	pthread_attr_setschedparam(&attr[1], &param);
	param.sched_priority = 1; 
	pthread_attr_setschedparam(&attr[2], &param);

	//Asignamos la política
	error = pthread_mutexattr_setprotocol(&mattr, PTHREAD_PRIO_PROTECT);
	if(error != 0){
		perror("Error en pthread_mutexattr_setprotocol: \n");
	}
	error = pthread_mutexattr_setprioceiling(&mattr, prioceiling);
	if (error != 0) {
		perror("Error en pthread_mutexattr_setprioceiling: \n");
	}
	
	//Creamos las hebras 
	rc = pthread_create(&th[0], &attr[0], tareaA, NULL);
	if(rc){
		perror("Error al crear la hebra 0: \n");
		fflush(stdout);
	}

	rc = pthread_create(&th[1], &attr[1], tareaM, NULL);
	if(rc){
		perror("Error al crear la hebra 1: \n");
		fflush(stdout);
	}

	rc = pthread_create(&th[2], &attr[2], tareaB, NULL);
	if(rc){
		perror("Error al crear la hebra 2: \n");
		fflush(stdout);
	}

	//Esperamos las hebras
	for(i=0; i<TAM; i++)
		pthread_join(th[i], NULL);

	return 0;
}
