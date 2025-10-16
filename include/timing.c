#define _POSIX_C_SOURCE 199309L

#include "../include/timing.h"
#include <time.h>
#include <stdio.h>

uint64_t timing_get_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)(ts.tv_sec * 1000) + (ts.tv_nsec / 1000000);
}

void timing_wait_until(uint64_t target_time_ms) {
    uint64_t now = timing_get_ms();
    
    if (target_time_ms <= now) {
        uint64_t retard = now - target_time_ms;
        
        // Logger seulement si retard > 10ms (significatif)
        if (retard > 10) {  // ← AJOUTER CE TEST
            printf("[TIMING] Retard detecte : %llu ms\n", retard);
        }
        return;
    }
    
    uint64_t sleep_duration = target_time_ms - now;
    
    struct timespec ts;
    ts.tv_sec = sleep_duration / 1000;
    ts.tv_nsec = (sleep_duration % 1000) * 1000000;
    
    nanosleep(&ts, NULL);
}

void timing_sleep_ms(uint32_t ms) {
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000;
    nanosleep(&ts, NULL);
}