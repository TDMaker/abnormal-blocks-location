#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <gmp.h>

#define block_max 256
#define security_level 512
#define level_a 4
#define level_b 4

mpz_t pmod;
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
int main(void)
{
	struct timeval start1, start2;
	gettimeofday(&start1, NULL);
	mpz_t g_a, g_b;
	mpz_init(g_a);
	mpz_init(g_b);
	char tmp_result[security_level];
	char tmp_str[security_level];
	char encrypt_b[security_level * level_b];
	char* final_result = (char*)malloc(security_level * level_b * 2);
	memset(encrypt_b, 0, sizeof(encrypt_b));
	getPmod();
	void * start_addr = 0;
	int fd = open("database/verifier", O_RDONLY, S_IRUSR | S_IWUSR);

	if (fd == -1)
	{
		perror("open");
		return(1);
	}
	long long flength = lseek(fd, 0, SEEK_END);
	char * mapped_mem = (char *) mmap(start_addr, flength, PROT_READ, MAP_SHARED, fd, 0);
	if (close(fd) == -1)
	{
		perror("close");
		return(1);
	}

	//char * mapped_mem1 = (char *) malloc(32 * level_b *2);
	//memset(mapped_mem1, 0, sizeof(mapped_mem1));
	char tmp_buffer[security_level];
	char tmp_buffer2[security_level];
	bool reset = false;

	
	for (int i = 0; i < block_max; i++)
	{
		memset(tmp_buffer, 0, sizeof(tmp_buffer));
		memset(tmp_buffer2, 0, sizeof(tmp_buffer2));
		memcpy(tmp_buffer, mapped_mem + (security_level + 16) * i, security_level + 16);
		memcpy(tmp_buffer2, strchr(tmp_buffer, ':') + 1, strchr(tmp_buffer, ',') - strchr(tmp_buffer, ':') - 1);
		if(i % level_b == 0 && i != 0)
		{
			mpz_set_str(g_a, encrypt_b, 16);
			mpz_mod(g_b, g_a, pmod);
			mpz_get_str(tmp_result, 16, g_b);
			sprintf(tmp_str, "%s%c", tmp_result, '\n');
			strcat(final_result, tmp_str);
			memset(tmp_result, 0, sizeof(tmp_result));
			memset(tmp_str, 0, sizeof(tmp_str));
			memset(encrypt_b, 0, sizeof(encrypt_b));
		}
		memcpy(encrypt_b + strlen(encrypt_b), tmp_buffer2, sizeof(tmp_buffer2));
	}
	mpz_set_str(g_a, encrypt_b, 16);
	mpz_mod(g_b, g_a, pmod);
	mpz_get_str(tmp_result, 16, g_b);
	sprintf(tmp_str, "%s%c", tmp_result, '\n');
	strcat(final_result, tmp_str);
	memset(tmp_result, 0, sizeof(tmp_result));
	memset(tmp_str, 0, sizeof(tmp_str));
	memset(encrypt_b, 0, sizeof(encrypt_b));
	char* start_pos = final_result;
	for(int m = block_max / level_b; m > 1; m /= level_a)
	{
		for(int n = 0; n < m; n++)
		{
			if(n % level_a == 0 && n != 0)
			{
				mpz_set_str(g_a, encrypt_b, 16);
				mpz_mod(g_b, g_a, pmod);
				mpz_get_str(tmp_result, 16, g_b);
				sprintf(tmp_str, "%s%c", tmp_result, '\n');
				strcat(final_result, tmp_str);
				memset(tmp_result, 0, sizeof(tmp_result));
				memset(tmp_str, 0, sizeof(tmp_str));
				memset(encrypt_b, 0, sizeof(encrypt_b));
			}
			memcpy(tmp_buffer2, start_pos, strchr(start_pos, '\n') - start_pos);
			memcpy(encrypt_b + strlen(encrypt_b), tmp_buffer2, sizeof(tmp_buffer2));
			memset(tmp_buffer2, 0, security_level);
			start_pos = strchr(start_pos, '\n') + 1;
		}
		mpz_set_str(g_a, encrypt_b, 16);
		mpz_mod(g_b, g_a, pmod);
		mpz_get_str(tmp_result, 16, g_b);
		sprintf(tmp_str, "%s%c", tmp_result, '\n');
		strcat(final_result, tmp_str);
		memset(tmp_result, 0, sizeof(tmp_result));
		memset(tmp_str, 0, sizeof(tmp_str));
		memset(encrypt_b, 0, sizeof(encrypt_b));
	}
	if (munmap(mapped_mem, flength) == -1)
	{
		perror("munmap");
		return(1);
	}

	int count = 0;
	int fd1 = open("database/verifiers_sort", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRWXO);

	if (fd1 == -1)
	{
		perror("open");
		return(1);
	}
	write(fd1, final_result, strlen(final_result));

	free(final_result);

	if (close(fd1) == -1)
	{
		perror("close");
		return(1);
	}
	gettimeofday(&start2, NULL);
	unsigned long long int timeuse = 1000000 * (start2.tv_sec - start1.tv_sec) + start2.tv_usec - start1.tv_usec;
	printf("time: %lld s,%lld ms,%lld us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse);

	return(0);
}