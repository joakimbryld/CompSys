#include "id_query_indexed.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct index_record {
    int64_t osm_id;
    const struct record *record;
};

struct indexed_data {
    struct index_record *irs;
    int n;
};


int compare_id (const void * a, const void * b) // fra nettet
{
  struct index_record *data_1 = (struct index_record *)a;
  struct index_record *data_2 = (struct index_record *)b;
  return ( data_1->osm_id - data_2->osm_id);
}



struct indexed_data* mk_indexedBin(struct record* rs, int n) {

    struct indexed_data *data = malloc(sizeof(struct indexed_data));
    struct index_record *irs = malloc(sizeof(struct index_record)*n);

    
    for (int i = 0; i < n; i++) {
        irs[i].record = &rs[i];
        irs[i].osm_id = rs[i].osm_id;
    }

    data->irs = irs;
    data->n = n;
    
    return data;
    
}

void free_indexedBin(struct indexed_data* data) {
  if (data != NULL){
    free(data);
  }
}

const struct record* binarySearch(struct indexed_data *data, int l, int r, int64_t x) // taget fra nettet
{
    if (r >= l) {
        int mid = l + (r - l) / 2;
  
        // If the element is present at the middle
        // itself
        if (data->irs[mid].osm_id == x)
            return data->irs[mid].record;
  
        // If element is smaller than mid, then
        // it can only be present in left subarray
        if (data->irs[mid].osm_id > x)
            return binarySearch(data, l, mid - 1, x);
  
        // Else the element can only be present
        // in right subarray
        return binarySearch(data, mid + 1, r, x);
    }
  
    // We reach here when element is not
    // present in array
    return NULL;
}


// https://www.geeksforgeeks.org/binary-search/


const struct record* lookup_indexedBin(struct indexed_data *data, int64_t needle) {
  if (data != NULL) {
    qsort(data, data->n, sizeof(struct index_record), compare_id);
    binarySearch(data, 0, data->n-1, needle);

  }
  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexedBin,
                    (free_index_fn)free_indexedBin,
                    (lookup_fn)lookup_indexedBin);
}





