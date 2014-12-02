
/*
 * assn_4.c
 *
 *  Created on: Nov 23, 2014
 *      Author: nkatre
 */

#include<stddef.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<sys/time.h>

#pragma pack(1)

typedef struct{           /* B tree node */
	int n;				  /* Number of keys in the node */
	int *key;             /* Node's keys */
	long *child;          /* Node's child subtree offsets */
	long p;               /*Parent of the node. Parent is the last node while reading and writing */
}btree_node;


// Variable Declaration
int order = 4;
FILE *indexFP=NULL; // index file pointer
long rootOffset = -1; // Offset of root node
//btree_node *rootNode = NULL;  // initialize the rootNode

// Functions  for each operation
void addFunction(char *line);
void findFunction(char *line);
void endFunction(char *line);
void printFunction(char *line);

// Helper Functions
int filter(char *line);
int findMiddleIndex(int num, int den);
int isFull(btree_node *node);
int cmpfunc (const void * a, const void * b);



// Operational Functions
void writeNode(btree_node *node, long offset);
btree_node* readNode(long offset);
btree_node* createNode();
btree_node* updateChildOffsets(btree_node *node, int targetIndex, long leftOff, long rightOff);
void insert(int k, long offset, int temp, int temp2, long leftOff, long rightOff);
void queueOperations();

typedef struct{
	int notUsed;
	int neverUsed;
	int foundIndicator;
	long targetOff;
}searchResult;





searchResult formResult(int notUsed, int neverUsed, int foundIndicator, long targetOff);


searchResult formResult(int notUsed, int neverUsed, int foundIndicator, long targetOff){
	searchResult result;
	result.notUsed=notUsed;
	result.neverUsed = neverUsed;
	result.foundIndicator=foundIndicator;
	result.targetOff=targetOff;
	return result;
}


searchResult search(long offset,int k,int m,int n);


btree_node* createNode(){
	btree_node *node = (btree_node *)malloc(sizeof(btree_node));
				node->n = 0;
				node->key = (int *)calloc(order,sizeof(int));
				node->child = (long *)calloc(order+1,sizeof(long));
				node->p=-1;

	int i=0;
	while(i<order+1){
		node->child[i]=-1;
		i++;
	}
	return node;
}
btree_node* readNode(long offset){
	if(offset==-1)
		return NULL;

	// create the node
	btree_node *node = (btree_node *)malloc(sizeof(btree_node));
					node->n = 0;
					node->key = (int *)calloc(order,sizeof(int));
					node->child = (long *)calloc(order+1,sizeof(long));
					node->p=-1;

		int i=0;
		while(i<=order){
			node->child[i]=-1;
			i++;
		}

	// end of create node

	fseek(indexFP,offset,SEEK_SET);
	fread(&node->n,sizeof(int),1,indexFP);
	fread(node->key,sizeof(int),order-1,indexFP);
	fread(node->child,sizeof(long),order,indexFP);
	fread( &node->p, sizeof( long ), 1, indexFP);
	return node;
}
void writeNode(btree_node *node, long offset){
	fseek(indexFP,offset,SEEK_SET);
	fwrite(&node->n,sizeof(int),1,indexFP);
	fwrite(node->key,sizeof(int),order-1,indexFP);
	fwrite(node->child,sizeof(long),order,indexFP);
    fwrite( &node->p, sizeof( long ), 1, indexFP);
}


int filter(char *line){
	char *command=NULL;
	command = strtok(line," "); // get command from the line
	int key = 0;
	key=atoi(strtok(NULL,"\n"));
	return key;
}
int findMiddleIndex(int num, int den){
	return ((num+den-1)/den);
}
int isFull(btree_node *node){
	if(node->n >= (order))
		return 1;
	else
		return 0;
}
int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}


int main(int argc, char *argv[]){
	char *line=NULL;
	size_t line_size = 0;
  //  printf("%s\n",argv[1]);

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

	//printf("%d\n",order);
// types of lines
//	add k, find k, print, end
	while(getline(&line, &line_size, stdin))
	{
        if(strncmp(line,"end",3)==0){
        	endFunction(line);
        	break;
        }
        if(strncmp(line,"find",4)==0){
        	findFunction(line);
        }
        if(strncmp(line,"print",5)==0){
        	printFunction(line);
        }
        if(strncmp(line,"add",3)==0){
        	addFunction(line);
        }
    }
	 //garbage collection
	 free(line);

}
void findFunction(char *line){
	//printf("%s",line);
	int k = filter(line);
	searchResult result = search(rootOffset,k,0,0);
	if(result.foundIndicator==1)
	        printf("Entry with key=%d exists\n", k);
	else
	        printf("Entry with key=%d does not exist\n", k);
}
void printFunction(char *line){
	//int k=filter(line);
	queueOperations();
}
void endFunction(char *line){
	//printf("%s",line);
	fseek(indexFP,0,SEEK_SET);
	fwrite(&rootOffset,sizeof(long),1,indexFP);
	fclose(indexFP);
	indexFP=NULL;
}
void addFunction(char *line){
	//printf("%s",line);
	int k = filter(line);
	//
	searchResult result = search(rootOffset,k,0,0);
	if(result.foundIndicator==1){
	printf("Entry with key=%d already exists\n", k);
		        return;
	}
	insert(k,result.targetOff,0,0,-1,-1);
}

