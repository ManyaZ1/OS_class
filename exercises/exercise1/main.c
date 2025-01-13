#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
int        N=10;
int        n=2;
/**pthread_create function allows specific arguement (επιτρέπoνται μόνο τα εξης ορίσματα:)*/
                                    /*int pthread_create(pthread_t * thread, const pthread_attr_t * attr, */
typedef struct thread_data          /* void * (*start_routine)(void *), void *arg); */
                                    /**therefore multiple arguements can be given to the thread only with a struct (+we cant return int)*/
                                    /**but we can give an int result as part of the struct*/
{                                   /**άρα πολλαπλά ορίσματα μόνο με struct γίνεται να περαστούν*/
    int  start; /*αρχή σειράς αθροίσματος*/
    int  end;   /*τελος σειράς αθροίσματος*/
    int result; /*αποτέλεσμα*/
} Thread_data;
typedef Thread_data* Dataptr;
void* adder(void *arg);
int total(Thread_data** data_arr);
int threader(int N,int n);

int main()
{
    int escape=0;
    int N;
    int n;
    int tot;
    while(escape==0){                                      /**< input handling */
        printf("Type number N: \n");
        scanf("%d", &N);
        printf("Type number n: \n");
        scanf("%d", &n);
        if(N%n!=0){
            printf("N should be a multiple of n!\n");
            continue;
        }
        tot=threader(N,n);                                 /**calls function where threading happens*/
        printf("calculation=%d\n",N*(N+1)/2);
        if(tot!=N*(N+1)/2){printf("mistake\n");}           /*check*/
        printf("if you want to repeat press zero: '0'\n");
        scanf("%d", &escape);
    }

    return 0;
}
/** \brief multiple threads that add concurrently numbers from 1 to N
 *
 * \param N = total nums
 * \param n = number of threads
 * \return total=sum of each thread's sum
 */

int threader(int N, int n){
    int num_of_threads;
    int tot;
    int k=N/n;
    int i=0;
                                /*int pthread_create(pthread_t * thread,*/
    pthread_t* pthread_t_arr; /*thread: pointer to an unsigned integer value that returns the thread id of the thread created. Geeksforgeeks*/
    Thread_data** thread_data_arr; /**array with poinetrs to structs, so we initialize pointer to pointer to Thread_data*/
    pthread_t_arr=malloc(n*sizeof(pthread_t)); /*array of pointers to threads' ids*/
    thread_data_arr=malloc((n+1)* (sizeof (Dataptr )));
    thread_data_arr[n]=NULL; /*used in while loop in total == symbolises end of list*/

    for(num_of_threads=0; num_of_threads<n; num_of_threads++) /*every iteration=new thread*/
    {
        thread_data_arr[num_of_threads]=malloc(sizeof(Thread_data)); /*struct thread data for each thread initialized*/
        if(thread_data_arr[num_of_threads]==NULL)
        {
            printf("null data");
            exit(1);
        }
        thread_data_arr[num_of_threads]->start=k*num_of_threads+1;/**< 1 to k, k+1 to 2k etc: [from iteration*k+1 up to (iteration+1)*k] */
        thread_data_arr[num_of_threads]->end=k*(num_of_threads+1);
        thread_data_arr[num_of_threads]->result=0; /**\initiallization \for \safety*/
/**< new thread created with the adder as a subroutine and the struct thread_data as arguements*/
        if (pthread_create(&pthread_t_arr[num_of_threads],NULL,adder,(void *)thread_data_arr[num_of_threads])!= 0)
        {
            printf("Error creating thread\n"); /*if it fails*/
            exit(1);
        }

    }/**pthread_join all threads so they are all required to finish before total is called and main terminates*/
    for (num_of_threads = 0; num_of_threads < n; num_of_threads++)
    {
        pthread_join(pthread_t_arr[num_of_threads], NULL);

    }
    tot=total(thread_data_arr); /*total is called to add all the partial results stored in the thread_data_arr*/
    printf("total result=%d\n",tot); /**< threads communicate through common memory!!! */
    return tot;
}


int total(Thread_data** data_arr) /**arguement is the array with the struct thread_data elements, the result of each thread is stored there*/
{                                 /*adds all the partial results*/
    int i=0;
    int tot=0;
    if (data_arr==NULL)return -1;
    while(data_arr[i]!=NULL)
    {
        tot+=data_arr[i]->result;
        i++;
    }
    return tot;

}
void* adder(void *arg)  /**start from start and stop at end. add all the numbers in between. e.g. 6+7+8+9+10*/
{
    int i=0;
    struct thread_data *data;

    data = (struct thread_data *) arg;
    /*printf("thread created data %p\n",data);*/
    for(i=data->start; i<=data->end; i++)
    {
        /*printf("%d\n",i);*/
        data->result+=i;
    }
    /*printf("result %d\n",data->result);*/
    return NULL;
}
