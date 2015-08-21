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
#include <sys/sem.h>

#define LEN_SYSTEM 68
#define ENTER 10
#define TAB 9

#define INPUT_N 0
#define CANAL_N 1
#define USER_N 2

WINDOW *chat, *canal, *input, *user, *sub_chat, *sub_canal, *sub_user, *sub_input, *sub_info;
int count_user, count_canal, count_row, count_col, exit_client, offset_all;
char *canal_name, *user_name;
char now_canal[68];

struct queue_sms {
	char buf[512];
	char window[68];
	char user_sour[68];
	char user_dest[68];
	int count;
	struct queue_sms *next;
};

struct queue_sms *list = NULL, *sever = NULL;

struct queue_sms *new(char *buf)
{
	struct queue_sms *p = NULL;
	p = (struct queue_sms *)malloc(sizeof(struct queue_sms));
	assert(p != NULL);
	memset(p->buf, 0, 512);
	memset(p->window, 0, 68);
	memset(p->user_sour, 0, 68);
	memset(p->user_dest, 0, 68);

	strcpy(p->buf, buf);
	strcpy(p->window, now_canal);
	p->next = NULL;

	return p;
}

struct queue_sms *add(struct queue_sms *list_t, char buf[512])
{
	struct queue_sms *newnode = NULL, *p = NULL;
	newnode = new(buf);
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
    initscr();
    cbreak();
    noecho();

    strcpy(now_canal, "1");
    offset_all = 0;

    keypad(stdscr, TRUE);
    add_window();
}

void print_information()
{
	mvwprintw(sub_info, 0, 0, "Hello, please write: USER name 0 * :real_name !!! Next sms: NICK my_nickname !!! exit sms: bay");
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


void read_name() 
{
    char num_user[68];
    int sizeof_buf;
    FILE *fd;

    fd = fopen("name_server", "r");
    assert(fd != NULL);

    fgets(num_user, LEN_SYSTEM, fd);
    count_user = atoi(num_user);

    sizeof_buf = sizeof(char) * 68 * count_user;
    user_name = (char *)malloc(sizeof_buf);
    if(user_name == NULL) close_win();

    while((fread(user_name, sizeof_buf, 1, fd)) != 0);
    write_name();

    fclose(fd);
}


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
	    for(prev = sever; prev != NULL && strcmp(prev->window, now_canal) != 0; prev = prev->next);
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
    int key = 0, flag = INPUT_N, x = 0, y_us = 0, y_can = 0, count_y = 0, count_y_us = 0;
    chtype c_canal[68];
    char name_canal[68];
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
		key = 0;
		wclear(sub_input);
		wprintw(sub_input, "%s", buf_input);
		refresh();
		wrefresh(sub_input);
	    }
	    default: {
		buf_input[index_buf_input++] = key;
		wclear(sub_input);
		wprintw(sub_input, "%s", buf_input);
		refresh();
		wrefresh(sub_input);
	    }
	}
    }
    if(key == ENTER && ((flag == CANAL_N) || (strcmp(buf_input, "join") == 0))) {

	memset(buf_input, 0, 512);
	wclear(sub_input);
	wclear(sub_chat);

	mvwinchstr(sub_canal, y_can, x, c_canal);
	
	memset(name_canal, 0, 68);
	memset(now_canal, 0, 68);
	for(index_n_c = 0; (index_n_c < 68) && (c_canal[index_n_c] != ' '); index_n_c++)
	    name_canal[index_n_c] = (c_canal[index_n_c] & A_CHARTEXT);
//	mvwprintw(sub_chat, 0, 0, "canal %s", name_canal);
	strcpy(now_canal, name_canal);
	offset_all = 0;
	wclear(sub_chat);

	restart();

	refresh();
	wrefresh(sub_chat);
	wrefresh(sub_input);

	return 0;
    }
    wclear(sub_input);
    refresh();
    wrefresh(sub_input);
    return 1;
}

void irc()
{
    char buf_input[512];
    int err;
    //all + key
    exit_client = 1;
    memset(buf_input, 0, 512);
    while(exit_client != 0) {
	err = key(buf_input);
	if(err == 0 || (strlen(buf_input) == 0)) continue;
	if(strcmp(buf_input, "EXIT") == 0) {
	    exit_client = 0;
	    continue;
	}
	list = add(list, buf_input);
	memset(buf_input, 0, 512);
	refresh();
	wrefresh(sub_chat);
    }
}

void *get_message(void *arg)
{
    struct queue_sms *p = NULL, *prev = NULL;
    int offset_dev = 0;

    while(exit_client != 0) {
	for(p = list; p != NULL ; p = p->next) {
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
			for(prev = sever; prev != NULL && strcmp(prev->window, now_canal) != 0; prev = prev->next);
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
		sever = add(sever, p->buf);
		memset(p->buf, 0, 512);
		list = del(list, p);
		usleep(10);
		refresh();
		wrefresh(sub_chat);
		wmove(sub_input, 0, 0);
		wrefresh(sub_input);
	    }
	}
    }
    return NULL;
}

int main()
{
    pthread_t pth;
    int pth_get;
    init();
    print_information();

    read_name();
    read_canal();
    
    pth_get = pthread_create(&pth, NULL, get_message, NULL);
    assert(pth_get == 0);

    irc();

    endwin();
    return 0;
}
