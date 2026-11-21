#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#define MAXABSAI 1000000000
#define I128 __int128

// gcc numuniquesums.c -o numuniquesums.bin -O3 -Wall -Wextra -mssse3

int myqsort(uint64_t size, int64_t *data, int level) {
  // In Place Quicksort
  if (level > 1000) return 1;
  int64_t temp;
  if (size <= 1) return 0;
  if (size == 2) {
    if (data[0] > data[1]) {
      temp = data[0];
      data[0] = data[1];
      data[1] = temp;
    }
    return 0;
  }
  I128 imean, sum = 0;
  uint64_t i,j, ltimean;
  for (i=0; i<size; i++) {
    sum += data[i];
  }
  imean = sum / size;
  ltimean = 0;
  while (ltimean == 0) {
    for (i=0; i<size; i++) {
      ltimean += (data[i] < imean);
    }
    if (ltimean == 0) imean++;
  }
  i = 0;
  j = ltimean;
  while (1) {
    // Find next element to swap in low partition
    while ((data[i] < imean) && (i < ltimean)) i++;
    if (i >= ltimean) break;
    // Find next element to swap in high partition
    while (data[j] >= imean) j++;
    temp = data[i];
    data[i] = data[j];
    data[j] = temp;
  }
  if (ltimean == size) return 0;
  if (myqsort(ltimean, data, level+1) != 0) return 1;
  if (myqsort(size - ltimean, data+ltimean, level+1) != 0) return 1;
  return 0;
}

_Bool appendsum(int64_t sum, int64_t **sumarray, uint64_t *sumarray_max_elements, uint64_t *sumarray_num_elements, uint32_t maxMB) {
  if (*sumarray_num_elements >= *sumarray_max_elements) {
    *sumarray_max_elements *= 2;
    if (1000000ULL*maxMB < *sumarray_max_elements*sizeof(int64_t)){
      *sumarray_max_elements /= 2;
      if ((1000000ULL*maxMB)/sizeof(int64_t) == *sumarray_max_elements) {
        fprintf(stderr, "Insufficient RAM!\n");
        return false;
      }
      *sumarray_max_elements = (1000000ULL*maxMB)/sizeof(int64_t);
    }
    *sumarray = realloc(*sumarray, *sumarray_max_elements*sizeof(int64_t));
    if (*sumarray == NULL) return false;
  }
  (*sumarray)[*sumarray_num_elements] = sum;
  (*sumarray_num_elements)++;
  return true;
}

int cmp_entryi32(const void *pa, const void *pb) {
  int32_t a,b;
  memcpy(&a, pa, sizeof(int32_t));
  memcpy(&b, pb, sizeof(int32_t));
  if (a == b) return 0;
  if (a < b) return -1;
  return 1;
}

int cmp_entryi64(const void *pa, const void *pb) {
  int64_t a,b;
  memcpy(&a, pa, sizeof(int64_t));
  memcpy(&b, pb, sizeof(int64_t));
  if (a == b) return 0;
  if (a < b) return -1;
  return 1;
}

_Bool deduplicate(int64_t *sumarray, uint64_t *sumarray_num_elements, uint32_t maxMB) {
  if (*sumarray_num_elements == 0) return true;
  if (*sumarray_num_elements*2*sizeof(int64_t) < 1000000ULL*maxMB) {
    qsort(sumarray, (size_t)(*sumarray_num_elements), sizeof(int64_t), &cmp_entryi64);
  } else {
    assert(0 == myqsort(*sumarray_num_elements, sumarray, 0));
  }
  uint64_t hare = 1;
  uint64_t tortoise = 0;
  while (hare < *sumarray_num_elements) {
    if (sumarray[hare] > sumarray[tortoise]) {
      tortoise++;
      sumarray[tortoise] = sumarray[hare];
    } else {
      if (sumarray[hare] < sumarray[tortoise]) return false;
    }
    hare++;
  }
  *sumarray_num_elements = 1+tortoise;
  return true;
}

int main(int argc, char *argv[]) {
  #ifdef NDEBUG
    printf("Do not compile with -DNDEBUG macro!\n");
    exit(1);
  #endif
  if (argc < 3) {
    printf("This program calculates the number of unique subset sums achievable from a list of up to 1000 non-zero integers a_i in [-%i, %i].\nRAM Requirement approx. -> 16*answer bytes.\nUsage:- %s maxMB a_1 ... a_n\n", MAXABSAI, MAXABSAI, argv[0]);
    exit(0);
  }
  uint32_t intarray_num_elements = argc - 2;
  int32_t maxMBi = atoi(argv[1]);
  assert(maxMBi > 0);
  uint32_t maxMB = maxMBi;
  assert(maxMB);
  assert(intarray_num_elements <= 1000);
  uint64_t sumarray_num_elements = 1;
  if (intarray_num_elements) {
    int32_t *intarray = malloc(intarray_num_elements*sizeof(int32_t));
    assert(intarray);
    for (uint32_t i=0; i<intarray_num_elements; i++) {
      assert(strlen(argv[2+i]) <= 11);
      int64_t ai = strtoll(argv[2+i], NULL, 10);
      assert(llabs(ai) <= (int64_t)MAXABSAI);
      assert(ai);
      intarray[i] = ai;
    }
    qsort(intarray, (size_t)intarray_num_elements, sizeof(int32_t), &cmp_entryi32);
    uint64_t sumarray_max_elements = (intarray_num_elements < 16 ? 1ULL << intarray_num_elements : 1ULL << 16);
    assert(sumarray_max_elements * sizeof(int64_t) <= 1000000ULL * maxMB);
    int64_t *sumarray = calloc(sumarray_max_elements, sizeof(int64_t));
    assert(sumarray);
    if (intarray[0] >= 0) {
      sumarray[0] = 0;
      sumarray[1] = intarray[0];
    } else {
      sumarray[1] = 0;
      sumarray[0] = intarray[0];
    }
    sumarray_num_elements = 2;
    for (uint32_t i=1; i<intarray_num_elements; i++) {
      //printf("%u ", i);
      //fflush(stdout);
      uint64_t sumarray_num_elements_init = sumarray_num_elements;
      for (uint64_t j=0; j<sumarray_num_elements_init; j++) {
        assert(appendsum(intarray[i] + sumarray[j], &sumarray, &sumarray_max_elements, &sumarray_num_elements, maxMB));
      }
      assert(deduplicate(sumarray, &sumarray_num_elements, maxMB));
      assert((sumarray = realloc(sumarray, sumarray_num_elements*sizeof(int64_t))));
      sumarray_max_elements = sumarray_num_elements;
    }
    #ifdef PRINTSUMS
      for (uint64_t j=0; j<sumarray_num_elements; j++) {
        printf("%" PRId64 " ", sumarray[j]);
      }
      printf("\n");
    #endif
    free(sumarray);
    free(intarray);
  }
  printf("There are %" PRIu64 " unique sums of these %u integers.\n", sumarray_num_elements, intarray_num_elements); 
}
