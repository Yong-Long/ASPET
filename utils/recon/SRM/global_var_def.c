#include "global_var_dec.h"

// global_var_def.c: definition of global variables
// Yun Dong 

char description_name[500];
float spacing = 60;  // [config] Distance of detector pairs: 60mm
float cw = 3.0;  // Crystal width: 3.0mm
float cl = 10;   // Crystal length: 20mm
float vx = 0.8;
float pitch = 0.2;

float vy, vz;
float pitch_half;
float py_edge_low, py_edge_up, pz_edge_low, pz_edge_up;
float kvy_edge_low[100], kvy_edge_up[100], kvz_edge_low[100], kvz_edge_up[100];
float kvx_edge_low, kvx_edge_up;
float kernel_corner[100][2];
float mu_lso = 0.083;  // LSO linear atenuation coefficients at 5llkeV , /mm

int n_crsl2pxl = 4;
int x_plane = 0;
int layer = 2;  // 1/2, layer of crystal for DOI
int ngrid = 4;
int ncy = 16;
int ncz = 32;
int nkernel;

int ncrow;
int nccol;

int ncy_k;
int ncz_k;
int ncrow_k;
int nccol_k;

int nimgx;
int nimgy;
int nimgz;
int imgybias; 
int imgzbias;

int cybias_k;
int czbias_k;

int vybias_k;
int vzbias_k;
int symy  = 24;
int symz  = 16;
float eps = 1.0e-6;

char kernel_name[100][10];