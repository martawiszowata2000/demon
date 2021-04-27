#include "help.h"
#include <stdio.h>

void help(bool full){
    printf("Poprawne uzycie programu:\n\n");
    printf("1. Aby uruchomic demona i zaczac synchronizacje:\n");
    printf("demon <sciezka zrodlowa> <sciezka docelowa> [opcje]\n\n");
    printf("2. Aby wymusic synchronizacje lub zatrzymac uruchomionego demona:\n");
    printf("demon <opcja>\n\n");

    if(!full){
        printf("Aby wyswietlic pomoc: demon -h\n");
        return;
    }

    printf("\nOpcje stosowane w 1. przypadku:\n\n");
    help_params();

    printf("\nOpcje stosowane w 2. przypadku:\n\n");
    help_actions();
}

void help_params(){
    printf(" -R : kopiowanie rekursywne\n\n");

    printf(" -s <liczba>[jednostka] : czas uspienia demona, jednostki:\n");
    printf("  [ brak ] : patrz: s\n");
    printf("  s : sekundy\n");
    printf("  m : minuty\n");
    printf("  h : godziny\n\n");

    printf(" -m <liczba>[jednostka] : pliki tego rozmiaru i wieksze kopiowane sa za pomoca mmap, jednostki:\n");
    printf("  [ brak ] : bajty\n");
    printf("  k : kilobajty \n");
    printf("  m : megabajty\n");
    printf("  g : gigabajty\n");
}

void help_actions(){
    printf(" -F : wybudzenie demona (synchronizacja)\n");
    printf(" -S : bezpieczne zakonczenie pracy demona (konczy synchronizacje, jesli rozpoczeta)\n");
    printf(" -K : zakonczenie pracy demona.\n");
}
