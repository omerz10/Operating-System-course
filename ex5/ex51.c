/*
 * Omer Zucker
 * 200876548
 */


#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <wait.h>
#include <stdlib.h>
#include <termio.h>

/*
 * message for errorS
 */
char* error = "Error in system call";

/*
 * given function
 */
char getch();

/*
 * kill son's process and exit the program
 */
void killSigAndExit(int pid);


/********************
 *
 *  Implementation
 *
 *******************/

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}

void killSigAndExit(int pid) {
    perror(error);
    kill(pid, SIGKILL);
    waitpid(pid, NULL, 0);
    exit(-1);
}

int main() {
	int fd[2];
	pid_t pid;
	char input = 0;

    // creating a pipe
	if (pipe(fd) < 0) { perror(error); }
    // creating process
	if ((pid = fork()) < 0) { perror(error); }

	//father
	if (pid > 0) {
        if (close(fd[0]) < 0) { perror(error); }  // we close the read fd
        // get inputs from user
        do {
            input = getch();
            // check for correct input
            if (input != 'a' && input != 'd' && input != 's' && input != 'w' && input != 'q') {
                continue;
            }

            // write to pipe
            if (write(fd[1], &input, sizeof(char)) < 0) { killSigAndExit(pid); }

            // send signal
            if (kill(pid,SIGUSR2) < 0) { killSigAndExit(pid); }

            // user wants to quit
            if (input == 'q') {
                waitpid(pid, NULL, 0);
                break;
            }
        } while (1);

        if (close(fd[1]) < 0) { perror(error); } // we close write fd
	}
	else {  // child

        char *args[]={"./draw.out", NULL};

        dup2(fd[0], 0);

        if (close(fd[1]) < 0) { perror(error); } // we close write fd

        execvp(args[0],args);

        dup2(0, fd[0]);

        if (close(fd[0]) < 0) { perror(error); } // we close read fd
	}
}

