#include <curses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <assert.h>
#include <pthread.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include <sys/sem.h>
#include <arpa/inet.h>
#include "irc_msgread.h"
#include "msgparse.h"

#define LEN_SYSTEM 68
#define ENTER 10
#define TAB 9

#define INPUT_N 0
#define CANAL_N 1
#define USER_N 2

WINDOW *chat, *canal, *input, *user, *sub_chat, *sub_canal, *sub_user, *sub_input, *sub_info;
int count_user, count_canal, count_row, count_col, exit_client, offset_all, port, sock;
char *canal_name, *user_name;
char now_canal[68], my_name[68], ip[32];

struct queue_sms {
	char buf[512];
	char window[68];
	char user_sour[68];
	char user_dest[68];
	int count;
	struct queue_sms *next;
};

struct queue_sms *list = NULL, *sever = NULL, *sender = NULL;

struct queue_sms *new(char *buf, char *canal)
{
	struct queue_sms *p = NULL;
	p = (struct queue_sms *)malloc(sizeof(struct queue_sms));
	assert(p != NULL);
	memset(p->buf, 0, 512);
	memset(p->window, 0, 68);
	memset(p->user_sour, 0, 68);
	memset(p->user_dest, 0, 68);

	strcpy(p->buf, buf);
	strcpy(p->window, canal);
	p->next = NULL;

	return p;
}

struct queue_sms *add(struct queue_sms *list_t, char buf[512], char *canal)
{
	struct queue_sms *newnode = NULL, *p = NULL;
	newnode = new(buf, canal);
	if(newnode != NULL) {
		if(list_t == NULL) {
		  list_t = newnode;
		  newnode->next = NULL;
		  return list_t;
		}
		for(p = list_t; p->next != NULL; p = p->next);
		p->next = newnode;
		newnode->next = NULL;
		return list_t;
	}
	return list_t;
}

struct queue_sms *del(struct queue_sms *list_t, struct queue_sms *our)
{
	struct queue_sms *p = NULL, *prev = NULL;

	for(p = list_t; p != our; p = p->next, prev = p);
	if(prev != NULL)
	  prev->next = p->next;
	else
	  list_t = list_t->next;
	free(p);
	return list_t;
}

void add_window()
{
  struct winsize size;
  ioctl(fileno(stdout), TIOCGWINSZ, (char *)&size);

  count_row = size.ws_row*0.8 - 4;
  count_col = size.ws_col*0.68 - 4;

  canal = newwin(size.ws_row*0.8, size.ws_col*0.17, 0, 0);
  chat = newwin(size.ws_row*0.8, size.ws_col*0.68, 0, (size.ws_col*0.17 - 1));
  user = newwin(size.ws_row*0.8, size.ws_col*0.17, 0, (size.ws_col - size.ws_col*0.17));
  input = newwin(size.ws_row*0.2, size.ws_col - 1, size.ws_row*0.8 - 1, 0);

  sub_canal = derwin(canal, size.ws_row*0.8 - 2, size.ws_col*0.17 - 2, 1, 1);
  sub_chat = derwin(chat, size.ws_row*0.8 - 2, size.ws_col*0.68 - 3, 1, 1);
  sub_user = derwin(user, size.ws_row*0.8 - 2, size.ws_col*0.17 - 2, 1, 1);
  sub_input = derwin(input, size.ws_row*0.2 - 3, size.ws_col - 1 - 2, 2, 1);
  sub_info = derwin(input, 1, size.ws_col - 1 - 2, 1, 1);

  box(canal, 0, 0);
  box(chat, 0, 0);
  box(user, 0, 0);
  box(input, 0, 0);

  refresh();
  wrefresh(canal);
  wrefresh(chat);
  wrefresh(user);
  wrefresh(input);
  wrefresh(sub_canal);
  wrefresh(sub_chat);
  wrefresh(sub_user);
  wrefresh(sub_input);
  wrefresh(sub_info);
}

void init()
{
  int sizeof_buf;
  initscr();
  cbreak();
  noecho();

  sizeof_buf = sizeof(char) * 68 * (count_row*2);

  user_name = (char *)malloc(sizeof_buf);
  canal_name = (char *)malloc(sizeof_buf);


  strcpy(now_canal, "#server");
  offset_all = 0;

  keypad(stdscr, TRUE);
  add_window();
}

