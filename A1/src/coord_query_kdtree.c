#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include "record.h"
#include "coord_query.h"

struct node{
    struct record *rs;
    double lat;
    double lon;
    struct node *left, *right;
    int ax;
};


struct k2Tree
{
  struct node* root;
};

struct nearest_node{
    struct node *node;
    double dst;
};



int ax = 0;

static int cmp (const void * a, const void * b)
{ 
  double ra, rb;

  if(ax==1)
  {
    ra = ((struct node*)a)->lat;
    rb = ((struct node*)b)->lat;
  }
  else{
    ra = ((struct node*)a)->lon;
    rb = ((struct node*)b)->lon;
  }
  
  if (ra > rb) return 1;
  else if (ra < rb) return -1;
  else return 0;  
}

struct node*
   med(struct node *start, int n){
     qsort(start, n, sizeof(struct node), &cmp);
     return (start)+n/2;
}

struct node* recMk_kdtree(struct node* start, int n, int depth){
  ax = depth%2;
  struct node* mediantmp = med(start, n);
  struct node* median = malloc((sizeof(struct node)));
  median->lat = mediantmp->lat;
  median->lon = mediantmp->lon;
  median->rs = mediantmp->rs;
  
  median->ax = ax;
  if((mediantmp-(start))>0){  
    median->left = recMk_kdtree(start, (mediantmp-(start)), (depth+1));
    }
  else
  {
    median->left = NULL;
  }

  if((n-1)/2>0)
  { 
    median->right = recMk_kdtree(mediantmp+1, (n-1)/2, (depth+1));
  }
  else{
    median->right = NULL;
  }
  return median;
}


struct k2Tree* mk_kdtree(struct record* rs, int n) {
  struct k2Tree* kdtree = (struct k2Tree*)malloc(sizeof(struct k2Tree));
  struct node* nodeN = malloc(n*(sizeof(struct node)));
  for (int i = 0; i<n; i++) {
        nodeN[i].rs = &rs[i];
        nodeN[i].lat = rs[i].lat;
        nodeN[i].lon = rs[i].lon;
    }


  struct node* treeroot = recMk_kdtree(nodeN, n, 0);
  kdtree->root = treeroot;
  free(nodeN);
  return kdtree;
}

double dst(struct node* node, double lon, double lat){
 return sqrt(pow(lat- (node->lat),2) + pow(lon- (node->lon),2));
}

double compD(struct node* node, double lon, double lat)
{
  if(node->ax==1)
  {
    return (node->lat -lat);
  }
  else{
    return (node->lon -lon);
  }
}
void free_all(struct node* root){
if(root==NULL){
  return;
}
else{
  free_all(root->right);
  free_all(root->left);
  free(root);
  } 
}

void free_kdtree(struct k2Tree* tree) {
  if (tree == NULL) {
    return;
  }
  free_all(tree->root);
  free(tree);
}

void recLookup_kdtree( struct nearest_node *nearest, double lon, double lat, struct node *node)
{
  double diff;
  double tmpdst;
  double r;
  if(node == NULL){
    return;
  }
  else
  { 
    tmpdst = dst(node, lon, lat);
    if( tmpdst < nearest->dst)
     { 
       nearest->node = node;
       nearest->dst = tmpdst;
     }
    r = nearest->dst;
    diff = compD(node, lon, lat);
    if(((diff>=0) ||r > fabs(diff)) && ((node->left) != NULL))
      {
        recLookup_kdtree(nearest, lon, lat, node->left);
      }
    if(((diff<=0) ||r > fabs(diff))&& ((node->right) != NULL) ) 
      {
        recLookup_kdtree(nearest, lon, lat, node->right);
      }
    return;
  }
}

const struct record* lookup_kdtree(struct k2Tree *tree, double lon, double lat) {
  struct nearest_node* nearest = malloc(sizeof(struct nearest_node));
  double qlon = lon;
  double qlat = lat;
  nearest->node = tree->root;
  nearest->dst = dst(nearest->node, qlon, qlat);
  recLookup_kdtree(nearest, qlon, qlat, tree->root);
  struct record* nearestrecord = nearest->node->rs;
  free(nearest);
  return nearestrecord;
}

int main(int argc, char** argv) {
  return coord_query_loop(argc, argv,
                          (mk_index_fn)mk_kdtree,
                          (free_index_fn)free_kdtree,
                          (lookup_fn)lookup_kdtree);
}
