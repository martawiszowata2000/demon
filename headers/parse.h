#ifndef PARSE_H
#define PARSE_H

#include <stdlib.h>
#include "config.h"

// zamiania czas w postaci tekstowej na ilosc sekund
int parse_time(const char *czas);

// zamienia rozmiar w postaci tekstowej na ilosc bajtow
size_t parse_size(const char *rozmiar);

// ustawia biezaca konfiguracje na podstawie przekazanych parametrow
config parse_args(int argc, char *argv[]);

#endif
