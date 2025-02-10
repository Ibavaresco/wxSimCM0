#define TEST

#if defined TEST
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#endif

#if defined TEST
int			Var1;
uint8_t		Var2;
uint8_t		Var3;
int			Var4;
uint8_t		Var5;
uint64_t	Var6;
char		Array1[33];
#endif

#if defined TEST
int __attribute__((noinline)) Func1( char c )
	{
	return c + 1;
	}
#endif

int main( void )
	{
//	qsort( Array1, 1, 1, NULL );
	
    while( 1 ) 
		{
		for( Var4 = 0; Var4 < 10000000; Var4++ )
			Func1( Var1++ );
		asm volatile( "nop" );
		}
		
	}
