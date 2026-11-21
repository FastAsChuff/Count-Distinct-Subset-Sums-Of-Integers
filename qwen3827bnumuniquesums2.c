#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <maxMB> <a1> [a2 ... an]\n", argv[0]);
        return 1;
    }

    char *end;
    long long maxMB = strtoll(argv[1], &end, 10);
    if (end == argv[1] || *end != '\0' || maxMB <= 0) {
        fprintf(stderr, "Invalid maxMB value.\n");
        return 1;
    }

    int n = argc - 2;
    if (n > 1000) {
        fprintf(stderr, "Too many integers (max 1000).\n");
        return 1;
    }

    long long *elems = malloc((size_t)n * sizeof(long long));
    if (!elems) { printf("Not enough RAM allowed.\n"); return 1; }

    for (int i = 0; i < n; i++) {
        elems[i] = strtoll(argv[2 + i], NULL, 10);
    }

    /* Total bytes available for working arrays */
    size_t budget = (size_t)maxMB * 1024 * 1024;

    /*
     * Strategy:
     *   Maintain a sorted array A of all distinct subset sums so far.
     *   For each element c, form the shifted array A+c (also sorted),
     *   then do a two-pointer merge of A and A+c to produce the new
     *   distinct-sum array.  Because both input sequences are strictly
     *   increasing, the only duplicates that can appear are where
     *   A[i] == A[j] + c, which the "equal" branch of the merge
     *   handles by advancing both pointers simultaneously.
     *
     *   Peak memory per step:  A (8m bytes) + B (up to 16m bytes) = 24m bytes.
     */

    long long *A = malloc(sizeof(long long));
    if (!A) { printf("Not enough RAM allowed.\n"); free(elems); return 1; 
}
    A[0] = 0;
    size_t m = 1;

    for (int i = 0; i < n; i++) {
        /* Check whether the peak working set fits in the budget */
        if ((size_t)24 * m > budget) {
            printf("Not enough RAM allowed.\n");
            free(A);
            free(elems);
            return 0;
        }

        long long *B = malloc(2 * m * sizeof(long long));
        if (!B) {
            printf("Not enough RAM allowed.\n");
            free(A);
            free(elems);
            return 0;
        }

        long long c = elems[i];
        size_t j = 0, p1 = 0, p2 = 0;

        /* Merge A (sorted) with A+c (sorted), dedup via equal-branch */
        while (p1 < m && p2 < m) {
            long long v1 = A[p1];
            long long v2 = A[p2] + c;
            if (v1 < v2)       { B[j++] = v1; p1++; }
            else if (v2 < v1)  { B[j++] = v2; p2++; }
            else               { B[j++] = v1; p1++; p2++; }
        }
        while (p1 < m) B[j++] = A[p1++];
        while (p2 < m) B[j++] = A[p2++] + c;

        free(A);
        A = B;
        m = j;
    }

    /* m = number of distinct subset sums (including the empty-subset sum 0) */
    printf("%zu\n", m);

    free(A);
    free(elems);
    return 0;
}