void print_information()
{
	mvwprintw(sub_info, 0, 0, 
	    "Hello, you on chat, your name %s!!! exit sms: EXIT", my_name);
	refresh();
	wrefresh(sub_info);
}

void close_win()
{
  getch();
  endwin();
  exit(0);
}

void write_name()
{
  int pos = 0;
  static int index_user = 0;
  wmove(sub_user, 0, 0);

  for( ; index_user < strlen(user_name); index_user++) {
	if(pos == count_row) {
	  pos = 0;
	  break;
	}
	if(user_name[index_user] == '\n') pos++;
	wprintw(sub_user, "%c", user_name[index_user]);
  }
  if(index_user == strlen(user_name)) index_user = 0;
  wmove(sub_user, 0, 0);
  refresh();
  wrefresh(sub_user);
}


void write_canal()
{
  int pos = 0;
  static int index_canal = 0;
  wmove(sub_canal, 0, 0);

  for( ; index_canal < strlen(canal_name); index_canal++) {
	if(pos == count_row) {
	  pos = 0;
	  break;
	}
	if(canal_name[index_canal] == '\n') pos++;
	wprintw(sub_canal, "%c", canal_name[index_canal]);
  }
  if(index_canal == strlen(canal_name)) index_canal = 0;
  wmove(sub_canal, 0, 0);
  refresh();
  wrefresh(sub_canal);
}

/*
void read_name() 
{
  char num_user[68];
  int sizeof_buf;
  FILE *fd;

  fd = fopen("name_server", "w+");
  assert(fd != NULL);

  fwrite("2\n", strlen("2\n"), 1, fd);
  fwrite(my_name, strlen(my_name), 1, fd);
  fwrite("\n", strlen("\n"), 1, fd);
  fwrite("lida", strlen("lida"), 1, fd);
  fclose(fd);

  fd = fopen("name_server", "r");
  assert(fd != NULL);

  fgets(num_user, LEN_SYSTEM, fd);
  count_user = atoi(num_user);

  sizeof_buf = sizeof(char) * 68 * count_user;
  if(user_name == NULL) close_win();

  while((fread(user_name, sizeof_buf, 1, fd)) != 0);
  write_name();

  fclose(fd);
}
*/
/*
void read_canal() 
{
  char num_canal[68];
  int sizeof_buf;
  FILE *fd;

  fd = fopen("name_canal", "r");
  assert(fd != NULL);

  fgets(num_canal, LEN_SYSTEM, fd);
  count_canal = atoi(num_canal);

  sizeof_buf = sizeof(char) * 68 * count_canal;
  canal_name = (char *)malloc(sizeof_buf);
  if(canal_name == NULL) close_win();

  while((fread(canal_name, sizeof_buf, 1, fd)) != 0);
  write_canal();

  fclose(fd);
}
*/
void restart()
{
  struct queue_sms *p = NULL, *prev = NULL;
  int offset_dev = 0, i = 0;
  for(p = sever; p != NULL; p = p->next, i++) {
	if(strcmp(p->window, now_canal) == 0) {
	mvwprintw(sub_chat, 0 + offset_all, 0, "%s:%s", p->window, p->buf);
	offset_dev = (strlen(p->buf))%count_col;
	if(offset_dev != 0) {
	  offset_all += ((strlen(p->buf))/count_col) + 1;
	}
	else {
	  offset_all += ((strlen(p->buf))/count_col);
	}
	if (offset_all >= count_row) {
	  for(prev = sever; prev != NULL && strcmp(prev->window, now_canal) != 0;
			     prev = prev->next);
	  memset(prev->buf, 0, 512);
	  sever = del(sever, prev);
	  offset_all -= 3;
	  wmove(sub_chat, 0, 0);
	  wdeleteln(sub_chat);
	  wmove(sub_chat, 0, 0);
	  wdeleteln(sub_chat);
	  wmove(sub_chat, 0, 0);
	  wdeleteln(sub_chat);
	  wmove(sub_chat, offset_all, 0);
	}
	
	usleep(10);
	refresh();
	wrefresh(sub_chat);
	wmove(sub_input, 0, 0);
	wrefresh(sub_input);
	}
  }
}

