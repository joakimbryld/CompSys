#ifndef BINSORT_H
#define BINSORT_H

#include "record.h"
#include "id_query.h" 


struct indexed_data* mk_indexedBin(struct record* rs, int n);

void free_indexedBin(struct indexed_data* data);

const struct record* lookup_indexedBin(struct indexed_data *data, int64_t needle);

#endif
