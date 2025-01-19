
// C program to illustrate
// pipe system call in C
// and multiprocessing

#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#define MSGSIZE 16
int *inputs(void);
int checker(unsigned long long int calculation,unsigned long long int N);
int multiprocessing(int N,int n);
unsigned long long int totalcalc(int p[],int n);
unsigned long long int calculator(int start,int end);
void child(int p[],int N,int n,int i);

int main()
{
        int flag=0; int flag2=1;
        while(flag2==1 && flag==0){
                //input handling
                int* y=NULL;
                y=inputs();
                printf("input = N %d n %d\n",y[0],y[1]);
                int N=y[0];
                int n=y[1];
                if(N%n!=0){
                  printf("N should be a multiple of n\nTry again!\n");
                  continue;
                }
                multiprocessing(N,n);
                printf("\ndo you wish to continue? if yes, press '0', if no, press any other key\n");
                 
                if (scanf("%d", &flag) != 1) { // Check if an integer was entered
                  int ch;       // Clear invalid input
                  while ((ch = getchar()) != '\n' && ch != EOF);
                  flag2 = 0;  // Break loop if input is invalid
                } 
                else {flag2 = 1; } // Continue if valid integer entered
        }
    return 0;
    exit(0);
}

int multiprocessing(int N,int n){
    int p[2], pid; //pipe file descriptor is int* and int pid for storing each process' id
    //create pipe
    if (pipe(p)>0){
      perror("pipe failed\n");
      exit(1); //if pipe fails
    }
    int i=0; //iteration for creating multiple processes (n processes for this exercise)
    //each iteration => one parent and one child process
        for(i=0;i<n;i++){
          pid=fork();
          if(pid==0){ //if child processes
            child(p,N,n,i);
          }
          else if(pid>0){          }
          else{ //if fork fails
              perror("fork failed");
              exit(1);
          }
        }
        close(p[1]);//close write end again
        for(int i=0;i<n;i++){//wait(NULL) means that the parent process waits until ANY CHILD has finished.
            wait(NULL);      //we have n children so a loop is needed. every time a child finishes we go to the next iteration
        }                    //we have now ensured that all the children have finished
        unsigned long long int total_sum = totalcalc(p,n);
        //total sum calculation
        if(checker(total_sum,N)){printf("\nsuccess!");}
        else{printf("mistake");return 1;}
        return 0;
}

unsigned long long int totalcalc(int p[],int n){
  unsigned long long int inbuf=0; 
  unsigned long long int total_sum=0;   //total sum calculation
  for(int k=0;k<n;k++){
    if(read(p[0],&inbuf,sizeof(inbuf))>0){    // checks if the read call was successful>0
      total_sum+=inbuf; //add "local" sums stored in the pipe to the total
                //printf("%d\n",inbuf);
    }
    else perror("read failed");
    }
    close(p[0]);
    printf("totalsum=%llu\n",total_sum);
    return total_sum;
}

int *inputs(void){
  int* x=(int*)malloc(2*sizeof(int));
  printf("give me N:");
  scanf("%d",&x[0]);
  printf("\ngive me n:");
  scanf("%d",&x[1]);
  printf("\n");
  return x;
}

int checker(unsigned long long int calculation,unsigned long long int N){
  if (calculation==(N*(N+1)/2))return 1;
  else return 0;
}

unsigned long long int calculator(int start,int end){
    unsigned long long int sum=0;
    for(int i=start;i<end;i++){
        sum+=i;
    }
    return sum;
}

void child(int p[],int N,int n,int i){                
  unsigned long long int sum=0; //local sum initialized to 0
  sum = calculator(i * (N / n), (i < n - 1) ? ((i + 1) * (N / n))  : (N + 1)); //first arg always the same but if we are at the last loop include N [N+1 arg], else up to (j+1)*N/n
  close(p[0]);  //close read end of pipe  for safety
  if(write(p[1],&sum,sizeof(sum))==-1){ //write end of pipe is p[1], we write what is stored in the &sum pointer (the sum)
    perror("pipe write failed\n");  //if write fails
    exit(1);
  }
  close(p[1]); //close write end of pipe for safety
  exit(0);  //terminate child process next iteration will create a new child
}