int key(char *buf_input)
{
  int key = 0, flag = INPUT_N, x = 0, y_us = 0, y_can = 0;
  int count_y = 0, count_y_us = 0;
  chtype c_canal[68], c_user[68];
  char name_canal[68], name_user[68], buf_tmp[512];
  int index_n_c = 0;
  int index_buf_input = 0;

  wmove(sub_input, 0, 0);
  refresh();
  wrefresh(sub_input);
  while((key = getch()) != ENTER) {
	switch(key) {
	  case KEY_DOWN: {
		if(flag == CANAL_N) {
		  if(y_can < count_row && count_y < (count_canal - 1)) {
			count_y++;
			wmove(sub_canal, ++y_can, x);
		  }
		  else {
			count_y++;
			y_can = 0;
			wmove(sub_canal, y_can, x);
			wclear(sub_canal);
			write_canal();
			if(count_y >= (count_canal - 1)) count_y = 0;
		  }
		  refresh();
		  wrefresh(sub_canal);
		  break;
		}
		else if(flag == USER_N) {
		  if(y_us < count_row && count_y_us < (count_user - 1)) {
			count_y_us++;
			wmove(sub_user, ++y_us, x);
		  }
		  else {
			count_y_us++;
			y_us = 0;
			wmove(sub_user, y_us, x);
			wclear(sub_user);
			write_name();
			if(count_y_us >= (count_user - 1)) count_y_us = 0;
		  }
		  refresh();
		  wrefresh(sub_user);
		  break;
		}
		break;
	  }
	  case TAB: {
		if(flag == INPUT_N) {
		  wmove(sub_canal, 0, 0);
		  refresh();
		  wrefresh(sub_canal);
		  flag = 1;
		  y_can = 0;
		  break;
		}
		else if(flag == CANAL_N) {
		  wmove(sub_user, 0, 0);
		  refresh();
		  wrefresh(sub_user);
		  flag = 2;
		  y_us = 0;
		  break;
		}
		else {
		  wmove(sub_input, 0, 0);
		  refresh();
		  wrefresh(sub_input);
		  flag = 0;
		  break;
		}
		break;
	  }
	  case KEY_BACKSPACE: {
		buf_input[--index_buf_input] = '\0';
		wclear(sub_input);
		wprintw(sub_input, "%s", buf_input);
		refresh();
		wrefresh(sub_input);
		break;
	  }
	  default: {
		if(index_buf_input <= 400) {
		  buf_input[index_buf_input++] = key;
		  wclear(sub_input);
		  mvwprintw(sub_input, 0, 0, "%s", buf_input);
		  refresh();
		  wrefresh(sub_input);
		}
		break;
	  }
	}
  }
  if((key == ENTER) && (flag == CANAL_N)) {
	memset(buf_input, 0, 512);
	wclear(sub_input);
	wclear(sub_chat);

	mvwinchstr(sub_canal, y_can, x, c_canal);
	
	memset(name_canal, 0, 68);
	memset(now_canal, 0, 68);
	for(index_n_c = 0; (index_n_c < 68) && (c_canal[index_n_c] != ' ');
				    index_n_c++)
	  name_canal[index_n_c] = (c_canal[index_n_c] & A_CHARTEXT);
	strcpy(now_canal, name_canal);
	offset_all = 0;
	wclear(sub_chat);

	restart();

	refresh();
	wrefresh(sub_chat);
	wrefresh(sub_input);

	return CANAL_N;
  }
  else if((key == ENTER) && (flag == USER_N)) {
	wclear(sub_input);
	mvwinchstr(sub_user, y_us, x, c_user);

	memset(name_user, 0, 68);
	memset(buf_tmp, 0, 512);

	for(index_n_c = 0; (index_n_c < 68) && (c_user[index_n_c] != ' '); index_n_c++)
	  name_user[index_n_c] = (c_user[index_n_c] & A_CHARTEXT);
	strcpy(buf_tmp, name_user);
	strcat(buf_tmp, " :");
	strcat(buf_tmp, buf_input);
	memset(buf_input, 0, 512);
	strcpy(buf_input, buf_tmp);
	memset(buf_tmp, 0, 512);

	refresh();
	wrefresh(sub_input);

	return USER_N;
  }
  else {
	wclear(sub_input);
	refresh();
	wrefresh(sub_input);
	return INPUT_N;
  }
}

