#ifndef SHELL_PROGRAMS_H
#define SHELL_PROGRAMS_H

void stdoutToPipe(int pfds[2]);
void pipeToStdin(int pfds[2]);
void fileToStdin(char* filename);
void addJob(pid_t pid);
void printJobs();
int findJob(int pid);
void completeJob(int pid);
void foreground(pid_t pid, int status, char* cmd_pieces[]);
void functCall(char* args[],int length);
void handleCmd(pid_t pid, int status, char* cmd_pieces[50], char* curr_cmd[50], int curr_length);

#endif
