#include <stdio.h>  
#include <stdlib.h>   
#include <mpi.h>    
#include <math.h>
#include <time.h>       

#define CONTROLLER 0   // the process which controlls the rest workers

// Globals 
double t_start;
double t_end;
int rank;
int nProcesses;
int * array;
int array_size;


int CmpFunc(const void * a, const void * b) {
    return ( * (int *)a - * (int *)b );
}

void Lower(int j) {
    int i = 0,k = 0, l = 0;

    /* Sends the biggest of the list and receive the smallest of the list */

    // Send entire array to paired H Process
    // Exchange with a neighbor whose (d-bit binary) processor number differs only at the jth bit.
    int * buffer_send = malloc((array_size) * sizeof(int));

    // Receive new min of sorted numbers
    int * buffer_recieve = malloc((array_size) * sizeof(int));

    //Initializing buffer_send
    for (int z = 0; z < array_size; z++)
    {
        buffer_send[z] = array[z];
    }
    
    // send partition to paired H process
    MPI_Send(
        buffer_send,                // Send values that are greater than min
        array_size,               // # of items sent
        MPI_INT,                    // INT
        rank ^ (1 << j),    // paired process calc by XOR with 1 shifted left j positions
        0,                          // tag 0
        MPI_COMM_WORLD              // default comm.
    );
  
    // receive info from paired H process
    MPI_Recv(
        buffer_recieve,             // buffer the message
        array_size,                 // whole array
        MPI_INT,                    // INT
        rank ^ (1 << j),    // paired process calc by XOR with 1 shifted left j positions
        0,                          // tag 0
        MPI_COMM_WORLD,             // default comm.
        MPI_STATUS_IGNORE           // ignore info about message received
    );
    // Take received buffer of values from H Process which are smaller than current max

    while (l<array_size)
    {
        if (buffer_send[i] > buffer_recieve[k]) {
            // Store value from message
            array[l] = buffer_recieve[k];
            k++;
        }
        else{
            array[l] = buffer_send[i];
            i++;
        }
        l++;
    }
    
    // Reset the state of the heap from Malloc
    free(buffer_send);
    free(buffer_recieve);

    return;
}


void Upper(int j) {

    int i = 0, k = 0, l = 0;

    // Receive max from L Process's entire array
    int * buffer_recieve = malloc((array_size) * sizeof(int));

    // Send min to L Process of current process's array
    int * buffer_send = malloc((array_size) * sizeof(int));


    // Receive blocks greater than min from paired slave
    MPI_Recv(
        buffer_recieve,             // buffer message
        array_size,                 // whole array
        MPI_INT,                    // INT
        rank ^ (1 << j),    // paired process calc by XOR with 1 shifted left j positions
        0,                          // tag 0
        MPI_COMM_WORLD,             // default comm.
        MPI_STATUS_IGNORE           // ignore info about message receiveds
    );

    for (int z = 0; z < array_size; z++)
    {
        buffer_send[z] = array[z];
    }
  

    // send partition to paired slave
    MPI_Send(
        buffer_send,                // all items smaller than max value
        array_size,               // # of values smaller than max
        MPI_INT,                    // INT
        rank ^ (1 << j),    // paired process calc by XOR with 1 shifted left j positions
        0,                          // tag 0
        MPI_COMM_WORLD              // default comm.
    );

    // Take received buffer of values from L Process which are greater than current min
    while (l<array_size)
    {
        if (buffer_send[array_size-i-1] < buffer_recieve[array_size-k-1]) {
            // Store value from message
            array[l] = buffer_recieve[array_size-k-1];
            k++;
        }
        else{
            array[l] = buffer_send[array_size-i-1];
            i++;
        }
        l++;
    }

    // Reset the state of the heap from Malloc
    free(buffer_send);
    free(buffer_recieve);

    return;
}


int main(int argc, char * argv[]) {
    int i, j;

    // Initialization, get # of processes & this PID/rank
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    // Initialize Array for Storing Random Numbers
    array_size = atoi(argv[1]) / nProcesses;
    array = (int *) malloc(array_size * sizeof(int));

    // Generate Random Numbers for Sorting (within each process)
    // Less overhead without CONTROLLER sending random numbers to each slave
    srand(time(NULL));  // Needed for rand()
    for (i = 0; i < array_size; i++) {
        array[i] = rand() % (atoi(argv[1]));
    }

    // Blocks until all processes have finished generating
    MPI_Barrier(MPI_COMM_WORLD);

    // Begin Parallel Bitonic Sort Algorithm from Assignment Supplement

    // Cube Dimension
    int dimensions = (int)(log2(nProcesses));

    // Start Timer before starting first sort operation (first iteration)
    if (rank == CONTROLLER) {
        printf("Number of Processes spawned: %d\n", nProcesses);
        t_start = MPI_Wtime();
    }

    // Sequential Sort
    qsort(array, array_size, sizeof(int), CmpFunc);

    // Bitonic Sort follows
    for (i = 0; i < dimensions; i++) {
        for (j = i; j >= 0; j--) {
            // (window_id is even AND jth bit of process is 0)
            // OR (window_id is odd AND jth bit of process is 1)
            if (((rank >> (i + 1)) % 2 == 0 && (rank >> j) % 2 == 0) ||
                ((rank >> (i + 1)) % 2 != 0 && (rank >> j) % 2 != 0)) {
                Lower(j);
            } else {
                Upper(j);
            }   
        }
    }

    // Blocks until all processes have finished sorting
    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == CONTROLLER) {
        t_end = MPI_Wtime();

        printf("Time Elapsed (Sec): %f\n", t_end - t_start);
    }

    free(array);

    MPI_Finalize();
    return 0;
}
