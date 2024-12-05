#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "mpi.h"

void verifica_particoes(long long *Input, int n, long long *P, int np, 
                        long long *Output, int *nO) {
    int processId;
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    int isCorrect = 1;

    for (int i = 0; i < *nO; i++) {
        if (Output[i] < (processId == 0 ? 0 : P[processId-1])
            || Output[i] >= P[processId]) {
            isCorrect = 0;
            break;
        }
    }

    // Exibir resultado final
    if (isCorrect) {
        printf("===> particionamento CORRETO\n");
    } else {
        printf("===> particionamento COM ERROS no processo %d\n", processId);
    }
}