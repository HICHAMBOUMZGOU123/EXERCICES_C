#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

int enAttente = 0;
int compteurInterruptions = 0;

void gererInterruption() {
    if (enAttente == 0) {
        compteurInterruptions = 1;
        enAttente = 1;
        printf("\nPremière interruption. Tapez à nouveau Ctrl+C pour arrêter.\n");
        alarm(2);
    } else { 
        printf("\nFin du programme.\n");
       return;
    }
}

void gererAlarme() {
    if (enAttente == 1) {
        enAttente = 0;        
        compteurInterruptions = 0; 
        printf("\nTemps expiré\n");
    }
}

int main() {
    signal(SIGINT, gererInterruption);
    signal(SIGALRM, gererAlarme);

    while (1) {
        if (enAttente == 0) {
            printf("working\n");
        }
        sleep(1);
    }
    
    return 0;
}
