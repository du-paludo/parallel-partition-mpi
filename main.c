// Eduardo Stefanel Paludo - GRR20210581
// Natael Pontarolo Gomes - GRR20211786

#include <stdio.h>
#include <pthread.h>
#include "mpi.h"
#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <limits.h>

#include "chrono.h"
#include "verifica_particoes.h"

#define NTIMES 10

#define ll long long

// pthread_t threads[MAX_THREADS]; // 8
// int threads_ids[MAX_THREADS]; // 8
// pthread_barrier_t thread_barrier;

// int localPos[MAX_THREADS][PARTITION_SIZE];
// ll* partialResults[PARTITION_SIZE][MAX_THREADS]; // 800_000 * 

// int nElements, nPartition, nThreads;

int nTotalElements;

void print_ll_array(ll* arr, int arrSize) {
    printf("[");
    for (int i = 0; i < arrSize-1; i++) {
        printf("%lld, ", arr[i]);
    }
    if (arrSize > 0) {
        printf("%lld]\n", arr[arrSize-1]);
    } else {
        printf("]\n");
    }
}

void print_int_array(int* arr, int arrSize) {
    printf("[");
    for (int i = 0; i < arrSize-1; i++) {
        printf("%d, ", arr[i]);
    }
    printf("%d]\n", arr[arrSize-1]);
}

int compare(const void* a, const void* b) {
   return (*(ll*)a - *(ll*)b);
}

long long gera_aleatorio_ll() {
    int a = rand(); // Returns a pseudo-random integer between 0 and RAND_MAX.
    int b = rand();
    long long v = ((long long)a * 100 + b) % 200;
    // long long v = (long long)a * 100 + b;
    return v;
}

ll upper_bound(ll arr[], int n, ll target) {
    ll lo = 0, hi = n - 1;
    ll res = n;

    while (lo <= hi) {
        ll mid = lo + (hi - lo) / 2;

        if (arr[mid] > target) {
            res = mid;
            hi = mid - 1;
        }
        else {
            lo = mid + 1;
        }
    }
    return res;
}


void multi_partition_mpi(ll* input, int n, ll* P, int np, ll* output, int* nO) {
    int processId;
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    ll partialResults[nTotalElements];
    for (int i = 0; i < np; i++) {
        for (int j = 0; j < n; j++) {
            partialResults[i * n + j] = -1;
        }
    }
    ll rcvBuffer[nTotalElements];

    int localPos[np];
    for (int i = 0; i < np; i++) {
        localPos[i] = 0;
    }

    for (int i = 0; i < n; i++) {
        // Retorna o índice da partição que o elemento pertence
        int partitionIdx = upper_bound(P, np, input[i]);
        partialResults[partitionIdx * n + localPos[partitionIdx]] = input[i];
        localPos[partitionIdx]++;
    }
    
    printf("Process %d: ", processId);
    print_ll_array(partialResults, nTotalElements);

    // Sends each partition to the process with the corresponding partition index
    MPI_Alltoall(partialResults, n, MPI_LONG_LONG,
                rcvBuffer, n, MPI_LONG_LONG, MPI_COMM_WORLD);

    int k = 0;
    for (int i = 0; i < nTotalElements; i++) {
        if (rcvBuffer[i] != -1) {
            output[k] = rcvBuffer[i];
            k++;
        }
    }
    *nO = k;

    MPI_Barrier(MPI_COMM_WORLD);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: mpirun -np <nProcesses> ./multi-partition <nTotalElements>\n");
        return 0;
    }

    int processId, np, n, nO;
    ll *input, *partitionArr, *output;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &np);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    nTotalElements = atoi(argv[argc-1]);
    n = nTotalElements / np;

    srand(2024 * 100 + processId);

    // printf("Process %d of %d\n", processId, np);

    input = malloc(sizeof(ll) * n);
    if (input == NULL) {
        printf("Failed to alloc input\n");
        return 1;
    }
    for (int i = 0; i < n; i++) {
        input[i] = gera_aleatorio_ll();
    }

    partitionArr = malloc(sizeof(ll) * (np));
    if (partitionArr == NULL) {
        printf("Failed to alloc partitionArr\n");
        return 1;
    }
    for (int i = 0; i < np-1; i++) {
        partitionArr[i] = (i+1) * 50;
        // partitionArr[i] = gera_aleatorio_ll();
    }
    qsort(partitionArr, np-1, sizeof(ll), compare);
    partitionArr[np-1] = LLONG_MAX;

    // printf("Input array: ");
    // print_ll_array(input, n);

    // printf("Partition array: ");
    // print_ll_array(partitionArr, np);
    // printf("\n");

    output = malloc(sizeof(ll) * nTotalElements);

    chronometer_t parallelPartitionTime;
    chrono_reset(&parallelPartitionTime);
    chrono_start(&parallelPartitionTime);

    for (int i = 0; i < NTIMES; i++) {
        printf("Iteration %d\n", i+1);
        multi_partition_mpi(input, n, partitionArr, np, output, &nO);
    }

    printf("Output array: ");
    print_ll_array(output, nO);
    printf("\n");

    chrono_stop(&parallelPartitionTime);
    // chrono_reportTime(&parallelPartitionTime, "parallelPartitionTime");

    double total_time_in_nanoseconds = (double) chrono_gettotal(&parallelPartitionTime);
    double total_time_in_seconds = total_time_in_nanoseconds / (1000 * 1000 * 1000);
    printf("Total time: %lf s\n", total_time_in_seconds);
    double average_time = total_time_in_seconds / (NTIMES);
    printf("Average time: %lf s\n", average_time);
                                  
    // double eps = nElements * NTIMES / total_time_in_seconds;
    // double megaeps = eps/1000000;
    // printf("Throughput: %lf MEPS/s\n", megaeps);

    // verifica_particoes(input, nElements, partitionArr, nPartition, output, pos);

    free(input);
    free(partitionArr);

    MPI_Finalize();

    return 0;
}