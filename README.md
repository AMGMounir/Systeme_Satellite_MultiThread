# Système de Contrôle Thermique Satellite Multi-Thread

Système embarqué temps réel en C pur simulant un contrôleur thermique pour satellite, développé avec POSIX threads et optimisé pour les contraintes spatiales.

![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![POSIX](https://img.shields.io/badge/POSIX-Threads-green?style=for-the-badge)

---

## Table des matières

- [Vue d'ensemble](#vue-densemble)
- [Fonctionnalités](#fonctionnalités)
- [Architecture](#architecture)
- [Installation](#installation)
- [Compilation et exécution](#compilation-et-exécution)
- [Structure du projet](#structure-du-projet)
- [Concepts embarqués implémentés](#concepts-embarqués-implémentés)
- [Analyse mémoire](#analyse-mémoire)
- [Démo](#démo)
- [Évolutions futures](#évolutions-futures)
- [Auteur](#auteur)

---

## Vue d'ensemble

Ce projet simule un **système de contrôle thermique** pour satellite avec :
- **4 threads concurrents** s'exécutant en temps réel
- **Communication inter-tâches** via files de messages (queues)
- **Surveillance système** avec watchdog timer
- **Machine à états** (FSM) pour la gestion des modes
- **Timing précis** avec détection de dépassement de deadline
- **Optimisations mémoire** pour environnement contraint (~1.1 KB)

---

## Fonctionnalités

### Système multi-thread
- **Thread Capteurs** : Lecture de 4 capteurs de température (1 Hz)
- **Thread Contrôle** : Analyse thermique et décision (1 Hz)
- **Thread Télémétrie** : Traitement et affichage des messages (0.5 Hz)
- **Thread Watchdog** : Surveillance des threads (1 Hz)

### Gestion thermique intelligente
- Activation automatique du **chauffage** si température moyenne < 10°C
- Activation automatique du **radiateur** si température moyenne > 40°C
- Machine à états : `INIT → NOMINAL → WARNING → CRITICAL → SAFE_MODE`

### Robustesse
- **Mutex** pour protéger les données partagées
- **Watchdog** avec timeout de 3 secondes
- **Détection de pannes** capteurs (10% de taux de panne simulé)
- **Détection de retard** sur tâches périodiques

### Optimisations embarquées
- Structures `__attribute__((packed))` pour réduire le padding
- Données constantes en ROM (`const`)
- Allocation statique (pas de `malloc`)
- Variables partagées marquées `volatile`

---


### Flux de données
```
[Capteurs] ──┐
             ├──> [Mutex] ──> [derniers_capteurs[]]
             │                        │
             └──> [Queue] ─────────> [Télémétrie]
                     ▲
                     │
              [Contrôle thermique]
```

---

## Installation

### Prérequis

- **OS** : Linux / WSL (Windows Subsystem for Linux)
- **Compilateur** : GCC avec support pthread
- **Make** : Pour la compilation automatisée

### Installation sur Ubuntu/Debian
```bash
sudo apt update
sudo apt install gcc make build-essential
```

### Installation sur WSL (Windows)
```powershell
# Dans PowerShell Admin
wsl --install

# Puis dans Ubuntu WSL
sudo apt update
sudo apt install gcc make
```

---

## Compilation et exécution

### Clone du projet
```bash
git clone https://github.com/username/satellite-thermal-control.git
cd satellite-thermal-control
```

### Compilation
```bash
make
```

### Exécution
```bash
make run
```

### Nettoyage
```bash
make clean
```

---

## Structure du projet
```
thermal_satellite/
├── include/
│   ├── config.h         # Manipulation de bits (registres)
│   ├── historique.h     # Buffer circulaire
│   ├── memory.h         # Analyse mémoire
│   ├── queue.h          # Files de messages thread-safe
│   ├── sensors.h        # Interface capteurs
│   ├── telemetry.h      # Télémétrie
│   ├── thermal.h        # Contrôle thermique + FSM
│   ├── timing.h         # Timing précis
│   ├── types.h          # Structures de données
│   └── watchdog.h       # Watchdog timer
│
├── src/
│   ├── config.c         # Gestion registre de configuration
│   ├── historique.c     # Implémentation buffer circulaire
│   ├── memory.c         # Calculs mémoire
│   ├── queue.c          # Queues avec mutex intégrés
│   ├── sensors.c        # Simulation capteurs
│   ├── telemetry.c      # Traitement télémétrie
│   ├── thermal.c        # Machine à états + contrôle
│   ├── timing.c         # Fonctions de timing précis
│   ├── watchdog.c       # Surveillance threads
│   └── main.c           # Point d'entrée + orchestration
│
├── Makefile             # Compilation automatisée
└── README.md            # Ce fichier
```

---

## Concepts embarqués implémentés

### 1. **Threads POSIX (pthread)**

Gestion de 4 threads concurrents avec synchronisation.
```c
pthread_t tid_capteurs, tid_controle, tid_telemetrie, tid_watchdog;
pthread_create(&tid_capteurs, NULL, thread_capteurs, NULL);
```

### 2. **Mutex (exclusion mutuelle)**

Protection des données partagées contre les conditions de course.
```c
pthread_mutex_lock(&mutex_capteurs);
derniers_capteurs[i] = donnees;
pthread_mutex_unlock(&mutex_capteurs);
```

### 3. **Files de messages (Queues)**

Communication asynchrone inter-tâches avec buffer circulaire FIFO.
```c
typedef struct {
    Message messages[TAILLE_QUEUE];
    uint8_t index_ecriture;
    uint8_t index_lecture;
    uint8_t nombre_messages;
    pthread_mutex_t mutex;  // Thread-safe
} Queue;
```

### 4. **Machine à états (FSM)**

Gestion des modes système avec transitions contrôlées.
```c
typedef enum {
    ETAT_INIT,        // Initialisation
    ETAT_NOMINAL,     // Fonctionnement normal
    ETAT_WARNING,     // Alerte légère (1 capteur)
    ETAT_CRITICAL,    // Alerte critique (2+ capteurs)
    ETAT_SAFE_MODE    // Mode survie
} EtatSysteme;
```

**Règles de transition :**
- 0 alerte → `NOMINAL`
- 1 alerte → `WARNING`
- 2+ alertes → `CRITICAL`
- 2 cycles consécutifs en `CRITICAL` → `SAFE_MODE` (irréversible)

### 5. **Buffer circulaire**

Historique des 10 dernières mesures par capteur.
```c
typedef struct {
    DonneesCapteur donnees[TAILLE_HISTORIQUE];  // 10 mesures
    uint8_t index_ecriture;
    uint8_t nombre_elements;
} HistoriqueCapteur;
```

**Principe :** Quand plein, écrase les anciennes données (FIFO).

### 6. **Watchdog Timer**

Détection de threads bloqués ou plantés.
```c
#define WATCHDOG_TIMEOUT_MS 3000  // 3 secondes

void watchdog_ping(uint8_t thread_id);  // Thread se signale
void watchdog_check(uint32_t current_time_ms);  // Vérification
```

**Fonctionnement :**
- Chaque thread doit appeler `watchdog_ping()` régulièrement
- Si pas de ping pendant > 3s → Alerte système

### 7. **Manipulation de bits**

Registre de configuration 8 bits pour contrôler le hardware.
```c
typedef struct {
    uint8_t registre;  // 1 byte = 8 bits
} ConfigSysteme;

// Bits :
// 0: Chauffage ON/OFF
// 1: Radiateur ON/OFF
// 2: Mode économie d'énergie
// 3: Télémétrie activée
```

**Opérations :**
```c
config_activer_bit(&config, BIT_CHAUFFAGE);    // OR
config_desactiver_bit(&config, BIT_RADIATEUR); // AND + NOT
bool actif = config_bit_actif(&config, BIT);   // AND + test
```

### 8. **Keyword `volatile`**

Variables partagées entre threads sans mutex (flags de contrôle).
```c
volatile bool system_running = true;
```

**Pourquoi ?** Empêche le compilateur d'optimiser les lectures en cache. Force la relecture depuis la RAM à chaque accès.

### 9. **Timing précis**

Tâches périodiques avec deadline exacte (pas de dérive).
```c
uint64_t next_execution = timing_get_ms();
const uint32_t PERIOD_MS = 1000;

while (system_running) {
    // instruc
    
    next_execution += PERIOD_MS;
    timing_wait_until(next_execution);  // Attente précise
}
```

**Avantage :** Période exacte de 1000 ms ±quelques µs, même si le travail prend 200 ms.

### 10. **Optimisations mémoire**

#### `__attribute__((packed))`
Élimine le padding des structures.
```c
typedef struct __attribute__((packed)) {
    uint8_t id;              // 1 byte
    float temperature;       // 4 bytes
    uint32_t timestamp_ms;   // 4 bytes
    bool est_valide;         // 1 byte
} DonneesCapteur;  // Total : 10 bytes (au lieu de 16)
```

#### `const` et `static const`
Données en ROM (pas en RAM).
```c
static const float TEMP_RANGES[4][2] = {
    {40.0f, 60.0f},   // ROM, pas RAM !
    // ...
};
```

---

## Analyse mémoire

### Empreinte mémoire totale : **~1.1 KB**
```
Structures de base :
  DonneesCapteur        : 10 octets  (packed)
  Message               : 24 octets
  WatchdogEntry         : 13 octets  (packed)
  ConfigSysteme         : 1 octet

Structures complexes :
  HistoriqueCapteur     : 102 octets
  Queue                 : 528 octets

Tableaux globaux :
  4x HistoriqueCapteur  : 408 octets
  4x DonneesCapteur     : 40 octets
  1x Queue              : 528 octets

Total estimé : 1137 octets (~1.11 KB)
```

**Comparaison :**
- Sans optimisations : ~1.8 KB
- Avec optimisations : ~1.1 KB
- **Gain : 38%** 

---

## Démo

### Sortie normale
```
=== Système Satellite Multi-Thread ===

[THERMAL] Initialisation...
[WATCHDOG] Initialisation - Timeout: 3000 ms
[THREAD CAPTEURS] Démarrage...
[THREAD CONTRÔLE] Démarrage...
[THREAD TÉLÉMÉTRIE] Démarrage...
[THREAD WATCHDOG] Démarrage - Surveillance active

[CAPTEURS] Lecture cycle (t=0 ms)
  Capteur 0 :  45.23°C [OK     ] (moy: 45.23°C)
  Capteur 1 : -12.34°C [OK     ] (moy: -12.34°C)
  Capteur 2 :  18.56°C [OK     ] (moy: 18.56°C)
  Capteur 3 :  15.67°C [OK     ] (moy: 15.67°C)

[CONTRÔLE] Analyse thermique (t=0 ms)
[THERMAL] Transition: INIT → NOMINAL
  >>> État système: NOMINAL
```

### Détection d'alerte
```
[CAPTEURS] Lecture cycle (t=4000 ms)
  Capteur 0 :  45.23°C [OK     ]
  Capteur 1 :  88.34°C [ALERTE!]  ← Panne détectée
  Capteur 2 :  19.12°C [OK     ]
  Capteur 3 :  92.45°C [ALERTE!]  ← Panne détectée

[CONTRÔLE] Analyse thermique (t=4000 ms)
[THERMAL] Radiateur activé (moy: 61.3°C)
[THERMAL] Transition: NOMINAL → CRITICAL (alertes: 2)
  >>> État système: CRITICAL

[TÉLÉMÉTRIE] Traitement des messages
[TELEMETRY] ALERTE #1 (t=4000) : Capteur 1 = 88.34°C
[TELEMETRY] ALERTE #2 (t=4000) : Capteur 3 = 92.45°C
```

### Passage en Safe Mode
```
[CONTRÔLE] Analyse thermique (t=8000 ms)
[THERMAL] !!! PASSAGE EN SAFE MODE !!!
[THERMAL] Transition: CRITICAL → SAFE_MODE
[Config: 0x0E = 0000 1110]
    Chauffage : OFF
    Radiateur : ON
    Mode Eco  : ON
    Télémétrie: ON
```

### Watchdog - Détection de panne thread

Si un thread ne répond plus :
```
[WATCHDOG] TIMEOUT sur thread CAPTEURS ! (3124 ms sans signal)
[WATCHDOG] ALERTE SYSTÈME - Threads bloqués détectés !
```

---

## Tests

### Test 1 : Fonctionnement nominal
```bash
make run
# Résultat attendu : Aucune alerte, système en NOMINAL
```

### Test 2 : Simulation panne capteur
Les capteurs ont 10% de chance de panne → Alertes aléatoires

### Test 3 : Watchdog
Commentez `watchdog_ping(THREAD_ID_CAPTEURS)` dans `src/main.c` :
```c
// watchdog_ping(THREAD_ID_CAPTEURS);  // Commenté
```
Résultat : Alerte watchdog après ~3 secondes.

### Test 4 : Analyse mémoire
À la fin de l'exécution, l'analyse mémoire s'affiche automatiquement.

---

## Configuration

### Modifier les périodes des threads

Dans `src/main.c` :
```c
const uint32_t PERIOD_MS = 1000;  // Capteurs : 1 Hz
const uint32_t PERIOD_MS = 2000;  // Télémétrie : 0.5 Hz
```

### Modifier le timeout watchdog

Dans `include/types.h` :
```c
#define WATCHDOG_TIMEOUT_MS 3000  // 3 secondes
```

### Modifier les limites de température

Dans `include/types.h` :
```c
#define TEMP_MIN_SAFE -30.0f
#define TEMP_MAX_SAFE 70.0f
```

### Modifier la taille de la queue

Dans `include/types.h` :
```c
#define TAILLE_QUEUE 20  // 20 messages max
```

---

## Ressources et références

### Documentation POSIX Threads
- [POSIX Threads Programming](https://computing.llnl.gov/tutorials/pthreads/)
- [pthread.h man pages](https://man7.org/linux/man-pages/man7/pthreads.7.html)

### Standards spatiaux
- **ECSS** (European Cooperation for Space Standardization)
- **DO-178C** (Software Considerations in Airborne Systems)

---

## Évolutions futures

### Phase 1 : Hardware réel
- [ ] Porter sur **STM32 Nucleo-F446RE**
- [ ] Implémenter avec **FreeRTOS**
- [ ] Vrais capteurs (DHT22, DS18B20)
- [ ] Communication UART pour télémétrie

### Phase 2 : Fonctionnalités avancées
- [ ] Protocole de communication (CCSDS)
- [ ] Chiffrement des données télémétrie
- [ ] Compression des logs
- [ ] Interface GUI (Python + matplotlib)

### Phase 3 : Robustesse
- [ ] Tests unitaires (Unity framework)
- [ ] Couverture de code (gcov)
- [ ] Analyse statique (Cppcheck, Clang-Tidy)
- [ ] Certification DO-178C/ECSS


---

## Licence

Ce projet est sous licence MIT - voir le fichier [LICENSE](LICENSE) pour plus de détails.

---

## Auteur

**Mounir AMGHAR**
- Systèmes embarqués pour applications spatiales
- LinkedIn : [Mounir AMGHAR](https://www.linkedin.com/in/mounir-amghar/)
- Email : end.munir@gmail.com

---

## Statistiques du projet
```
Lignes de code   : ~2000
Fichiers         : 20
Modules          : 10
Threads          : 4
Mémoire utilisée : 1.1 KB
Temps développé  : 2-3 semaines
```
