#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

struct termios t_o_p;

void die(char *s) {
    perror(s);
    exit(1);
}
void disableRaw()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_o_p) == -1)
        die("tcsetattr");
}

int enableRaw()
{
    struct termios t_p;
    atexit(disableRaw);
    if (tcgetattr(STDIN_FILENO, &t_o_p) != 0)
        die("tcgetattr");
    t_p = t_o_p;
    t_p.c_oflag = ~(OPOST);
    t_p.c_iflag = ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    t_p.c_cflag |= (CS8);
    t_p.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    t_p.c_cc[VMIN] = 0;
    t_p.c_cc[VTIME] = 1;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &t_p) == -1)
        die("tcsetattr");
}

int main()
{
    enableRaw();
    char c;
    while(1) {
        if(read(STDIN_FILENO, &c, 1) == -1 && errno != EAGAIN) 
            die("read");
        if(iscntrl(c))
            printf("%d\r\n", c);
        else
            printf("%d (%c)\r\n", c, c);
        if(c == 'q')
            break;
    }
    return 0;
}
