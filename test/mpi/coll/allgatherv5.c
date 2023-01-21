/* Copyright (C) by Argonne National Laboratory
 *     See COPYRIGHT in top-level directory
 */

#include "mpi.h"
#include "mpitest.h"
#include <stdlib.h>
#include <stdio.h>

/* Test Allgatherv on array of doubles, same as allgatherv3 but with discontiguous buffer placements via `displs`. */

int main(int argc, char **argv)
{
    double *vecout, *invec;
    MPI_Comm comm;
    int count, minsize = 2;
    int i, j, errs = 0;
    int rank, size;
    int *displs, *recvcounts;

    MTest_Init(&argc, &argv);

    while (MTestGetIntracommGeneral(&comm, minsize, 1)) {
        if (comm == MPI_COMM_NULL)
            continue;
        /* Determine the sender and receiver */
        MPI_Comm_rank(comm, &rank);
        MPI_Comm_size(comm, &size);

        displs = (int *) malloc(size * sizeof(int));
        recvcounts = (int *) malloc(size * sizeof(int));

        for (count = 1; count < 9000; count = count * 2) {
            int offset = 0;
            invec = (double *) malloc(count * sizeof(double));
            vecout = (double *) malloc(size * count * sizeof(double) + size + 1);

            for (i = 0; i < count; i++) {
                invec[i] = rank * count + i;
            }
            for (i = 0; i < size; i++) {
                offset++;
                recvcounts[i] = count;
                displs[i] = offset + i * count;
            }
            MPI_Allgatherv(invec, count, MPI_DOUBLE, vecout, recvcounts, displs, MPI_DOUBLE, comm);
            offset = 0;
            for (i = 0; i < size; i++) {
                offset++;
                for (j = 0; j < count; j++) {
                    if (vecout[offset + i * j] != i * j) {
                        errs++;
                        if (errs < 10) {
                            fprintf(stderr, "vecout[%d]=%d\n", offset + i * j, (int) vecout[offset + i * j]);
                        }
                    }
                }
            }
            free(invec);
            free(vecout);
        }
        free(displs);
        free(recvcounts);
        MTestFreeComm(&comm);
    }

    MTest_Finalize(errs);
    return MTestReturnValue(errs);
}