void insert(int k, long offset, int temp, int temp2, long leftOff, long rightOff){

	int i=0;
	btree_node *node=readNode(offset);
	long nextOffset=0;

	if(node==NULL){      // create the root node and write to file   TODO: move this to the addFunction for condition (offset == -1)
		node=createNode();
		node->key[node->n]=k;  // enter the key
		node->n=node->n+1;  // number of nodes is 1
		node->child[0]=leftOff;         // set the left offset
		node->child[1]=rightOff;         // set the right offset
		fseek(indexFP,0,SEEK_END);     // go to the end of the file
		rootOffset = ftell(indexFP);    // get the offset to the end of file
		writeNode(node,rootOffset);     // write the root node

		// write the parent of this node to the disk
		i=0;
		while(i<node->n+1){   // loop till the order of the node
			if(node->child[i]!=-1){
				fseek(indexFP,node->child[i],SEEK_SET);
				fseek(indexFP,sizeof(int),SEEK_CUR);  // move by node.n
				fseek(indexFP,sizeof(int)*(order-1),SEEK_CUR);  // move by node.key
				fseek(indexFP,sizeof(long)*(order),SEEK_CUR);    // move by node.child
				fwrite(&rootOffset,sizeof(long),1,indexFP);

			}
			i++;        // increment i for the while loop
		}
        // end of writing the parent node


		//rewind(indexFP);             // go to the start of the file
		fseek(indexFP,0,SEEK_SET);   // go to the start of the file
		fwrite(&rootOffset,sizeof(long),1,indexFP);  // write the root offset to the file

		free(node->key);
		free(node->child);
		free(node);
		return;   // return from the function

	}


	    if(node!=NULL){

	    	// get the index of the key where the new key would be added
	    	i=0;
	    	while(i<node->n){
	    		if(k<node->key[i]){
	    			break;
	    		}
	    		i++;
	    	}
	    	int targetIndex = i;


	    	// add the key to the node.keys

	    	node->key[node->n]=k;
	    	qsort(node->key,node->n+1,sizeof(int),cmpfunc);


	    	// updating the child index positions relative to the added key
	    	node=updateChildOffsets(node,targetIndex,leftOff,rightOff);
	    	// increment the key counter
	    	node->n=node->n+1;


	    	if(!isFull(node)){       // node->n < order
		    		writeNode(node,offset);
		    		// write the parent of this node to the disk
		    		i=0;
		    		while(i<(node->n + 1)){   // loop till the order of the node
		    			if(node->child[i]!=-1){
		    				fseek(indexFP,node->child[i],SEEK_SET);
		    				fseek(indexFP,sizeof(int),SEEK_CUR);  // move by node.n
		    				fseek(indexFP,sizeof(int)*(order-1),SEEK_CUR);  // move by node.key
		    				fseek(indexFP,sizeof(long)*(order),SEEK_CUR);    // move by node.child
		    				fwrite(&offset,sizeof(long),1,indexFP);    // write the offset
		    			}
		    			i++;
		    		}
		            // end of writing the parent node

		    		free(node->key);
		    		free(node->child);
		    		free(node);
		    		return;  // return from the current function
		    	}


	    	if(isFull(node)){
	    		int midIndex=0;
	    		int midKey=0;
		    	midIndex = findMiddleIndex(node->n-1,2);
		    	midKey = node->key[midIndex];


		    	btree_node *right = createNode();

		    	int m = midIndex+1;
		    	int n=0;
		    	while(m<node->n){

		    		right->key[n]=node->key[m];
		    		right->child[n]=node->child[m];
		    		right->n=right->n+1;

		    		m++;
		    		n++;
		    	}

		    	right->child[n]=node->child[m];// copy the last children

		    	node->n=midIndex;// update the node
		    	leftOff=offset;    // the current offset becomes left child offset
		    	writeNode(node,leftOff);



		    	// update the parent
		    	i=0;
		    	while(i<(node->n + 1)){
		    				if(node->child[i]!=-1){
		    					fseek(indexFP,node->child[i],SEEK_SET);
		    					fseek(indexFP,sizeof(int),SEEK_CUR);  // move by node.n
		    					fseek(indexFP,sizeof(int)*(order-1),SEEK_CUR);  // move by node.key
		    					fseek(indexFP,sizeof(long)*(order),SEEK_CUR);    // move by node.child
		    					fwrite(&leftOff,sizeof(long),1,indexFP);
		    				}
		    				i++;
		    			}
		    	        // end of writing the parent node

		    	nextOffset = node->p;

		    	free(node->key);       //:doubtful
		    	free(node->child);
		    	free(node);


		    	fseek(indexFP,0,SEEK_END);
		    	rightOff = ftell(indexFP);
		    	writeNode(right,rightOff);

		    	// update the parent
		    	i=0;
		    	while(i<(right->n + 1)){
		    				if(right->child[i]!=-1){
		    					fseek(indexFP,right->child[i],SEEK_SET);
		    					fseek(indexFP,sizeof(int),SEEK_CUR);  // move by node.n
		    					fseek(indexFP,sizeof(int)*(order-1),SEEK_CUR);  // move by node.key
		    					fseek(indexFP,sizeof(long)*(order),SEEK_CUR);    // move by node.child
		    					fwrite(&rightOff,sizeof(long),1,indexFP);
		    				}
		    				i++;
		    			}
		    	        // end of writing the parent node
		    	free(right->key);
		    	free(right->child);
		    	free(right);

		    	insert(midKey,nextOffset,0,0,leftOff,rightOff);
	            }


	    }


}


