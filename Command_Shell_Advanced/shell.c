#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include "shell_programs.h"

#define INPUT_BYTES 100

//function to install handler for signal from professor's sigchld.c
typedef void(*sighandler_t)(int);

sighandler_t signal(int signo, sighandler_t handler){
        struct sigaction act, oact;

        act.sa_handler = handler;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;

        if(signo == SIGALRM){
#ifdef SA_INTERRUPT
                act.sa_flags |= SA_INTERRUPT; /*SunOS 4.x*/
#endif
        }
        else{
#ifdef SA_RESTART
                act.sa_flags |= SA_RESTART; /* SVR4, 4.4BSD */
#endif
        }

        if(sigaction(signo, &act, &oact) < 0){
                return(SIG_ERR);
        }

        return(oact.sa_handler);
}

char* cmd_pieces[50];
int length;

//SIGCHLD Handler
void int_handler(int sig){
        pid_t pid;
        int stat;

        pid = waitpid(WAIT_ANY, &stat, WNOHANG);
        completeJob(pid);
}

//parser code recieved from professor's parser.c example
void parser(char* shell_input){
        char* word;
        int i = 0;
        if(shell_input != NULL){
                while((word = strsep(&shell_input, " ")) != NULL){
                        cmd_pieces[i] = word;
                        i++;
                }
		//Strtok() added by me to remove \n from last argument (added on by fgets automatically)
                strtok(cmd_pieces[i-1], "\n");
		length = i;
                cmd_pieces[i] = NULL;
	}
}

//handle pipelined commands
void pipeline_cmd(char* input_cmd[50], int input_length, int bg){
	pid_t pid;
	int status, pfds[2];
	int i = 0, new_length = 0, curr_length = 0;
	char* new_input_cmd[50];
	char* curr_cmd[50];
	while(i < input_length && strcmp(input_cmd[i], "|") != 0){
		curr_cmd[i] = input_cmd[i];
		i++;
	}
	curr_length = i;
	if(curr_length > 2 && strcmp(curr_cmd[1], "<") == 0) fileToStdin(curr_cmd[2]);
	if(i < input_length && strcmp(input_cmd[i], "|") == 0){
		int j = 0;
	        i++;
		while(i < input_length){
			new_input_cmd[j] = input_cmd[i];
			new_length++;
			j++;
			i++;
		}

		if(pipe(pfds) == -1){
			fprintf(stderr, "Pipe failed!\n");
			exit(1);
		}
		
		if((pid = fork()) < 0){
			fprintf(stderr, "Fork failed!\n");
			exit(1);
		}
		else if(pid == 0){
			pipeToStdin(pfds);
			pipeline_cmd(new_input_cmd, new_length, bg);
			exit(0);
		}
		else{
			addJob(pid);
			stdoutToPipe(pfds);
		}
	}
	handleCmd(pid, status, cmd_pieces, curr_cmd, curr_length);
}		

//Handle first call and when shell prompt appears		
void cs350shell(){
	pid_t pid;
	int status;
	int bg = 0;
        char buffer[INPUT_BYTES];
        char* shell_input = buffer;
        signal(SIGCHLD, int_handler);
        while(1){
                printf("cs350sh>");
                fgets(shell_input,INPUT_BYTES, stdin);
                parser(shell_input);
		if(strcmp(cmd_pieces[length-1], "&") == 0){
			bg = 1;
		}
		if((pid = fork()) < 0){
			fprintf(stderr, "Fork failed!");
			exit(1);
		}
		if(pid == 0){
			if(bg == 1) length -= 1;
			pipeline_cmd(cmd_pieces, length, bg);
			exit(0);
		}
		else{
			if(strcmp(cmd_pieces[length-1], "&") != 0){
				waitpid(pid, &status, 0);
			}
			else{
				addJob(pid);
			}
		}
		bg = 0;
        }
}

int main(){
        cs350shell();
        return(0);
}
