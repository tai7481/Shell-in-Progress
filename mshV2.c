//William Tai
//CS240, Spring 2019
//HW#3
//Shell that uses
//bash syntax for commands
//with additional features

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MaxChar 256
#define MaxCMD 20

//function prototypes
int GetCMD(char *Input);
void WorkingDir();
void Spawn(char **CMD);
void PipeSpawn(char **CMD, char **CMDPipe);
int BuiltIns(char **CMD);
int PipeScan(char *Line, char **LinePipe);
void Parse(char *Line, char **CMD);
int ParseProcess(char *Line, char **CMD, char **LinePipe);
void ReadFile();
void FileParse(char *Buff);

int main()
{
    printf("\033[0;33m");
    char *Input = malloc(sizeof(char) * 256);
    char **CMD = malloc(256 * sizeof(char *));
    char **LinePipe = malloc(256 * sizeof(char *));
    int FlagEXE = 0;

    using_history();	//inits history from readline
    stifle_history(20); //sets max history to 20

    while (1)
    {
	if (GetCMD(Input))
	    continue;
	
	FlagEXE = ParseProcess(Input, CMD, LinePipe);

	if (FlagEXE == 1)
	    Spawn(CMD);
	
	if (FlagEXE == 2)
	    PipeSpawn(CMD, LinePipe);
    }

    free(Input);
    free(CMD);
    free(LinePipe);
    return 0;
}

void FileParse(char *Buff)
{
    char *Line;
    char *SpaceChar = " ";
    char **CMD = malloc(256 * sizeof(char *));
    int i = 0;
    
    Line = strtok(Buff, SpaceChar);

    while (Line != NULL)
    {
	CMD[i] = Line;
	Line = strtok(NULL, SpaceChar);
	i++;
    }

    CMD[i] = NULL;
    Spawn(CMD);
    free(CMD);
}

void ReadFile()
{
    FILE *fp;

    //char *buffer = malloc(sizeof(char) * 256);
    char buffer[10] = {"\0"};
    char ReadCharacter;
    int j;

    fp = fopen("mshrc", "r");	//reads from mshrc

    if (fp == NULL)
    {
	printf("\nError opening file!\n");
	exit(-1);
    }

    while (1)
    {
	//buffer[0] = '\0';
	j = 0;

	while ((ReadCharacter = fgetc(fp)) != 0xa)
	{
	    if (ReadCharacter == EOF)
		return;

	    buffer[j] = ReadCharacter;
	    j++;
	}
	ReadCharacter = '\0';
	buffer[j] = '\0';

	FileParse(buffer);
	//sleep(2);
    }

    fclose(fp);
}

int GetCMD(char *Input)
{
    char *Line = malloc(sizeof(char) * 256);

    Line = readline("\n>?: ");
    
    if (strlen(Line) != 0)
    {
	add_history(Line);
	strcpy(Input, Line);

	return 0;
    }
    else
	return 1;
}

void WorkingDir()
{
    char cwd[256];
    getcwd(cwd, sizeof(cwd));
    printf("\n%s\n", cwd);
}

void Spawn(char **CMD)
{
    pid_t pid;
    int *status;

    pid = fork();

    if (pid == -1)
    {
	printf("Failed forking.\n");

	return;
    }
    else if (pid == 0)	//child code
    {
	if (execvp(CMD[0], CMD) < 0)
	{
	    printf("No such file or directory.\n");
	}

	exit(0);
    }
    else	//parent code
    {
	pid = wait(status);
    }

    return;
}

