#include "connect.h"
#include "msgparse.h"
#include "users.h"
#include "server_respond.h"

void *ClientHandler(void *arg)
{
  struct Client *client = ((struct Client *)arg);
  struct ParsedMsg msg;
  struct ThreadChanList chan_list = {
    .size = 0,
    .head = NULL
  };
  struct NamesList name_list = {
    .cnt = 0,
    .names = NULL
  };
  struct ThreadChanNode *ptr;
  union RegistrationFlags registered;
  char raw_msg[IRC_MSG_MAX_LENGTH];
  char send_msg[IRC_MSG_MAX_LENGTH];
  char nick[IRC_NICK_BUF_SIZE];
  ssize_t bytes = 0;
  int ret, index, connect = 1;

  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

  registered.clear = 0;

  memset(&raw_msg, 0, IRC_MSG_MAX_LENGTH);
  memset(&send_msg, 0, IRC_MSG_MAX_LENGTH);
  memset(&nick, 0, IRC_NICK_BUF_SIZE);

  while (registered.flags.user == 0 || registered.flags.nick == 0) {
    if ((bytes = IRCMsgRead(client->sockfd, raw_msg)) < 0) {
      printf("disconnected\n");
      registered.flags.fail = 1;
      break;
    }
    printf("raw: %s len %d\n", raw_msg, (int)strlen(raw_msg));
    FormParsedMsg(raw_msg, &msg);
    if (msg.cmd == IRCCMD_QUIT) {
      registered.flags.fail = 1;
      break;
    }
    if (msg.cmd == IRCCMD_USER) {
      registered.flags.user = 1;
    } else if (msg.cmd == IRCCMD_NICK) {
      if (registered.flags.nick == 0) {
        if (msg.cnt == 0) {
          ErrorHandler(client->sockfd, 
                      "nickname parameter expected for a command and isn’t found",
                       ERR_ERRONEUSNICKNAME);
          FreeParsedMsg(&msg);
          continue;
        }
        if (msg.params[0][0] == '#') {
          ErrorHandler(client->sockfd, "USER NICK: #nick invalid", ERR_ERRONEUSNICKNAME);
          FreeParsedMsg(&msg);
          continue;
        }
        ret = AddUser(&all_users, msg.params[0], client);
        if (ret == IRC_USERERR_EXIST) {
          ErrorHandler(client->sockfd, "NICK already exist", ERR_NICKNAMEINUSE);
          FreeParsedMsg(&msg);
          continue;
        } else if (ret == IRC_USERERR_NICK) {
          ErrorHandler(client->sockfd, "INCORRECT NICK", ERR_ERRONEUSNICKNAME);
          FreeParsedMsg(&msg);
          continue;
        } else if (ret == IRC_USERERR_CANTADD) {
          ErrorHandler(client->sockfd, "USER CANT ADD", IRC_USERERR_CANTADD);
          FreeParsedMsg(&msg);
          registered.flags.fail = 1;
        } else {
          ret = strlen(msg.params[0]);
          strncpy(nick, msg.params[0], ret);
          nick[ret] = '\0';
          registered.flags.nick = 1;
        }
      }
    }
    FreeParsedMsg(&msg);
  }

  if (!registered.flags.fail) {
    printf("successfully registered user: %s\n", nick);
    registered.flags.connect = 1;
    SendConnectMsg(&all_users, "anonimus", nick);
    while (registered.flags.connect) {
      if ((bytes = IRCMsgRead(client->sockfd, raw_msg)) < 0) {
        printf("disconnected...\n");
        break;
      }
      printf("raw: %s len %d\n", raw_msg, (int)strlen(raw_msg));
      FormParsedMsg(raw_msg, &msg);

      switch (msg.cmd) {
        case IRCCMD_QUIT:
          registered.flags.connect = 0;
          if (chan_list.size > 0) {
            if (FormSendMsg(send_msg, raw_msg, nick) == 0) {
              for (ptr = chan_list.head; ptr != NULL; ptr = ptr->next) 
                SendMsgToChannel(&all_chan, &all_users, ptr->chan, nick,
                              send_msg);
            }
          }
          break;

        case IRCCMD_JOIN:
          if (msg.cnt == 0) {
            ErrorHandler(client->sockfd, "JOIN :Not enough parameters", ERR_NEEDMOREPARAMS);
            break;
          }
          if (AddUserToChannel(&all_chan, &all_users, msg.params[0],
                              nick) == 0) {
            printf("add to channel %s\n", msg.params[0]);
            chan_list.head = ThrListAddFront(&chan_list, msg.params[0]);
            if (FormSendMsg(send_msg, raw_msg, nick) == 0) {
              SendMsgToUser(&all_users, nick, send_msg);
              SendMsgToChannel(&all_chan, &all_users, msg.params[0], nick,
                              send_msg);
              pthread_mutex_lock(&client->send_lock);
              SendChannelList(client->sockfd, &all_chan, &all_users, msg.params[0],
                              "anonimous", nick);
              pthread_mutex_unlock(&client->send_lock);
            }
            printf("TO SEND: %s\n", send_msg);
          } else {
            ErrorHandler(client->sockfd, "Cannot join channel", ERR_CHANNELISFULL);
          }
          break;

        case IRCCMD_PRIVMSG:
          if (msg.cnt == 2) {
            if (FormSendMsg(send_msg, raw_msg, nick) == 0) {
              printf("send %s \nto %s len %d\n", send_msg, msg.params[0],
                                                (int)strlen(send_msg));
              if (msg.params[0][0] == '#') {
                if (SendMsgToChannel(&all_chan, &all_users, msg.params[0], 
                                    nick, send_msg) < 0)
                  perror("SendMsgToChannel failed");
              } else {
                SendMsgToUser(&all_users, msg.params[0], send_msg);
              }
            }
          } else {
            ErrorHandler(client->sockfd, "PRIVMSG: Not enough parameters", 
                        ERR_NEEDMOREPARAMS);
          }
          break;

        case IRCCMD_PART:
          if (msg.cnt != 0) {
            RemoveUserFromChannel(&all_chan, &all_users, msg.params[0], nick);
            chan_list.head = DeleteThrNode(&chan_list, msg.params[0]);
            if (FormSendMsg(send_msg, raw_msg, nick) == 0) {
              printf("send %s \nto %s\n", send_msg, msg.params[0]);
              SendMsgToUser(&all_users, nick, send_msg);
              SendMsgToChannel(&all_chan, &all_users, msg.params[0], nick, send_msg);
            }
          } else {
            ErrorHandler(client->sockfd, "PART: Not enough parameters", 
                        ERR_NEEDMOREPARAMS);
          }
          break;
         
        case IRCCMD_NICK:
          if (msg.cnt == 0) {
            ErrorHandler(client->sockfd, 
                      "nickname parameter expected for a command and isn’t found",
                       ERR_ERRONEUSNICKNAME);
            break;
          }
          if (msg.params[0][0] == '#') {
            ErrorHandler(client->sockfd, "USER NICK: #nick invalid", ERR_ERRONEUSNICKNAME);
            break; 
          }
          printf("change nick: %s -> %s\n", nick, msg.params[0]);
          if ((ret = RenameUser(&all_users, nick, msg.params[0])) == 0) { 
            ret = strlen(msg.params[0]);
            strncpy(nick, msg.params[0], ret);
            nick[ret] = '\0';
          } else {
            if (ret == IRC_USERERR_NOTFOUND) {
              ErrorHandler(client->sockfd, "User not found", ERR_NICKNAMEINUSE);
            } else if (ret == IRC_USERERR_EXIST) {
              ErrorHandler(client->sockfd, "User exist", ERR_NICKNAMEINUSE);
            }
            perror("RenameUser");
          }
          break;
          
        case IRCCMD_PING:
          if (msg.cnt == 1) {
            FormPongMsg(&all_users, raw_msg, nick);
          } else {
            ErrorHandler(client->sockfd, "PING: Not enough parameters", 
                        ERR_NEEDMOREPARAMS);
          }
          break;
        case IRCCMD_LIST:
          pthread_mutex_lock(&client->send_lock);
          SendAllChannelsList(client->sockfd, &all_chan, &all_users,
                              "anonimous", nick);
          pthread_mutex_unlock(&client->send_lock);
          break;
      }
      FreeParsedMsg(&msg);
    }
  }
  if (chan_list.size > 0) {
    for (ptr = chan_list.head; ptr != NULL; ptr = ptr->next)
      RemoveUserFromChannel(&all_chan, &all_users, ptr->chan, nick);
    FreeThreadList(&chan_list);
  }

  close(client->sockfd);
  pthread_mutex_destroy(&client->send_lock);
  DelUser(&all_users, (const char *)&nick);
  free(client);
  printf("close... %s\n", nick);
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
      client = (struct Client *)malloc(sizeof(struct Client));
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
  pthread_mutex_destroy(&all_chan.lock);
  close(listen_sock);
  return 0;
}
