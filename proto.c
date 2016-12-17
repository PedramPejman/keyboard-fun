#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

int checktty(struct termios *p, int term_fd)
{
    struct termios ck;
    return (
       tcgetattr(term_fd, &ck) == 0 &&
      (p->c_lflag == ck.c_lflag) &&
      (p->c_cc[VMIN] == ck.c_cc[VMIN]) &&
      (p->c_cc[VTIME] == ck.c_cc[VMIN])
    );
}


int
keypress(int term_fd, char *buf)
{
   int retval=read(term_fd, buf, sizeof *buf);
   return retval;
}

int   /* TCSAFLUSH acts like fflush for stdin */
flush_term(int term_fd, struct termios *p)
{
   struct termios newterm;
   errno=0;
   tcgetattr(term_fd, p);  /* get current stty settings*/

   newterm = *p; 
   newterm.c_lflag &= ~(ECHO | ICANON); 
   newterm.c_cc[VMIN] = 0; 
   newterm.c_cc[VTIME] = 0; 

   return( 
       tcgetattr(term_fd, p) == 0 &&
       tcsetattr(term_fd, TCSAFLUSH, &newterm) == 0 &&
       checktty(&newterm, term_fd) != 0
   );
}
void 
term_error(void)
{
     fprintf(stderr, "unable to set terminal characteristics\n");
     perror("");                                                
     exit(1);                                                   
}


void
wait_and_exit(void)
{
    struct timespec tsp={0,500};  /* sleep 500 usec (or likely more ) */
    struct termios  attr;
    struct termios *p=&attr;
    int keepon=0;
    int term_fd=fileno(stdin);
    char key_pressed;

    fprintf(stdout, "press any key to continue:");
    fflush(stdout);
    if(!flush_term(term_fd, p) )
       term_error();
    for(keepon=1; keepon;)
    {
        nanosleep(&tsp, NULL);
        switch(keypress(term_fd, &key_pressed) )
        {
              case 0:
              default:
                 break;
            case -1:
                 fprintf(stdout, "Read error %s", strerror(errno));
                 exit(1);
                 break;
            case 1:       /* time to quit */
                 //keepon=0;
                 fprintf(stdout, "[%d]\n", key_pressed);
                 //break;                 
        } 
    }
    if( tcsetattr(term_fd, TCSADRAIN, p) == -1 && 
          tcsetattr(term_fd, TCSADRAIN, p) == -1 )
          term_error();
    exit(0);
}

int main()
{
      wait_and_exit();
      return 0;  /* never reached */
}
