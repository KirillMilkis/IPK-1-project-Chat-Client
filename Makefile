ARGS = "-t", "tcp", "-s", "anton5.fit.vutbr.cz", "-p", "4567"

client: client.c tcp.c udp.c
	gcc -g -o client client.c tcp.c udp.c -lpthread
	
run:
	./client ${ARGS}
clean:
	rm -f client
