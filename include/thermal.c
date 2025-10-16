#include "thermal.h"
#include "config.h"
#include <stdio.h>

// Variables internes (static = privees à ce fichier)
static EtatSysteme etat_actuel = ETAT_INIT;
static int compteur_critical = 0;  // Compte combien de cycles en CRITICAL
static ConfigSysteme config;

void thermal_init(void) {
    etat_actuel = ETAT_INIT;
    compteur_critical = 0;
    config_init(&config);
    config_activer_bit(&config, BIT_TELEMETRIE);
    printf("[THERMAL] Initialisation...\n");
}

void thermal_update(DonneesCapteur capteurs[], int nombre_capteurs) {
    // Compter combien de capteurs sont en alerte
    int alertes = 0;
    float temp_moyenne = 0.0f;
    
    for (int i = 0; i < nombre_capteurs; i++) {
        if (!capteurs[i].est_valide) {
            alertes++;
        }
        temp_moyenne += capteurs[i].temperature;
    }
    temp_moyenne /= nombre_capteurs;

    // ====== CONTRÔLE THERMIQUE ======
    
    // Desactiver tout par defaut
    config_desactiver_bit(&config, BIT_CHAUFFAGE);
    config_desactiver_bit(&config, BIT_RADIATEUR);
    config_desactiver_bit(&config, BIT_MODE_ECO);
    
    // Logique de contrôle selon la temperature moyenne
    if (temp_moyenne < 10.0f) {
        config_activer_bit(&config, BIT_CHAUFFAGE);
        printf("[THERMAL] Chauffage active (moy: %.1f C)\n", temp_moyenne);
    } else if (temp_moyenne > 40.0f) {
        config_activer_bit(&config, BIT_RADIATEUR);
        printf("[THERMAL] Radiateur active (moy: %.1f C)\n", temp_moyenne);
    }
    
    // ====== MACHINE À ETATS ======
    
    // Depuis INIT → toujours vers NOMINAL au premier cycle
    if (etat_actuel == ETAT_INIT) {
        etat_actuel = ETAT_NOMINAL;
        printf("[THERMAL] Transition: INIT vers NOMINAL\n");
        return;
    }
    
    // Depuis SAFE_MODE → on ne sort JAMAIS (necessite reboot)
    if (etat_actuel == ETAT_SAFE_MODE) {
        config_activer_bit(&config, BIT_MODE_ECO);
        printf("[THERMAL] En SAFE MODE (redemarrage necessaire)\n");
        return;
    }
    
    // Logique de transition selon le nombre d'alertes
    EtatSysteme ancien_etat = etat_actuel;
    
    if (alertes == 0) {
        // Tout va bien
        etat_actuel = ETAT_NOMINAL;
        compteur_critical = 0;  // Reset du compteur
        
    } else if (alertes == 1) {
        // Une alerte
        etat_actuel = ETAT_WARNING;
        compteur_critical = 0;
        
    } else {
        // 2+ alertes
        etat_actuel = ETAT_CRITICAL;
        compteur_critical++;
        
        // Si 2 cycles consecutifs en CRITICAL → SAFE_MODE
        if (compteur_critical >= 2) {
            etat_actuel = ETAT_SAFE_MODE;
            config_activer_bit(&config, BIT_MODE_ECO);
            printf("[THERMAL] !!! PASSAGE EN SAFE MODE !!!\n");
        }
    }
    
    // Afficher la transition si changement d'etat
    if (ancien_etat != etat_actuel) {
        printf("[THERMAL] Transition: %s vers %s (alertes: %d)\n",
               thermal_get_nom_etat(ancien_etat),
               thermal_get_nom_etat(etat_actuel),
               alertes);
    }

    // Afficher la configuration
    config_afficher(&config);
}

EtatSysteme thermal_get_etat(void) {
    return etat_actuel;
}

const char* thermal_get_nom_etat(EtatSysteme etat) {
    switch (etat) {
        case ETAT_INIT:      return "INIT";
        case ETAT_NOMINAL:   return "NOMINAL";
        case ETAT_WARNING:   return "WARNING";
        case ETAT_CRITICAL:  return "CRITICAL";
        case ETAT_SAFE_MODE: return "SAFE_MODE";
        default:             return "INCONNU";
    }
}

void thermal_envoyer_telemetrie(Queue *queue_telemetry, uint32_t timestamp) {
    Message msg;
    msg.type = MSG_RAPPORT_ETAT;
    msg.etat = etat_actuel;
    msg.timestamp_ms = timestamp;
    
    if (queue_envoyer(queue_telemetry, msg)) {
        printf("[THERMAL] Message envoye a telemetrie\n");
    }
}