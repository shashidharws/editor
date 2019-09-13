#include <stdio.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

#define CTRL_KEY(k) ((k) & 0x1f)

void editorRefreshScreen();

typedef struct _editorCtx {
    struct termios t_o_p;
    struct winsize w_size;
} e_ctx;

e_ctx E;

void die(char *s) {
    editorRefreshScreen();
    perror(s);
    exit(1);
}

void editorWindowSize()
{
    if(ioctl(STDOUT_FILENO, TIOCGWINSZ, &E.w_size) == -1)
        die("ioctl");
}

void disableRaw()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.t_o_p) == -1)
        die("tcsetattr");
}

int enableRaw()
{
    struct termios t_p;
    atexit(disableRaw);
    if (tcgetattr(STDIN_FILENO, &E.t_o_p) != 0)
        die("tcgetattr");
    t_p = E.t_o_p;
    t_p.c_oflag = ~(OPOST);
    t_p.c_iflag = ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    t_p.c_cflag |= (CS8);
    t_p.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    t_p.c_cc[VMIN] = 0;
    t_p.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_p) == -1)
        die("tcsetattr");
}

void editorRefreshScreen()
{
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

}

char editorReadKey()
{
    char c;
    while(read(STDIN_FILENO, &c, 1) != 1) 
        if(errno == EAGAIN)
            die("read");
    return c;
}

char editorProcessKeyPress()
{
    char c;
    c = editorReadKey();

    switch(c) {
    case CTRL_KEY('q') : 
                        editorRefreshScreen();
                        exit(0);
                        break;

    }

    return c;
}

void editorDrawRows()
{
    int i;
    for(i = 0 ; i < E.w_size.ws_row; i++)
        write(STDOUT_FILENO, "~\r\n", 3);
}

int main()
{
    enableRaw();
    editorRefreshScreen();
    editorWindowSize();
    editorDrawRows();
    write(STDOUT_FILENO, "\x1b[H", 3);
    while(1) {
        editorProcessKeyPress();
    }
    return 0;
}
