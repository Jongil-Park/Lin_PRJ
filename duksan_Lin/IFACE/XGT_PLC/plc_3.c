char plc_3_ip[] = "192.168.0.12";


int g_nCountPlc3 = 7;


struct XGT_PLC_COUNT plc_3_count[] = {
	{XGT_AIO_TYPE, 0},	
	{XGT_AIO_TYPE, 50},
	{XGT_AIO_TYPE, 100},
	{XGT_AIO_TYPE, 150},
	{XGT_AIO_TYPE, 300},
	{XGT_AIO_TYPE, 300},
	{XGT_DIO_TYPE, 0},
};


int  xgt_connect_plc_3(int nSocket)
{
	struct sockaddr_in server_addr;
	struct timeval timeo;
	int res; 
	
#ifdef ACCEPT_NONBLOCKING
	long arg; 		
	fd_set myset; 
	struct timeval tv; 
	int valopt; 
	socklen_t lon;
#endif
		
	
	memset( &server_addr, 0, sizeof(server_addr) );
	timeo.tv_sec = 3;
	timeo.tv_usec = 0;
	
	nSocket = socket( AF_INET, SOCK_STREAM, 0 );
	if( nSocket < 0 ) {
		fprintf( stdout, "+ Socket creation error\n" );
		fflush( stdout );
		close( nSocket);
		return -1;					
	}	
	
	setsockopt( nSocket, SOL_SOCKET, SO_RCVTIMEO, &timeo, sizeof(timeo) );

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr( plc_3_ip );
	server_addr.sin_port = htons( XGT_PLC_PORT );

	fprintf( stdout, "try connect %s\n", plc_3_ip );
	fflush( stdout );	


#ifdef ACCEPT_NONBLOCKING
	// Set non-blocking 
	// 출처:[LINUX] linux에서 network 접속시 non-block으로 연결하기..
	if( (arg = fcntl(nSocket, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( nSocket);
		return -1;
	} 
	
	arg |= O_NONBLOCK; 
	if( fcntl(nSocket, F_SETFL, arg) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( nSocket);
		return -1;
	}

	// Trying to connect with timeout 
	res =  connect(nSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
	if (res < 0) { 
		if (errno == EINPROGRESS) { 
			fprintf(stdout, "ELBA Progress in connect() - selecting\n"); 
			fflush( stdout );
			do { 
				tv.tv_sec = 5;  // 5초후에 빠져나오도록 한다.
				tv.tv_usec = 0; 
				FD_ZERO(&myset); 
				FD_SET(nSocket, &myset); 
				res = select(nSocket+1, NULL, &myset, NULL, &tv); 
				if (res < 0 && errno != EINTR) { 
					//fprintf(stdout, "Elba Error connecting %d - %s\n", errno, strerror(errno)); 
					//fflush( stdout );
					close( nSocket );
					return -1; 
				} 
				else if (res > 0) { 
					// Socket selected for write 
					lon = sizeof(int); 
					if (getsockopt(nSocket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
						//fprintf(stdout, "Elba Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
						//fflush( stdout );
						close( nSocket );
						return -1;
					} 
					// Check the value returned... 
					if (valopt) { 
						fprintf( stdout, "[ELBA] connection() %d - %s\n", valopt, strerror(valopt)); 
						fflush( stdout );							
						close( nSocket );
						return -1;
					} 
					break; 
				} 
				else { 
					//fprintf(stdout, "Elba Timeout in select() - Cancelling!\n"); 
					//fflush( stdout );
					close( nSocket);
					return -1;
				} 
			} while (1); 
		} 
		else { 
			//fprintf(stdout, "Error connecting %d - %s\n", errno, strerror(errno)); 
			//fflush( stdout );
			close( nSocket );
			return -1;
		} 
	} 

	// Set to blocking mode again... 
	if( (arg = fcntl(nSocket, F_GETFL, NULL)) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( nSocket);
		return -1; 
	} 
	
	arg &= (~O_NONBLOCK); 
	if( fcntl(nSocket, F_SETFL, arg) < 0) { 
		fprintf(stdout, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
		fflush( stdout );
		close( nSocket );
		return -1; 
	}
#else
	res =  connect( nSocket, (struct sockaddr *)&server_addr, sizeof(server_addr) ); 
	if (res < 0) { 
		return -1;
	}	
#endif		
	
	return nSocket;
}

void xgt_close_plc_3(void)
{
	
}

void xgt_plc_3_process(void)
{
	int i = 0;
	int nRes = 0;
	int nSocket = 0;
	int nPlcSelect = 3;
	
	printf("\n\nxgt plc %d\n", nPlcSelect);
	
	// connect plc 3
	nRes = xgt_connect_plc_3(nSocket);
	if ( nRes < 0 )
		return;
	
	for ( i = 0; i < g_nCountPlc3; i++ ) {
		if ( plc_1_count[i].type == XGT_AIO_TYPE ) {
			xgt_read_aio(nPlcSelect,  plc_3_count[i].addr);
		}
		else if  ( plc_1_count[i].type == XGT_DIO_TYPE ) {
			xgt_read_dio(nPlcSelect, plc_3_count[i].addr);
		}
	}
	
	// close plc 1
	xgt_close_plc_3();	
}
