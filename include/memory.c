#include "memory.h"
#include "types.h"
#include <stdio.h>

void memory_print_sizes(void) {
    printf("\n=== ANALYSE MÉMOIRE ===\n");
    
    printf("\nStructures de base :\n");
    printf("  DonneesCapteur        : %zu octets\n", sizeof(DonneesCapteur));
    printf("  Message               : %zu octets\n", sizeof(Message));
    printf("  WatchdogEntry         : %zu octets\n", sizeof(WatchdogEntry));
    printf("  ConfigSysteme         : %zu octets\n", sizeof(ConfigSysteme));
    
    printf("\nStructures complexes :\n");
    printf("  HistoriqueCapteur     : %zu octets\n", sizeof(HistoriqueCapteur));
    printf("  Queue                 : %zu octets\n", sizeof(Queue));
    
    printf("\nTableaux globaux :\n");
    printf("  4x HistoriqueCapteur  : %zu octets\n", sizeof(HistoriqueCapteur) * 4);
    printf("  4x DonneesCapteur     : %zu octets\n", sizeof(DonneesCapteur) * 4);
    printf("  1x Queue              : %zu octets\n", sizeof(Queue));
    
    printf("\nMutex et synchronisation :\n");
    printf("  pthread_mutex_t       : %zu octets\n", sizeof(pthread_mutex_t));
    printf("  pthread_t             : %zu octets\n", sizeof(pthread_t));
}

uint32_t memory_get_total_usage(void) {
    uint32_t total = 0;
    
    // Variables globales du main.c
    total += sizeof(Queue);                      // queue_telemetry
    total += sizeof(HistoriqueCapteur) * 4;      // historiques[4]
    total += sizeof(DonneesCapteur) * 4;         // derniers_capteurs[4]
    total += sizeof(pthread_mutex_t);            // mutex_capteurs
    total += sizeof(bool);                       // system_running
    
    // Threads
    total += sizeof(pthread_t) * 4;              // threads
    
    // Watchdog
    total += sizeof(WatchdogEntry) * 3;          // 3 entrées watchdog
    total += sizeof(pthread_mutex_t);            // mutex_watchdog
    
    // Thermal
    total += sizeof(ConfigSysteme);              // config
    total += sizeof(int) * 2;                    // etat_actuel + compteur_critical
    
    printf("\n=== MÉMOIRE TOTALE ESTIMÉE ===\n");
    printf("Total (approximatif) : %u octets (~%.2f KB)\n", 
           total, total / 1024.0f);
    
    return total;
}