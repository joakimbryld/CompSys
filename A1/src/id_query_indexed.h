#ifndef INDEXED_H
#define INDEXED_H

#include "record.h"
#include "id_query.h" 


struct indexed_data* mk_indexed(struct record* rs, int n);

void free_indexed(struct indexed_data* data);

const struct record* lookup_indexed(struct indexed_data *data, int64_t needle);

#endif
