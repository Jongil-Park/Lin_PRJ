#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>


#define  BUFF_SIZE   1024
#define  FILE_SERVER "/tmp/cmdSvr"

int   main( int argc, char **argv)
{
   int   client_socket;

   struct sockaddr_un   server_addr;

   char   buff[BUFF_SIZE+5];

   client_socket  = socket( PF_FILE, SOCK_STREAM, 0);
   if( -1 == client_socket)
   {
      printf( "socket 생성 실패n");
      exit( 1);
   }

   memset( &server_addr, 0, sizeof( server_addr));
   server_addr.sun_family  = AF_UNIX;
   strcpy( server_addr.sun_path, FILE_SERVER);

   if( -1 == connect( client_socket, (struct sockaddr*)&server_addr, sizeof( server_addr) ) )
   {
      printf( "접속 실패n");
      exit( 1);
   }
   write( client_socket, argv[1], strlen( argv[1])+1);      // +1: NULL까지 포함해서 전송
   read ( client_socket, buff, BUFF_SIZE);
   printf( "%sn", buff);
   close( client_socket);
   
   return 0;
}
