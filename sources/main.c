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
void widelec(){
    syslog(LOG_INFO, "Forkowanie procesu");
    //forkowanie rodzica
    pid_t pid, sid;
    pid = fork();
    if(pid < 0){
        syslog(LOG_CRIT, "blad - nie udalo sie rozdzielic procesu");
        exit(EXIT_FAILURE);
    }
    if(pid > 0){
        exit(EXIT_SUCCESS);
    }
    //zmiana maski plikow
    umask(0);
    //tworzenie SIDa dla dziecka
    sid = setsid();
    if(sid < 0){
        syslog(LOG_CRIT, "blad - nie mozna utworzyc SID dla procesu potomnego");
        exit(EXIT_FAILURE);
    }
    //zmiana katalogu
    if((chdir("/")) < 0) {
        syslog(LOG_CRIT, "BLAD: nie mozna zmienic katalogu biezacego");
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
            syslog(LOG_INFO, "Pomyslnie zablokowano plik blokady: /tmp/demon.lock");
            return 0; // Udało się zablokować
        }
        else{
            return -1; // Nie można zablokować
        }
    }
    else return lockp.l_pid; // Zwraca pid procesu, który założył blokadę
}

void sig_force_sync(){
    if(SYNC_IN_PROGRESS) syslog(LOG_INFO, "Wymuszenie w trakcie synchronizacji, kontunuuje");
    else syslog(LOG_INFO, "Wymuszenie synchronizacji, wybudzam");
}

void sig_kill(){
    syslog(LOG_INFO, "Zakonczenie pracy demona");
    exit(EXIT_SUCCESS);
}

void sig_stop(){
    syslog(LOG_INFO, "Bezpieczne zakonczenie pracy, czekam na koniec synchronizacji");
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
        printf("BLAD: nie mozna zablokowac pliku demon.pid");
        return EXIT_FAILURE;
    }
    if(pid != 0){
        if(argc != 2 || argv[1][0] != '-'){
            printf("Demon jest już uruchomiony, uzyj jednej z podanych opcji:\n");
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
        printf("BLAD: nieprawidlowa skladnia\n");
        help(false);
        return EXIT_FAILURE;
    }

    if(!(katalog_uprawnienia_error(c.sciezkaZrodlowa) && katalog_uprawnienia_error(c.sciezkaDocelowa))){
        printf("BLAD: Ktorys z katalogow nie istnieje. Prosze podac poprawne katalogi.\n");
        return EXIT_FAILURE;
    }

    char bufor_a[PATH_MAX + 1], bufor_b[PATH_MAX + 1];
    c.sciezkaZrodlowa = realpath(c.sciezkaZrodlowa, bufor_a);
    c.sciezkaDocelowa = realpath(c.sciezkaDocelowa, bufor_b);

    if(czy_zawiera(c.sciezkaZrodlowa, c.sciezkaDocelowa)){
        printf("BLAD: katalogi nie moga sie zawierac\n");
        return EXIT_FAILURE;
    }

    openlog("demon_log", LOG_PID | LOG_CONS, LOG_USER);
    syslog(LOG_INFO, "Start programu");
    widelec();

    if(blokuj(true) != 0){
        syslog(LOG_CRIT, "BLAD: Nie mozna zablokowac pliku blokady");
        sig_kill();
    }

    while(true){
        syslog(LOG_INFO, "Rozpoczynam synchronizacje");
        SYNC_IN_PROGRESS = true;
        sync_all(c);
        SYNC_IN_PROGRESS = false;
        if(TIME_TO_DIE) sig_kill();
        syslog(LOG_INFO, "Zakonczono synchronizacje, usypiam na %d sekund", c.czasSpania);
        sleep(c.czasSpania);
    }

    closelog();
    return 0;
}
