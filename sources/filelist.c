#include "filelist.h"
#include <string.h>

listaPlikow *lista_utworz(){
    listaPlikow *lista = malloc(sizeof(listaPlikow));
    lista->next = NULL;
    lista->nazwa = NULL;
    lista->sciezka = NULL;
    return lista;
}

listaPlikow *lista_dodaj(listaPlikow *lista, char *nazwa, char *sciezka, FILE_TYPE typ, bool mmap){
    while(lista->next != NULL) lista = lista->next;
    lista->next = malloc(sizeof(listaPlikow));
    lista = lista->next;
    lista->nazwa = malloc(strlen(nazwa)+1);
    strcpy(lista->nazwa, nazwa);
    lista->sciezka = malloc(strlen(sciezka)+1);
    strcpy(lista->sciezka, sciezka);
    lista->czyMmap = mmap;
    lista->next = NULL;
    lista->typ = typ;
    return lista;
}

void lista_czysc(listaPlikow *first){
    if(first == NULL) return;
    while(first->next != NULL){
        listaPlikow *prev = first;
        first = first->next;
        free(prev->nazwa);
        free(prev->sciezka);
        free(prev);
    }
    free(first);
}

listaPlikow *lista_polacz(listaPlikow *first, listaPlikow *next){
    while(first->next != NULL) first = first->next;
    first->next = next->next;
    free(next->sciezka);
    free(next->nazwa);
    free(next);
    return first;
}

listaPlikow *lista_odwroc(listaPlikow *lista){
    listaPlikow *first = NULL, *next = NULL;
    while(lista->next != NULL){
        lista = lista->next;
        next = first;
        first = malloc(sizeof(listaPlikow));
        first->next = next;
        first->nazwa = malloc(strlen(lista->nazwa)+1);
        strcpy(first->nazwa, lista->nazwa);
        first->sciezka = malloc(strlen(lista->sciezka)+1);
        strcpy(first->sciezka, lista->sciezka);
        first->czyMmap = lista->czyMmap;
        first->typ = lista->typ;
    }
    next = first;
    first = malloc(sizeof(listaPlikow));
    first->next = next;
    first->nazwa = NULL;
    first->sciezka = NULL;
    return first;
}
