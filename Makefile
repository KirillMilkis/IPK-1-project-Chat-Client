ARGS = "-t", "tcp", "-s", "anton5.fit.vutbr.cz", "-p", "4567"

client: client.c
	gcc -o client client.c -lpthread
run:
	./client ${ARGS}
clean:
	rm -f client
