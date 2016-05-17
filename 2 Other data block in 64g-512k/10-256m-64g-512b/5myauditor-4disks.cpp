/* 
 * The module is related to Challenge
 * The auditor generate a series of random keys.
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

#define block_max 256
#define thread_num 32
#define block_offset 8
#define security_level 512

pthread_t thread[thread_num];
pthread_mutex_t mut;

char* mapped_mem[thread_num];
long long flength[thread_num];
char sub_source_file[thread_num][64];
char sub_result_file1[thread_num][64];
char sub_result_file2[thread_num][64];
int para[thread_num];
mpz_t pmod, total, gbase;

void *thread1(void* arg)
{
	int tmp_thread = *(int*)arg; 
	int tmp_block = 1;
	int tmp_block_max = tmp_block + block_offset;
	int tmp_current_id = 0;

	char tmp_buffer[security_level + 16];
	char current_id[security_level];
	char pmod_tmp[security_level];
	char tmp_result1[security_level + 1];
	char tmp_result2[security_level + 16];
	char tmp_result3[security_level + 16];
	char tmp_result4[security_level + 1];
	char tmp_result5[security_level + 16];
	char tmp_result6[security_level + 16];
	printf("tmp_block = %d and tmp_block_max = %d\n", tmp_block, tmp_block_max);
	
	FILE *fs4;
	if ((fs4 = fopen(sub_result_file1[tmp_thread - 1], "a+")) == NULL)
	perror("open");
	FILE *fs8;
	if ((fs8 = fopen(sub_result_file2[tmp_thread - 1], "a+")) == NULL)
	perror("open");

	for(; tmp_block < tmp_block_max; tmp_block++)
	{
		memset(tmp_buffer, 0, sizeof(tmp_buffer));
		memset(current_id, 0, sizeof(current_id));
		memset(pmod_tmp, 0, sizeof(pmod_tmp));

		memcpy(tmp_buffer, mapped_mem[tmp_thread - 1] + (security_level + 16) * (tmp_block - 1), (security_level + 16));
		memcpy(current_id, tmp_buffer, strchr(tmp_buffer, ':') - tmp_buffer);
		memcpy(pmod_tmp, strchr(tmp_buffer, ':') + 1, strchr(tmp_buffer, ',') - strchr(tmp_buffer, ':') - 1);

		tmp_current_id = atoi(current_id);

		mpz_t a1, a4, pbase, g_result;
		mpz_init_set_ui(a1, 1);
		mpz_init(a4);
		mpz_init(pbase);
		mpz_init(g_result);

		srand((int) time(0) + tmp_block);
		unsigned int r = rand();
		mpz_set_str(pbase, pmod_tmp, 16);//Set the value of pbase from pmod_tmp, a null-terminated C string in base 16. 
		mpz_set_ui(a4, r); //Set the value of a4 from r.

		mpz_powm_sec(a1, pbase, a4, pmod); // set a1 to pbase^a4 mod pmod
		mpz_powm_sec(g_result, gbase, a4, pmod); //set g_result to gbase^a4 mod pmod

		memset(tmp_result1, 0, sizeof(tmp_result1));
		memset(tmp_result2, 0, sizeof(tmp_result2));
		memset(tmp_result3, '0', sizeof(tmp_result3));
		memset(tmp_result4, 0, sizeof(tmp_result4));
		memset(tmp_result5, 0, sizeof(tmp_result5));
		memset(tmp_result6, '0', sizeof(tmp_result6));

		mpz_get_str(tmp_result1, 16, a1);       //Convert    a1    to a string of digits named tmp_result1 in base 16
		mpz_get_str(tmp_result4, 16, g_result); //Convert g_result to a string of digits named tmp_result4 in base 16

		sprintf(tmp_result2, "%d:%s,0.", tmp_current_id, tmp_result1); // tmp_result2 is like [tmp_current_id]:[tmp_result1],0.
		//sprintf(tmp_result3 + (sizeof(tmp_result3) - strlen(tmp_result2)), "%s", tmp_result2);
		memcpy(tmp_result3 + (sizeof(tmp_result3) - strlen(tmp_result2)), tmp_result2, strlen(tmp_result2));
		//tmp_result3 is like 0000000...[tmp_block]:[tmp_result1],0.[one \0 and nine '0']
		
		fwrite(tmp_result3, sizeof(tmp_result3), 1, fs4);

		sprintf(tmp_result5, "%d:%s,0.", tmp_current_id, tmp_result4); // tmp_result5 is like [tmp_current_id]:[tmp_result4],0.
		//sprintf(tmp_result6 + (sizeof(tmp_result6) - strlen(tmp_result5)), "%s", tmp_result5);
		memcpy(tmp_result6 + (sizeof(tmp_result6) - strlen(tmp_result5)), tmp_result5, strlen(tmp_result5));
		//tmp_result6 is like 0000000...[tmp_block]:[tmp_result4],0.[one \0 and nine '0']

		fwrite(tmp_result6, sizeof(tmp_result6), 1, fs8);
		mpz_mul(total, total, a1); //Set total to total × a1.
		mpz_mod(total, total, pmod); //Set total to total mod pmod. 

		mpz_clear(a1);
		mpz_clear(a4);
		mpz_clear(pbase);
		mpz_clear(g_result);
	}
	fclose(fs4);
	fclose(fs8);
	printf("thread %d:Is the main funcion waiting for me?\n", tmp_thread);
	pthread_exit(NULL);
}


void thread_create(void)
{
	int	temp = 0;
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
	{
		if (thread[j] != 0)
		{
			pthread_join(thread[j], NULL);
			printf("Thread No.%d has finished\n", j + 1);
		}
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
void getPbase()
{
	FILE *fs2;
	if ((fs2 = fopen("database/base", "r")) == NULL)
	{
		printf("file database/base open failed");
		return;
	}
	mpz_inp_raw(gbase, fs2);
	fclose(fs2);
}
void getClient()
{
	int	fd;
	void* start_addr = 0;
	for(int file_count = 0; file_count < thread_num; file_count++)
	{
		fd = open(sub_source_file[file_count], O_RDONLY, S_IRUSR | S_IWUSR);
		if (fd == -1)
		{
			printf("File % open failed.\n", sub_source_file[file_count]);
			return;
		}
		flength[file_count] = lseek(fd, 0, SEEK_END);
		//printf("flength[%d] = %d\n", file_count, flength[file_count]);
		mapped_mem[file_count] = (char *) mmap(start_addr, flength[file_count], PROT_READ, MAP_PRIVATE, fd, 0);
		//if(file_count == 15)printf("length of mapped_mem[%d] is\n%d\n", file_count, strlen(mapped_mem[file_count]));
		if (close(fd) == -1)
		{
			printf("File % close failed.\n", sub_source_file[file_count]);
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
}
int main(void)
{
	for(int count = 0; count < thread_num; count++)
	{
		para[count] = count + 1;
		sprintf(sub_source_file[count],  "database/clients/client%d", count);
		sprintf(sub_result_file1[count], "database/verifiers/verifier%d", count);
		sprintf(sub_result_file2[count], "database/verifiers_r/verifier_r%d", count);
	}
	mpz_init(pmod);
	mpz_init(gbase);
	mpz_init_set_ui(total, 1);
	
	struct timeval start1, start2;
	gettimeofday(&start1, NULL);
	getPmod();
	getPbase();
	getClient();

	pthread_mutex_init(&mut, NULL);
	printf("I am the main function, I am creating all the threads!\n");
	thread_create();
	thread_wait();
	printf("I am the main function, I've destroyed all the threads.\n");

	for(int check_count = 0; check_count < thread_num; check_count++)
	{
		if (munmap(mapped_mem[check_count], flength[check_count]) == -1)
		{
			perror("munmap");
			return(1);
		}
	}

	gmp_printf("!!!total result:%Zx\n", total);
	mpz_clear(pmod);
	mpz_clear(gbase);
	mpz_clear(total);

	gettimeofday(&start2, NULL);
	unsigned long long int timeuse = 1000000 * (start2.tv_sec - start1.tv_sec) + start2.tv_usec - start1.tv_usec;
	printf("time: %lld s,%lld ms,%lld us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse);
	for(int count_cat = 0; count_cat < thread_num; count_cat++)
	{
		fconcat("database/verifier", sub_result_file1[count_cat]);
		fconcat("database/verifier_r", sub_result_file2[count_cat]);
	}
	return(0);
}