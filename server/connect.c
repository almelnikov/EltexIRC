#include "connect.h"

int search_available()
{
	int i;
	for (i = 0; i < IRC_MAX_CLIENT; i++){
		if (client_pool[i].sockfd == -1){
			return i;
		}
	}
	return -1;
}

int IRC_Msg_Read(int sockfd, char *buf)
{
	int bytes;
	bytes = read(sockfd, buf, IRC_MSG_SIZE);
	if (buf[bytes - 1] == '\n' && buf[bytes - 2] == '\r') {
		buf[bytes - 2] = '\0';
		return bytes - 2;
	} else {
		return -1;
	}
}

