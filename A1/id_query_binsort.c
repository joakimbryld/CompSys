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

int compare_id(const void *p, const void *q) // fra nettet
{
    
    int64_t l = ((struct index_record *)p)->osm_id;
    int64_t r = ((struct index_record *)q)->osm_id;
    if (l < r) {return -1; }
    if (l > r) {return 1; }
    return 0;

    
    
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
  qsort(data->irs, data->n, sizeof(struct index_record), compare_id);
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

        if (data->irs[mid].osm_id == x)
            return data->irs[mid].record;
  
        if (data->irs[mid].osm_id > x)
            return binarySearch(data, l, mid - 1, x);
  
        return binarySearch(data, mid + 1, r, x);
    }

    return NULL;
}





const struct record* lookup_indexedBin(struct indexed_data *data, int64_t needle) {
  if (data != NULL) {   
    return (binarySearch(data, 0, data->n-1, needle));
  }
  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexedBin,
                    (free_index_fn)free_indexedBin,
                    (lookup_fn)lookup_indexedBin);
}





