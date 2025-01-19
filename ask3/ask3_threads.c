#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define BUFFER_SIZE 5
#define PRODUCERS_NUM 2
#define CONSUMERS_NUM 2

int buffer[BUFFER_SIZE];
int in = 0;
int out = 0;
//circular buffer logic no need to insert new produced element where the last element was consumed

sem_t empty;
sem_t full;
pthread_mutex_t mutex;

void buffer_printer(int* buffer);
void *producer(void *param);
void *consumer(void *param);

int main() {
    pthread_t producers[2], consumers[2];

    // Αρχικοποίηση σημαφόρων και mutex
    sem_init(&empty, 0, BUFFER_SIZE);
    sem_init(&full, 0, 0);
    pthread_mutex_init(&mutex, NULL); // NULL για τις προεπιλεγμένες τιμές

    // Δημιουργία νημάτων παραγωγών
    for (int i = 0; i < PRODUCERS_NUM; i++) {
        if (pthread_create(&producers[i], NULL, producer, NULL) != 0) {
            perror("Failed to create producer thread");
            exit(1);
        }
    }

    // Δημιουργία νημάτων καταναλωτών
    for (int i = 0; i < CONSUMERS_NUM; i++) {
        if (pthread_create(&consumers[i], NULL, consumer, NULL) != 0) {
            perror("Failed to create consumer thread");
            exit(1);
        }
    }

    // Αναμονή για τα νήματα (θα τρέχουν επ' αόριστον)
    //pthread join all consumers and producers
    for (int i = 0; i < PRODUCERS_NUM; i++) {
        pthread_join(producers[i], NULL);
    }
    for (int i = 0; i < CONSUMERS_NUM; i++) {
        pthread_join(consumers[i], NULL);
    }

    // Καθαρισμός
    sem_destroy(&empty);
    sem_destroy(&full);
    pthread_mutex_destroy(&mutex);

    return 0;
}

void *producer(void *param) {
    int item;
    while (1) {
        item = rand() % 100; // Δημιουργία τυχαίου αντικειμένου
        sem_wait(&empty); // Αναμονή για κενό slot στον buffer με το sem_wait να μειώνει τον αριθμό των κενών slots
        //Αν το empty είναι 0, το thread παραγωγού θα μπλοκάρει μέχρι να γίνει post από το thread καταναλωτή και το empty να αυξηθεί κατά 1
        //Αλλιώς το empty θα μειωθεί κατά 1 και το thread θα συνεχίσει την εκτέλεσή του

        pthread_mutex_lock(&mutex); // Κλείδωμα του mutex για να μην υπάρχει ανταγωνισμός μεταξύ των νημάτων 
        // Μόνο ένα νήμα μπορεί να έχει πρόσβαση στον buffer τη φορά

        // Τοποθέτηση αντικειμένου στον buffer
        buffer[in] = item;
        printf("Producer %ld: Inserted item %d at %d\n", pthread_self(), item, in);
        buffer_printer(buffer);
        in = (in + 1) % BUFFER_SIZE; // το υπολοιπο διαφαλίζει ότι αν φτάσουμε στο τέλος του buffer, θα πρέπει να επιστρέψουμε στην αρχή του
        pthread_mutex_unlock(&mutex); // Ξεκλείδωμα του mutex

        // Το sem_post αυξάνει τον αριθμό των γεμάτων slots στον buffer, sem_post(&full) αυξάνει το full κατά 1
        sem_post(&full); // Αύξηση του full κατά 1 ώστε να ειδοποιηθεί το νήμα καταναλωτή ότι υπάρχει ένα γεμάτο slot

        sleep(2); // Προσομοίωση καθυστέρησης 
    }
}

void *consumer(void *param) {
    int item;
    while (1) {
        sem_wait(&full);// Αναμονή για γεμάτο slot στον buffer με το sem_wait να μειώνει τον αριθμό των γεμάτων slots 
        //αν όλα άδεια το thread καταναλωτή θα μπλοκάρει μέχρι να γίνει post από το thread παραγωγό και το full να μειωθεί κατά 1
        //Αλλιώς το full θα μειωθεί κατά 1 και το thread θα συνεχίσει την εκτέλεσή του καθώς ένα slot θα γίνει κενό

        pthread_mutex_lock(&mutex); // Κλείδωμα του mutex για να μην υπάρχει ανταγωνισμός μεταξύ των νημάτων

        // Αφαίρεση αντικειμένου από τον buffer
        item = buffer[out]; 
        // zero it
        buffer[out] = 0;
        printf("Consumer %ld: Removed item %d from %d\n", pthread_self(), item, out);
        buffer_printer(buffer);
        out = (out + 1) % BUFFER_SIZE; //out λειτουργει οπως το in

        pthread_mutex_unlock(&mutex); // Ξεκλείδωμα του mutex
        sem_post(&empty); // Αύξηση του empty κατά 1 ώστε να ειδοποιηθεί το νήμα παραγωγό ότι υπάρχει ένα κενό slot

        sleep(2); // Προσομοίωση καθυστέρησης
    }
}

void buffer_printer(int* buffer){ //print buffer
    printf("Buffer: ");
    for (int i = 0; i < BUFFER_SIZE; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");
}
