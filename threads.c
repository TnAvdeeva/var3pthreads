/*Вариант 3
Необходимо решить проблему "Поиск-вставка-удаление" с использованием библиотеки PTHREAD с учетом следующих ограничений:
	1)3 типа нитей имеют доступ к общему связному списку: поисковики (N штук), вставщики (M штук) и удалятели (K штук).
	2)Поисковики могут просматривать список одновременно.
	3)Вставщики вставляют элементы в конец списка, поэтому только 1 из них может делать это в один момент времени. Его работа не мешает поисковикам осуществлять поиск в списке.
	4)Удалятели могут удалить элемент из любого места в списке, поэтому во время работы 1 удалятеля никто другой не может иметь доступ к списку.
	5)Все нити ожидают завершения работы других нитей.
	6)Доступ нитей к списку по возможности должен осуществляться в порядке очереди.
	7)Время выполнения всех операций случайно.

Результаты работы программы должны отображаться по мере их появления следующим образом. Вывод должен быть синхронизированным, т.е. последовательность строк в логе должна соответствовать последовательности операций, выполняемых программой.*/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#define A 5		
#define N 5 //кол-во: поисковиков
#define M 5 //        вставщиков
#define K 5 //        удалятелей

//прототипы ф-ций, которые будут использоваться потоками соотственно
void *put(void*x);
void *find(void*x);
void *delete(void*x);
int  number_find[N], number_put[M], number_delete[K];//каждый элемент будет хранить номер поисковика, вставщика, удалятеля


//для времени
time_t rawtime;
struct tm * timeinfo;
char buffer [80];

void get_time()//для получения времени
{
	time(&rawtime);
	timeinfo = localtime(&rawtime);
	strftime(buffer, 80, "%H:%M:%S", timeinfo);
}

pthread_mutex_t my_mutex = PTHREAD_MUTEX_INITIALIZER;//мьютекс
pthread_cond_t find_var = PTHREAD_COND_INITIALIZER;//условная переменная для ожидания потока
pthread_cond_t put_var = PTHREAD_COND_INITIALIZER;
pthread_cond_t delete_var = PTHREAD_COND_INITIALIZER;
int check = 1, synch = 1;

int main()
{
	srand(time(NULL));
	pthread_t find_thread[M], put_thread[N], delete_thread[K];//идентификаторы потоков
	for (int i = 0; i < A; i++)
	{
		for (int j = 0; j < N; j++)//создание поисковиков
		{
			number_find[j] = j + 1;
			int *ukaz = &number_find[j];
			pthread_create(&find_thread[j], NULL, find, (void *)ukaz);
		}
		for (int j = 0; j < M; j++)//создание вставщиков
		{
			number_put[j] = j + 1;
			int *ukaz = &number_put[j];
			pthread_create(&put_thread[j], NULL, put, (void *)ukaz);
		}
		for (int j = 0; j < K; j++)//создание удалятелей
		{
			number_delete[j] = j + 1;
			int *ukaz = &number_delete[j];
			pthread_create(&delete_thread[j], NULL, delete, (void *)ukaz);
		}	
		for (int k = 0; k < N; k++)//ожидание завершения поисковиков
		{
			pthread_join(find_thread[k], NULL);
		}
		for (int k = 0; k < M; k++)//ожидание завершения вставщиков
		{
			pthread_join(put_thread[k], NULL);
		}
		for (int k = 0; k < K; k++)//ожидание завершения удалятелей
		{
			pthread_join(delete_thread[k], NULL);
		}
		//printf("\n");
	}
	
	return 0;
}

void *find(void*x)
{
	int *nomer = (int *)x;
	if ((rand() % 2) == 1)//будет поисковик делать поиск или нет
	{
		while (synch != 1) pthread_cond_wait(&find_var, &my_mutex);
		check = 0;
		sleep(1);
		get_time();
		printf("%s     Поисковик №%d хочет начать поиск в списке.\n", buffer, *nomer);
		sleep(1);
		get_time();
		printf("%s     Поисковик №%d начал поиск в списке.\n", buffer, *nomer);
		sleep(1);
		get_time();
		printf("%s     Поисковик №%d закончил поиск в списке.\n", buffer, *nomer);
		pthread_cond_signal(&delete_var);
		check = 1;
	}
	pthread_exit(0);
}

void *put(void*x)
{
	int *nomer = (int *)x;
	if ((rand() % 2) == 1)
	{
		pthread_mutex_lock(&my_mutex);
		while (synch != 1) pthread_cond_wait(&put_var, &my_mutex);
		check = 0;
		sleep(1);
		get_time();
		printf("%s     Вставщик №%d хочет начать вставку в список.\n", buffer, *nomer);
		sleep(1);
		get_time();
		printf("%s     Вставщик №%d начал вставку в список.\n", buffer, *nomer);
		sleep(1);
		get_time();
		printf("%s     Вставщик №%d закончил вставку в список.\n", buffer, *nomer);
		pthread_mutex_unlock(&my_mutex);
		pthread_cond_signal(&delete_var);
		check = 1;
	}
	pthread_exit(0);
}

void *delete(void*x)
{
	int *nomer = (int *)x;
	if ((rand() % 2) == 1)
	{
		pthread_mutex_lock(&my_mutex);
		while (check != 1) pthread_cond_wait(&delete_var, &my_mutex);
		synch = 0;
		sleep(1);
		get_time();
		printf("%s     Удалятель №%d хочет начать удаление из списка.\n", buffer, *nomer);
		sleep(1);
		get_time();
		printf("%s     Удалятель №%d начал удаление из списка.\n", buffer, *nomer);
		sleep(1);
		get_time();
		printf("%s     Удалятель №%d закончил удаление из списка.\n", buffer, *nomer);
		pthread_mutex_unlock(&my_mutex);
		pthread_cond_signal(&find_var); 
		pthread_cond_signal(&put_var);
		synch = 1;
	}
	pthread_exit(0);
}