void IrcMsgSend_canal(char *buf_input)
{
  char buf_send_server[512];
  memset(buf_send_server, 0, 512);

  strcpy(buf_send_server, "PRIVMSG ");
  strcat(buf_send_server, now_canal);
  strcat(buf_send_server, " :");
  strcat(buf_send_server, buf_input);
  strcat(buf_send_server, "\r\n");

  send(sock, buf_send_server, strlen(buf_send_server), 0);
}

void IrcMsgSend_command(char *buf_input)
{
  char buf_send_server[512];
  memset(buf_send_server, 0, 512);
  
  if(buf_input != NULL)
	strcpy(buf_send_server, (buf_input + 1));
  strcat(buf_send_server, "\r\n");

  send(sock, buf_send_server, strlen(buf_send_server), 0);
}

void IrcMsgSend_user(char *buf_input)
{
  char buf_send_server[512];
  memset(buf_send_server, 0, 512);
  
  strcpy(buf_send_server, "PRIVMSG ");
  strcat(buf_send_server, buf_input);
  strcat(buf_send_server, "\r\n");

  send(sock, buf_send_server, strlen(buf_send_server), 0);
}


struct sembuf lock[2] = {
	{ 0, 0, 0 },
	{ 0, 1, 0 }
};

struct sembuf unlock[1] = {
	{ 0, -1, 0 }
};


void irc()
{
  char buf_input[512];
  int err, sem;
  key_t key_sem;
  
  key_sem = ftok("client.c", 'A');
  sem = semget(key_sem, 3, 0666 | IPC_CREAT);

  //all + key
  exit_client = 1;
  memset(buf_input, 0, 512);
  while(exit_client != 0) {
	err = key(buf_input);
	if((err == CANAL_N || (strlen(buf_input) == 0))) continue;
	if(strcmp(buf_input, "EXIT") == 0) {
	  exit_client = 0;
	  continue;
	}
	if((buf_input[0] != '/') && (err == INPUT_N)) {
	  semop(sem, lock, 2);
	  list = add(list, buf_input, now_canal);
	  semop(sem, unlock, 1);
	  IrcMsgSend_canal(buf_input);
	}
	else if((buf_input[0] != '/') && (err == USER_N)) {
	  semop(sem, lock, 2);
	  list = add(list, buf_input, now_canal);
	  semop(sem, unlock, 1);
	  IrcMsgSend_user(buf_input);
	}
	else if((buf_input[0] == '/')){
	  IrcMsgSend_command(buf_input);
	}
	memset(buf_input, 0, 512);
	refresh();
	wrefresh(sub_chat);
  }
}

int StrCanalName(char *canal)
{
  char name[68];
  int i = 0, j = 0, k = 0;
  
  while(canal_name[j] != '\0') {
	mvwprintw(sub_chat, 15, 0, "%s", canal);
	refresh();
	wrefresh(sub_chat);
	for(i = 0; i < 68 && canal_name[j] != '\n' && canal_name[j] != '\0';
		    i++, j++)
	  name[i] = canal_name[j];
	name[i] = '\0';
	j++;
	mvwprintw(sub_chat, 15, 0, "%s == %s", canal, name);
	refresh();
	wrefresh(sub_chat);
	if(strcmp(name, canal) == 0) {
	    for(k = j - (strlen(name)); canal_name[j] != '\0'; k++, j++) 
		canal_name[k] = canal_name[j];
	    for(; k != j; k++)
		canal_name[k] = '\0';
	}
	memset(name, 0, 68);
  }
  
  return 0;
}


int StrCanalDell(char *canal)
{
  char name[68];
  int i = 0, j = 0;
  
  while(canal_name[j] != '\0') {
	mvwprintw(sub_chat, 15, 0, "%s", canal);
	refresh();
	wrefresh(sub_chat);
	for(i = 0; i < 68 && canal_name[j] != '\n' && canal_name[j] != '\0';
		    i++, j++)
	  name[i] = canal_name[j];
	name[i] = '\0';
	j++;
	mvwprintw(sub_chat, 15, 0, "%s == %s", canal, name);
	refresh();
	wrefresh(sub_chat);
	if(strcmp(name, canal) == 0) return 1;
	memset(name, 0, 68);
  }
  
  return 0;
}


