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
    else return UNKNOWN;
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

file_list* czytaj_zawartosc_katalogu(char *sciezka, bool rekursywne){
    file_list *lista = list_create();
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
        if(typ == REGULAR_FILE) list_add(lista, wejscie->d_name, sciezka, typ, true);
        if(typ == DIRECTORY && rekursywne){
            lista_dodaj(lista, wejscie->d_name, sciezka, typ, true);
            lista_polacz(lista, czytaj_zawartosc_katalogu(sciezka_szczegol, true));
        }
    }
    closedir(katalog);
    return lista;
}

void utworz_pusty_plik(char* sciezka){
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH; // uprawnienia do pliku
    int fd = open(sciezka, O_WRONLY | O_EXCL | O_CREAT, mode); // tworzenie pliku
    if (fd == -1) {
        syslog(LOG_CRIT, "BŁĄD: nie można utworzyć pliku");
        exit(EXIT_FAILURE);
    }
    close(fd);
}

void kopiuj_plik(char* sciezka_zrodlowy, char* sciezka_docelowy){
    unsigned char bufor[16];
    size_t offset = 0;
    size_t bytes_read;
    int i;

    /* Otwieranie pliku źródłowego */
    int zrodlo_fd = open (sciezka_zrodlowy, O_RDONLY);

    /* Tworzenie pliku docelowego */
    utworz_pusty_plik (sciezka_docelowy);
    int docel_fd = open (sciezka_docelowy, O_WRONLY);
    /* Czytanie częściami do momentu gdy część
     * jest mniejsza niż powinna */
    do {
        bytes_read = read(zrodlo_fd, bufor, sizeof(bufor));
        write_all(docel_fd, bufor, bytes_read);

        /* Pilnowanie pozycji w pliku */
        offset += bytes_read;
    } while(bytes_read == sizeof(bufor));
    /* Zamykanie file descriptor */
    close(zrodlo_fd);
    close(docel_fd);
}

void kopiuj_duzy_plik(char *sciezka_zrodlowy, char *sciezka_docelowy){
    /* Deklaracje zmiennych */
    int zrodlo_fd, docel_fd;
    char *zrodlo, *docelowe;
    struct stat s;
    size_t rozmiar;

    /* Plik źródłowy */
    zrodlo_fd = open(sciezka_zrodlowy, O_RDONLY);
    filesize = lseek(zrodlo_fd, 0, SEEK_END);
    zrodlo = mmap(NULL, rozmiar, PROT_READ, MAP_PRIVATE, zrodlo_fd, 0);

    /* Plik docelowy */
    docel_fd = open(sciezka_docelowy, O_RDWR | O_CREAT, 0666);
    ftruncate(docel_fd, rozmiar);
    docelowe = mmap(NULL, rozmiar, PROT_READ | PROT_WRITE, MAP_SHARED, docel_fd, 0);

    /* Kopiowanie */
    memcpy(docelowe, zrodlo, rozmiar);
    munmap(zrodlo, rozmiar);
    munmap(docelowe, rozmiar);

    close(zrodlo);
    close(docelowe);
}

void kopiuj_opcje(char *sciezka_zrodlowy, char *sciezka_docelowy, bool czy_duzy_plik){
    if(czy_duzy_plik) kopiuj_duzy_plik(sciezka_zrodlowy, sciezka_docelowy);
    else kopiuj_plik(sciezka_zrodlowy, sciezka_docelowy);
}

ssize_t zapisz_bufor(int fd, const void* bufor, size_t count) {
    size_t zostalo = count;
    while (zostalo > 0) {
        size_t zapisane = write(fd, bufor, count);
        if (zapisane == -1) return -1;
        else
            /* Pilnowanie ile jescze zostało do zapisu  */
            zostalo -= zapisane;
    }
    /* Pilnowanie żeby nie zapisać za dużo   */
    assert (zostalo == 0);
    /* Liczba zapisanych bajtów to count  */
    return count;
}

void usun_pliki(config c){
    /* Tworzenie i odwrócenie listy z plikami
     * katalogu docelowego oraz tworzenie punktu
     * powrotu (begin) */
    file_list *lista = czytaj_zawartosc_katalogu(c.dest_dir, c.recursive_sync);
    file_list *lista_odwrocona = lista_odwroc(lista);
    file_list *poczatek = lista_odwrocona;
    lista_czysc(lista);

    while(lista_odwrocona->next != NULL){
        lista_odwrocona = lista_odwrocona->next;

        /* Tworzenie pełnej ścieżki pliku z katalogu źródłowego */
        int dlugosc = strlen(lista_odwrocona->sciezka) + strlen(lista_odwrocona->nazwa) - strlen(c.sciezkaDocelowa) + strlen(c.sciezkaZrodlowa) + 2;
        char plik_zrodlowy[dlugosc];
        snprintf(plik_zrodlowy, dlugosc, "%s%s/%s", c.sciezkaZrodlowa, lista_odwrocona->sciezka + strlen(c.sciezkaDocelowa), lista_odwrocona->nazwa);

        /* Tworzenie pełnej ścieżki pliku z katalogu docelowego */
        dlugosc = strlen(lista_odwrocona->sciezka) + strlen(lista_odwrocona->nazwa) + 3;
        char do_usuniecia[dlugosc];
        snprintf(do_usuniecia, dlugosc, "%s/%s", lista_odwrocona->sciezka, lista_odwrocona->nazwa);

        /* Jeżeli plik nie istnieje w katalogu źródłowym
         * jest on usuwany z katalogu docelowego */
        if(!czy_istnieje(plik_zrodlowy)){
            syslog(LOG_INFO, "Usuwam: %s", do_usuniecia);
            if(lista_odwrocona->typ == DIRECTORY){
                rmdir(do_usuniecia);
            }
            else {
                remove(do_usuniecia);
            }
        }
    }
    /* Usuwanie listy */
    lista_czysc(poczatek);
}

bool czy_zawiera(const char *katalog1, const char *katalog2){
    char *a, *b, bufor_a[PATH_MAX + 1], bufor_b[PATH_MAX + 1];
    a = realpath(katalog1, bufor_a);
    b = realpath(katalog2, bufor_b);
    int n = strlen(a) < strlen(b) ? strlen(a) : strlen(b);
    return strncmp(a, b, n) ? false : true;
}
