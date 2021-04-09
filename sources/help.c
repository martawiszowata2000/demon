#include "help.h"
#include <stdio.h>

void help(bool full){
    printf("Poprawne uzycie programu:\n\n");
    printf(" (1) demon <sciezka zrodlowa> <sciezka docelowa> [opcje]\n");
    printf(" (2) demon <opcja>\n\n");

    printf("Jezeli chcesz uruchomic demona i rozpoczac synchronizacje, uzyj rozwiazania (1).\n");
    printf("Jezeli chcesz wymusic synchronizacje lub zatrzymaa uruchomionego demona, uzyj rozwiazania (2).\n");

    if(!full){
        printf("\nWyswietlanie pomocy: demon -h\n");
        return;
    }

    printf("\nOpis opcji stosowanych w (1) przypadku uzycia:\n\n");
    help_params();

    printf("\nOpis opcji stosowanych w (2) przypadku uzycia:\n\n");
    help_actions();
}

void help_params(){
    printf(" -R : kopiowanie rekursywne\n\n");

    printf(" -s <liczba>[jednostka] : czas uspienia demona, jednostki:\n");
    printf("  [ brak ] : to samo co s\n");
    printf("  s : sekundy\n");
    printf("  m : minuty\n");
    printf("  h : godziny\n\n");

    printf(" -m <liczba>[jednostka] : pliki tego rozmiaru i wieksze beda kopiowane za pomocą mmap, jednostki:\n");
    printf("  [ brak ] : bajty\n");
    printf("  k : kilobajty \n");
    printf("  m : megabajty\n");
    printf("  g : gigabajty\n");
}

void help_actions(){
    printf(" -F : powoduje natychmiastowe wybudzenie demona (synchronizacje)\n");
    printf(" -S : bezpieczne zakonczenie pracy demona (konczy synchronizacje, jesli rozpoczeta)\n");
    printf(" -K : natychmiastowe zakonczenie pracy demona.\n");
}