int ParsedStructServer(struct ParsedMsg *message)
{
  char tmp_name[512], nick[68];
  int ind = 0, y_ind = 0;
  memset(tmp_name, 0, 512);

  switch(message->cmd) {
	case IRCCMD_PRIVMSG: {
	  if(message->cnt < 2) return -1;
	  if(message->params[0][0] == '#') {
		list = add(list, message->params[1], message->params[0]);
	  }
	  else {
		memset(tmp_name, 0, 512);
		strcpy(tmp_name, message->params[0]);
		strcat(tmp_name, ":");
		strcat(tmp_name, message->params[1]);
		list = add(list, tmp_name, now_canal);
	  }
	  break;
	}
	case IRCCMD_JOIN: {
	  if(message->cnt < 1) return -1;
	  mvwprintw(sub_chat, 12, 0, "%s", message->params[0]);
	  ind++;
	  refresh();
	  wrefresh(sub_canal);
	  wrefresh(sub_chat);
	  if(StrCanalName(message->params[0]) == 0) {
		if(canal_name[0] != '\0') {
		  strcat(canal_name, "\n");
		  strcat(canal_name, message->params[0]);
		}
		else {
		  strcpy(canal_name, message->params[0]);
		}
		count_canal++;
		write_canal();
	  }
	  else {
	    strcpy(nick, GetNickFromHost(message));
	    mvwprintw(sub_chat, 20, 0, "nick %s", nick);
	    refresh();
	    wrefresh(sub_chat);
	    if(nick == NULL) break;
	    else {
		count_user++;
		strcat(user_name, "\n");
		strcat(user_name, tmp_name);
		write_name();
		wmove(sub_input, 0, 0);
		refresh();
		wrefresh(sub_input);
	    }
	  }
	  wmove(sub_input, 0, 0);
	  refresh();
	  wrefresh(sub_input);
	  break;
	}
	case IRCCMD_NUMERIC: {
	  if(message->cnt < 1) return -1;
	  if(message->numeric == 353 && strcmp(message->params[(message->cnt) - 2], now_canal) == 0) {
		wclear(sub_user);
		while(message->params[(message->cnt) - 1][y_ind] != '\0' && 
			    y_ind < strlen(message->params[(message->cnt) - 1])) {
		  if(user_name[0] != '\0') {
			strcat(user_name, "\n");
			for(ind = 0; ind < 512 && y_ind < strlen(message->params[(message->cnt) - 1]) && 
					    message->params[(message->cnt) - 1][y_ind] != ' '; ind++, y_ind++)
			  tmp_name[ind] = message->params[(message->cnt) - 1][y_ind];
			tmp_name[ind] = '\0';
			count_user++;
			y_ind++;
			strcat(user_name, tmp_name);
			memset(tmp_name, 0, 512);
		  }
		  else {
			for(ind = 0; ind < 512 && y_ind < strlen(message->params[(message->cnt) - 1]) && 
					    message->params[(message->cnt) - 1][y_ind] != ' '; ind++, y_ind++)
			  tmp_name[ind] = message->params[(message->cnt) - 1][ind];
			tmp_name[ind] = '\0';
			count_user++;
			y_ind++;
			strcpy(user_name, tmp_name);
			memset(tmp_name, 0, 512);
		  }
		}
		y_ind = 0;
		write_name();
		wmove(sub_input, 0, 0);
		refresh();
		wrefresh(sub_input);
	  }
	  break;
	}
	case IRCCMD_PART: {
	    memset(nick, 0, 68);
	    strcpy(nick, GetNickFromHost(message));
	    if(strcmp(nick, my_name) == 0) {
	        if(StrCanalName(message->params[0]) == 0) {
		    StrCanalDell(message->params[0]);
		    count_canal--;
		    write_canal();
		}
		else break;
	    }
	}
	default: {
	  if(message->cnt == 0) return -1;
	  //list = add(list, message->params[0], "#server");
	  break;
	}
	break;
  }
  return 0;
}

