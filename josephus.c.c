#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

#define READ 0
#define WRITE 1

typedef struct {
    int* array;
    int size;
    int current;
} Circle;

Circle* createCircle(int n) {
    Circle* circle = malloc(sizeof(Circle));
    circle->array = malloc(n * sizeof(int));
    circle->size = n;
    circle->current = 0;
    
    for (int i = 0; i < n; i++) {
        circle->array[i] = i + 1;
    }
    return circle;
}

void freeCircle(Circle* circle) {
    free(circle->array);
    free(circle);
}

int removeNext(Circle* circle) {
    if (circle->size <= 0) return -1;
    
    int skipCount = 1;
    int current = circle->current;
    
    while (skipCount > 0) {
        current = (current + 1) % circle->size;
        skipCount--;
    }
    
    int eliminated = circle->array[current];
    
    for (int i = current; i < circle->size - 1; i++) {
        circle->array[i] = circle->array[i + 1];
    }
    
    circle->size--;
    circle->current = current % circle->size;
    
    return eliminated;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_people>\n", argv[0]);
        exit(1);
    }
    
    int n = atoi(argv[1]);
    if (n < 1) {
        fprintf(stderr, "Number of people must be positive\n");
        exit(1);
    }
    
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(1);
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }
    
    if (pid == 0) {
        close(pipefd[READ]);
        Circle* circle = createCircle(n);
        
        while (circle->size > 1) {
            int eliminated = removeNext(circle);
            write(pipefd[WRITE], &eliminated, sizeof(int));
        }
        
        close(pipefd[WRITE]);
        freeCircle(circle);
        exit(0);
    } else {
        close(pipefd[WRITE]);
        int eliminated;
        
        printf("Elimination order: ");
        while (read(pipefd[READ], &eliminated, sizeof(int)) > 0) {
            printf("%d ", eliminated);
        }
        
        Circle* circle = createCircle(n);
        while (circle->size > 1) {
            removeNext(circle);
        }
        printf("\nSurvivor: %d\n", circle->array[0]);
        
        close(pipefd[READ]);
        wait(NULL);
        freeCircle(circle);
    }
    
    return 0;
}
