/*
    --* Hecho en conjunto con Hector Alexis Reyes Manrique (A01339607) *--
    Alumno: Daniela Vignau León
    Fecha: 17 de septiembre de 2020
    Materia: Programación Avanzada
    Profesor: Vicente Cubells
    Tarea 5
*/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define R 0
#define W 1

typedef struct {
    int fd[2];
} pipe_t;

void print_help();

int main(int argc, char * const * argv) {
    char * input = NULL;
    int argument, start = 0, index, n;
    
    while((argument = getopt (argc, argv, "n:")) != -1)
    
    switch(argument) {
        case 'n':
            input = optarg;
            if(isdigit(*input)) {
                n = atoi(input);
                if(n <= 1) {
                    fprintf(stderr, "-----------------------\nOpción -n require un número >= 2 como argumento\n");
                    return 1; 
                } else {
                    start = 1;
                }
            } else {
                fprintf(stderr, "-----------------------\nOpción -n require un número entero >=2 como argumento\n");
            }
            break;
        case '?':
            if(optopt == 'n')
                fprintf(stderr, "-----------------------\nOpción -%c require un número >= 2 entero como argumento\n", optopt);
            else if(isprint (optopt))
                fprintf(stderr, "-----------------------\nOpción desconocida '-%c'\n", optopt);
                print_help();
            return 1;
        default:
            abort();
    }
    
    if (argc != 3) {
        printf("-----------------------\nError");
        print_help();
        exit(-1);
    }
    
    if(start == 1) {
        int current_lap = 1, i;
        char message = 'T', m;
        pid_t pid;
        
        pipe_t * pipes = (pipe_t *) malloc(sizeof(pipe_t) * n);
        pipe_t * aux = pipes;
        
        for(; aux < pipes + n; aux++) {
            if(pipe(aux->fd) == -1) {
                printf("Error en creación de pipes");
                exit(-1);
            }
        }
        
        // Craete n children
        for (i = 0; i < n - 1; i++) {
            pid = fork();
            switch (pid) {
                case -1:
                    printf("Error. %d hijos creados\n", i + 1);
                    exit(-1);
                case 0: // Child
                    while (1) {
                        // Close all other reading pipes
                        for(int j = 0; j < n - 1; j++) {
                            if((pipes + j)->fd[R] != (pipes + i)->fd[R]) {
                                close((pipes + j)->fd[R]);
                            }
                        }
                        // Read from pipe
                        close((pipes + i)->fd[W]);
                        read((pipes + i)->fd[R], &m, sizeof(char));
                        printf("--> Soy el proceso con PID %d y recibí el testigo %c, el cual tendré por 5 segundos\n", getpid(), m);
                        
                        sleep(5);
                        
                        // Write into pipe
                        close((pipes + i + 1)->fd[R]);
                        printf("<-- Soy el proceso con PID %d y acabo de enviar el testigo %c\n", getpid(), m);
                        write((pipes + i + 1)->fd[W], &m, sizeof(char));
                    }
            } 
        }
        
        // The parent should write the first message
        printf("\nVuelta #%d\n", current_lap);
        printf("<-- Soy el proceso padre con PID %d y acabo de enviar el testigo %c\n", getpid(), message);
        write(pipes->fd[W], &message, sizeof(char));
        
        while (1) {
            // Read from last pipe
            close((pipes + 1)->fd[W]);
            read((pipes + n - 1)->fd[R], &m, sizeof(char));
            printf("--> Soy el proceso padre con PID %d y recibí el testigo %c, el cual tendré por 5 segundos\n", getpid(), m);
            
            sleep(5);
            
            // Write to first pipe
            close(pipes->fd[R]);
            current_lap++;
            printf("\nVuelta #%d\n", current_lap);
            printf("<-- Soy el proceso padre con PID %d y acabo de enviar el testigo %c\n", getpid(), m);
            write(pipes->fd[W], &m, sizeof(char));
        }
    }
    return 0;
}

void print_help() {
    printf("\nUse: ./a.out -n [entero]\n");
    printf("   Donde: [entero] >= 2\n");
}
