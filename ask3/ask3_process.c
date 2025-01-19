#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <signal.h>
//for wait
#include <sys/wait.h>
#define BUFFER_SIZE 5
#define PRODUCERS_NUM 4
#define CONSUMERS_NUM 3

int *buffer; // circular buffer
int *in; // in pointer
int *out;   // out pointer
sem_t *empty; // empty slots
sem_t *full;    // full slots
sem_t *mutex; // mutex

void buffer_printer(int *buffer);
void producer();
void consumer();

// cleanup function
// Unlink and close semaphores and shared memory
void cleanup(int signum) { //signal handler
    sem_close(empty);      //close semaphore
    sem_close(full);       //close semaphore
    sem_close(mutex);
    sem_unlink("empty");
    sem_unlink("full");
    sem_unlink("mutex");
    shmdt(buffer); //detach shared memory
    shmdt(in); //detach shared memory
    shmdt(out); //detach shared memory
    shmctl(shmget(1234, 0, 0), IPC_RMID, NULL); //remove shared memory
    shmctl(shmget(5678, 0, 0), IPC_RMID, NULL); //remove shared memory
    shmctl(shmget(9012, 0, 0), IPC_RMID, NULL); //remove shared memory
    exit(0);
}

/*Separate Memory: Processes do not share memory by default; each process has its own address space.
Shared Memory Setup: Explicit shared memory is required (via shmget, shmat, etc.) 
for processes to communicate. This setup allows buffer, in, and out to be accessible across processes.
*/
/** Synchronization requires named semaphores (sem_open, sem_post, etc.), which are accessible across process boundaries.
Named semaphores exist independently of the processes, ensuring synchronization even when they run in separate address spaces.
This approach is more complex as it requires explicit management of semaphores (e.g., sem_unlink to clean up resources) */

int main() {
    // Register signal handler
    signal(SIGINT, cleanup);
    // Create shared memory segments
    //δημιουργία buffer, in, out σε μοιραζόμενη μνήμη
    int buffer_shmid = shmget(1234, BUFFER_SIZE * sizeof(int), IPC_CREAT | 0666); // 0666: Read and write permissions for all users to shared memory buffer
    int in_shmid = shmget(5678, sizeof(int), IPC_CREAT | 0666); // in pointer σε μοιραζόμενη μνήμη
    int out_shmid = shmget(9012, sizeof(int), IPC_CREAT | 0666); // out pointer σε μοιραζόμενη μνήμη

    /** shmget is a system call used to allocate a shared memory segment. It takes three parameters:
     * - key: A unique identifier for the shared memory segment.
     * - size: The size of the shared memory segment in bytes.
     * - shmflg: Flags to control the creation and access permissions of the shared memory segment.
     * 
     * The function returns an identifier for the shared memory segment, which can be used in subsequent
     * operations such as attaching the segment to the process's address space or controlling the segment. */

    buffer = shmat(buffer_shmid, NULL, 0); // Attach shared memory segments to buffer, in, and out
    in = shmat(in_shmid, NULL, 0); // now in,out,buffer are pointers to shared memory. Η in γίνεται ένας δείκτης στη μοιραζόμενη μνήμη
    out = shmat(out_shmid, NULL, 0); 

    /* shmat is a system call */

    *in = 0; // Initialize in pointer
    *out = 0; // Initialize out pointer
    /* Η γραμμή *in = 0 και *out = 0 απλώς τροποποιεί την τιμή στη μοιραζόμενη μνήμη, στην οποία συνδέονται οι δείκτες*/

    empty = sem_open("empty", O_CREAT, 0666, BUFFER_SIZE); // Create semaphores and initialize them
    full = sem_open("full", O_CREAT, 0666, 0);
    mutex = sem_open("mutex", O_CREAT, 0666, 1);

    for (int i = 0; i < PRODUCERS_NUM; i++) {
        // Create producer processes
        if (fork() == 0) { // Child process
            producer();   // Producer function 
            exit(0); // Exit child process
        }
    }

    for (int i = 0; i < CONSUMERS_NUM; i++) {
        // Create consumer processes
        if (fork() == 0) { // Child process
            consumer(); // Consumer function
            exit(0); // Exit child process
        }
    }

    for (int i = 0; i < PRODUCERS_NUM + CONSUMERS_NUM; i++) { // Wait for all child processes to finish
        wait(NULL); // Wait for child process to finish
    }

    cleanup(0); //never happens due to infinite loop in producer/consumer code
    return 0;
}

// Producer function
void producer() {
    while (1) {
        int item = rand() % (getpid());
        sem_wait(empty); // Wait for empty slot in buffer
        sem_wait(mutex); // Lock buffer

        buffer[*in] = item;
        printf("Producer %d: Inserted item %d at %d\n", getpid(), item, *in);
        buffer_printer(buffer);
        *in = (*in + 1) % BUFFER_SIZE;

        sem_post(mutex); // Unlock buffer
        sem_post(full); // Signal full slot

        sleep(2);
    }
}

void consumer() {
    while (1) {
        sem_wait(full); // Wait for full slot in buffer
        sem_wait(mutex); // Lock buffer

        int item = buffer[*out]; // Remove item from buffer
        buffer[*out] = 0; 
        printf("Consumer %d: Removed item %d from %d\n", getpid(), item, *out);
        buffer_printer(buffer);
        *out = (*out + 1) % BUFFER_SIZE;

        sem_post(mutex);
        sem_post(empty);

        sleep(2);
    }
}

void buffer_printer(int *buffer) {
    printf("Buffer: ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");
}

//το *in και το *οut βρίσκονται στην κοινή μνήμη και είναι ακέραιοι αριθμοί που λειτουργούν ως δείκτες στον buff/er