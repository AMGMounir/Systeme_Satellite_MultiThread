#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include "sensors.h" 
#include "thermal.h"
#include "historique.h"
#include "types.h"
#include "queue.h"
#include "telemetry.h"
#include "watchdog.h"



Queue queue_telemetry;
HistoriqueCapteur historiques[NUM_CAPTEURS];
DonneesCapteur derniers_capteurs[NUM_CAPTEURS];
pthread_mutex_t mutex_capteurs = PTHREAD_MUTEX_INITIALIZER;
volatile bool system_running = true;




//capteur
void* thread_capteurs(void* arg) {
    printf("[THREAD CAPTEURS] Démarrage...\n");

    uint32_t timestamp = 0;

    uint64_t next_execution = timing_get_ms(); 
    const uint32_t PERIOD_MS = 1000;    

    while (system_running) {
        watchdog_ping(THREAD_ID_CAPTEURS);
        
        printf("\n[CAPTEURS] Lecture cycle (t=%u ms)\n", timestamp);
        
        for (int i = 0; i < NUM_CAPTEURS; i++) {

            DonneesCapteur donnees = lire_capteur(i, timestamp);
            
            pthread_mutex_lock(&mutex_capteurs);
            derniers_capteurs[i] = donnees;
            historique_ajouter(&historiques[i], donnees);
            pthread_mutex_unlock(&mutex_capteurs);
            
            if (!donnees.est_valide) {
                Message msg;
                msg.type = MSG_ALERTE_THERMIQUE;
                msg.donnees = donnees;
                msg.timestamp_ms = timestamp;
                queue_envoyer(&queue_telemetry, msg);
            }
            
            float moyenne = historique_moyenne(&historiques[i]);
            printf("  Capteur %d : %6.2f°C [%s] (moy: %.2f°C)\n",
                   donnees.id,
                   donnees.temperature,
                   donnees.est_valide ? "OK     " : "ALERTE!",
                   moyenne);
        }
        
        timestamp += PERIOD_MS; 
        next_execution += PERIOD_MS;
        timing_wait_until(next_execution);
    }
    
    printf("[THREAD CAPTEURS] Arrêt\n");
    return NULL;
}

//control thermique
void* thread_controle(void* arg) {
    printf("[THREAD CONTRÔLE] Démarrage...\n");

    uint32_t timestamp = 0;

    uint64_t next_execution = timing_get_ms();
    const uint32_t PERIOD_MS = 1000; 
    
    while (system_running) {
        watchdog_ping(THREAD_ID_CONTROLE);
        
        sleep(1); 
        
        printf("\n[CONTRÔLE] Analyse thermique (t=%u ms)\n", timestamp);
        

        DonneesCapteur capteurs_locaux[NUM_CAPTEURS];
        pthread_mutex_lock(&mutex_capteurs);
        for (int i = 0; i < NUM_CAPTEURS; i++) {
            capteurs_locaux[i] = derniers_capteurs[i];
        }
        pthread_mutex_unlock(&mutex_capteurs);
        
 
        thermal_update(capteurs_locaux, NUM_CAPTEURS);
        printf("  >>> État système: %s\n", thermal_get_nom_etat(thermal_get_etat()));
        

        thermal_envoyer_telemetrie(&queue_telemetry, timestamp);
        
        timestamp += PERIOD_MS;
        next_execution += PERIOD_MS; 
        timing_wait_until(next_execution);
    }
    
    printf("[THREAD CONTRÔLE] Arrêt\n");
    return NULL;
}

//telemetrie
void* thread_telemetrie(void* arg) {

    uint64_t next_execution = timing_get_ms();
    const uint32_t PERIOD_MS = 2000;  
    
    printf("[THREAD TÉLÉMÉTRIE] Démarrage...\n");
    
    while (system_running) {
        watchdog_ping(THREAD_ID_TELEMETRIE);
        
        printf("\n[TÉLÉMÉTRIE] Traitement des messages\n");
        telemetry_traiter(&queue_telemetry);
        queue_afficher_etat(&queue_telemetry);

        next_execution += PERIOD_MS;
        timing_wait_until(next_execution); 
    }
    
    printf("[THREAD TÉLÉMÉTRIE] Arrêt\n");
    return NULL;
}


int main() {
    srand(time(NULL));
    
    printf("=== Système Satellite Multi-Thread ===\n\n");
    
    // Initialisation
    sensors_init();
    thermal_init();
    telemetry_init();
    queue_init(&queue_telemetry);
    watchdog_init();
    
    for (int i = 0; i < NUM_CAPTEURS; i++) {
        historique_init(&historiques[i]);
    }
    
    pthread_t tid_capteurs, tid_controle, tid_telemetrie, tid_watchdog;
    
    pthread_create(&tid_capteurs, NULL, thread_capteurs, NULL);
    pthread_create(&tid_controle, NULL, thread_controle, NULL);
    pthread_create(&tid_telemetrie, NULL, thread_telemetrie, NULL);
    pthread_create(&tid_watchdog, NULL, thread_watchdog, NULL);
    
    printf("\n[MAIN] Système en marche pendant 10 secondes...\n");
    sleep(10);
    
    printf("\n[MAIN] Arrêt du système...\n");
    system_running = false;
    
    pthread_join(tid_capteurs, NULL);
    pthread_join(tid_controle, NULL);
    pthread_join(tid_telemetrie, NULL);
    
    queue_destroy(&queue_telemetry);
    pthread_mutex_destroy(&mutex_capteurs);
    
    printf("\n=== Système arrêté proprement ===\n");

    memory_print_sizes();
    memory_get_total_usage();
    
    return 0;
}