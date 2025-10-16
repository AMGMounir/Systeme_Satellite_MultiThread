#include "sensors.h"
#include <stdlib.h>


// Données constantes en ROM au lieu de stack
static const float TEMP_RANGES[NUM_CAPTEURS][2] = {
    {40.0f, 60.0f},   // Capteur 0
    {-20.0f, 0.0f},   // Capteur 1
    {15.0f, 25.0f},   // Capteur 2
    {10.0f, 20.0f}    // Capteur 3
};

void sensors_init(void) {
    // Vide pour l'instant
}

DonneesCapteur lire_capteur(uint8_t id_capteur, uint32_t timestamp_ms) {
    DonneesCapteur donnees;
    donnees.id = id_capteur;
    donnees.timestamp_ms = timestamp_ms;
    donnees.est_valide = true;
    
    const float temp_min = TEMP_RANGES[id_capteur][0];
    const float temp_max = TEMP_RANGES[id_capteur][1];
    
    // Simulation de panne (10%)
    if (rand() % 100 < 10) {
        donnees.temperature = -50.0f + ((float)rand() / RAND_MAX) * 150.0f;
    } else {
        donnees.temperature = temp_min + ((float)rand() / RAND_MAX) * (temp_max - temp_min);
    }
    
    // Verifier validite
    if (donnees.temperature < TEMP_MIN_SAFE || donnees.temperature > TEMP_MAX_SAFE) {
        donnees.est_valide = false;
    }
    
    return donnees;
}

bool temperature_est_safe(float temperature) {
    return (temperature >= TEMP_MIN_SAFE && temperature <= TEMP_MAX_SAFE);
}