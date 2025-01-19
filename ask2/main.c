#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

// semaphores
sem_t sa2b1,sc2,sc1;
int a1, a2, b1, b2, c1, c2, y, w, x, z;

void* thread1(void* arg) {
    a1 = 10;
    a2 = 11;
    sem_post(&sa2b1);
    sem_wait(&sc1);
    y = a1 + c1;
    sem_post(&sc2);
    sem_wait(&sc1);
    printf("x: %d\n", x);
    return NULL;
}
void* thread2(void* arg) {
 b1 = 20;
 b2 = 21;
 sem_post(&sa2b1);
 sem_wait(&sc2);
 sem_wait(&sc2);
 sem_wait(&sc2);
 w = b2 + c2;
 x = z - y + w;
 sem_post(&sc1);
 return NULL;
}

void* thread3(void* arg) {
 c1 = 30;
 sem_post(&sc1);
 c2 = 31;
 sem_post(&sc2);
 sem_wait(&sa2b1);
 sem_wait(&sa2b1);
 z = a2 + b1;
 sem_post(&sc2);
 return NULL;
}

int main() {
    pthread_t t1, t2, t3;
    // Initialize semaphores
    sem_init(&sa2b1, 0, 0);
    sem_init(&sc1, 0, 0);
    sem_init(&sc2, 0, 0);
    // Create threads
    pthread_create(&t1, NULL, thread1, NULL);
    pthread_create(&t2, NULL, thread2, NULL);
    pthread_create(&t3, NULL, thread3, NULL);
    // Join threads
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    //destroy semaphores
    sem_destroy(&sa2b1);
    sem_destroy(&sc1);
    sem_destroy(&sc2);
    return 0;
}
