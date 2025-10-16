#include "telemetry.h"
#include "queue.h"
#include "thermal.h"
#include <stdio.h>

static int messages_traites = 0;

void telemetry_init(void) {
    messages_traites = 0;
    printf("[TELEMETRY] Initialisation...\n");
}

void telemetry_traiter(Queue *queue_telemetry) {
    Message msg;
    
    // Traiter TOUS les messages disponibles
    while (queue_recevoir(queue_telemetry, &msg)) {
        messages_traites++;
        
        switch (msg.type) {
            case MSG_RAPPORT_ETAT:
                printf("[TELEMETRY] Rapport #%d (t=%u) : Etat = %s\n",
                       messages_traites,
                       msg.timestamp_ms,
                       thermal_get_nom_etat(msg.etat));
                break;
                
            case MSG_ALERTE_THERMIQUE:
                printf("[TELEMETRY] ALERTE #%d (t=%u) : Capteur %d = %.2f C\n",
                       messages_traites,
                       msg.timestamp_ms,
                       msg.donnees.id,
                       msg.donnees.temperature);
                break;
                
            case MSG_DONNEES_CAPTEUR:
                printf("[TELEMETRY] Donnees #%d (t=%u) : Capteur %d = %.2f C\n",
                       messages_traites,
                       msg.timestamp_ms,
                       msg.donnees.id,
                       msg.donnees.temperature);
                break;
        }
    }
}