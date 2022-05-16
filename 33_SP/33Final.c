#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>

#define FILE_NAME_LENGHT 30
#define SEARCH_WORD 30


void* searchWord(void* input);
void* reader(void* input);

int fd[10][2]; 


typedef struct ARGUMENTS{ 
    FILE *fp;
    char* search;
    char* filename;
    int index;
   
}args;

typedef struct PipeStruct{ 
        int _count;
        char filename[20];
        int _wordPlace[50];
    }strct;



int main(){

    printf("Number of files to search from:");
    int NUM_FILES;
    scanf("%d", &NUM_FILES);
  
    for(int i=0; i<NUM_FILES; i++){
        if (pipe(fd[i]) < 0){
        perror("pipe ");
        exit(1);
        }
    }
    
    FILE **fp; // filepointer arr
    fp = malloc(NUM_FILES *sizeof(FILE*));

    char strArr[NUM_FILES][FILE_NAME_LENGHT];
    printf("Which files would you like to open?:\n");

    for(int i = 0; i< NUM_FILES; i++){

        scanf("%s", strArr[i]);
        strcat(strArr[i], ".txt");

        fp[i] = fopen(strArr[i], "r");
        if(fp[i] == NULL){
                    printf("Error opening file. File is either corrupted or non-existant! Exiting program...\n");
                    return 1; 
                }else{
                    printf("File %s opened!\n", strArr[i]);
                }
    }

    char  word[SEARCH_WORD];
    
    printf("Enter desired word to search:");
    scanf("%s", word);
    printf("\nSearching for word: ");
    puts(word);
    printf("\n");

    args strct[NUM_FILES];
   
    for(int i = 0; i< NUM_FILES; i++){
        strct[i].fp = fp[i];
        strct[i].search = word;
        strct[i].filename = strArr[i];
        strct[i].index = i;
    }

    pthread_t threads[NUM_FILES*2];

    printf("----------RESULTS:------------\n\n");

    for(int i =0 ; i<NUM_FILES; ++i){
        int rc = pthread_create(&threads[i], NULL, searchWord, (void *) &strct[i]);
        int rc2 = pthread_create(&threads[i+NUM_FILES], NULL, reader, (void*)(intptr_t)i);
        if(rc){
            printf("Error: couldn't create thread %d\n", i);
            exit(-1);
        }
        else{
            //printf("Thread %d  pid = %lu created\n", i, threads[i]);
        }

        if(rc2){
            printf("Error: couldn't create thread %d\n", i+NUM_FILES);
            exit(-1);
        }else{
            //printf("Thread %d  pid = %lu created\n", i+NUM_FILES, threads[i+NUM_FILES]);
        }
    }


    for(int i = 0; i< NUM_FILES*2; i++){
        int rc = pthread_join(threads[i], NULL);
        if(rc){
            printf("Error: couldn't join thread %d", i);
            exit(-1);
        }else{
           // printf("Thread %d  pid = %lu joined\n", i, threads[i]);
        }
    }

    for(int i =0; i < NUM_FILES; i++){
        fclose(fp[i]);
    }

    free(fp);
    return 0;   
}



void* searchWord(void* input){

    char arr[50], ch;
    
    int rc, cntr=0, count = 0, place = 0, 
    wordPlaceCounter = 0;
    int wordPlace[50];

        while ((ch = fgetc(((struct ARGUMENTS*)input)->fp)) != EOF){
    
            if(ch == ' ' || ch =='.' || ch == ',' || ch == '"' || ch == '-' || ch =='?' || ch == '!'){
                place++;
                if(ch != ' '){
                    cntr = 1;
                }
                int rc = strcasecmp(arr, ((struct ARGUMENTS*)input)->search);
                if(rc == 0){  
                        wordPlace[wordPlaceCounter] = place;
                        memset(&arr[0], 0, sizeof(arr));
                        ch= '\0';
                        count++;
                        wordPlaceCounter++;
                }else{
                        memset(&arr[0], 0, sizeof(arr));
                        ch = '\0';
                }
            }     
            if(cntr == 1){
                place--;
                cntr = 0;
            }
            strcat(arr, &ch); 
        }   


        strct WriterPipe;
        int writerindex = ((struct ARGUMENTS*)input)->index;
        //printf("WriterIndex is %d\n", writerindex);

        WriterPipe._count = count;
        for(int i = 0; i< count; i++){
            WriterPipe._wordPlace[i] = wordPlace[i];
            
        }
        strcpy(WriterPipe.filename,((struct ARGUMENTS*)input)->filename);
        
        if(write(fd[writerindex][1], &WriterPipe, sizeof(struct PipeStruct)) < 0){
            printf("Couldnt write to pipe %d", writerindex);
            exit(-1);        
        }
        
        close(fd[writerindex][1]);

    
}

void* reader(void* input){
    
    strct ReaderPipe;
    int readerindex = (int)(intptr_t)input;
    //printf("Reader Index is %d\n", index);
    if(read(fd[readerindex][0], &ReaderPipe, sizeof(struct PipeStruct)) < 0){
        printf("Couldn't read to pipe %d", readerindex);
        exit(-1);
    }
    if(ReaderPipe._count != 0){
        printf("Reader: The word has appeared %d times in Filename:%s\n", ReaderPipe._count, ReaderPipe.filename);
        printf("The word has appeared at positions:");
        for(int i =0; i < ReaderPipe._count; i++){
        if(ReaderPipe._wordPlace[i] != 0){
            printf(" %d ", ReaderPipe._wordPlace[i]);
        }
    }
    }else{
        printf("Reader: The word has appeared %d times in Filename:%s\n", ReaderPipe._count, ReaderPipe.filename);

    }
    close(fd[readerindex][0]);
    printf("\n\n");

}


