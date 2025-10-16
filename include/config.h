#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"
void config_init(ConfigSysteme *cfg);

void config_activer_bit(ConfigSysteme *cfg, uint8_t bit);

void config_desactiver_bit(ConfigSysteme *cfg, uint8_t bit);

bool config_bit_actif(ConfigSysteme *cfg, uint8_t bit);

void config_afficher(ConfigSysteme *cfg);

#endif
