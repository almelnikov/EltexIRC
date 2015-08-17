#include "connect.h"
#include "msgparse.h"

void *client_handler(void *arg)
{
	struct Client client = *((struct Client *)arg);
	char raw_msg[IRC_MSG_SIZE];
	int index = client.index;
	int ret;
	ssize_t bytes = 0;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	//for (;;) {
		if ((bytes = IRC_Msg_Read(client.sockfd, raw_msg)) < 0)
				printf("wrong irc msg\n");
		printf("%s %d\n", raw_msg, bytes);
		
//	}	
	
	close(client.sockfd);
	client_pool[index].sockfd = -1;
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in ser_addr, cl_addr;
	int listen_sock, connect, i;
	socklen_t len = (socklen_t)sizeof(struct sockaddr_in);

	if (argc != 3) {
		printf("format: IP_ADDR PORT\n");
		exit(EXIT_FAILURE);
	}

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	memset(&ser_addr, 0, sizeof(struct sockaddr_in));

	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = inet_addr(argv[1]);
	ser_addr.sin_port = htons(atoi(argv[2]));

	if (bind(listen_sock, (struct sockaddr *)&ser_addr, len) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	if (listen(listen_sock, IRC_MAX_CLIENT) < 0) {
	 	perror("listen");
		exit(EXIT_FAILURE);
	}

	memset(&client_pool, -1, sizeof(struct Client) * IRC_MAX_CLIENT);

	while(1) {
		if ((connect = accept(listen_sock, (struct sockaddr *)&cl_addr, &len)) > 0) {
			if ((i = search_available()) == -1) {
				continue;
			}
			client_pool[i].sockfd = connect;
			client_pool[i].index = i;
			client_pool[i].internal_fd = internal_fd;
			if (pthread_create(&(client_pool[i].pid), NULL, client_handler, (void *)&client_pool[i]) > 0) {
				perror("pthread_create");
			}
		} else {
			perror("accept");
			break;
		}
	}
	close(listen_sock);
	return 0;
}


