#ifndef ELTEXIRC_THREAD_INFO_H
#define ELTEXIRC_THREAD_INFO_H

#include <pthread.h>

struct Client {
  pthread_t pid;
  int sockfd;
  pthread_mutex_t send_lock;
};

#endif //ELTEXIRC_THREAD_INFO_H
