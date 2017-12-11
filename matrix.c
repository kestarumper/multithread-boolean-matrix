#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

typedef struct MatrixCell {
    pthread_mutex_t mutex;
    volatile char calculated;
    int val;
} MatrixCell;

typedef struct Matrix {
    int height;
    int width;
    MatrixCell ** rows;
} Matrix;

typedef struct ThreadParams {
    pthread_t tid;
    int row;
    int col;
} ThreadParams;

Matrix a;
Matrix b;
Matrix c;

void initMatrixMem(Matrix * mtx) {
    const int n = mtx->height;
    const int m = mtx->width;

    mtx->rows = malloc(n * sizeof(MatrixCell *));
    for(int i = 0; i < n; i++) {
        mtx->rows[i] = malloc(m * sizeof(MatrixCell));
        for(int j = 0; j < m; j++) {
            mtx->rows[i][j].val = 0;
            mtx->rows[i][j].calculated = '\0';
            pthread_mutex_init(&mtx->rows[i][j].mutex, NULL);
        }
    }
}

void randomMatrix(Matrix * mtx) {
    const int n = mtx->height;
    const int m = mtx->width;

    printf("%ix%i\n", n, m);
    mtx->rows = malloc(n * sizeof(MatrixCell *));
    for(int i = 0; i < n; i++) {
        printf("| ");
        mtx->rows[i] = malloc(m * sizeof(MatrixCell));
        for(int j = 0; j < m; j++) {
            mtx->rows[i][j].val = rand()%2;
            printf("%i ", mtx->rows[i][j].val);

        }
        printf("|\n");

    }
    printf("\n");

}

void printMatrix(Matrix * mtx) {
    const int n = mtx->height;
    const int m = mtx->width;
    printf("%ix%i\n", n, m);
    for(int i = 0; i < n; i++) {
        printf("| ");
        for(int j = 0; j < m; j++) {
            printf("%i ", mtx->rows[i][j].val);
        }
        printf("|\n");
    }
    printf("\n");
}

void * scalarMultiplication(void * param) {
    ThreadParams * params = (ThreadParams *)param;

    pthread_t id = params->tid;
    int row = params->row;
    int col = params->col;

    while(1) {
        // double-checked locking
        if(c.rows[row][col].calculated == '\0') {
            if(pthread_mutex_trylock(&c.rows[row][col].mutex) == 0) {
                // lock acquired
                if(c.rows[row][col].calculated == '\0') {
                    for(int i = 0; i < a.width; i++) {
                        // TODO: mnoze zle tablice sama ze soba, trzeba wziac matrixa &a i &b
                        c.rows[row][col].val ^= a.rows[row][i].val * b.rows[i][col].val;
                    }
                    c.rows[row][col].calculated = 'y';
                }
                pthread_mutex_unlock(&c.rows[row][col].mutex);
            }
        }

        // try next one
        col++;
        if(col == c.width) {
            row++;
            col = 0;
        }

        if(row == c.height) {
            pthread_exit(NULL);
        }
    }
}

int main(int argc, char * argv[]) {
    srand(time(NULL));

    printf("Hello world!\n");

    if(argc != 5) {
        printf("Not enough arguments");
        exit(1);
    }

    int n = atoi(argv[1]);
    int m = atoi(argv[2]);
    int p = atoi(argv[3]);
    int num_of_threads = atoi(argv[4]);

    // n*m o m*p = n*p
    a.width = b.height = m;
    a.height = c.height = n;
    b.width = c.width = p;    

    randomMatrix(&a);
    randomMatrix(&b);
    initMatrixMem(&c);

    ThreadParams * tparams = malloc(sizeof(ThreadParams) * num_of_threads);
    if(tparams == NULL) {
        perror("tparams malloc");
        exit(1);
    }

    int row = 0;
    int col = 0;
    for(int i = 0; i < num_of_threads; i++) {
        tparams[i].row = row;
        tparams[i].col = col;
    
        if(pthread_create(&tparams->tid, NULL, scalarMultiplication, &tparams[i])) {
            perror("pthread_create");
            exit(1);
        }

        col++;
        if(col == c.width) {
            row++;
            col = 0;
        }

        if(row == c.height) {
            break;
        }
    }

    void * retval;
    for(int i = 0; i < num_of_threads; i++) {
        /* wait for the second thread to finish */
        if(pthread_join(tparams[i].tid, &retval)) {
            perror("pthread_join");
        }
    }

    printMatrix(&c);

    return 0;
}