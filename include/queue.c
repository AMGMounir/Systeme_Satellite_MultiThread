#include "queue.h"
#include <stdio.h>

void queue_init(Queue *q) {
    q->index_ecriture = 0;
    q->index_lecture = 0;
    q->nombre_messages = 0;
    pthread_mutex_init(&q->mutex, NULL); 
}

void queue_destroy(Queue *q) {
    pthread_mutex_destroy(&q->mutex);
}

bool queue_envoyer(Queue *q, Message msg) {

    pthread_mutex_lock(&q->mutex);

    if (queue_est_pleine(q)) {
        printf("[QUEUE] Queue pleine ! Message perdu.\n");
        pthread_mutex_unlock(&q->mutex);
        return false;
    }
    
    q->messages[q->index_ecriture] = msg;
    
    q->index_ecriture = (q->index_ecriture + 1) % TAILLE_QUEUE;
    
    q->nombre_messages++;
    
    pthread_mutex_unlock(&q->mutex);
    return true;
}

bool queue_recevoir(Queue *q, Message *msg) {
    if (queue_est_vide(q)) {
        printf("[QUEUE]Queue vide!\n");
        return false;
    }
    
    *msg = q->messages[q->index_lecture];

    q->index_lecture = (q->index_lecture + 1) % TAILLE_QUEUE;

    q->nombre_messages--;
    
    return true;
}

bool queue_est_vide(Queue *q) {
    return q->nombre_messages == 0;
}

bool queue_est_pleine(Queue *q) {
    return q->nombre_messages >= TAILLE_QUEUE;
}

void queue_afficher_etat(Queue *q) {
    pthread_mutex_lock(&q->mutex);
    printf("[QUEUE] Messages: %d/%d (R:%d W:%d)\n", 
           q->nombre_messages, 
           TAILLE_QUEUE,
           q->index_lecture,
           q->index_ecriture);
    pthread_mutex_unlock(&q->mutex);
}