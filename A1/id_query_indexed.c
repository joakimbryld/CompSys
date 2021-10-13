#include "id_query_indexed.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

struct index_record
{
  int64_t osm_id;
  const struct record *record;
};

struct indexed_data
{
  struct index_record *irs;
  int n;
};

struct indexed_data *mk_indexed(struct record *rs, int n)
{

  struct indexed_data *data = malloc(sizeof(struct indexed_data));
  struct index_record *irs = malloc(sizeof(struct index_record) * n);

  for (int i = 0; i < n; i++)
  {
    irs[i].record = &rs[i];
    irs[i].osm_id = rs[i].osm_id;
  }

  data->irs = irs;
  data->n = n;
  return data;
}

void free_indexed(struct indexed_data *data)
{
  if (data != NULL)
  {
    free(data);
  }
}

const struct record *lookup_indexed(struct indexed_data *data, int64_t needle)
{
  if (data != NULL)
  {
    for (int i = 0; i < data->n; i++)
    {
      if (data->irs[i].osm_id == needle)
      {
        return data->irs[i].record;
      }
    }
  }
  return NULL;
}

int main(int argc, char **argv)
{
  return id_query_loop(argc, argv,
                       (mk_index_fn)mk_indexed,
                       (free_index_fn)free_indexed,
                       (lookup_fn)lookup_indexed);
}
