#ifndef DIR_H
#define DIR_H

#include "bool.h"
#include "filelist.h"
#include "filetype.h"
#include "config.h"

// Sprawdza jakiego typu jest plik
FILE_TYPE pobierz_typ_pliku(const char* sciezka);

// Sprawdza czy podany katalog jest katalogiem, czy istnieje i czy użytkownik ma dostęp do niego
// jeśli nie to wyświetla komunikat 
bool katalog_uprawnienia_error(const char *nazwa);

// Sprawdza czy jeden katalog jest zawarty w drugim 
bool czy_zawiera(const char *katalog1, const char *katalog2);

// Dziala tak samo jak katalog_uprawnienia_error, ale nie wyświetla komunikatu
 * Nie wyświetla żadnych komunikatów. */
bool katalog_uprawnienia(const char *nazwa);

// Sprawdza czy plik istnieje
bool czy_istnieje(const char *nazwa);

// Zwraca zawartość katalogu przekazanego w parametrach oraz jego podkatalogów
listaPlikow* czytaj_zawartosc_katalogu(char *sciezka, bool rekursywne);

// Tworzy pusty plik 
void utworz_pusty_plik(char *sciezka);

// Kopiuje zawartosc pliku (bufor)
void kopiuj_plik(char* sciezka_zrodlowy, char* sciezka_docelowy);

// Kopiuje zawartosc pliku (mmap)
void kopiuj_duzy_plik(char* sciezka_zrodlowy, char* sciezka_docelowy);

// Wybiera funkcje kopiujaca kompatybilna do rozmiaru pliku
void kopiuj_opcje(char *sciezka_zrodlowy, char *sciezka_docelowy, bool czy_duzy_plik);

// Zapisuje cały bufor do pliku - uzywana w kopiuj_plik
ssize_t zapisz_bufor(int fd, const void* bufor, size_t count);

// Usuwa pliki i foldery zależnie od tego czyistnieja w katalogu źródłowym 
void usun_pliki(config c);


#endif
