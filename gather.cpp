#include "mpi.h"
#include <cstring>
#include <iostream>
#include <unistd.h>
using namespace std;

void print(int rank, int nprcs, int n, int* buf, const char info[]);
void myGather(int& total_len, int local_len, int* buf, MPI_Comm comm);

int main(int argc, char** argv)
{
    int rank, nprcs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        printf("Gather: Gather data to root\n\n");
    }

    int m, n, *buf;
    buf = new int[nprcs * 7];

    srand((unsigned long long)time(NULL) + rank);
    n = rand() % 5 + 3;

    for (int j = 0; j < n; j++) {
        buf[j] = rank * 100 + j;
    }

    print(rank, nprcs, n, buf, "########## before Gather:");

    myGather(m, n, buf, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);

    if (rank == 0) {
        sleep(1);

        printf("\n########## after Gather:\n");
        printf("rank = %d, m = %d\n", rank, m);
        for (int j = 0; j < m; j++) {
            printf("%03d ", buf[j]);
        }
        printf("\n");
    }

    delete[] buf;

    MPI_Finalize();
    return 0;
}

void myGather(int& total_len, int local_len, int* buf, MPI_Comm comm)
{
    /*
     *   process number must be the power of 2
     */
    int rank, nprcs;
    int cur_len, nex_len;
    MPI_Status sta;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &nprcs);

    cur_len = local_len;
    for (int stride = 1; stride < nprcs; stride *= 2) {

        if (rank % stride == 0 && ((rank / stride) % 2) == 1) {
            int dest = rank - stride;
            MPI_Send(&cur_len, 1, MPI_INT, dest, 777, comm);
            MPI_Send(buf, cur_len, MPI_INT, dest, 888, comm);
        }

        if (rank % stride == 0 && ((rank / stride) % 2) == 0) {
            int src = rank + stride;
            MPI_Recv(&nex_len, 1, MPI_INT, src, 777, comm, &sta);
            MPI_Recv(buf + cur_len, nex_len, MPI_INT, src, 888, comm, &sta);
            cur_len = cur_len + nex_len;
        }
    }
    if (rank == 0)
        total_len = cur_len;
}

void print(int rank, int nprcs, int n, int* buf, const char info[])
{
    if (rank == 0) {
        printf("%s\n", info);
    }
    for (int r = 0; r < nprcs; r++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == r) {
            printf("rank = %d, n = %d\n", rank, n);
            for (int j = 0; j < n; j++) {
                printf("%03d ", buf[j]);
            }
            printf("\n");
        }
    }
}