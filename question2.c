
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MEMORY 1024

struct proc {
	char name[256];
	int priority;
	pid_t pid;
	int address;
	int memory;
	int runtime;
	bool suspended;
};

struct node {
	struct proc process;
	struct node* next;
};

struct queue {
	struct node* head;
	struct node* tail;
};

void push (struct queue* queue, struct proc process) {
	struct node* newNode = malloc(sizeof(struct node));
	newNode->process = process;
	newNode->next = NULL;

	if (queue->head == NULL) {
		queue->head = newNode;
		queue->tail = newNode;
	} else {
		queue->tail->next = newNode;
		queue->tail = newNode;
	}
}

struct proc* pop (struct queue* queue) {
	if (queue->head == NULL) {
		return NULL;
	}

	struct node* firstNode = queue->head;
	struct proc* process = &(queue->head->process);
	if (queue->head == queue->tail) {
		queue->head = NULL;
		queue->tail = NULL;
	} else {
		queue->head = queue->head->next;
	}
	free(firstNode);

	return process;
}

struct proc createProcess (char name[], int priority, int memory, int runtime) {
	struct proc newProcess;

	strcpy(newProcess.name, name);
	newProcess.priority = priority;
	newProcess.pid = 0;
	newProcess.address = -1;
	newProcess.memory = memory;
	newProcess.runtime = runtime;
	newProcess.suspended = false;

	return newProcess;
}

void freeMemory (int availMem[], int address, int memory) {
	for (int i = address; i < address+memory; i++) {
		availMem[i] = 0;
	}
}

int allocateMemory (int availMem[], int memory) {
	bool allocated = true;

	for (int i = 0; i < MEMORY; i++) {
		if (availMem[i] == 0) {
			for (int j = i; j < i+memory; j++) {
				if (j >= 1024) {
					// exceeded MEMORY, erase changes
					freeMemory(availMem, i, 1024-i);
					return -1; // could not allocate memory
				}

				if (availMem[j] == 0)
					availMem[j] = 1;
				else {
					// memory block occupied by other process, erase changes
					freeMemory(availMem, i, j-i);
					i += j; // allow i to jump to the point the new block was detected
					allocated = false;
					break;
				}
			}
			if (allocated == true)
				return i; // return start of allocated memory block
		}
	}
	return -1; // could not allocate memory
}

int main(void) {
	// variable declaration
	struct queue *primary, *secondary;
	struct proc* process;
	int availMem[MEMORY];
	pid_t pid;

	// initialize memory array
	for (int i = 0; i < MEMORY; i++)
		availMem[i] = 0;

	// initialize queues
	primary = malloc(sizeof(struct queue));
	secondary = malloc(sizeof(struct queue));

	primary->head = NULL;
	secondary->head = NULL;

	primary->tail = NULL;
	secondary->tail = NULL;

	// read in processes from processes_q5.txt
	FILE *file = fopen("processes_q5.txt", "r");
	if (file != NULL) {
		char line[256];
		char args[4][256];

		while (fgets(line, 256, file) != NULL) {
			char delim[] = ", ";
			char* token = strtok(line, delim);

			int i = 0;
			while (token != NULL) { // loop through the line seperate each word to add to args
				strcpy(args[i], token);
				token = strtok(NULL, delim);
				i++;
			}
			// create a new process from each line of the file
			struct proc newProcess = createProcess (args[0], args[1][0]-'0', atoi(args[2]), args[3][0]-'0');

			if (args[1][0]-'0' == 0) {
				push(primary, newProcess); // add to primary queue
			}
			else {
				push(secondary, newProcess); // add to secondary queue
			}
		}
	} else {
		printf("File could not be opened\nClosing program...\n");
		exit(0);
	}
	free(file); // close file

	// execute all processes in the primary queue first
	/*while (primary->head != NULL) {
		process = pop(primary); // get a process from the queue
		process->address = allocateMemory(availMem, process->memory); // allocate memory

		pid = fork();
		if (pid == 0) {
			process->pid = getpid();
			// print some information about the process
			printf("Name: %s 	Priority: %d 	PID: %d 	Memory: %d 	Runtime: %d\n",
					process->name, process->priority, process->pid, process->memory, process->runtime);

			execl(process->name, 0);
			exit(0);
		}
		sleep(process->runtime); // wait for the process to run for specified amount of time
		kill(pid, SIGTSTP); // stop process
		waitpid(pid, NULL, 0); // join process
		freeMemory(availMem, process->address, process->memory);
	}*/

	while (secondary->head != NULL) {
		process = pop(secondary); // get a process from the queue

		if (process->address == -1) { // if process has unallocated memory, try to allocate it
			process->address = allocateMemory(availMem, process->memory); // allocate memory

			if (process->address == -1) {
				// not enough memory available, push back onto the queue
				//printf("Not enough memory to allocate: %s\n", process->name);
				push(secondary, *process);
				continue;
			}
		}

		/*printf("\n------------------------------------\n");
		for (int i = 0; i < MEMORY; i++) {
			printf("%d", availMem[i]);
		}
		printf("\n");*/


		pid = fork();
		if (pid == 0) {
			process->pid = getpid();
			// print some information about the process
			printf("Name: %s 	Priority: %d 	PID: %d 	Memory: %d 	Runtime: %d\n",
					process->name, process->priority, process->pid, process->memory, process->runtime);

			execl(process->name, 0);
			exit(0);
		}
		if (process->suspended == true) {
			kill(pid, SIGCONT);
		}
		sleep(1); // wait for the process to run for 1 second
		if (process->runtime == 1) {
			kill(pid, SIGINT); // terminate process
			process->runtime -= 1;
			freeMemory(availMem, process->address, process->memory); // free available memory for other processes to use
		} else {
			kill(pid, SIGTSTP); // pause process
			process->suspended = true;
			process->runtime -= 1; // decrease runtime
			push(secondary, *process);
		}
		waitpid(pid, NULL, 0); // join process
	}

	free(primary), free(secondary), free(process); // free resources
}