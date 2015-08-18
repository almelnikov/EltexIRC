#include "connect.h"

void Release(struct IRCUser *user)
{
	close(user->thr_info->sockfd);
	pthread_mutex_destroy(&user->thr_info->send_lock);
	DelUser(&all_users, user->nick);
}

int IRCMsgRead(int sockfd, char *raw_msg)
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


