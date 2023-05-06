#include "mpi.h"
#include <cstring>
#include <iostream>
using namespace std;

void print(int rank, int nprcs, int n, int* buf, const char info[]);
void myScatter(int total_len, int& local_len, int* buf, MPI_Comm comm);

int main(int argc, char** argv)
{
    int rank, nprcs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprcs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int m = 15, n, *buf;
    buf = new int[m];

    if (rank == 0) {
        printf("Scatter: Divide data as balancedly as possible\n\n");
        printf("########## before Scatter:\n");
        for (int j = 0; j < m; j++) {
            buf[j] = j;
            printf("%03d ", buf[j]);
        }
        printf("\n\n");
    }

    myScatter(m, n, buf, MPI_COMM_WORLD);

    print(rank, nprcs, n, buf, "########## after Scatter:");

    delete[] buf;

    MPI_Finalize();
    return 0;
}

void myScatter(int total_len, int& local_len, int* buf, MPI_Comm comm)
{
    /*
     *   process number must be the power of 2
     */
    int rank, nprcs;
    MPI_Status sta;
    MPI_Comm tcomm, scomm;

    MPI_Comm_dup(comm, &tcomm);
    int cur_len = total_len, nex_len;

    MPI_Comm_rank(tcomm, &rank);
    MPI_Comm_size(tcomm, &nprcs);

    while (nprcs > 1) {
        int half = nprcs / 2;
        if (rank == 0) {
            nex_len = cur_len / 2;
            MPI_Send(&nex_len, 1, MPI_INT, half, 482, tcomm);
            MPI_Send(buf + (cur_len - nex_len), nex_len, MPI_INT, half, 777, tcomm);
            cur_len = cur_len - nex_len;
        }
        if (rank == half) {
            MPI_Recv(&cur_len, 1, MPI_INT, 0, 482, tcomm, &sta);
            MPI_Recv(buf, cur_len, MPI_INT, 0, 777, tcomm, &sta);
        }

        MPI_Comm_split(tcomm, rank / half, rank, &scomm);
        MPI_Comm_dup(scomm, &tcomm);

        MPI_Comm_rank(tcomm, &rank);
        MPI_Comm_size(tcomm, &nprcs);
    }
    local_len = cur_len;
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