#include "timestamp.h"
#include <sys/stat.h>
#include <utime.h>

void kopiuj_timestamp(char* sciezka_zrodlowy, char* sciezka_docelowy);
    struct stat st;
    struct utimbuf new_times;
    stat(sciezka_zrodlowy, &st);
    new_times.actime = st.st_atim.tv_sec;
    new_times.modtime = st.st_mtim.tv_sec;
    utime(sciezka_docelowy, &new_times);
    chmod(sciezka_docelowy, st.st_mode);
}

bool porownaj_timestamp(char* sciezka_zrodlowy, char* sciezka_docelowy){
    struct stat st1;
    struct stat st2;
    stat(sciezka_zrodlowy, &st1);
    stat(sciezka_docelowy, &st2);
    return (st1.st_mtim.tv_sec == st2.st_mtim.tv_sec);
}
