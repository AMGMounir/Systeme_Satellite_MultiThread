#include "historique.h"
#include <stdio.h>

void historique_init(HistoriqueCapteur *hist) {
    hist->index_ecriture = 0;
    hist->nombre_elements = 0;
}

void historique_ajouter(HistoriqueCapteur *hist, DonneesCapteur donnee) {
    // ecrire a la position actuelle
    hist->donnees[hist->index_ecriture] = donnee;
    
    // Avancer l'index (circulaire : retour à 0 après 9)
    hist->index_ecriture = (hist->index_ecriture + 1) % TAILLE_HISTORIQUE;
    
    // Incrementer le compteur (maximum TAILLE_HISTORIQUE)
    if (hist->nombre_elements < TAILLE_HISTORIQUE) {
        hist->nombre_elements++;
    }
}

float historique_moyenne(HistoriqueCapteur *hist) {
    if (hist->nombre_elements == 0) {
        return 0.0f;
    }
    
    float somme = 0.0f;
    
    // Parcourir seulement les elements valides
    for (int i = 0; i < hist->nombre_elements; i++) {
        somme += hist->donnees[i].temperature;
    }
    
    return somme / hist->nombre_elements;
}

void historique_afficher(HistoriqueCapteur *hist) {
    printf("  [Historique: %d mesures]\n", hist->nombre_elements);
    
    for (int i = 0; i < hist->nombre_elements; i++) {
        DonneesCapteur d = hist->donnees[i];
        printf("    [%d] t=%u : %.2f C %s\n", 
               i,
               d.timestamp_ms,
               d.temperature,
               d.est_valide ? "OUI" : "NON");
    }
}