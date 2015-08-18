#include "connect.h"
#include "msgparse.h"
#include "users.h"

void *ClientHandler(void *arg)
{
	struct Client *client = ((struct Client *)arg);
	struct ParsedMsg msg;
	struct IRCUser *user = NULL;
	char raw_msg[IRC_MSG_SIZE];
	ssize_t bytes = 0;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	registered.clear = 0;
	
	while (registered.flags.user == 0 && registered.flags.nick == 0) {
		if ((bytes = IRCMsgRead(client->sockfd, raw_msg)) < 0) {
			printf("disconnected\n");
			registered.flags.fail = 1;
			break;
		}
		FormParsedMsg(raw_msg, &msg);
		if (msg.cmd == IRCCMD_USER) {
			registered.flags.user = 1;
		} else if (msg.cmd == IRCCMD_NICK) {
			if (registered.flags.nick == 0) {
				registered.flags.nick = 1;
				if (AddUser(&all_users, msg.param[0], client) < 0) {
					perror("AddUser faild");
					registered.flags.fail = 1;
				} else {
					user = GetUserPtr(&all_users, msg.param[0]);
				}
			}
		}
		FreeParseMsg(&msg);
	}

	if (!registered.flag.fail) {
		for (;;) {
			if ((bytes = IRCMsgRead(client->sockfd, raw_msg)) < 0) {
				printf("disconnected...\n");
				break;
			}
			FormParsedMsg(raw_msg, &msg);
			if (msg.cmd == IRCCMD_QUIT) {
				break;
			}
			
		}
	}
	
	Release(user);
	free(client);
	printf("close...\n");
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in ser_addr, cl_addr;
	struct Client *client;
	int listen_sock, connect;
	socklen_t len = (socklen_t)sizeof(struct sockaddr_in);
	pthread_attr_t attr;
	const int kOpt = 1;

	if (argc != 3) {
		printf("format: IP_ADDR PORT\n");
		exit(EXIT_FAILURE);
	}

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &kOpt,
								sizeof(int)) == -1 ) {
    perror("setsockopt");
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
	ChannelsInit(&all_chan);
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	while (1) {
		if ((connect = accept(listen_sock, (struct sockaddr *)&cl_addr,
													&len)) > 0) {
			client = malloc(sizeof(struct Client));
			if (client == NULL) {
				perror("malloc");
				break;
			}
			client->sockfd = connect;
			pthread_mutex_init(&client->send_lock, NULL);
			if (pthread_create(&(client->pid), &attr, ClientHandler,
												(void *)client) > 0) {
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
