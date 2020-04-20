#include <stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<unistd.h>
#include<stdlib.h>
#include<iostream>
#include<string>
#include<cstring>
#include<readline/readline.h>
#include<readline/history.h>

using namespace std;

int main() {
	int i = -1; //Number of Processes
	bool ops = true; //to run while loop
	char* in = NULL; //to take input command from user
	int status;
	pid_t proc[128]; //array to store processes
	int stat[128]; // array to store status of process id 1:running; 3:stopped; 0:terminated 
	pid_t endID[128]; // array to store status returned from waitpid() of all processes 
	int count = 0; //to maintain count of running processes 
	pid_t pid;
	char* proc_names[128]; //store process names
	while (ops)
	{
		for (int j = 0; j <= i; j++) 
		{
			endID[j] = waitpid(proc[j], &status, WNOHANG); //check change in any of the child processes immediately without waiting using WNOHANG 
			if (endID[j] > 0 && stat[j] != 0) 
			{
				stat[j] = 0;
				cout << proc[j] << " completed" << endl; //to check if any process has been completed 
				count--;
				for (int k = 0; k <= i; k++) 
				{  // as soon as a child process is completed automatically restart a stopped child process if present
					if (count < 3 && stat[k] == 3) 
					{
						stat[k] = 1;
						kill(proc[k], SIGCONT); //automatically starting a stopped process 
						count++;
						cout << proc[k] << " automatically restarted" << endl;
					}
				}
			}
		}
		in = readline("BP> ");
		
		while (strcmp(in,"")==0){
			in = readline("BP> ");
		}
		if (strcmp(in, "exit") == 0)
		{
			for (int j = 0; j <= i; j++)
			{
				if (stat[j] != 0) 
				{
					kill(proc[j], SIGTERM); //if command is exit then kill all non-terminated processes i.e status should not be 0(terminated)
					cout << proc[j] << " killed" << endl;
				}
			}
			ops = false; //end loop
			break;
		}
		char* p = strtok(in, " \t");
		if (strcmp(p, "bg") == 0)
		{
			i++; //increasing array index 
			p = strtok(NULL, " \t");
			proc_names[i] = p;
			char child[] = "./";
			strcat(child, proc_names[i]);
			char* arg_array[] = { child,NULL,NULL,NULL,NULL }; //array to be sent as parameter during execvp()
			p = strtok(NULL, " \t");
			arg_array[1] = p;
			p = strtok(NULL, " \t");
			arg_array[2] = p;
			p = strtok(NULL, " \t");
			arg_array[3] = p;           //storing parameters of input in array 
			pid = fork(); //create a process 
			if (pid > 0) //parent process 
			{
				proc[i] = pid;
				if (count < 3) { //if less than 3 process running increment count and continue normally 
					count++;
					stat[i] = 1; //set status to running 
				}
				else {
					kill(pid, SIGSTOP); //if 3 processes already running then stop 
					stat[i] = 3; //set status to stopped
				}
			}
			else if (pid == 0) //child process 
			{
				if (execvp(arg_array[0], arg_array) < 0) //execute execvp()
				{
					cout << "Fail" << endl;
				}
			}
			else
			{
				cout << "fork failed" << endl;
				exit(EXIT_FAILURE);
			}
		}
		else if (strcmp(p, "bglist") == 0)
		{
			int j = 0;
			for (j = 0; j <= i; j++) //if command is bglist display processes and their status 
			{
				if (stat[j] == 1)
				{
					cout << proc[j] << " : "<<proc_names[j]<<" (running)" << endl;
				}
				else if (stat[j] == 0)
				{
					cout << proc[j] << " : " << proc_names[j] << " (terminated)" << endl;
				}
				else if (stat[j] == 3)
				{
					cout << proc[j] << " : " << proc_names[j] << " (stopped)" << endl;
				}
			}
		}
		else if (strcmp(p, "bgstop") == 0)
		{
			p = strtok(NULL, " \t");
			bool found = false; //to check if process exist 
			for (int j = 0; j <= i; j++)
			{
				if (proc[j] == atoi(p))
				{
					if (stat[j] == 3) 
					{
						cout << proc[j] << " already stopped" << endl; //if already stopped then display message and break 
						found = true;
						break;
					}
					else if (stat[j] == 0)
					{
						cout << proc[j] << " already terminated" << endl; //if already stopped then display message and break 
						found = true;
						break;
					}
					else
					{
						stat[j] = 3;
						kill(atoi(p), SIGSTOP);
						cout << p << " STOPPED" << endl; // stop process and display message 
						found = true;
						count--;
						for (int j = 0; j <= i; j++) 
						{
							if (proc[j] != atoi(p) && count < 3 && stat[j] == 3) //search for stopped process and automatically restart
							{
								stat[j] = 1;
								count++;
								cout << proc[j] << " automatically restarted" << endl;
								kill(proc[j], SIGCONT);
							}
						}
					
					}

				}
			}
			
			if(found==false) {
				cout << atoi(p) << " does not exist" << endl; //if process not found
			}
		}
		else if (strcmp(p, "bgkill") == 0)
		{
			p = strtok(NULL, " \t");
			bool found = false; //to check if such process exists 
			for (int j = 0; j <= i; j++)
			{
				if (proc[j] == atoi(p))
				{
					if (stat[j] == 0) {
						cout << proc[j] << " already terminated" << endl; //if process already killed 
						found = true;
						break;
					}
					else 
					{
						stat[j] = 0;
						kill(atoi(p), SIGTERM); //kill process 
						cout << p << " killed" << endl;
						found = true;
						count--;
						for (int j = 0; j <= i; j++)
						{
							if (proc[j] != atoi(p) && count < 3 && stat[j] == 3) 
							{
								stat[j] = 1;
								count++;
								cout << proc[j] << " automatically restarted" << endl; //automatically start a stopped process 
								kill(proc[j], SIGCONT); 
							}
						}
						
					}

				}
			}
			if (found==false) {
				cout << atoi(p) << " does not exist" << endl; //if such process does not exist
			}
			
		}
	}
}

