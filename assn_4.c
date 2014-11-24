/*
 * assn_4.c
 *
 *  Created on: Nov 8, 2014
 *      Author: nkatre
 */

#include<stddef.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<sys/time.h>

#pragma pack(1)

int order = 4;
FILE *indexFP=NULL; // index file pointer
long rootOffset = -1; // Offset of root node

typedef struct{           /* B tree node */
	int n;				  /* Number of keys in the node */
	int *key;             /* Node's keys */
	long *child;          /* Node's child subtree offsets */
}btree_node;

// Functions  for each operation
void addFunction(char *line);
void findFunction(char *line);
void endFunction(char *line);
void printFunction(char *line);

// Helper Functions
int filter(char *line);
int ceil(int num, int den);
int isFull(btree_node *node);
int cmpfunc (const void * a, const void * b);


// Operational Functions
int * copyKeys(int const * src, size_t len,int key);
btree_node* addKey(btree_node *node, int key);


int ceil(int num, int den){
	return ((num+den-1)/den);
}
int * copyKeys(int const * L, size_t len,int key)
{
   int *K = malloc((len+1)* sizeof(int));  // +1 to store the added key
   memcpy(K, L, len * sizeof(int));
   K[len]=key;
   return K;
}
int isFull(btree_node *node){
	if(node->n <= (order-1))
		return 1;
	else
		return 0;
}
int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}
btree_node* addKey(btree_node *node, int key){
	if(node==NULL){
			node = (btree_node *)malloc(sizeof(btree_node));
			node.n = 0;
			node.key = (int *)calloc(order-1,sizeof(int));
			node.child = (long *)calloc(order,sizeof(long));
		}
	node->key[node->n]=key;
	node->n++;
	qsort(node->key,node->n,sizeof(int),cmpfunc);  // sort the keys if the node
	return node;
}





int main(int argc, char *argv[]){
	char *line=NULL;
	size_t line_size = 0;
    printf("%s\n",argv[1]);

	if((indexFP=fopen(argv[1], "r+b"))==NULL)//check whether index file exists. If not then create it
	{
		rootOffset=-1;
		indexFP = fopen( argv[1], "w+b" );
		fwrite(&rootOffset,sizeof(long),1,indexFP);
	}
	else{
		// get the offset of the root node
		fread(&rootOffset,sizeof(long),1,indexFP);
	}

	// get the order of the b-tree
	order=atoi(argv[2]);

	printf("%d\n",order);
// types of lines
//	add k, find k, print, end
	while(getline(&line, &line_size, stdin))
	{
        if(strncmp(line,"end",3)==0){
        	// function call
        	endFunction(line);
        	break;
        }
        if(strncmp(line,"find",4)==0){
        	// function call
        	findFunction(line);
        }
        if(strncmp(line,"print",5)==0){
        	//function call
        	printFunction(line);
        }
        if(strncmp(line,"add",3)==0){
        	//function call
        	addFunction(line);
        }
    }
	 //garbage collection
	 free(line);
	 fclose(indexFP);
	 indexFP=NULL;
}
void findFunction(char *line){
	printf("%s",line);
}
void printFunction(char *line){
	printf("%s",line);
}
void endFunction(char *line){
	printf("%s",line);
}
int filter(char *line){
	char *command=NULL;
	command = strtok(line," "); // get command from the line
	int nodeID = 0;
	nodeID=atoi(strtok(NULL,"\n"));
	return nodeID;
}

void printNode(btree_node *node){
	printf("%i\n",node.n);
}
void addFunction(char *line){
	printf("%s",line);
	int nodeID = filter(line);
	if(rootOffset==-1){
		btree_node *node=NULL;
		node=addKey(node,nodeID);
	}
	else{
		if(isFull(rootOffset)){

		}
	}
	//printNode(node);

}

