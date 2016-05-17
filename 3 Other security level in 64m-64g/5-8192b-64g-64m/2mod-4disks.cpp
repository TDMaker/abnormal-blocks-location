/*
 * Generate a big prime.
 * The seed is FFFFF...(128)
 * make a prime with gmp lib
 * a random n range in [0] - (100)
 * generate a big prime according n loop
*/
#include <iostream>
#include <gmp.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <error.h>
#include <sys/time.h>  /*计时*/

#define security 8192

using namespace std;
int main()
{
	/*计时初始化*/
	struct timeval start1, start2;
	gettimeofday( &start1, NULL );

	srand( (int) time( 0 ) );
	//int r = rand() % 100;
	int r = 1;
	printf( "In mod with 4 disks, the random number generated is [%d]\n", r );

	char a1_str[security + 1];
	memset( a1_str, 'F', security );
	memset( a1_str + security, 0, 1 );

	mpz_t a;
	mpz_init_set_str( a, a1_str, 16 );
	gmp_printf( "mpz_t a = %ZX\n", a );

	for ( int i = 0; i < r; i++ )
	{
		mpz_nextprime( a, a );
	}
	FILE *fs;

	if ( (fs = fopen( "database/mod", "wb+" ) ) == NULL )
	{
		perror( "Error occured while open data/mod\n" );
		return(1);
	}

	mpz_out_raw( fs, a );

	fclose( fs );

	mpz_clear( a );

	/*计时*/
	gettimeofday( &start2, NULL );
	unsigned long long int timeuse = 1000000 * (start2.tv_sec - start1.tv_sec) + start2.tv_usec - start1.tv_usec;
	printf( "time: %lld s,%lld ms,%lld us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse );

	return(0);
}