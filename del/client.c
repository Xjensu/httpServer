#include <stdio.h>
#include <stdint.h>

#include "extclib/net.h"

#define BUFF_SIZE 256
#define INDEX(ptr, init) (ptr - init)
void inputs_io(char *buffer, size_t size);

int main(void){
	int conn = connect_net("127.0.0.1:8080");
	if (conn < 0){
		fprintf(stderr, "%d\n", conn);
		return 1;
	}

	char buffer[BUFF_SIZE];
	inputs_io(buffer, BUFF_SIZE);
	
	send_net(conn, buffer, BUFF_SIZE);
	recv_net(conn, buffer, BUFF_SIZE);

	printf("%s\n", buffer);
	close_net(conn);

	return 0;
}

void inputs_io(char *buffer, size_t size)
{
	if (size == 0) return;

	char *ptr = buffer;
	while ( (*ptr = getchar()) != '\n' && INDEX(ptr, buffer) < (size-1) ) {
		++ ptr;
	}
	*ptr = '\0';
}

