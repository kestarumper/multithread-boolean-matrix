#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

typedef struct MatrixCell {
    int val;
} MatrixCell;

typedef struct Matrix {
    int height;
    int width;
    MatrixCell ** rows;
} Matrix;

Matrix a;
Matrix b;
Matrix c;
pthread_mutex_t ROWMUTEX;
int ROWNUMBER = 0;

void initMatrixMem(Matrix * mtx) {
    const int n = mtx->height;
    const int m = mtx->width;

    mtx->rows = malloc(n * sizeof(MatrixCell *));
    for(int i = 0; i < n; i++) {
        mtx->rows[i] = malloc(m * sizeof(MatrixCell));
        for(int j = 0; j < m; j++) {
            mtx->rows[i][j].val = 0;
        }
    }
}

void randomMatrix(Matrix * mtx) {
    const int n = mtx->height;
    const int m = mtx->width;

    mtx->rows = malloc(n * sizeof(MatrixCell *));
    for(int i = 0; i < n; i++) {
        mtx->rows[i] = malloc(m * sizeof(MatrixCell));
        for(int j = 0; j < m; j++) {
            mtx->rows[i][j].val = rand()%2;
        }
    }
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
    int row;

    while(1) {
        if(pthread_mutex_lock(&ROWMUTEX) == 0) {
            row = ROWNUMBER;
            ROWNUMBER++;
            pthread_mutex_unlock(&ROWMUTEX);
        }

        if(row >= c.height) {
            // no rows left
            break;
        }

        for(int col = 0; col < c.width; col++) {
            for(int i = 0; i < a.width; i++) {
                c.rows[row][col].val = a.rows[row][i].val & b.rows[i][col].val;
                if(c.rows[row][col].val) {
                    break;
                }
                // c.rows[row][col].val ^= a.rows[row][i].val * b.rows[i][col].val;
            }
        }

    }

    pthread_exit(NULL);
}

int main(int argc, char * argv[]) {
    srand(time(NULL));

    printf("Started...\n");

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

    // printMatrix(&a);
    // printMatrix(&b);

    pthread_t * tids = malloc(sizeof(pthread_t) * num_of_threads);
    if(tids == NULL) {
        perror("pthread_t malloc");
        exit(1);
    }

    pthread_mutex_init(&ROWMUTEX, NULL);

    for(int i = 0; i < num_of_threads; i++) {
        if(pthread_create(&tids[i], NULL, scalarMultiplication, NULL)) {
            perror("pthread_create");
            exit(1);
        }
    }

    void * retval;
    for(int i = 0; i < num_of_threads; i++) {
        if(pthread_join(tids[i], &retval)) {
            perror("pthread_join");
        }
    }

    // printMatrix(&c);

    return 0;
}