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

#define MAX_THREADS 8
#define PARTITION_SIZE 100000
#define NTIMES 10

#define ll long long
#define MAX_TOTAL_ELEMENTS (500*250*250)

// pthread_t threads[MAX_THREADS]; // 8
// int threads_ids[MAX_THREADS]; // 8
// pthread_barrier_t thread_barrier;
ll inputG[MAX_TOTAL_ELEMENTS]; // 31_250_000
ll partitionArrG[MAX_TOTAL_ELEMENTS]; // 31_250_000

int localPos[MAX_THREADS][PARTITION_SIZE];
ll* partialResults[PARTITION_SIZE][MAX_THREADS]; // 800_000 * 

int nElements, nPartition, nThreads;

ll* input;
ll* partitionArr;

void print_ll_array(ll* arr, int arrSize) {
    printf("[");
    for (int i = 0; i < arrSize-1; i++) {
        printf("%lld, ", arr[i]);
    }
    printf("%lld]\n", arr[arrSize-1]);
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
    long long v = (long long)a * 100 + b;
    return v;
}

void calculate_indexes(int threadIndex, int* first, int* last) {
    int baseChunkSize = nElements / nThreads;
    int remainder = nElements % nThreads;

    if (threadIndex < remainder) {
        *first = threadIndex * (baseChunkSize + 1);
        *last = *first + baseChunkSize;
    } else {
        *first = threadIndex * baseChunkSize + remainder;
        *last = *first + baseChunkSize - 1;
    }
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

void* thread_worker(void* ptr) {
    int index = *((int*) ptr);
    int first, last;
    int* currArrSize = malloc(sizeof(int) * nPartition);
    for (int i = 0; i < nPartition; i++) {
        currArrSize[i] = 16;
    }
    calculate_indexes(index, &first, &last);

    for (int i = 0; i < nPartition; i++) {
        partialResults[i][index] = malloc(sizeof(ll) * currArrSize[i]);
        if (partialResults[i][index] == NULL) {
            printf("Failed to alloc partialResults[%d][%d]\n", i, index);
            exit(1);
        }
    }

    while (1) {
        pthread_barrier_wait(&thread_barrier);

        for (int i = 0; i < PARTITION_SIZE; i++) {
            localPos[index][i] = 0;
        }

        for (int i = first; i <= last; i++) {
            // Retorna o índice da partição que o elemento pertence
            int partitionIdx = upper_bound(partitionArr, nPartition, input[i]);
            partialResults[partitionIdx][index][localPos[index][partitionIdx]] = input[i];
            localPos[index][partitionIdx]++;

            if (localPos[index][partitionIdx] == currArrSize[partitionIdx]) {
                currArrSize[partitionIdx] *= 2;
                partialResults[partitionIdx][index] = realloc(partialResults[partitionIdx][index], sizeof(ll) * currArrSize[partitionIdx]);
                if (partialResults[partitionIdx][index] == NULL) {
                    printf("Failed to realloc partialResults[%d][%d]\n", partitionIdx, index);
                    exit(1);
                }
            }
        }
        
        // #if DEBUG
        // printf("Thread %d: ", index);
        // print_ll_array(partialResults[index], k);
        // printf("Thread %d: localPos: ", index);
        // printIntArray(localPos[index], last-first+1);
        // #endif

        pthread_barrier_wait(&thread_barrier);

        if (index == 0) {
            return NULL;
        }
    }

    if (index != 0) {
        pthread_exit(NULL);
    }

    return NULL;
}

void multi_partition_mpi(ll* input, int n, ll* P, int np, ll* output, int* pos) {
    static int initialized = 0;

    if (!initialized) {
        printf("Initializing threads\n");
        pthread_barrier_init(&thread_barrier, NULL, nThreads);

        threads_ids[0] = 0;
        for (int i = 1; i < nThreads; i++) {
            threads_ids[i] = i;
            pthread_create(&threads[i], NULL, thread_worker, &threads_ids[i]);
        }

        initialized = 1;
    }

    thread_worker(&threads_ids[0]);

    int k = 0;
    for (int i = 0; i <  np; i++) {
        for (int j = 0; j < nThreads; j++) {
            for (int l = 0; l < localPos[j][i]; l++) {
                output[k] = partialResults[i][j][l];
                k++;
            }
        }
    }
    
    pos[0] = 0;
    for (int i = 0; i < np-1; i++) {
        pos[i+1] = pos[i];
        for (int j = 0; j < nThreads; j++) {
            pos[i+1] += localPos[j][i];
        }
    }
}

int main(int argc, char* argv[]) {
    int processId, nProcesses;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &processId);

    srand(2024 * 100 + processId);
    nElements = 8000000;

    printf("Process %d of %d\n", processId, nProcesses);


    // // if (argc != 3) {
    // //      printf("Usage: %s <nPartition> <nThreads>\n", argv[0]); 
    // //      return 0;
    // // } else {
    // //     nThreads = atoi(argv[2]);
    // //     if (nThreads == 0) {
    // //         printf("Usage: %s <nPartition> <nThreads>\n", argv[0]);
    // //         printf("<nThreads> can't be 0\n");
    // //         return 0;
    // //     }     
    // //     if (nThreads > MAX_THREADS) {  
    // //         printf("Usage: %s <nPartition> <nThreads>\n", argv[0]);
    // //         printf( "<nThreads> must be less than %d\n", MAX_THREADS);
    // //         return 0;
    // //     }
    // //     nPartition = atoi(argv[1]);
    // // }
    // // printf("Will use %d threads to run partition, with %d total long long elements on input array and %d elements on partition array\n", nThreads, nElements, nPartition);

    // // Inicializações
    // input = malloc(sizeof(ll) * nElements);
    // if (input == NULL) {
    //     printf("Failed to alloc input\n");
    //     return 1;
    // }
    // for (int i = 0; i < nElements; i++) {
    //     input[i] = gera_aleatorio_ll();
    // }

    // for (int i = 0; i < MAX_TOTAL_ELEMENTS; i++) {
    //     inputG[i] = input[i%nElements];
    // }

    // // printf("Input array: ");
    // // print_ll_array(input, nElements);

    // free(input);
    // input = NULL;

    // partitionArr = malloc(sizeof(ll) * (nPartition));
    //     if (partitionArr == NULL) {
    //     printf("Failed to alloc partitionArr\n");
    //     return 1;
    // }
    // for (int i = 0; i < nPartition; i++) {
    //     partitionArr[i] = gera_aleatorio_ll();
    // }
    // qsort(partitionArr, nPartition-1, sizeof(ll), compare);
    // partitionArr[nPartition-1] = LLONG_MAX;

    // for (int i = 0; i < MAX_TOTAL_ELEMENTS; i++) {
    //     partitionArrG[i] = partitionArr[i%nPartition];
    // }

    // // printf("Partition array: ");
    // // print_ll_array(partitionArr, nPartition);

    // free(partitionArr);
    // partitionArr = NULL;

    // ll* output = malloc(sizeof(ll) * nElements);
    // if (output == NULL) {
    //     printf("Failed to alloc output\n");
    //     return 1;
    // }

    // int* pos = malloc(sizeof(int) * nPartition);
    // if (pos == NULL) {
    //     printf("Failed to alloc pos\n");
    //     return 1;
    // }

    // // for (int i = 0; i < nPartition; i++) {
    // //     for (int j = 0; j < nThreads; j++) {
    // //         partialResults[i][j] = malloc(sizeof(ll) * nElements/nThreads);
    // //         if (partialResults[i][j] == NULL) {
    // //             printf("Failed to alloc partialResults[%d][%d]\n", i, j);
    // //             return 1;
    // //         }
    // //     }
    // // }

    // chronometer_t parallelPartitionTime;
    // chrono_reset(&parallelPartitionTime);
    // chrono_start(&parallelPartitionTime);

    // // Execução
    // int start_position_input = 0;
    // int start_position_partition = 0;
    // input = &inputG[start_position_input];
    // partitionArr = &partitionArrG[start_position_partition];
    // for (int i = 0; i < NTIMES; i++) {
    //     printf("Iteration %d\n", i+1);

    //     multi_partition(input, nElements, partitionArr, nPartition, output, pos);

    //     start_position_input += nElements;
    //     start_position_partition += nPartition;

    //     if ((start_position_input + nElements) > MAX_TOTAL_ELEMENTS) 
    //         start_position_input = 0;
    //     input = &inputG[start_position_input];

    //     if ((start_position_partition + nPartition) > MAX_TOTAL_ELEMENTS) 
    //         start_position_partition = 0;
    //     partitionArr = &partitionArrG[start_position_partition];
    // }
    
    // // printf("Output array: ");
    // // print_ll_array(output, nElements);
    // // printf("Pos array: ");
    // // print_int_array(pos, nPartition);

    // chrono_stop(&parallelPartitionTime);
    // chrono_reportTime(&parallelPartitionTime, "parallelPartitionTime");

    // double total_time_in_nanoseconds = (double) chrono_gettotal(&parallelPartitionTime);
    // double total_time_in_seconds = total_time_in_nanoseconds / (1000 * 1000 * 1000);
    // printf("Total time: %lf s\n", total_time_in_seconds);
    // double average_time = total_time_in_seconds / (NTIMES);
    // printf("Average time: %lf s\n", average_time);
                                  
    // double eps = nElements * NTIMES / total_time_in_seconds;
    // double megaeps = eps/1000000;
    // printf("Throughput: %lf MEPS/s\n", megaeps);

    // verifica_particoes(input, nElements, partitionArr, nPartition, output, pos);

    // free(output);
    // free(pos);

    // for (int i = 0; i < nPartition; i++) {
    //     for (int j = 0; j < nThreads; j++) {
    //         free(partialResults[i][j]);
    //     }
    // }

    return 0;
}