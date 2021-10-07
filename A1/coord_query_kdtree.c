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

struct index_record {
    double lon;
    double lat;
    const struct record *record;
};

struct indexed_data {
    struct index_record *irs;
    int n;
};

struct node {
    int point;
    int axis;
    double lon;
    double lat;
    struct node* left;
    struct node* right;
    struct record* rec;
};

int compare_lon(const void *p, const void *q) // fra nettet
{
    
    int64_t l = ((struct index_record *)p)->lon;
    int64_t r = ((struct index_record *)q)->lon;
    if (l < r) {return -1; }
    if (l > r) {return 1; }
    return 0;
    
}

int compare_lat(const void *p, const void *q) // fra nettet
{
    
    int64_t l = ((struct index_record *)p)->lon;
    int64_t r = ((struct index_record *)q)->lon;
    if (l < r) {return -1; }
    if (l > r) {return 1; }
    return 0;
    
}

struct node* mk_kdtree(struct record* rs, int depth, int n) {
    if (n==0) return NULL;
    
    struct node* curr = malloc(sizeof(struct node));
    int axis = depth % 2;

    curr->axis = axis;
    int new_n1;
    int new_n2;

    if (n==1){
    curr->lat= rs->lat;
    curr->lon= rs->lon;
    curr->rec = rs;
    curr->left = NULL;
    curr->right = NULL;
    return curr;

    }
    if (depth % 2 == 0){
        qsort(rs,(size_t)n, (size_t)sizeof(struct record), compare_lon);
    }
    else{
        qsort(rs,(size_t)n, (size_t)sizeof(struct record), compare_lat);
    }

}

void free_kdtree(struct node* data) {
  if (data != NULL){
    free(data);
  }
}

const struct record* lookup_kdtree(struct naive_data *data, double lon, double lat) {

}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}