
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
	int pid;
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
	struct node* newNode = malloc(sizeof(newNode));
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

	struct node* nextNode = queue->head->next;
	struct proc* process = &queue->head->process;
	if (queue->head == queue->tail)
		queue->tail = NULL;
	queue->head = NULL;
	free(queue->head);
	queue->head = nextNode;

	return process;
}

struct proc createProcess (char name[], int priority, int memory, int runtime) {
	struct proc newProcess;

	strcpy(newProcess.name, name);
	newProcess.priority = priority;
	newProcess.pid = 0;
	newProcess.address = 0;
	newProcess.memory = memory;
	newProcess.runtime = runtime;
	newProcess.suspended = false;

	return newProcess;
}

int main(void) {
	// variable declaration
	struct queue *primary, *secondary;
	int availMem[MEMORY];

	// initialize queues
	primary = malloc(sizeof(primary));
	secondary = malloc(sizeof(secondary));

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

			struct proc newProcess = createProcess (args[0], args[1][0]-'0', atoi(args[2]), args[3][0]-'0');

			if (args[1][0]-'0' == 0) {
				push(primary, newProcess);
				printf("%s\n", primary->tail->process.name);
			}
			else {
				push(secondary, newProcess);
				printf("%s\n", secondary->tail->process.name);
			}
		}
	} else {
		printf("File could not be opened\nClosing program...\n");
		exit(0);
	}
	free(file);

	printf("%s\n", pop(primary)->name);
	printf("%s\n", pop(primary)->name);

	free(primary), free(secondary);
}