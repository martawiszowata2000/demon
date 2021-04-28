#include "file.h"

#include <dirent.h> //katalogi
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <assert.h>
#include <unistd.h> //close
#include <fcntl.h> //open, read
#include <sys/mman.h> //mmap
#include <syslog.h>

FILE_TYPE pobierz_typ_pliku(const char *sciezka){
    struct stat st;
    stat(sciezka, &st);
    if(S_ISLNK(st.st_mode)) return SYMBOLIC_LINK;
    else if(S_ISDIR(st.st_mode)) return DIRECTORY;
    else if(S_ISCHR(st.st_mode)) return CHARACTER_DEVICE;
    else if(S_ISBLK(st.st_mode)) return BLOCK_DEVICE;
    else if(S_ISFIFO(st.st_mode))return FIFO;
    else if(S_ISSOCK(st.st_mode))return SOCKET;
    else if(S_ISREG(st.st_mode)) return REGULAR_FILE;
    else return UNKNOWN;//
}

bool katalog_uprawnienia_error(const char *nazwa){
    DIR* katalog = opendir(nazwa);
    if(katalog){
        closedir(katalog);
        return true;
    }
    else {
        perror(nazwa);
        return false;
    }
}

bool katalog_uprawnienia(const char *nazwa){
    DIR* katalog = opendir(nazwa);
    if(katalog){
        closedir(katalog);
        return true;
    }
    return false;
}

bool czy_istnieje(const char *nazwa){
    if(access(nazwa, F_OK) == 0) return true;
    else return false;
}

listaPlikow* czytaj_zawartosc_katalogu(char *sciezka, bool rekursywne){
    listaPlikow *lista = lista_utworz();
    DIR *katalog;
    struct dirent *wejscie;
    katalog = opendir(sciezka);
    while((wejscie = readdir(katalog)) != NULL){
        if(strcmp(wejscie->d_name, ".") == 0 || strcmp(wejscie->d_name, "..") == 0) continue;
        int dlugosc = strlen(sciezka);
        dlugosc += strlen(wejscie->d_name) + 2;
        char sciezka_szczegol[dlugosc];
        snprintf(sciezka_szczegol, dlugosc, "%s/%s", sciezka, wejscie->d_name);
        FILE_TYPE typ;
        typ = pobierz_typ_pliku(sciezka_szczegol);
        if(typ == REGULAR_FILE) lista_dodaj(lista, wejscie->d_name, sciezka, typ, true);
        if(typ == DIRECTORY && rekursywne){
            lista_dodaj(lista, wejscie->d_name, sciezka, typ, true);
            lista_polacz(lista, czytaj_zawartosc_katalogu(sciezka_szczegol, true));
        }
    }
    closedir(katalog);
    return lista;
}

