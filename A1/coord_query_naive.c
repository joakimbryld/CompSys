#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <math.h>

#include "record.h"
#include "coord_query.h"

struct naive_data
{
  struct record *rs;
  int n;
};

struct naive_data *mk_naive(struct record *rs, int n)
{
  struct naive_data *data = malloc(sizeof(struct naive_data));
  data->rs = rs;
  data->n = n;
  return data;
}

void free_naive(struct naive_data *data)
{
  if (data != NULL)
  {
    free(data);
  }
}

const struct record *lookup_naive(struct naive_data *data, double lon, double lat)
{
  if (data != NULL)
  {
    struct record *rec = NULL;

    for (int i = 0; i < data->n; i++)
    {
      double dist = sqrt(pow((lon - data->rs[i].lon), 2) + pow((lat - data->rs[i].lat), 2));
      double tempDist = INFINITY;
      if (dist < tempDist)
      {
        tempDist = dist;
        rec = &data->rs[i];
      }
    }
    return rec;
  }
  return NULL;
}

int main(int argc, char **argv)
{
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_naive,
                          (free_index_fn)free_naive,
                          (lookup_fn)lookup_naive);
}