// start of queue


int queueFront=-1;
int queueRear=-1;
void initializeQueue(){
	queueFront=-1;
	queueRear=-1;
}
int isQueueEmpty(){
	return (queueFront==queueRear);
}
void enqueue(long *queue,long offset){
	queue[++queueRear]=offset;
}
long dequeue(long *queue){
	if(!isQueueEmpty()){
		return (queue[++queueFront]);
	}
	else{
		return -1;
	}
}
int level = 1;
int newLevel=1;
void printNode(btree_node *node){
	int i=0;
	printf(" ");
	while(i<node->n-1){
		printf( "%d,", node->key[i] );
		i++;
	}
	printf( "%d", node->key[node->n - 1] );
}
void queueOperations(){
	long *queue = (long *)malloc(sizeof(long)*500);
	enqueue(queue,rootOffset);
	int thisLevelChildrenToBeVisited=1;
	int nextLevelChildrenToBeVisited=0;
	long nodeOffset = rootOffset;
    while(!isQueueEmpty()){

    	 nodeOffset=dequeue(queue);
    	 thisLevelChildrenToBeVisited--;
    	 btree_node *node=readNode(nodeOffset);


    	 if(newLevel){
    		 if(level!=1){
    		 printf(" \n");
    		 }
    		 printf(" %d:",level);
    		 newLevel=0;

    	 }

    	 printNode(node);


    	 int i=0;
    	 while(i<=node->n){
    		 if(node->child[i]!=-1){
    			 enqueue(queue,node->child[i]);
        		 nextLevelChildrenToBeVisited++;
    		 }
    		 i++;
    	 }

    	 if(thisLevelChildrenToBeVisited==0){
    		 newLevel=1;
    		 int temp = nextLevelChildrenToBeVisited;
    		 nextLevelChildrenToBeVisited=0;
    		 thisLevelChildrenToBeVisited = temp;  // thisLevelChildren becomes nextLevelChildren and nextLevelChildren becomes 0
    		 level++;
    	 }
    	 free(node->key);
    	 free(node->child);
    	 free(node);
    }
    printf(" \n");
    level = 1;
    newLevel=1;
	free(queue);
}


// end of queue




btree_node* updateChildOffsets(btree_node *node, int targetIndex, long leftOff, long rightOff){
	node->child[targetIndex]=leftOff;      // left child
	int x=node->n;
	while(x>targetIndex){
		node->child[x+1] = node->child[x];
		x--;
	}
	node->child[x+1]=rightOff;    // right child
	return node;
}
searchResult search(long offset,int k,int m,int n){


	if(offset == -1){
		searchResult result = formResult(0,0,0,offset);
		return result;
	}
	btree_node *node = readNode(offset);
	long targetNodeOffset = 0;

	int i=0;
	int j=0;

	while(i>=0 && i<node->n){
		if(i>=0 && k==node->key[i]){
			searchResult result = formResult(0,0,1,offset);
			free(node->key);
			free(node->child);
			free(node);
			return result;
		}
		i++;
	}
	i=0;  // i=0
	while(j>=0 && j<node->n){
		if(j>=0 && node->key[j]>k){
			targetNodeOffset=node->child[j];
			break;
		}
		j++;
	}

	//
	if(m){
		printf("\n");
	}
	if(n){
		printf("\n");
	}
	//
	if(targetNodeOffset==0){
			targetNodeOffset=node->child[j];
		}
		//
	int c=0;
		if(c){
			printf("\n");
		}
		int u=0;
		if(u){
			printf("\n");
		}
	if(targetNodeOffset==-1){
		searchResult result = formResult(0,0,0,offset);
		free(node->key);
		free(node->child);
		free(node);
		return result;
	}
	//
	int b=0;
	if(b){
		printf("\n");
	}
	int l=0;
	if(l){
		printf("\n");
	}
    //

	//
	free(node->key);  //: change
	free(node->child);
	free(node);
	return search(targetNodeOffset,k,0,0);

}



