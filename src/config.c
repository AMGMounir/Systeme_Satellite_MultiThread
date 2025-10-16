#include "config.h"
#include <stdio.h>

void config_init(ConfigSysteme *cfg) {
    cfg->registre=0b00000000;
}

void config_activer_bit(ConfigSysteme *cfg, uint8_t bit) {
    cfg->registre |= (1<<bit);
}

void config_desactiver_bit(ConfigSysteme *cfg, uint8_t bit) {
    cfg->registre &= ~(1<<bit);
}

bool config_bit_actif(ConfigSysteme *cfg, uint8_t bit) {
    return (cfg->registre & (1<<bit)) != 0;
}

void config_afficher(ConfigSysteme *cfg) {
    printf("[Config: 0x%02X = ", cfg->registre);
    
    // Afficher en binaire (de gauche à droite, bit 7 à 0)
    for (int i = 7; i >= 0; i--) {
        printf("%d", (cfg->registre >> i) & 1);
        if (i == 4) printf(" ");  // Espace au milieu pour lisibilite
    }
    printf("]\n");
    
    // Afficher l'etat de chaque système
    printf("Chauffage : %s\n", config_bit_actif(cfg, BIT_CHAUFFAGE) ? "ON" : "OFF");
    printf("Radiateur : %s\n", config_bit_actif(cfg, BIT_RADIATEUR) ? "ON" : "OFF");
    printf("Mode Eco  : %s\n", config_bit_actif(cfg, BIT_MODE_ECO) ? "ON" : "OFF");
    printf("Telemetrie: %s\n", config_bit_actif(cfg, BIT_TELEMETRIE) ? "ON" : "OFF");
}