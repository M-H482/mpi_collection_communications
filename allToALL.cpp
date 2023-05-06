#include "mpi.h"
#include <cstring>
#include <iostream>
using namespace std;

void print(int rank, int nprcs, int n, int* buf, const char info[]);
void myAllToAll(int n, int* sendbuf, int* recvbuf, MPI_Comm comm);

int main(int argc, char** argv)
{
    int rank, nprcs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int n = 4, *sendbuf, *recvbuf;
    sendbuf = new int[n * nprcs];
    recvbuf = new int[n * nprcs];

    for (int i = 0; i < nprcs; i++) {
        for (int j = 0; j < n; j++) {
            sendbuf[i * nprcs + j] = rank * 100 + i * nprcs + j;
        }
    }

    print(rank, nprcs, n, sendbuf, "########## before all-to-all:");

    myAllToAll(n, sendbuf, recvbuf, MPI_COMM_WORLD);

    print(rank, nprcs, n, recvbuf, "########## after all-to-all:");

    delete[] sendbuf;
    delete[] recvbuf;
    MPI_Finalize();
    return 0;
}

void myAllToAll(int n, int* sendbuf, int* recvbuf, MPI_Comm comm)
{
    int rank, nprcs;
    MPI_Status sta;
    MPI_Request req;

    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &nprcs);

    // recvbuf[rank] = sendbuf[rank]
    memcpy(recvbuf + rank * n, sendbuf + rank * n, sizeof(int) * n);

    for (int s = 1; s < nprcs; s++) {
        // stride： proces k send sendbuf[k] to prcoess (k+stride)%nprcs

        int i = (rank + s) % nprcs;
        int j = (rank - s + nprcs) % nprcs;

        // send sendbuf[i] to process i
        MPI_Isend(sendbuf + i * n, n, MPI_INT,
            i, 777, MPI_COMM_WORLD, &req);

        // recv recvbuf[j] from process j
        MPI_Recv(recvbuf + j * n, n, MPI_INT,
            j, 777, MPI_COMM_WORLD, &sta);

        MPI_Wait(&req, &sta);
    }
}

void print(int rank, int nprcs, int n, int* buf, const char info[])
{
    if (rank == 0) {
        printf("%s\n", info);
    }
    for (int r = 0; r < nprcs; r++) {
        MPI_Barrier(MPI_COMM_WORLD);
        if (rank == r) {
            printf("rank = %d,\n", rank);
            for (int i = 0; i < nprcs; i++) {
                for (int j = 0; j < n; j++) {
                    printf("%03d ", buf[i * nprcs + j]);
                }
                printf("\n");
            }
            printf("\n");
        }
    }
}