#include "sync.h"
#include "file.h"
#include "timestamp.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>


void sync_all(config c){
    // Usuwa pliki których nie ma w sciezce zrodlowej
    usun_pliki(c);

    // Czyta pliki do synchronizacji do listy oraz tworzy punktu powrotu
    listaPlikow *list = czytaj_zawartosc_katalogu(c.sciezkaZrodlowa, c.rekursywnaSynch);
    listaPlikow *punktPowrotu = list;

    while(list->next != NULL){
        list = list->next;
        bool mmap_on;

        // Tworzy pelna sciezke docelowa pliku
        int len = strlen(list->sciezka) + strlen(list->nazwa) - strlen(c.sciezkaZrodlowa) + strlen(c.sciezkaDocelowa) + 2;
        char pelnaSciezkaDocelowa[len];
        snprintf(pelnaSciezkaDocelowa, len, "%s%s/%s", c.sciezkaDocelowa, list->sciezka + strlen(c.sciezkaZrodlowa), list->nazwa);

        // Tworzy pelna sciezke zrodlowa pliku
        len = strlen(list->sciezka);
        len += strlen(list->nazwa) + 2;
        char pelnaSciezkaZrodlowa[len];
        snprintf(pelnaSciezkaZrodlowa, len, "%s/%s", list->sciezka, list->nazwa);

        // Sprawdza rozmiar pliku
        struct stat st;
        stat(pelnaSciezkaZrodlowa, &st);
        int size = st.st_size;
        if(size >= c.prog) {
            syslog(LOG_INFO, "Uzywam mmap");
            mmap_on = true;
        }

        // Porownuje timestampy oraz aktualizuje pliki na ich podstawie 
        if(czy_istnieje(pelnaSciezkaDocelowa)){
            if(list->typ == DIRECTORY){
                if(!porownaj_timestamp(pelnaSciezkaZrodlowa, pelnaSciezkaDocelowa)){
                    kopiuj_timestamp(pelnaSciezkaZrodlowa, pelnaSciezkaDocelowa);
                }
            }
            else if(list->typ == REGULAR_FILE){
                if(!porownaj_timestamp(pelnaSciezkaZrodlowa, pelnaSciezkaDocelowa)){
                    syslog(LOG_INFO, "Plik nieaktualny, usuwam: %s\n", pelnaSciezkaDocelowa);
                    remove(pelnaSciezkaDocelowa);
                    syslog(LOG_INFO, "Kopiuje plik do: %s\n", pelnaSciezkaDocelowa);
                    kopiuj_opcje(pelnaSciezkaZrodlowa, pelnaSciezkaDocelowa, mmap_on);
                    kopiuj_timestamp(pelnaSciezkaZrodlowa, sciezkaDocelowa);
                }
            }
        }
        // Kopiuje pozostale pliki i/lub katalogi
        else{
            if(list->typ == DIRECTORY){
                syslog(LOG_INFO, "Tworzę katalog: %s\n", pelnaSciezkaDocelowa);
                mkdir(pelnaSciezkaDocelowa, 0700); // 0700 - uzytkownik ma wszystkie prawa
                kopiuj_timestamp(pelnaSciezkaZrodlowa, pelnaSciezkaDocelowa);
            }
            else{
                syslog(LOG_INFO, "Kopiuje plik do: %s\n", pelnaSciezkaDocelowa);
                kopiuj_opcje(pelnaSciezkaZrodlowa, pelnaSciezkaDocelowa, mmap_on);
                kopiuj_timestamp(pelnaSciezkaZrodlowa, pelnaSciezkaDocelowa);
            }
        }
    }
    // Usuwa listy
    lista_czysc(punktPowrotu);
}
