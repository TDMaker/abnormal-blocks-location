#include <pthread.h>
#include <unistd.h>

#include <iostream>
#include <string.h>

#include <sys/time.h>   /*计时*/
#include <gmp.h>        /*大数库*/

#define block_max	32 * 32 * 4
#define thread_num	1
#define buffer_result	200

pthread_t	thread[thread_num];
pthread_mutex_t mut;

static unsigned long int block_id = 1;

mpz_t pmod, pbase;


/*线程*/

void *thread1( void* )
{
	unsigned int		tmp_thread;
	unsigned long int	tmp_block;
	unsigned int		tmp_i = 0;


	pthread_mutex_lock( &mut );
	tmp_block = block_id;
	block_id++;
	pthread_mutex_unlock( &mut );

	while ( tmp_block <= block_max )
	{
		mpz_t begin;

		mpz_init( begin );

		char filename[100];
		memset( filename, 0, 100 );
		sprintf( filename, "data/%ld", tmp_block );

		FILE *fs3;
		if ( (fs3 = fopen( filename, "r" ) ) == NULL )
			perror( "open" );
		unsigned int i = mpz_inp_raw( begin, fs3 );
		fclose( fs3 );

		pthread_mutex_lock( &mut );

		tmp_block = block_id;
		block_id++;

		pthread_mutex_unlock( &mut );


		/* gmp_printf("current result:%Zx\n", begin); */

		mpz_clear( begin );
	}

	printf( "thread:%d :Is the main funcion waiting for me?\n", tmp_thread );
	pthread_exit( NULL );
}


void thread_create( void )
{
	int	temp;
	int	i;
	memset( &thread, 0, sizeof(thread) );                                           /* comment1 */
	for ( i = 0; i < thread_num; i++ )
		if ( (temp = pthread_create( &thread[i], NULL, thread1, NULL ) ) != 0 ) /* comment2 */
			printf( "thread No.%d creation failture!\n", i + 1 );
		else printf( "thread No.%d creation success!\n", i + 1 );
}


void thread_wait( void )
{
	int j;
	for ( j = 0; j < thread_num; j++ )
		if ( thread[j] != 0 ) /* comment4 */
		{
			pthread_join( thread[j], NULL );
			printf( "Thread No.%d has finished\n", j + 1 );
		}
}


int main( void )
{
	/*计时初始化*/
	struct timeval start1, start2;
	gettimeofday( &start1, NULL );

	pthread_mutex_init( &mut, NULL );
	printf( "HAHA! I am the main function, I am creating all the threads!\n" );

	thread_create();
	thread_wait();

	printf( "I am the main function, I am destroyed all the threads.\n" );

	/*计时*/
	gettimeofday( &start2, NULL );
	unsigned long long int timeuse = 1000000 * (start2.tv_sec - start1.tv_sec) + start2.tv_usec - start1.tv_usec;
	printf( "time: %lld s,%lld ms,%lld us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse );

	return(0);
}