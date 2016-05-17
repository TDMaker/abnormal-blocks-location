/* 
 * This module is related to "TagGen"
 * for each file block, client calculates the block tag
 * and save as a file named "client" which is saved 
 * or sent to the server in reality.
 *
 */
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <gmp.h> 

#define block_max 2048
#define thread_num 32
#define block_offset 64
/*
 * block_max = 32 * 32 * 4 = 4096
 * we have 4 RAID so we define the file I/O is 4
 * but our cpu has 32 logical core so the thread_num is 32
 * block_offset = block_max / thread_num = 4096 / 32 = 128
*/
pthread_t thread[thread_num];
pthread_mutex_t mut;

static int thread_id = 1;
static unsigned long block_id = 1;

struct mypara{
	int arg_thread_id;
	unsigned long arg_block_id;
};
struct mypara my_args[thread_num];

void *thread1(void* arg)
{
	mypara *pstru = (struct mypara *) arg;
	int	tmp_thread = pstru->arg_thread_id;
	unsigned long tmp_block = pstru->arg_block_id;
	unsigned long tmp_block_max = tmp_block + block_offset;
	printf("Into Thread [%d] and the block range from [%d] to (%d)\n\n", tmp_thread, tmp_block, tmp_block_max);
	char filename[100];
	memset(filename, 0, sizeof(filename));
	FILE *fs3 = NULL;
	mpz_t begin;
	for (;tmp_block < tmp_block_max; tmp_block++)
	{
		mpz_init(begin);
		if(tmp_block % 100 == 0)
		{
			printf("Now tmp_block is %d\n", tmp_block);
		}
		if(tmp_block % 4 == 1)
		{
			sprintf(filename, "/mnt/sdb1/data_32m/%ld", tmp_block);
		}
		else if(tmp_block % 4 == 2)
		{
			sprintf(filename, "/mnt/sdc1/data_32m/%ld", tmp_block);
		}
		else if(tmp_block % 4 == 3)
		{
			sprintf(filename, "/mnt/sdd1/data_32m/%ld", tmp_block);
		}
		else if(tmp_block % 4 == 0)
		{
			sprintf(filename, "/mnt/sde1/data_32m/%ld", tmp_block);
		}
		
		if ((fs3 = fopen(filename, "r")) == NULL)
		{
			printf("File block %s open failed[first]\n", filename);
		}
		if(mpz_inp_raw(begin, fs3) == 0)
		{
			printf("filename is %s\n", filename);
			printf("File block read to gmp_value error!\n");
		}
		fclose(fs3);
		mpz_clear(begin);
	}
	printf("thread:%d/t:Is the main funcion waiting for me?\n", tmp_thread);
	pthread_exit(NULL);
}

void thread_create(void)
{
	memset(&thread, 0, sizeof(thread));
	for (int thread_count = 0; thread_count < thread_num; thread_count++)
	{
		if ((pthread_create(&thread[thread_count], NULL, thread1, &my_args[thread_count])) != 0)
			printf("Thread No.%d creation failture!\n", thread_count + 1);
		else 
		{
		//	printf("Thread No.%d creation success!\n", thread_count + 1);
		}
	}
}

void thread_wait(void)
{
	printf("Into thread_wait\n");
	for (int j = 0; j < thread_num; j++)
	{
		if (thread[j] != 0)
		{
			pthread_join(thread[j], NULL);
			printf("Thread No.%d has finished\n", j + 1);
		}
	}
}

int main(void)
{
	//init thread arguments.
	for(int count = 0; count < thread_num; count++)
	{
		my_args[count].arg_thread_id = thread_id + count;
		my_args[count].arg_block_id = block_id + block_offset * count;
	}
	
	struct timeval start1, start2;
	gettimeofday(&start1, NULL);
	
	pthread_mutex_init(&mut, NULL);
	printf("I am the main function, I am creating all the threads!\n");
	thread_create();
	thread_wait();
	printf("I am the main function, I've destroyed all the threads.\n");

	gettimeofday(&start2, NULL);
	unsigned long long int timeuse = 1000000 * (start2.tv_sec - start1.tv_sec) + start2.tv_usec - start1.tv_usec;
	printf("time: %lld s,%lld ms,%lld us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse);

	return(0);
}