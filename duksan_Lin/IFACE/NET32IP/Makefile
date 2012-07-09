all : udp_server udp_client

udp_server : udp_server.c
	arm-linux-gcc -o $@ $< -lpthread

udp_client : udp_client.c
	arm-linux-gcc -o $@ $<

clean :
	rm udp_server
	rm udp_client
