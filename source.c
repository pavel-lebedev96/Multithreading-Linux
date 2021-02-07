#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*пять магазинов*/
static unsigned int stores[5];

/*мьютекс для синхронизации потоков*/
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

/*флаг для остановки потока погрузчика при присоединении всех потоков покупателей*/
static bool stop = false;

/*функция для потока погрузчика*/
static void* loader(void* arg)
{
	/*количество товара*/
	unsigned int goods_num;
	
	/*номер магазина*/
	unsigned int n_store;

	/*бесконечный цикл*/
	for (;;)
	{
		/*проверка, если другие потоки завершили свою работу*/
		if (stop)
			return 0;
		
		/*закрытие мьютекса*/
		if (pthread_mutex_lock(&mtx) != 0)
			return (void *) 1;
		
		/*выбор количества товара от 300 до 500*/
		goods_num = (unsigned int) (300 + rand() % 201);
		/*выбор магазина*/
		n_store = (unsigned int) rand() % 5;
		stores[n_store] += goods_num;
		
		/*открытие мьютекса*/		
		if (pthread_mutex_unlock(&mtx) != 0)
			return (void *) 1;

		/*приостановка потока на 1 секунду*/
		sleep(1);
	}
	
}

/*функция для потоков покупателей, arg - потребность покупателя*/
static void* customer(void* arg)
{
	/*потребность покупателя*/
	unsigned int need = (unsigned int) arg;
	
	/*номер магазина*/
	unsigned int n_store;
		
	for (;;)
	{
		/*закрытие мьютекса*/
		if (pthread_mutex_lock(&mtx) != 0)
			return (void *) 1;
		
		/*выбор магазина*/
		n_store = (unsigned int) (rand() % 5);
		/*пока в магазине есть товар*/
		while (stores[n_store] != 0)
		{
			/*если потребность больше нуля*/
			if (need != 0)
			{
				stores[n_store]--;
				need--;
			}
			else
			{
				/*иначе завершить процесс*/
				if (pthread_mutex_unlock(&mtx) != 0)
					return (void *) 1;
				return 0;
			}
		}
		
		/*открытие мьютекса*/
		if (pthread_mutex_unlock(&mtx) != 0)
			return (void *) 1;
		
		/*приостановка потока на 3 секунды*/
		sleep(3);
	}
}

/*главный поток*/
int main(int argc, char* argv[])
{
	pthread_t loader_thread, customer_thread[3];
	
	/*инициализация ГСЧ*/
	srand((unsigned)time(NULL));
	
	printf("The program is running\n");
	/*заполнение магазинов*/
	for (int i = 0; i < 5; i++)
		/*кол-во товаров в диапазоне 1000 - 1200*/
		stores[i] = (unsigned int) (1000 + rand() % 201);

	/*создание потока с погрузчиком*/
	if (pthread_create(&loader_thread, NULL, loader, NULL) != 0)
		return 1;

	/*создание потоков с покупателями*/
	for (int i = 0; i < 3; i++)
		/*потребности покупателей в диапазоне 3000 - 3500*/
		if (pthread_create(&customer_thread[i], NULL, customer, (void *) (3000 + rand() % 501))  != 0)
			return 1;

	/*переменная для хранения возвращаемого значения функции потока*/
	void* res;
	
	/*присоединение потоков с покупателями*/
	for (int i = 0; i < 3; i++)
		if (pthread_join(customer_thread[i], &res) != 0)
			return 1;
		else
			/*если ошибка*/
			if (((int)res) != 0)
				return 1;

	/*присоединение потока погрузчика*/
	stop = true;
	if (pthread_join(loader_thread, &res) != 0)
		return 1;
	
	printf("All customers met their needs. The program has finished\n");
	sleep(5);
	return 0;
}