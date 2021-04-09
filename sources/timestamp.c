#include "timestamp.h"
#include <sys/stat.h>
#include <utime.h>

void klonuj_timestamp(char *plikZrodlowy, char *plikDocelowy){
    struct stat st;
    struct utimbuf new_times;
    stat(plikZrodlowy, &st);
    new_times.actime = st.st_atim.tv_sec;
    new_times.modtime = st.st_mtim.tv_sec;
    utime(plikDocelowy, &new_times);
    chmod(plikDocelowy, st.st_mode);
}

bool porownaj_timestamp(char *plikZrodlowy, char *plikDocelowy){
    struct stat st1;
    struct stat st2;
    stat(plikZrodlowy, &st1);
    stat(plikDocelowy, &st2);
    return (st1.st_mtim.tv_sec == st2.st_mtim.tv_sec);
}
