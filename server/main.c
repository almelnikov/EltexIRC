#include "connect.h"
#include "msgparse.h"
#include "users.h"

void *client_handler(void *arg)
{
	struct Client *client = ((struct Client *)arg);
	char raw_msg[IRC_MSG_SIZE];
	ssize_t bytes = 0;

	all_users.users[client->index].thr_info = client;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

	for (;;) {
		if ((bytes = IRC_Msg_Read(client->sockfd, raw_msg)) < 0) {
			printf("disconnected...\n");
			break;
		}
		//	printf("%s %d\n", raw_msg, bytes);
		// ....
	}

	close(client->sockfd);
	pthread_mutex_destroy(&client->send_lock);
	all_users.users[client->index].thr_info = NULL;
	free(client);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in ser_addr, cl_addr;
	struct Client *client;
	int listen_sock, connect, i;
	socklen_t len = (socklen_t)sizeof(struct sockaddr_in);
	pthread_attr_t attr;

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

	if (listen(listen_sock, IRC_USERS_MAX) < 0) {
	 	perror("listen");
		exit(EXIT_FAILURE);
	}

	UsersInit(&all_users);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	while(1) {
		if ((connect = accept(listen_sock, (struct sockaddr *)&cl_addr, &len)) > 0) {
			if ((i = search_available()) == -1) {
				continue;
			}
			client = malloc(sizeof(struct Client));
			client->sockfd = connect;
			client->index = i;
			pthread_mutex_init(&client->send_lock, NULL);
			if (pthread_create(&(client->pid), &attr, client_handler, (void *)client) > 0) {
				perror("pthread_create");
			}

		} else {
			perror("accept");
			break;
		}
	}
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&all_users.lock);
	close(listen_sock);
	return 0;
}


