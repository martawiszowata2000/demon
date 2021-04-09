#ifndef FILELIST_H
#define FILELIST_H

#include <stdlib.h>
#include "bool.h"
#include "filetype.h"

typedef struct listaPlikow listaPlikow; // lista do przechowywania listy plikow w katalogu

struct listaPlikow {
  char *nazwa; // katalogu lub pliku
  char *sciezka; // do katalogu nadrzednego
  bool czyMmap; // czy uzywamy kopiowania duzych plikow (za pomoca mmap)
  FILE_TYPE typ; //typ pliku
  listaPlikow *next; //wskaznik na kolejny element
};

listaPlikow *lista_utworz();

//dodaje nowy element do listy i zwraca adres ostatniego elementu
listaPlikow *lista_dodaj(listaPlikow *lista, char *nazwa, char *sciezka, FILE_TYPE typ, bool czyMmap);

//dodaje druga liste na koniec pierwszej listy
listaPlikow *lista_polacz(listaPlikow *pierwsza, listaPlikow *kolejna);

listaPlikow *lista_odwroc(listaPlikow *lista);

void lista_czysc(listaPlikow *lista);

#endif
