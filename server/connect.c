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

int IRC_Msg_Read(int sockfd, char *raw_msg)
{
	char buf[IRC_MSG_SIZE];
	int bytes, total = 0;
	for (;;) {
		if (total >= IRC_MSG_SIZE) 
			return -1;
		if ((bytes = read(sockfd, &buf[total], 1)) <= 0)
			return -1;
			
		if (buf[total] == '\r') {
			if (++total < IRC_MSG_SIZE) {
				if (read(sockfd, &buf[total], 1) <= 0) 
					return -1;
				if (buf[total] == '\n') {
					buf[total - 1] = '\0';
					strncpy(raw_msg, buf, total);
					return total - 1;
				} else 
					return -1;
			} else {
				return -1;
			}
		} else {
				total += bytes;
			}
	}
}


