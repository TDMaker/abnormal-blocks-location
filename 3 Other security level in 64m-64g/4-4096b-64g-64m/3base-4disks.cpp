/*
 * Generate a big prime.
 * The basic number is FFFFF...(128)
 * make a prime with gmp lib
 * a random n range in [0] - (100)
 * generate num c by a mod b
*/
#include <iostream>
#include <gmp.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <error.h>
#include <sys/time.h>  /*计时*/

#define security 4096
using namespace std;
/* g */
int main()
{
	/*计时初始化*/
	struct timeval start1, start2;
	gettimeofday( &start1, NULL );

	srand( (int) time( 0 ) );
	//int r = rand() % 100;
	int r = 1;
	printf( "In base with 4 disks, the random number generated is [%d]\n", r );

	char a1_str[security + 1];
	memset( a1_str, 'F', security );
	memset( a1_str + security, 0, 1 );

	mpz_t a, b, c;
	mpz_init_set_str( a, a1_str, 16 );
	mpz_init_set_str( b, a1_str, 16 );
	mpz_init( c );
	gmp_printf( "The mpz_t a and b both = %ZX\n", a );

	for ( int i = 0; i < r; i++ )
		mpz_nextprime( a, a );

	mpz_mod( c, a, b );// Set c to a mod b, a is a big prime and b is a big num.
	gmp_printf( "The mpz_t c = %ZX\n", c );

	FILE *fs;

	if ( (fs = fopen( "database/base", "wb+" ) ) == NULL )
	{
		perror( "open" );
		return(0);
	}

	mpz_out_raw( fs, c );

	fclose( fs );

	mpz_clear( a );
	mpz_clear( b );
	mpz_clear( c );

	/*计时*/
	gettimeofday( &start2, NULL );
	unsigned long long int timeuse = 1000000 * (start2.tv_sec - start1.tv_sec) + start2.tv_usec - start1.tv_usec;
	printf( "time: %lld s,%lld ms,%lld us\n", timeuse / (1000 * 1000), timeuse / 1000, timeuse );

	return(0);
}