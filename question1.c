#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define CHAR_LENGTH 256

typedef struct {
	char parent[CHAR_LENGTH];
	char name[CHAR_LENGTH];
	int priority;
	int memory;
}proc;

typedef struct node{
	proc * process;
	struct node * left;
	struct node * right;
} node_t;

void insert(node_t * tree, proc * process)
{
	if(tree->process == NULL){
		tree->process = process;
	}else if(strcmp(process->parent,tree->process->name) == 0){
		if(tree->left == NULL){
			tree->left = malloc(sizeof(node_t));
			tree->left->process = process;
		} else if(tree->right == NULL){
			tree->right = malloc(sizeof(node_t));
			tree->right->process = process;
		} else {
			fprintf(stderr, "Unable to add process to tree: Parent process already has 2 children.");
		}
	} else {
		if(tree->left != NULL){
			insert(tree->left, process);
		}
		if(tree->right != NULL){
			insert(tree->right, process);
		}
	}
}

void print_tree(node_t * tree){
	if(tree != NULL){
		printf("%s ", tree->process->name);
		if(tree->left != NULL){
			printf("L:%s ", tree->left->process->name);
		}
		if(tree->right != NULL){
			printf("R:%s", tree->right->process->name);
		} 
		printf("\n");
		if(tree->left != NULL){
			print_tree(tree->left);
		}
		if(tree->right != NULL){
			print_tree(tree->right);
		}
	}
}

int main(void)
{
	node_t * process_tree = malloc(sizeof(node_t));
	char filename[] = "processes_tree.txt";
	FILE *in_file = fopen(filename, "r");
	if(in_file == NULL){
		fprintf(stderr, "Unable to open file '%s'.\n", filename);
		return 1;
	}

	char buffer[CHAR_LENGTH];
	char token[] = ", ";
	while(fgets(buffer, CHAR_LENGTH, in_file) != NULL)
	{
		char *parent = strtok(buffer, token);
		char *name = strtok(NULL, token);
		char *priority = strtok(NULL, token);
		char *memory = strtok(NULL, "\n");
		
		proc *new_process = malloc(sizeof(proc));
		strcpy(new_process->parent, parent);
		strcpy(new_process->name, name);
		new_process->priority = atoi(priority);
		new_process->memory = atoi(memory);
		
		insert(process_tree, new_process);
	}
	fclose(in_file);
	
	print_tree(process_tree);
	free(process_tree);
}