void *listen_server(void *arg) 
{
  char buf_stock[512];
  struct ParsedMsg message;
  int err, err_parse;

  while(exit_client != 0) {
	memset(buf_stock, 0, 512);
	err = IRCMsgRead(sock, buf_stock);

	mvwprintw(sub_chat, 12, 0, "COMMAND %s", buf_stock);
	refresh();
	wrefresh(sub_chat);

	if(err == -1) exit_client = 0;
	FormParsedMsg(buf_stock, &message);
	err_parse = ParsedStructServer(&message);
	if(err_parse == -1) continue;
	FreeParsedMsg(&message);
	refresh();
	wrefresh(sub_chat);
  }
}

void *get_message(void *arg)
{
  struct queue_sms *p = NULL, *prev = NULL;
  int offset_dev = 0, sem, pth_listen;
  pthread_t lis_server;

  while(exit_client != 0) {
	for(p = list; p != NULL; p = p->next) {
	  if((p->window != NULL) && (now_canal != NULL) && (strcmp(p->window, now_canal) == 0)) {
		usleep(10);
		mvwprintw(sub_chat, 0 + offset_all, 0, "%s:%s", p->window, p->buf);
		offset_dev = (strlen(p->buf))%count_col;
		if(offset_dev != 0) {
		  offset_all += ((strlen(p->buf))/count_col) + 1;
		}
		else {
		  offset_all += ((strlen(p->buf))/count_col);
		}
		if (offset_all >= count_row) {
			for(prev = sever; prev != NULL && strcmp(prev->window, now_canal) != 0;
						    prev = prev->next);
			memset(prev->buf, 0, 512);
			sever = del(sever, prev);
			usleep(10);
			refresh();
			wrefresh(sub_chat);
			wmove(sub_input, 0, 0);
			wrefresh(sub_input);
			offset_all -= 3;
			wmove(sub_chat, 0, 0);
			wdeleteln(sub_chat);
			wmove(sub_chat, 0, 0);
			wdeleteln(sub_chat);
			wmove(sub_chat, 0, 0);
			wdeleteln(sub_chat);
			wmove(sub_chat, offset_all, 0);
		}
		sever = add(sever, p->buf, now_canal);
		memset(p->buf, 0, 512);
		list = del(list, p);
		usleep(10);
		refresh();
		wrefresh(sub_chat);
		wmove(sub_input, 0, 0);
		wrefresh(sub_input);
		pth_listen = pthread_create(&lis_server, NULL, listen_server, NULL);
		assert(pth_listen == 0);
	  }
	}
  }
  return NULL;
}

void connect_sock(int argc, char **argv)
{
  struct sockaddr_in cl;
  
  if(argc < 4) exit(0);
  strcpy(ip, argv[1]);
  port = atoi(argv[2]);
  strcpy(my_name, argv[3]);
  printf("%s %d %s\n", ip, port, my_name);

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock <= 0) {
	printf("error socket\n");
	exit(0);
  }

  bzero(&cl, sizeof(cl));

  cl.sin_family = AF_INET;
  cl.sin_addr.s_addr = inet_addr(ip);
  cl.sin_port = htons(port);

  if((connect(sock, (struct sockaddr *)&cl, sizeof(cl))) < 0) {
	printf("error connect\n");
	exit(0);
  }
}

void send_start_sms()
{
  char start_sms[512];
  //NICK my_nick
  strcpy(start_sms, "NICK ");
  strcat(start_sms, my_name);
  strcat(start_sms, "\r\n");
  send(sock, start_sms, strlen(start_sms), 0);

  //USER my_nick 0 * :sms
  strcpy(start_sms, "USER ");
  strcat(start_sms, my_name);
  strcat(start_sms, " 0 * :1234");
  strcat(start_sms, "\r\n");
  send(sock, start_sms, strlen(start_sms), 0);

  //JOIN #server
  strcpy(start_sms, "JOIN #server\r\n");
  send(sock, start_sms, strlen(start_sms), 0);
}

int main(int argc, char **argv)
{
  pthread_t pth;
  int pth_get;
  
  connect_sock(argc, argv);
  send_start_sms();

  init();
  print_information();

  pth_get = pthread_create(&pth, NULL, get_message, NULL);
  assert(pth_get == 0);

  irc();

  close(sock);
  endwin();
  return 0;
}