void PipeSpawn(char **CMD, char ** CMDPipe)
{
    int Pipefd[2];	//0 read, 1 write
    pid_t Pipe1, Pipe2;
    int *status1, *status2;

    if (pipe(Pipefd) < 0)
    {
	printf("Could not pipe...\n");
	
	return;
    }

    Pipe1 = fork();

    if (Pipe1 < 0)
    {
	printf("Could not fork...\n");

	return;
    }

    if (Pipe1 == 0)	//child code
    {
	close(Pipefd[0]);
	dup2(Pipefd[1], STDOUT_FILENO);
	close(Pipefd[1]);

	if (execvp(CMD[0], CMD) < 0)
	{
	    printf("Command not found...\n");
	    exit(-1);
	}
    }
    else	//parent code
    {
	Pipe2 = fork();

	if (Pipe2 < 0)
	{
	    printf("\nCould not fork!\n");
	    return;
	}
	
	if (Pipe2 == 0)	//second child code
	{
	    close(Pipefd[0]);
	    dup2(Pipefd[1], STDIN_FILENO);
	    close(Pipefd[1]);

	    if (execvp(CMDPipe[0], CMDPipe) < 0)
	    {
		printf("\nNo such file or directory\n");
		exit(0);
	    }
	}
	else	//parent again
	{
	    Pipe1 = wait(status1);
	    Pipe2 = wait(status2);
	}
    }
}

int BuiltIns(char **CMD)
{
    int CMDNum = 9;
    int i;
    int SwitchCMD = 0;
    char *BuiltInList[CMDNum];
    
    HISTORY_STATE *Hist = history_get_history_state();

    HIST_ENTRY **HList = history_list();

    BuiltInList[0] = "exit";
    BuiltInList[1] = "cd";
    BuiltInList[2] = "red";
    BuiltInList[3] = "blue";
    BuiltInList[4] = "green";
    BuiltInList[5] = "default";
    BuiltInList[6] = "history";
    BuiltInList[7] = "read";
    BuiltInList[8] = "path";
    
    for (i = 0; i < CMDNum; i++)
    {
	if ((strcmp(CMD[0], BuiltInList[i])) == 0)
	{
	    SwitchCMD = i + 1;
	    break;
	}
    }

    switch (SwitchCMD)
    {
	case 1:
	    exit(1);
	case 2:
	    chdir(CMD[1]);
	    return 1;
	case 3:
	    printf("\033[0;31m");
	    return 1;
	case 4:
	    printf("\033[0;34m");
	    return 1;
	case 5:
	    printf("\033[0;32m");
	    return 1;
	case 6:
	    printf("\033[0;33m");
	    return 1;
	case 7:
	    for (i = 0; i < Hist -> length; i++)
	    {
		printf("\n%d  %s", i + 1, HList[i] -> line);
	    }
	    return 1;
	case 8:
	    ReadFile();
	    return 1;
	case 9:
	    WorkingDir();
	    return 1;
	default:
	    break;
    }

    return 0;
}

int PipeScan(char *Line, char **LinePipe)
{
    int i;

    for (i = 0; i < 6; i++)
    {
	LinePipe[i] = strsep(&Line, "|");
	
	if (LinePipe[i] == NULL)
	    break;
    }

    if (LinePipe[1] == NULL)
	return 0;		//no pipe found
    else
	return 1;
}

void Parse(char *Line, char **CMD)
{
    int i;

    for (i = 0; i < MaxCMD; i++)
    {
	CMD[i] = strsep(&Line, " ");
	
	if (CMD[i] == NULL)
	    break;
	if (strlen(CMD[i]) == 0)
	    i--;
    }
    //printf("Finished parse\n");
    //printf("%s\n", CMD[0]);
}

int ParseProcess(char *Line, char **CMD, char **LinePipe)
{
    char *CMDPipe[2];
    int Piped = 0;

    Piped = PipeScan(Line, CMDPipe);

    if (Piped)
    {
	Parse(CMDPipe[0], CMD);
	Parse(CMDPipe[1], LinePipe);
    }
    else
	Parse(Line, CMD);

    printf("\n%s\n", CMD[0]);
    
    if (BuiltIns(CMD))
	return 0;
    else
	return (Piped + 1);
}
