#ifndef CONFIG_H
#define CONFIG_H

#include <stdlib.h>
#include "bool.h"

//struktura przechowywujaca biezaca konfiguracje programu

typedef struct {
    bool czyPrawidlowa; // czy dana konfiguracja jest prawidlowa
    int czasSpania; // czas spania procesu podany w sekundach
    bool rekursywnaSynch; // czy synchronizacja ma byc rekursywna
    size_t prog; // prog dzielacy pliki na male i duze - w bajtach
    char *sciezkaZrodlowa; // sciezka do folderu zrodlowego
    char *sciezkaDocelowa; // sciezka do folderu docelowego
} config;
config domyslna_konfiguracja(); // funkcja zwracajaca domyslna konfiguracje

#endif
  
