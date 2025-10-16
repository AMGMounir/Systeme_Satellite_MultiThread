#define _POSIX_C_SOURCE 199309L

#include "watchdog.h"
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>



static WatchdogEntry threads[NUM_THREADS_SURVEILLES];
static pthread_mutex_t mutex_watchdog = PTHREAD_MUTEX_INITIALIZER;

extern volatile bool system_running;

void watchdog_init(void) {
    pthread_mutex_lock(&mutex_watchdog);
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    uint32_t current_time = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    

    threads[THREAD_ID_CAPTEURS].nom = "CAPTEURS";
    threads[THREAD_ID_CAPTEURS].is_alive = true;

    threads[THREAD_ID_CAPTEURS].last_ping_ms = current_time;
    
    threads[THREAD_ID_CONTROLE].nom = "CONTROLE";
    threads[THREAD_ID_CONTROLE].is_alive = true;
    threads[THREAD_ID_CONTROLE].last_ping_ms = current_time;
    
    threads[THREAD_ID_TELEMETRIE].nom = "TELEMETRIE";
    threads[THREAD_ID_TELEMETRIE].is_alive = true;
    threads[THREAD_ID_TELEMETRIE].last_ping_ms = current_time;
    
    pthread_mutex_unlock(&mutex_watchdog);
    
    printf("[WATCHDOG] Initialisation - Timeout: %d ms\n", WATCHDOG_TIMEOUT_MS);
}

void watchdog_ping(uint8_t thread_id) {
    if (thread_id >= NUM_THREADS_SURVEILLES) return;
    
    pthread_mutex_lock(&mutex_watchdog);
    

    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    threads[thread_id].last_ping_ms = ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
    threads[thread_id].is_alive = true;
    
    pthread_mutex_unlock(&mutex_watchdog);
}

void watchdog_check(uint32_t current_time_ms) {
    pthread_mutex_lock(&mutex_watchdog);
    
    bool probleme_detecte = false;
    
    for (int i = 0; i < NUM_THREADS_SURVEILLES; i++) {
        if (!threads[i].is_alive) continue;
        
        uint32_t temps_ecoule = current_time_ms - threads[i].last_ping_ms;
        
        if (temps_ecoule > WATCHDOG_TIMEOUT_MS) {
            printf("[WATCHDOG]TIMEOUT sur thread %s ! (%u ms sans signal)\n",
                   threads[i].nom,
                   temps_ecoule);
            probleme_detecte = true;
        }
    }
    
    if (probleme_detecte) {
        printf("[WATCHDOG]ALERTE SYSTEME - Threads bloques detectes !\n");
    }
    
    pthread_mutex_unlock(&mutex_watchdog);
}

void* thread_watchdog(void* arg) {
    printf("[THREAD WATCHDOG] Demarrage - Surveillance active\n");
    uint64_t next_execution = timing_get_ms();
    const uint32_t PERIOD_MS = 1000;  

    while (system_running) {
        uint64_t current_time = timing_get_ms(); 
        watchdog_check((uint32_t)current_time);
        
        next_execution += PERIOD_MS;  
        timing_wait_until(next_execution); 
    }
    
    printf("[THREAD WATCHDOG] Arret\n");
    return NULL;
}