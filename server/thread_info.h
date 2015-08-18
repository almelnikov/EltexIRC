#ifndef ELTEXIRC_THREAD_INFO_H
#define ELTEXIRC_THREAD_INFO_H

#include <pthread.h>

#define IRC_PASS_MAX 18

struct Client {
	pthread_t pid;
	int sockfd;
	int index;
	pthread_mutex_t send_lock;
};

#endif //ELTEXIRC_THREAD_INFO_H
