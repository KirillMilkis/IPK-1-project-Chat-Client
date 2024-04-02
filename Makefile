ARGS = "-t", "tcp", "-s", "anton5.fit.vutbr.cz", "-p", "4567"

ipk24chat-client: client.c tcp.c udp.c
	gcc -g -o ipk24chat-client client.c tcp.c udp.c -lpthread
	
run:
	./ipk24chat-client ${ARGS}
clean:
	rm -f client
