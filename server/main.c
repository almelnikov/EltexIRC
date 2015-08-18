#include "connect.h"
#include "msgparse.h"
#include "users.h"

int search(pid_t pid)
{
	int index;
	for (index = 0; index < IRC_USERS_MAX; index++) {
		if (all_users.users[index].thr_info != NULL) {
			if (all_users.users[index].thr_info->pid != pid) {
				return index;
			}
		}
	}
	return -1;
}
	
int parse(char *buf, ssize_t len)
{
	if (strncmp(buf, "hello", 5) == 0) 
		return 1;
	
	if (strncmp(buf, "kill", 4) == 0) 
		return 2;
		
	return -1;
}

void *ClientHandler(void *arg)
{
	struct Client *client = ((struct Client *)arg);
	struct ParsedMsg *msg;
	struct IRCUser *user = &all_users.users[client->index], ;
	char raw_msg[IRC_MSG_SIZE];
	ssize_t bytes = 0;
	int ret;

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	user->thr_info = client;

	for (;;) {
		if ((bytes = IRCMsgRead(client->sockfd, raw_msg)) < 0) {
			printf("disconnected...\n");
			break;
		}
		/*FormParsedMsg(raw_msg, &msg);
		if (msg.cmd == IRCCMD_PASS) {
			if (user->nick != NULL) {
				ret = strlen(msg.params[0]);
				if (ret <= IRC_PASS_MAX) {
					strncpy(client->password, msg.params[0], ret);
				}
			}
		} else if (msg.cmd == IRCCMD_USER) {
			ret = AddUser(&all_users, msh.params[0], 
		} else if (msg.cmd == IRCCMD_PRIVMSG) {
			//...
		} else if (msg.cmd == IRCCMD_KILL) {
			
			/*user = GetUserPtr(&all_users, msg.params[0]);
			if (pthread_cancel(user.thr_info->pid) != 0) 
				perror("pthread_create");
			Release();
		}*/
		printf("%s, %d\n", raw_msg, bytes);
		ret = parse(raw_msg, bytes);
		if (ret == 1) {
			ret = search(client->pid);
			write(all_users.users[ret].th_info->sockfd, "hey", 3);
		} else if (ret == 2) {
			printf("try kill..");
			ret = search(client->pid);
			if (ret != -1) {
				if (pthread_cancel(all_users.users[ret].thr_info->pid) != 0) {
					perror("pthread_cancel");
				}
				Release(ret);
			}
		} else if (ret == -1) {
			printf("oops\n");
		}
	}
	
	printf("close...\n");
	Release(client->index);
	pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
	struct sockaddr_in ser_addr, cl_addr;
	struct Client *client;
	int listen_sock, connect, i;
	socklen_t len = (socklen_t)sizeof(struct sockaddr_in);
	pthread_attr_t attr;
	int opt = 1;

	if (argc != 3) {
		printf("format: IP_ADDR PORT\n");
		exit(EXIT_FAILURE);
	}

	listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	if (setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int)) == -1 ) {
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
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	while(1) {
		if ((connect = accept(listen_sock, (struct sockaddr *)&cl_addr, &len)) > 0) {
			if ((i = SearchAvailable()) == -1) {
				continue;
			}
			client = malloc(sizeof(struct Client));
			if (client == NULL) {
				perror("malloc");
				break;
			}
			client->sockfd = connect;
			client->index = i;
			pthread_mutex_init(&client->send_lock, NULL);
			if (pthread_create(&(client->pid), &attr, ClientHandler, (void *)client) > 0) {
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


