#include <stdio.h>
#include <stdlib.h> 
#include <sys/types.h>
#include <sys/stat.h> // umask
#include <syslog.h> 
#include <dirent.h> // opendir
#include <unistd.h> 
#include <string.h>
#include <sys/file.h>
#include <errno.h>

#include <fcntl.h>
#include <signal.h>

#include "bool.h"
#include "config.h"
#include "parse.h"
#include "file.h"
#include "sync.h"
#include "filelist.h"
#include "help.h"

bool SYNC_IN_PROGRESS = false;
bool TIME_TO_DIE = false;

//funkcja forkujaca rodzica
void custom_fork(){
    syslog(LOG_INFO, "Forkowanie procesu");
    //forkowanie rodzica
    pid_t pid, sid;
    pid = fork();
    if(pid < 0){
        syslog(LOG_CRIT, "blad - rozdzielenie procesu nie powiodlo sie");
        exit(EXIT_FAILURE);
    }
    if(pid > 0){
        exit(EXIT_SUCCESS);
    }
    //zmiana maski plikow
    umask(0);
    //stworzenie SID dla dziecka
    sid = setsid();
    if(sid < 0){
        syslog(LOG_CRIT, "blad - do procesu potomnego nie mozna stworzyc SID");
        exit(EXIT_FAILURE);
    }
    //zmiana katalogu
    if((chdir("/")) < 0) {
        syslog(LOG_CRIT, "blad - nie mozna zmienic biezacego katalogu");
    }
    //zamykanie STD
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

}

int blokuj(bool lock){
    int lock_file = open("/tmp/demon.lock", O_CREAT | O_RDWR, 0666);
    struct flock lockp;
    fcntl(lock_file, F_GETLK, &lockp);
    if(lockp.l_type == 2){
        lockp.l_type = lock;
        if(fcntl(lock_file, F_SETLKW, &lockp) != -1){
            syslog(LOG_INFO, "plik blokady zostal pomyslnie zablokowany: /tmp/demon.lock");
            return 0; // Udalo sie zablokowac
        }
        else{
            return -1; // Nie Udalo sie zablokowac
        }
    }
    else return lockp.l_pid; // Zwraca pid procesu ktory zalozyl blokade
}

void sig_force_sync(){
    if(SYNC_IN_PROGRESS) syslog(LOG_INFO, "Wymuszenie jest w trakcie synchronizacji - kontynuacja");
    else syslog(LOG_INFO, "Wymuszenie synchronizacji - wybudzanie");
}

void sig_kill(){
    syslog(LOG_INFO, "Koniec pracy demona");
    exit(EXIT_SUCCESS);
}

void sig_stop(){
    syslog(LOG_INFO, "Bezpieczny koniec pracy - czekanie na koniec synchronizacji");
    if(SYNC_IN_PROGRESS) TIME_TO_DIE = true;
    else sig_kill();
}

void polacz(){
    signal(SIGUSR1, sig_force_sync);
    signal(SIGUSR2, sig_stop);
    signal(SIGTERM, sig_kill);
}

int main(int argc, char *argv[]){
    int pid = blokuj(false);
    polacz();

    if(argc == 2 && argv[1][0] == '-' && argv[1][1] == 'h'){
        help(true);
        return EXIT_FAILURE;
    }

    if(pid == -1){
        printf("blad - nie mozna zablokowac pliku demon.pid");
        return EXIT_FAILURE;
    }
    if(pid != 0){
        if(argc != 2 || argv[1][0] != '-'){
            printf("Demon jest uruchomiony, skorzystaj z podanych opcji:\n");
            help_actions();
            return EXIT_FAILURE;
        }
        else {
            switch (argv[1][1]){
            case 'F':
                printf("Wysylam SIGUSR1\n");
                kill(pid, SIGUSR1);
                return EXIT_SUCCESS;
            case 'S':
                printf("Wysylam SIGUSR2\n");
                kill(pid, SIGUSR2);
                return EXIT_SUCCESS;
            case 'K':
                printf("Wysylam SIGTTERM\n");
                kill(pid, SIGTERM);
                return EXIT_SUCCESS;
            }
            return EXIT_FAILURE;
        }
    }

    config c = parse_args(argc, argv);

    if(!c.czyPrawidlowa){
        printf("blad - skladnia jest niepoprawna \n");
        help(false);
        return EXIT_FAILURE;
    }

    if(!(katalog_uprawnienia_error(c.sciezkaZrodlowa) && katalog_uprawnienia_error(c.sciezkaDocelowa))){
        printf("blad - jakis katalog nie istnieje. Podaj poprawne katalogi.\n");
        return EXIT_FAILURE;
    }

    char bufor_a[PATH_MAX + 1], bufor_b[PATH_MAX + 1];
    c.sciezkaZrodlowa = realpath(c.sciezkaZrodlowa, bufor_a);
    c.sciezkaDocelowa = realpath(c.sciezkaDocelowa, bufor_b);

    if(czy_zawiera(c.sciezkaZrodlowa, c.sciezkaDocelowa)){
        printf("blad - katalogi nie moga sie zawierac\n");
        return EXIT_FAILURE;
    }

    openlog("demon_log", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Start programu");
    custom_fork();

    if(blokuj(true) != 0){
        syslog(LOG_CRIT, "blad - pliku blokady nie moze zostac zablokowany ");
        sig_kill();
    }

    while(true){
        syslog(LOG_INFO, "Rozpoczecie synchronizacji");
        SYNC_IN_PROGRESS = true;
        sync_all(c);
        SYNC_IN_PROGRESS = false;
        if(TIME_TO_DIE) sig_kill();
        syslog(LOG_INFO, "Zakonczenie synchronizacji, usypianie na %d sekund", c.czasSpania);
        sleep(c.czasSpania);
    }

    closelog();
    return 0;
}
