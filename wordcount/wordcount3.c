#include<stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include<string.h>
#define MAX_VALIDWORD_LENGTH 20

typedef struct validWordList {
	char word[MAX_VALIDWORD_LENGTH];
	int occurences;
	struct validWordList *next;
} validWords;

validWords * addWordToList(char *wstart,char *wend,validWords *list);
validWords * merge(validWords *a, validWords *b);
validWords * mergeSort(validWords *list);
void split(validWords *list, validWords **a, validWords **b);
void printList(validWords *list);
int main(int argc, char **argv){

	int fd;
	char *fname;
	struct stat finfo;
	char *fdata;
	char *fstart;
	char *fend;
	char *fcurrent;	
	validWords *list=NULL;
	if(argc<=2){
		if(argc==2){
			printf("Third argument is missing\n");
			exit(-1);
		} else {
			printf("File location not specified\n");
			exit(-1);
		}
	}
	
	//reading file name from command line argument
	fname=argv[1];

	//Open system call will open the file for you. It returns the file descripter
	fd=open(fname, O_RDONLY);
	if(fd<0){
		perror("Opening the file failed");
		exit(-1);
	}

	//initializing fstats with the statistics of the file to retrieve the file size
	if(fstat(fd, &finfo) < 0){
		perror("couldn't fetch the stats of the file");
		exit(-1);
	}
	
	//creating a private memory of size equal to size of the file and mapping it to the address space of this process
	fdata = (char *) mmap(0, finfo.st_size + 1, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
	if(fdata==MAP_FAILED){
		perror("couldn't map a memory of file content with process address space");
		exit(-1);
	}
	
	fstart=fdata;
	fend=fdata+finfo.st_size;

	fcurrent=fstart;
	
	//Reaching to the first valid word
	while((toupper(*fcurrent)<'A' || toupper(*fcurrent)>'Z') && *fcurrent!='-' && fcurrent<fend){
		fcurrent++;
	}
	char *wordStartChar=fcurrent;
	char *wordEndChar=NULL;
	int isGoodWord=1;
//	int count=0;
	char c;
	while(fcurrent<fend){
		c = toupper(*fcurrent);
		if((c<'A' || c>'Z') && c!='-' && c!=' ' && c!='\n'){
			isGoodWord=0;
		}else if(c==' ' || c=='\n'){
			if(isGoodWord){
				//add semaphore here
				list = addWordToList(wordStartChar, wordEndChar, list);
				//leaveSemaphore lock here
//				count++;		
			}
			while((c<'A' || c>'Z') && c!='-' && fcurrent<fend){
				fcurrent++;
				c = toupper(*fcurrent);
			}
			if(fcurrent>=fend){
				break;
			}else {
				wordStartChar=fcurrent;
			        wordEndChar=NULL;
			        isGoodWord=1;	
			}		
		}
		wordEndChar=fcurrent;		
		fcurrent++;
	}
//	printList(list);
	list=mergeSort(list);
//	printf("%d\n",count);
	while(list!=NULL){
		printf("%s\t%d\n",list->word,list->occurences);
		list=list->next;
	}
	
}

validWords * addWordToList(char *wstart, char *wend, validWords *list){
	char *iterator=wstart;
	char validWord[MAX_VALIDWORD_LENGTH];
	int i=0;
	while(iterator<=wend && i<MAX_VALIDWORD_LENGTH){
		//printf("%c",*iterator);
		validWord[i]=toupper(*iterator);
		iterator++;
		i++;
	}
	//printf("\n");
	validWord[i]='\0';
	if(list==NULL){
		list=malloc(sizeof(validWords));
		strcpy(list->word,validWord);
		list->occurences=1;
		list->next=NULL;
	}else{
		validWords *previous, *current=list;
		do{
			if(strcmp(current->word,validWord)==0){
				current->occurences++;		
				break;
			}
			previous=current;
			current=current->next;
		}while(current!=NULL);
		if(current==NULL){
			current=malloc(sizeof(validWords));
			strcpy(current->word,validWord);
			current->occurences=1;
			current->next=NULL;
			previous->next=current;
		}
	}
	return list;
}

validWords * merge(validWords *a, validWords *b){
//	printf("merge starts here =========================================== \n");
//	printf("printing A first\n");
//	printList(a);
//	printf("printing B Second\n");
//	printList(b);
	validWords *result=NULL;
	if(a==NULL){
		return b;
	}else if(b==NULL){
		return a;
	}
	if(a->occurences>b->occurences){
		result=a;
		a=a->next;
	}else{
		result=b;
		b=b->next;
	}
	validWords *current=result;
	while(a!=NULL && b!=NULL){
		if(a->occurences>b->occurences){
			current->next=a;
			a=a->next;
			current=current->next;
		}else{
			current->next=b;
                        b=b->next;
                        current=current->next;
		}
	}
	if(a==NULL)
		current->next=b;
	else
		current->next=a;
//	printf("after merging the result is:\n");
//	printList(result);
	return result;
	
} 

validWords * mergeSort(validWords *list){
	if(list->next==NULL)
		return list;
	validWords *a=NULL, *b=NULL;
	split(list,&a,&b);
//	printList(a);
//	printList(b);
	a = mergeSort(a);
	b = mergeSort(b);
	return merge(a,b);
}

void split(validWords *list, validWords **a, validWords **b){
	validWords *fast;
	validWords *slow;
	if(list==NULL || list->next==NULL){
		*a=list;
		*b=NULL;
	}else{
		slow=list;
		fast=list->next;
		while(fast!=NULL){
			fast=fast->next;
			if (fast != NULL){
			        slow = slow->next;
			        fast = fast->next;
      			}
		}
		*a=list;
		*b=slow->next;
		slow->next=NULL;
	}
}

void printList(validWords *list){
	while(list!=NULL){
		printf("%s\t%d\n",list->word,list->occurences);
		list=list->next;
	}
}
