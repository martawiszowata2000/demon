#ifndef DIR_H
#define DIR_H

#include "bool.h"
#include "filelist.h"
#include "filetype.h"
#include "config.h"

/* Sprawdza jakiego typu jest plik podany w parametrze:
 * symlink, katalog, zwykły plik itp. */
FILE_TYPE pobierz_typ_pliku(const char* sciezka);

/* Sprawdza czy podany katalog rzeczywiście jest katalogiem,
 * czy istnieje oraz czy użytkownik ma prawo otworzyć ten katalog.
 * w razie błędu wyświetla komunikat na stdout. */
bool katalog_uprawnienia_error(const char *nazwa);

/* Sprawdza czy jeden z katalogów zawiera drugi */
bool czy_zawiera(const char *katalog1, const char *katalog2);

/* Sprawdza czy podany katalog rzeczywiście jest katalogiem,
 * czy istnieje oraz czy użytkownik ma prawo otworzyć ten katalog.
 * Nie wyświetla żadnych komunikatów. */
bool katalog_uprawnienia(const char *nazwa);

/* Sprawdza czy plik istnieje */
bool czy_istnieje(const char *nazwa);

/* Zwraca zawartość katalogu podanego jako parametr
 * oraz jego wszystkich podkatalogów. */
listaPlikow* czytaj_zawartosc_katalogu(char *sciezka, bool rekursywne);

/* Tworzy pusty plik */
void utworz_pusty_plik(char *sciezka);

/* Kopiuje zawartość jednego pliku i tworzy identyczną kopię */
void kopiuj_plik(char* sciezka_zrodlowy, char* sciezka_docelowy);

/*Kopiuje plik za pomocą mmap */
void kopiuj_duzy_plik(char* sciezka_zrodlowy, char* sciezka_docelowy);

/* Wybór odpowiedniego kopiowania */
void kopiuj_opcje(char *sciezka_zrodlowy, char *sciezka_docelowy, bool czy_duzy_plik);

/* Zapisuje cały bufor do file descriptora */
ssize_t zapisz_bufor(int fd, const void* bufor, size_t count);

/*Usuwa pliki i foldery na podstawie ich obecności w katalogu źródłowym */
void usun_pliki(config c);


#endif
