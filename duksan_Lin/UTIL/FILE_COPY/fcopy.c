#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

/*******************************************************************/
int main(int argc, char* argv[])
/*******************************************************************/
{
	FILE    *fp_sour;
	FILE    *fp_dest;
	char     buff[1024];
	size_t   n_size;
										  
	fp_sour  = fopen( "/duksan/APP/duksan_upload\0"  , "r");
					
	if ( fp_sour == NULL ) {
		fprintf( stderr, "Empty duksan_upload file\n");
		fflush( stderr );
		return 1;
	}

	fseek(fp_sour, 0L, SEEK_END);
	if ( ftell(fp_sour) < 204800 ) {
		fprintf( stderr, "duksan_upload file size under 2K\n");
		fflush( stderr );
		return 1;
	}

	rewind(fp_sour);
	fp_dest  = fopen( "/duksan/APP/duksan\0", "w");

	while( 0 < (n_size = fread( buff, 1, 1024, fp_sour))) {
		fwrite( buff, 1, n_size, fp_dest);
	}                            
	
	fclose( fp_sour);
	fclose( fp_dest);

	fprintf( stderr, "rm /duksan/APP/duksan_upload\n");
	fflush( stderr );	
	system("rm /duksan/APP/duksan_upload");
	
	return 1;
}



