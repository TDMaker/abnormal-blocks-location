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

#define block_max 131072
#define thread_num 32
#define block_offset 4096
#define security_level 512
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
mpz_t pmod, pbase;

struct mypara{
	int arg_thread_id;
	unsigned long arg_block_id;
};
struct mypara my_args[thread_num];
char sub_result_file[thread_num][32];

void *thread1(void* arg)
{
	mypara *pstru = (struct mypara *) arg;
	int	tmp_thread = pstru->arg_thread_id;
	unsigned long tmp_block = pstru->arg_block_id;
	unsigned long tmp_block_max = tmp_block + block_offset;
	mpz_t begin, a1, a4;
	printf("Into Thread [%d] and the block range from [%d] to (%d)\n\n", tmp_thread, tmp_block, tmp_block_max);
	char filename[100];
	memset(filename, 0, sizeof(filename));
	FILE *fs3 = NULL;
	FILE *fs4 = NULL;
	if ((fs4 = fopen(sub_result_file[tmp_thread - 1], "a+")) == NULL) perror("open");
	char tmp_result1[security_level];
	char tmp_result2[security_level + 16];
	char tmp_result3[security_level + 16];
	
	for (;tmp_block < tmp_block_max; tmp_block++)
	{
		mpz_init(begin);
		mpz_init(a1);
		mpz_init(a4);
		if(tmp_block % 4 == 1)
		{
			sprintf(filename, "/mnt/sdb1/data_512k/%ld", tmp_block);
		}
		else if(tmp_block % 4 == 2)
		{
			sprintf(filename, "/mnt/sdc1/data_512k/%ld", tmp_block);
		}
		else if(tmp_block % 4 == 3)
		{
			sprintf(filename, "/mnt/sdd1/data_512k/%ld", tmp_block);
		}
		else if(tmp_block % 4 == 0)
		{
			sprintf(filename, "/mnt/sde1/data_512k/%ld", tmp_block);
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
		mpz_mod(a4, begin, pmod); //Set a4 to begin mod pmod
		mpz_powm_sec(a1, pbase, a4, pmod); //Set a1 to pbase^a4 mod pmod

		memset(tmp_result1, 0, sizeof(tmp_result1));
		memset(tmp_result2, 0, sizeof(tmp_result2));
		memset(tmp_result3, '0', sizeof(tmp_result3));
		
		mpz_get_str(tmp_result1, 16, a1);//Convert a1 to a string of digits named tmp_result1 in base 16.
		sprintf(tmp_result2, "%ld:%s,0.", tmp_block, tmp_result1);
		sprintf(tmp_result3 + (sizeof(tmp_result3) - strlen(tmp_result2)), "%s", tmp_result2);
		//tmp_result3 is like 0000000...[tmp_block]:[tmp_result1],0.[one \0 and nine '0']
		//write to the file database/client each 200 count because the buffer tmp_result will full every 200-loop

		
		fwrite(tmp_result3, sizeof(tmp_result3), 1, fs4);
		mpz_clear(begin);
		mpz_clear(a1);
		mpz_clear(a4);
	}
	fclose(fs4);

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
void fconcat(const char* destfile,const char* srcfile) {  
    FILE* destfp;
    FILE* srcfp;  
    char buffer[8];  
    int len;
  
    if ((destfp = fopen(destfile,"a+")) == NULL) {  
        perror("open dest file failed!");  
        return;  
    }  
    if ((srcfp = fopen(srcfile,"r")) == NULL) {  
        perror("open source file failed!"); 
        return;  
    }
    while ((len = fread(buffer, 1, 1, srcfp)) > 0) {  
        fwrite(buffer, 1, len, destfp);  
    }  
    fclose(srcfp); 
    fclose(destfp); 
    return;  
}

void getPmod()
{
	mpz_init(pmod);
	FILE *fs1;
	if ((fs1 = fopen("database/mod", "r")) == NULL)
	{
		perror("open");
	}
	mpz_inp_raw(pmod, fs1);
	fclose(fs1);
}
void getPbase()
{
	mpz_init(pbase);
	FILE *fs2;
	if ((fs2 = fopen("database/base", "r")) == NULL)
	{
		perror("open");
	}
	mpz_inp_raw(pbase, fs2);
	fclose(fs2);
}

int main(void)
{
	//init thread arguments.
	for(int count = 0; count < thread_num; count++)
	{
		//my_args[count] = {thread_id + count, block_id + block_offset * count};
		my_args[count].arg_thread_id = thread_id + count;
		my_args[count].arg_block_id = block_id + block_offset * count;
	//	printf("my_args[%d].arg_thread_id = %d\n", count, my_args[count].arg_thread_id);
	//	printf("my_args[%d].arg_block_id = %d\n", count, my_args[count].arg_block_id);
	}
	//init sub file names.
	for(int count2 = 0; count2 < thread_num; count2++)
	{
		sprintf(sub_result_file[count2], "database/clients/client%d", count2);
	}
	
	struct timeval start1, start2;
	gettimeofday(&start1, NULL);
	
	getPmod();
	getPbase();
	gmp_printf("Read from the database:pmod is:%Zx\n pbase is:%Zx\n", pmod, pbase);
	
	pthread_mutex_init(&mut, NULL);
	printf("I am the main function, I am creating all the threads!\n");
	thread_create();
	thread_wait();
	printf("I am the main function, I've destroyed all the threads.\n");
	for(int count_cat = 0; count_cat < thread_num; count_cat++)
	{
		fconcat("database/client", sub_result_file[count_cat]);
	}
	mpz_clear(pmod);
	mpz_clear(pbase);

	gettimeofday(&start2, NULL);
	unsigned long long int timeuse = 1000000 * (start2.tv_sec - start1.tv_sec) + start2.tv_usec - start1.tv_usec;
	printf("time: %lld s,%lld ms,%lld us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse);

	return(0);
}