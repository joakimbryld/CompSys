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

int comp (const void * a, const void * b) {
    const struct index_record *a1 = a;
    const struct index_record *b1 = b;

    return  (a1->osm_id - b1->osm_id);

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
    qsort(data, n, sizeof(struct index_record), comp);
   
    return data;
    
}

void free_indexedBin(struct indexed_data* data) {
  if (data != NULL){
    free(data);
  }
}

const struct record* lookup_indexedBin(struct indexed_data *data, int64_t needle) {
    
  if (data != NULL) {
    bsearch(&needle, data, sizeof(struct index_record), data->n, comp);
    
  }
  return NULL;
}

int main(int argc, char** argv) {
  return id_query_loop(argc, argv,
                    (mk_index_fn)mk_indexedBin,
                    (free_index_fn)free_indexedBin,
                    (lookup_fn)lookup_indexedBin);
}


