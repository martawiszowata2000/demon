#include "parse.h"

#include <string.h>
#include <stdio.h>

int parse_time(const char *czas){
    int sekundy = 0;
    for(int i=0; i<strlen(czas)-1; i++){
        if(czas[i] >=48 && czas[i] <= 57){ 
            sekundy *= 10;
            sekundy += (czas[i]-48); 
        }
        else return -1; 
    }
    char ostatni = czas[strlen(czas)-1]; 
    if(ostatni >=48 && ostatni <= 57){ 
        sekundy *= 10;
        sekundy += (ostatni-48);
    }
    else { 
        switch (ostatni) {
        case 's':
            // sekundy - nic nie zmieniamy
            break;
        case 'm':
            sekundy *= 60; // Minuta
            break;
        case 'h':
            sekundy *= 3600; // 60s * 60m = 1h
            break;
        default:
            return -1; // bledny znak
            break;
        }
    }
    return sekundy;
}

size_t parse_size(const char *rozmiar){
    size_t bity = 0;
    for(int i=0; i<strlen(rozmiar)-1; i++){
        if(rozmiar[i] >=48 && rozmiar[i] <= 57){ 
            bity *= 10;
            bity += (rozmiar[i]-48); 
        }
        else return -1; /
    }
    char ostatni = rozmiar[strlen(rozmiar)-1]; 
    if(ostatni >=48 && ostatni <= 57){ 
        bity *= 10;
        bity += (ostatni-48);
    }
    else {
        switch (ostatni) {
        case 'K':
        case 'k':
            bity *= 1024; // Kilobajt
            break;

        case 'M':
        case 'm':
            bity *= 1024*1024; // Megabajt
            break;

        case 'G':
        case 'g':
            bity *= 1024*1024*1024; // Gigabajt
            break;

        default:
            return -1; // bledny znak
            break;
        }
    }
    return bity;
}

config parse_args(int argc, char *argv[]){
    config c = domyslna_konfiguracja();
    if(argc <3) return c;
    int il_katalogow = 0;
    for(int i=1; i<argc; i++){
        char *arg = argv[i];
        if(arg[0] == '-'){
            char opcja = arg[1];
            if(strlen(arg) != 2){
                printf("blad - niepoprawna opcja: %s\n", arg);
                return c;
            }
            switch (opcja) {
            case 's':
                c.czasSpania = parse_time(argv[i+1]);
                if(c.czasSpania == -1){
                    printf("blad - niepoprawny format czasu: %s\n", argv[i+1]);
                    return c;
                }
                i++;
                break;

            case 'R':
                c.rekursywnaSynch = true;
                break;

            case 'm':
                c.prog = parse_size(argv[i+1]);
                if(c.prog == -1){
                    printf("blad - niepoprawny format rozmiaru: %s\n", argv[i+1]);
                    return c;
                }
                i++;
                break;

            default:
                printf("blad - niepoprawna opcja %c\n", opcja);
                return c;
                break;
            }
        }
        else {
            if(arg[strlen(arg)-1] == '/') arg[strlen(arg)-1] = '\0';
            if(il_katalogow == 0) c.sciezkaZrodlowa = arg;
            else if(il_katalogow == 1) c.sciezkaDocelowa = arg;
            il_katalogow++;
        }
    }
    if(il_katalogow == 2) c.czyPrawidlowa = true;
    return c;
}
