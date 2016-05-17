/*
 * This module is related to GetProof
 *
 *
 *
 *
 */
#include <pthread.h>
#include <iostream>
#include <string.h>
#include <sys/time.h>
#include <gmp.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define block_max 4096
#define thread_num 32
#define block_offset 128
#define security_level 512

pthread_t thread[thread_num];
pthread_mutex_t mut;

char* mapped_mem[thread_num];
long long flength[thread_num];
char sub_source_file[thread_num][64];
char sub_result_file[thread_num][64];
int para[thread_num];
mpz_t pmod, total;

void *thread1(void* arg)
{
	int tmp_thread = *(int*)arg;
	int tmp_block = 1;
	int tmp_block_max = tmp_block + block_offset;
	int	tmp_i = 0;
	int tmp_current_id = 0;

	//char tmp_result[security_level * buffer_result];
	//memset(tmp_result, 0, sizeof(tmp_result));
	
	char tmp_buffer[security_level + 16];
	char current_id[security_level];
	char pbase_tmp[security_level];
	
	char tmp_result1[security_level + 1];
	char tmp_result2[security_level + 16];
	char tmp_result3[security_level + 16];
	char filename[100];
	FILE *fs4;
	if ((fs4 = fopen(sub_result_file[tmp_thread - 1], "a+")) == NULL)
		perror("open");
	for(; tmp_block < tmp_block_max; tmp_block++)
	{
		if(tmp_block % 100 == 0)
		{
			printf("Now tmp_block is %d\n", tmp_block);
		}
		memset(tmp_buffer, 0, sizeof(tmp_buffer));
		memset(current_id, 0, sizeof(current_id));
		memset(pbase_tmp, 0, sizeof(pbase_tmp));

		memcpy(tmp_buffer, mapped_mem[tmp_thread - 1] + (security_level + 16) * (tmp_block - 1), (security_level + 16));
		memcpy(current_id, tmp_buffer, strchr(tmp_buffer, ':') - tmp_buffer);
		memcpy(pbase_tmp, strchr(tmp_buffer, ':') + 1, strchr(tmp_buffer, ',') - strchr(tmp_buffer, ':') - 1);

		tmp_i++;
		tmp_current_id = atoi(current_id);

		mpz_t server_result, pbase, m, mmod;
		mpz_init_set_ui(server_result, 1);
		mpz_init(m);
		mpz_init(pbase);
		mpz_init(mmod);
		memset(filename, 0, 100);
		if(tmp_current_id % 4 == 1)
		{
			sprintf(filename, "/mnt/sdb1/data_16m/%ld", tmp_current_id);
		}
		else if(tmp_current_id % 4 == 2)
		{
			sprintf(filename, "/mnt/sdc1/data_16m/%ld", tmp_current_id);
		}
		else if(tmp_current_id % 4 == 3)
		{
			sprintf(filename, "/mnt/sdd1/data_16m/%ld", tmp_current_id);
		}
		else if(tmp_current_id % 4 == 0)
		{
			sprintf(filename, "/mnt/sde1/data_16m/%ld", tmp_current_id);
		}
		//printf("In thread %d, filename is %s\n", tmp_thread, filename);
		FILE *fs3;
		if ((fs3 = fopen(filename, "r")) == NULL)
			perror("open");
		mpz_inp_raw(m, fs3);
		fclose(fs3);

		mpz_set_str(pbase, pbase_tmp, 16);
		mpz_mod(mmod, m, pmod);
		mpz_powm_sec(server_result, pbase, mmod, pmod);


		memset(tmp_result1, 0, security_level);
		memset(tmp_result2, 0, security_level);
		memset(tmp_result3, '0', security_level);

		mpz_get_str(tmp_result1, 16, server_result);

		sprintf(tmp_result2, "%d:%s,0.", tmp_current_id, tmp_result1);
		memcpy(tmp_result3 + (sizeof(tmp_result3) - strlen(tmp_result2)), tmp_result2, strlen(tmp_result2));
		fwrite(tmp_result3, sizeof(tmp_result3), 1, fs4);

		mpz_mul(total, total, server_result);
		mpz_mod(total, total, pmod);

		mpz_clear(server_result);
		mpz_clear(m);
		mpz_clear(pbase);
		mpz_clear(mmod);
	}	
	fclose(fs4);
	printf("Thread %d :Is the main funcion waiting for me?\n", tmp_thread);
	pthread_exit(NULL);
}

void thread_create(void)
{
	int	temp;
	memset(&thread, 0, sizeof(thread));
	for (int i = 0; i < thread_num; i++)
	{
		if ((temp = pthread_create(&thread[i], NULL, thread1, &(para[i]))) != 0)
		{
			printf("thread No.%d creation failture!\n", i + 1);
		}
		else
		{
			printf("thread No.%d creation success!\n", i + 1);
		}
	}
}


void thread_wait(void)
{
	for (int j = 0; j < thread_num; j++)
		if (thread[j] != 0)
		{
			pthread_join(thread[j], NULL);
			printf("Thread No.%d has finished\n", j + 1);
		}
}

void getPmod()
{
	FILE *fs1;
	if ((fs1 = fopen("database/mod", "r")) == NULL)
	{
		printf("file database/mod open failed");
		return;
	}
	mpz_inp_raw(pmod, fs1);
	fclose(fs1);
}

void getVerifiers_r()
{
	int fd;
	void * start_addr = 0;
	for(int file_count = 0; file_count < thread_num; file_count++)
	{
		fd = open(sub_source_file[file_count], O_RDONLY, S_IRUSR | S_IWUSR);
		if (fd == -1)
		{
			printf("File % open failed.\n", sub_source_file[file_count]);
			return;
		}

		flength[file_count] = lseek(fd, 0, SEEK_END);
		mapped_mem[file_count] = (char *) mmap(start_addr, flength[file_count], PROT_READ, MAP_PRIVATE,fd, 0);
		if (close(fd) == -1)
		{
			printf("File % close failed.\n", sub_result_file[file_count]);
			return;
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
}; 
int main(void)
{
	for(int count = 0; count < thread_num; count++)
	{
		para[count] = count + 1;
		sprintf(sub_source_file[count], "database/verifiers_r/verifier_r%d", count);
		sprintf(sub_result_file[count], "database/servers/server%d", count);
	}
	mpz_init(pmod);
	mpz_init_set_ui(total, 1);

	struct timeval start1, start2;
	gettimeofday(&start1, NULL);
	getPmod();
	getVerifiers_r();
	
	pthread_mutex_init(&mut, NULL);
	printf("I am the main function, I am creating all the threads!\n");
	thread_create();
	thread_wait();
	printf("I am the main function, I am destroyed all the threads.\n");

	for(int check_count = 0; check_count < thread_num; check_count++)
	{
		if (munmap(mapped_mem[check_count], flength[check_count]) == -1)
		{
			perror("munmap");
			return(1);
		}
	}

	gmp_printf("!!!total result:%Zx\n", total);

	for(int count_cat = 0; count_cat < thread_num; count_cat++)
	{
		fconcat("database/server", sub_result_file[count_cat]);
	}

	mpz_clear(pmod);
	mpz_clear(total);

	gettimeofday(&start2, NULL);
	unsigned long long int timeuse = 1000000 * (start2.tv_sec - start1.tv_sec) + start2.tv_usec - start1.tv_usec;
	printf("time: %lld s,%lld ms,%lld us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse);
	return(0);
}