void utworz_pusty_plik(char* sciezka){
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH; // uprawnienia do pliku (664)
    int fd = open(sciezka, O_WRONLY | O_EXCL | O_CREAT, mode); // tworzy pliku
    if (fd == -1) {
        syslog(LOG_CRIT, "blad - nie mozna stworzyc pliku");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void kopiuj_plik(char* sciezka_zrodlowy, char* sciezka_docelowy){
    unsigned char bufor[16];
    size_t offset = 0;
    size_t bytes_read;
    int i;

    int zrodlo_fd = open (sciezka_zrodlowy, O_RDONLY);
    utworz_pusty_plik (sciezka_docelowy);
    int docel_fd = open (sciezka_docelowy, O_WRONLY);

    do {
        bytes_read = read(zrodlo_fd, bufor, sizeof(bufor));
        zapisz_bufor(docel_fd, bufor, bytes_read);

        // Pilnuje pozycje w pliku
        offset += bytes_read;
    } while(bytes_read == sizeof(bufor));
    
    close(zrodlo_fd);
    close(docel_fd);
}

void kopiuj_duzy_plik(char *sciezka_zrodlowy, char *sciezka_docelowy){
    int zrodlo_fd, docel_fd;
    char *zrodlo, *docelowe;
    struct stat s;
    size_t rozmiar;

    // zrodlowy - mapowanie
    zrodlo_fd = open(sciezka_zrodlowy, O_RDONLY);
    rozmiar = lseek(zrodlo_fd, 0, SEEK_END);
    zrodlo = mmap(NULL, rozmiar, PROT_READ, MAP_PRIVATE, zrodlo_fd, 0);

    // docelowy - mapowanie
    docel_fd = open(sciezka_docelowy, O_RDWR | O_CREAT, 0666);
    ftruncate(docel_fd, rozmiar);
    docelowe = mmap(NULL, rozmiar, PROT_READ | PROT_WRITE, MAP_SHARED, docel_fd, 0);

    memcpy(docelowe, zrodlo, rozmiar);
    munmap(zrodlo, rozmiar);
    munmap(docelowe, rozmiar);

    close(zrodlo_fd);
    close(docel_fd);
}

void kopiuj_opcje(char *sciezka_zrodlowy, char *sciezka_docelowy, bool czy_duzy_plik){
    if(czy_duzy_plik) 
        kopiuj_duzy_plik(sciezka_zrodlowy, sciezka_docelowy);
    else 
        kopiuj_plik(sciezka_zrodlowy, sciezka_docelowy);
}

ssize_t zapisz_bufor(int fd, const void* bufor, size_t count) {
    size_t zostalo = count;
    while (zostalo > 0) {
        size_t zapisane = write(fd, bufor, count);
        if (zapisane == -1) return -1;
        else
            // sprawdza ile jest jescze do zapisania
            zostalo -= zapisane;
    }
    // pilnuje aby nie zapisac zbyt duzo 
    assert (zostalo == 0);
    return count;
}

void usun_pliki(config c){
    listaPlikow *lista = czytaj_zawartosc_katalogu(c.sciezkaDocelowa, c.rekursywnaSynch); // tworzy liste z plikami docelowego katalogu
    listaPlikow *lista_odwrocona = lista_odwroc(lista); // odwraca wyzej utworzona liste
    listaPlikow *poczatek = lista_odwrocona; // poczatek list
    lista_czysc(lista);

    while(lista_odwrocona->next != NULL){
        lista_odwrocona = lista_odwrocona->next;

        // tworzy sciezke pliku z katalogu zrodlowego
        int dlugosc = strlen(lista_odwrocona->sciezka) + strlen(lista_odwrocona->nazwa) - strlen(c.sciezkaDocelowa) + strlen(c.sciezkaZrodlowa) + 2;
        char plik_zrodlowy[dlugosc];
        snprintf(plik_zrodlowy, dlugosc, "%s%s/%s", c.sciezkaZrodlowa, lista_odwrocona->sciezka + strlen(c.sciezkaDocelowa), lista_odwrocona->nazwa);

        // tworzy z docelowego
        dlugosc = strlen(lista_odwrocona->sciezka) + strlen(lista_odwrocona->nazwa) + 3;
        char do_usuniecia[dlugosc];
        snprintf(do_usuniecia, dlugosc, "%s/%s", lista_odwrocona->sciezka, lista_odwrocona->nazwa);

        // usuwa plik z katalogu docelowego jesli plik nie istnieje w źródłowym
        if(!czy_istnieje(plik_zrodlowy)){
            syslog(LOG_INFO, "usuwam: %s", do_usuniecia);
            if(lista_odwrocona->typ == DIRECTORY){
                rmdir(do_usuniecia);
            }
            else {
                remove(do_usuniecia);
            }
        }
    }
    lista_czysc(poczatek);
}

bool czy_zawiera(const char *katalog1, const char *katalog2){
    char *a, *b, bufor_a[PATH_MAX + 1], bufor_b[PATH_MAX + 1];
    a = realpath(katalog1, bufor_a);
    b = realpath(katalog2, bufor_b);
    int n = strlen(a) < strlen(b) ? strlen(a) : strlen(b);
    return strncmp(a, b, n) ? false : true;
}
