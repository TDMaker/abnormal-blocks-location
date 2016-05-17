#include <stdio.h>
#include <cstring>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdlib.h>
#include <sys/time.h>

#include <time.h>

#define BLOCKMOUNT 32768

using namespace std;

int main()
{
	typedef struct FileBlock{
		unsigned char		file_len[4];			/* 1 byte * 4 */
		unsigned char		data[1024 - 48];		/* 1 byte * (1024 - 48) */
		char				operation;				/* 1 byte */
		char				filled[7];				/* 1 byte * 7 */
		unsigned int		rand_num;				/* 4 bytes */
		unsigned long		block_id;				/* 8 bytes */
		unsigned long		previous_block_id;		/* 8 byte */
		unsigned long		next_block_id;			/* 8 byte */
		unsigned long long	user_id;				/* 8 bytes */
	} MyFileBlock;
	/* The sort of members define the size of the struct*/
	/* length of head is 8 + 1 + 8 + 8 + 4 + 8 + 4 + 7 = 48 bytes */
	/* length of whole file block is 48 + (1024 - 48) = 1024 bytes  = 1k*/
	MyFileBlock file_block;
	file_block.block_id = 0;
	file_block.operation = 0;
	file_block.previous_block_id = 0;
	file_block.next_block_id = 0;
	file_block.rand_num = 0;
	file_block.user_id = 100;
	file_block.file_len[0] = 0x00;
	file_block.file_len[1] = 0x1F;
	file_block.file_len[2] = 0xFF;
	file_block.file_len[3] = 0xFC;
	strcpy(file_block.filled, "AAAAAAA");
	int		k, j;
	char	tmp_file_name[100];
	struct 	timeval	start, end;
	unsigned char fill_data[1024];
	memset(&(file_block.data), 0, sizeof(file_block.data) );
	memset( tmp_file_name, 0, sizeof(tmp_file_name) );
	srand( (int) time( 0 ) );
	/*
	if(access("data", F_OK))
	{
		mkdir("data",0777);
	}
	else
	{
		system("rm -rf data");
	}
	*/
	gettimeofday( &start, NULL );

	for ( j = 0; j < BLOCKMOUNT; j++ )
	{
		file_block.rand_num = rand();
		file_block.block_id = j + 1;
		file_block.previous_block_id = file_block.block_id - 1;
		file_block.next_block_id = file_block.block_id + 1;
		if(file_block.block_id % 4 == 1)
		{
			sprintf( tmp_file_name, "/mnt/sdb1/data_2m/%ld", file_block.block_id);
		}
		else if(file_block.block_id % 4 == 2)
		{
			sprintf( tmp_file_name, "/mnt/sdc1/data_2m/%ld", file_block.block_id);
		}
		else if(file_block.block_id % 4 == 3)
		{
			sprintf( tmp_file_name, "/mnt/sdd1/data_2m/%ld", file_block.block_id);
		}
		else if(file_block.block_id % 4 == 0)
		{
			sprintf( tmp_file_name, "/mnt/sde1/data_2m/%ld", file_block.block_id);
		}
		//sprintf( tmp_file_name, "data/%ld", file_block.block_id);
		printf( "test:%s\n", tmp_file_name );

		int stream = open( tmp_file_name, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );

		write( stream, &file_block, sizeof(file_block) );
		/* 1024*64 */
		for ( k = 1; k < 2048; k++ )
		{
			memset( fill_data, k % 1024, 1024 );
			write( stream, &fill_data, sizeof(fill_data) );
		}
		//65535 * 1024 + 1024 = 65536 * 1024 = 64 * 1024 *1024 = 64M
		close( stream );
		memset( tmp_file_name, 0, sizeof(tmp_file_name) );
	}

	gettimeofday( &end, NULL );

	int timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
	printf( "time: %d s,%d ms,%d us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse );
	return(0);
}


