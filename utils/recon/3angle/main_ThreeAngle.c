#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <math.h>
#define _GNU_SOURCE
#include <getopt.h>
#define OUTFILE1 "DHAPET_original_forward.DAT"
#define OUTFILE2 "DHAPET_original_backward.DAT"
#include "global_var_dec.h"
#include "psf_analytic.h"

void reshape(float *input, int nimgx, int nimgy, int nimgz, float ****output_ptr) {
    float ***output = (float ***)calloc(nimgx, sizeof(float **));
    int x, y, z;
    
    for (x = 0; x < nimgx; x++) {
        output[x] = (float **)calloc(nimgz, sizeof(float *));

        for (z = 0; z < nimgz; z++) {
            output[x][z] = (float *)calloc(nimgy, sizeof(float));

            for (y = 0; y < nimgy; y++) {
                int input_index = x * nimgz * nimgy + z * nimgy + y;
                output[x][z][y] = input[input_index];
            }
        }
    }

    *output_ptr = output;
}


void gaussian_smoothing(float ****src_image, float ****dest_ptr, int nimgx, int nimgy, int nimgz, double sigma) {
    int filter_size = (int)round(sigma*3)*2 + 1;
    int i, j, k, x, y, z;
    
    double *gaussian_filter = (double *)malloc(filter_size * sizeof(double));

    if (!gaussian_filter) {
        perror("Memory allocation failed for gaussian_filter");
        return;
    }

    float ***dest_image = (float ***)calloc(nimgx, sizeof(float **));
    if (!dest_image) {
        perror("Memory allocation failed for dest_image");
        free(gaussian_filter);
        return;
    }

    for (x = 0; x < nimgx; x++) {
        dest_image[x] = (float **)calloc(nimgz, sizeof(float *));
        if (!dest_image[x]) {
            perror("Memory allocation failed for dest_image[x]");
            for (j = 0; j < x; j++) {
                for (k = 0; k < nimgz; k++)
                    free(dest_image[j][k]);
                free(dest_image[j]);
            }
            free(dest_image);
            free(gaussian_filter);
            return;
        }

        for (z = 0; z < nimgz; z++) {
            dest_image[x][z] = (float *)calloc(nimgy, sizeof(float));
            if (!dest_image[x][z]) {
                perror("Memory allocation failed for dest_image[x][z]");
                for (j = 0; j <= x; j++) {
                    for (k = 0; k < (j == x ? z : nimgz); k++)
                        free(dest_image[j][k]);
                    free(dest_image[j]);
                }
                free(dest_image);
                free(gaussian_filter);
                return;
            }
        }
    }

    double sum = 0.0;
    for (i = 0; i < filter_size; i++) {
        int x = i - filter_size / 2;
        gaussian_filter[i] = exp(-(x*x) / (2*sigma*sigma));
        sum += gaussian_filter[i];
    }

    for (i = 0; i < filter_size; i++)
        gaussian_filter[i] /= sum;

    for (x = 0; x < nimgx; x++) {
        for (z = 0; z < nimgz; z++) {
            for (y = 0; y < nimgy; y++) {
                double value = 0.0;
                sum = 0.0;
                for (i = 0; i < filter_size; i++) {
                    int xi = x + i - filter_size / 2;
                    if (xi >= 0 && xi < nimgx) {
                        value += (*src_image)[xi][z][y] * gaussian_filter[i];
                                                                                          
		                 sum += gaussian_filter[i];
                    }
                }
                dest_image[x][z][y] = value / sum;
            }
        }
    }

    *dest_ptr = dest_image;

    free(gaussian_filter);
}


float bilinear_interpolation(float ***input, int nimgx, int nimgz, int nimgy, float x, float z, float y) {
    int x0 = floor(x);
    int x1 = x0 + 1;
    int z0 = floor(z);
    int z1 = z0 + 1;
    int y0 = floor(y);
    int y1 = y0 + 1;

    float wx = x - x0;
    float wz = z - z0;
    float wy = y - y0;

    float v00 = 0, v01 = 0, v10 = 0, v11 = 0;

    if (x0 >= 0 && x0 < nimgx && z0 >= 0 && z0 < nimgz-1) {
        //v00 = input[x0][z0][y0];
        //v01 = input[x0][z0][y1];
	v00 = input[x0][z0][y0];
        v01 = input[x0][z1][y0];

    }
    if (x1 >= 0 && x1 < nimgx-1 && z0 >= 0 && z0 < nimgz-1) {
      //  v10 = input[x1][z0][y0];
       // v11 = input[x1][z0][y1];
	v10 = input[x1][z0][y0];
        v11 = input[x1][z1][y0];

    }

    float v0 = (1 - wx) * v00 + wx * v10;
    float v1 = (1 - wx) * v01 + wx * v11;

   // output = (1 - wz) * v0 + wz * v1;
    return (1 - wz) * v0 + wz * v1;
}


void rotate_y(float ****input, float *input2,int nimgx, int nimgy, int nimgz, float angle, float ****outputR_ptr) {
    float rad_angle = angle * M_PI / 180.0;
    float cos_angle = cos(rad_angle);
    float sin_angle = sin(rad_angle);
    int x_cen = nimgx / 2;    int z_cen = nimgz / 2;
    float ***outputR = (float ***)calloc(nimgx, sizeof(float **));
    int x, y, z;
    for (x = 0; x < nimgx; x++) {
        outputR[x] = (float **)calloc(nimgz, sizeof(float *));
        for (z = 0; z < nimgz; z++) {
            outputR[x][z] = (float *)calloc(nimgy, sizeof(float));
        }
    }

for (x = 0; x < nimgx; x++) {
        for (z = 0; z < nimgz; z++) {
            for (y = 0; y < nimgy; y++) {
	    outputR[x][z][y]=input2[x * nimgz * nimgy + z * nimgy + y];
	    }}}

    for (x = 0; x < nimgx; x++) {
        for (z = 0; z < nimgz; z++) {
            for (y = 0; y < nimgy; y++) {
                int x_rel = x - x_cen;
                int z_rel = z - z_cen;
                float x_rot = x_rel * cos_angle - z_rel * sin_angle + x_cen;
                float z_rot = x_rel * sin_angle + z_rel * cos_angle + z_cen;
		
		int x_r=round(x_rot);
	        int z_r=round(z_rot);


   if (x_r >= 0 && x_r < nimgx-1 && z_r >= 0 && z_r < nimgz-1 ) {

           outputR[x][z][y] =bilinear_interpolation(*input, nimgx, nimgz, nimgy, x_rot, z_rot, y);

                }
            }
        }
}
    *outputR_ptr = outputR;
}


float s_time()
{
  return (float)clock()/CLOCKS_PER_SEC;
}

int nboundary(int cy, int cz)
{
  if( cy < ncy_k && cy >= 0 &&
      cz < ncz_k && cz >= 0    )
      return 1;
  else return 0;
}

int boundary(int cy, int cz)
{
  if( cy < ncy && cy >= 0 &&
      cz < ncz && cz >= 0    )
      return 1;
  else return 0;
}

int boundary_z(int cz)
{
  if(cz < ncz && cz >= 0) return 1;
  else return 0;
}

int boundary_y(int cy)
{
  if(cy < ncy && cy >= 0) return 1;
  else return 0;
}

void config()
{
    nkernel=4;
    nimgx = spacing / vx;
    nimgy = ncy*n_crsl2pxl;
    nimgz = ncz*n_crsl2pxl;    
    ncrow = ncy * ncz;
    nccol = ncy * ncz;
    ncy_k = 2 * ncz;
    ncz_k = 2 * ncz;
    ncrow_k = ncy_k * ncz_k;
    nccol_k = ncrow_k;

  /*printf("  ncy=%d, ncz=%d\n",ncy,ncz); 
    printf("  pitch=%.4f\n",pitch); 
    printf("  nimgy=%d, nimgz=%d, nimgx=%d\n",nimgy,nimgz,nimgx); 
    printf("  ngrid=%d\n",ngrid); 
    printf("  spacing=%4.0fmm\n",spacing);
    printf("  index of xplane=%d\n",x_plane);
    printf("  the object data will be saved with description: %s\n",description_name);
    */
    py_edge_low = -ncy_k * cw / 2.;
    py_edge_up  =  ncy_k * cw / 2.;
    pz_edge_low = -ncz_k * cw / 2.;
    pz_edge_up  =  ncz_k * cw / 2.;
    
    // vx = spacing / nimgx;
    vy = cw / (n_crsl2pxl);
    vz = cw / (n_crsl2pxl);

    kvx_edge_low = vx * (x_plane - 0.5);
    kvx_edge_up  = vx * (x_plane + 0.5);

}

// ====== ====== ====== ====== Forward ====== ====== ====== ======
void forward(float* pFwd,float* phantom)
{
 int fx_plane = x_plane +160,i,j;
 TABLE_ZIP* pTable = (TABLE_ZIP*) calloc(nccol_k+1, sizeof(TABLE_ZIP));
 FILE *inTAB,*inINCRE,*inSparse;
 for(i=0; i<4; i++)
 {
  for(j=0; j<4; j++)
  {
   char name[500];
   sprintf(kernel_name[i*nkernel+j],"%d_%d",j,i);
   sprintf(name,"data/SRM/_srm_sp%.0f_vx%.1f/sparse_table_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
   inTAB = fopen(name,"rb");
   fread(pTable,sizeof(TABLE_ZIP),nccol_k+1,inTAB);
   INCREMENT_ZIP* pIncre = (INCREMENT_ZIP*) calloc(pTable[nccol_k].bias,sizeof(INCREMENT_ZIP));
   float* pSparse= (float*) calloc(pTable[nccol_k].bias,sizeof(float));  
   sprintf(name,"data/SRM/_srm_sp%.0f_vx%.1f/sparse_incre_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
   inINCRE = fopen(name,"rb");
   fread(pIncre,sizeof(INCREMENT_ZIP),pTable[nccol_k].bias,inINCRE);
   sprintf(name,"data/SRM/_srm_sp%.0f_vx%.1f/sparse_matrix_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
   inSparse = fopen(name,"rb");
   fread(pSparse,sizeof(float),pTable[nccol_k].bias,inSparse); 
   int LOR_initial=pTable[nccol_k].bias;
   int iz,jy,kcz,kcy;
   for(iz=0;iz<ncz;iz++)
   { 
    int shift_z= (iz*4-ncz*2)/4;
    for(jy=0;jy<ncy;jy++)
    { 
     int shift_y = (jy*4-ncy*2)/4;  
     for(kcz=0;kcz<ncz;kcz++)
     {
      int cz1=kcz+symz-shift_z;
      for(kcy=0;kcy<ncy;kcy++)
      {
       int cy1=kcy+symy-shift_y;
       
       if(nboundary(cy1,cz1))
        {
        int cy2min=pTable[cz1*ncy_k+cy1].cymin+shift_y;
        int cz2min=pTable[cz1*ncy_k+cy1].czmin+shift_z;
        int c2total=pTable[cz1*ncy_k+cy1+1].bias;
        int ktotal;
        for(ktotal=pTable[cz1*ncy_k+cy1].bias;ktotal<c2total;ktotal++)
        {
         int cy2=pIncre[ktotal]._y+cy2min;
         int cz2=pIncre[ktotal]._z+cz2min;
         if(boundary(cy2-symy,cz2-symz))
         {
          pFwd[(kcz)*ncy*ncrow+(kcy)*ncrow+(cz2-symz)*ncy+(cy2-symy)]+=pSparse[ktotal]*phantom[fx_plane*nimgz*nimgy+(iz*nkernel+i)*nimgy+jy*nkernel+j];  
         }
        }
       }
      }
     }
    }
   }
   fclose(inTAB);
   fclose(inINCRE);
   fclose(inSparse);
   free(pIncre);
   free(pSparse);
  }
 }
 free(pTable);
}

void ratio(float *pFW, float *pHist, double *pRatio,float alpha)
{
  float loss=0;
  int i;
  for(i = 0; i < ncrow * nccol; i++)
  {
    if(pHist[i] >= 0)
    {
      if((pFW[i]) > 0)
      {
        pRatio[i] =  (pHist[i]) /(pFW[i]);
        //printf("First Condition     index= %d   Ratio= %f \n", i ,pRatio[i]);
      }
      else
      {
        if(pHist[i] > 0){
          pRatio[i] = 0; // non_zero / zero
        }
        else{
          pRatio[i] = 1.0; // zero / zero
        }
      }
      loss=loss +(pFW[i]-(alpha*pHist[i]));
    }
    // if(pRatio[i]>1024.0)
    // {
    //  pRatio[i]=1024.0;
    // }
    // if(pFW[i] != pFW[i]) printf("--This is NaN at LOR=%d, with pHist=%f.\n", i, pHist[i]);
  }
  printf("LOSS_Function Iside the function  = %f \n ", (loss));
}

// ====== ====== ====== ====== Backward ====== ====== ====== ======
void backward(double* pRatio, float* pRecon, float* pNorm)
{
 int rx_plane = x_plane +160;
 TABLE_ZIP* pTable = (TABLE_ZIP*) calloc(nccol_k+1, sizeof(TABLE_ZIP));
 int i,j;
 FILE *inTAB,*inINCRE,*inSparse;

 for(i=0;i<nkernel;i++)
 {
  for(j=0;j<nkernel;j++)
  {///media/atiq/ADATAHV620S/SRMGamma_NoEBlur_Correct/SRAM_DATA/S3
   char name[500];
   sprintf(kernel_name[i*nkernel+j],"%d_%d",j,i);
   sprintf(name,"data/SRM/_srm_sp%.0f_vx%.1f/sparse_table_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
   //printf("- SRM name (table): %s",name);
   inTAB = fopen(name,"rb");
   fread(pTable,sizeof(TABLE_ZIP),nccol_k+1,inTAB);
   INCREMENT_ZIP* pIncre = (INCREMENT_ZIP*) calloc(pTable[nccol_k].bias,sizeof(INCREMENT_ZIP));
   float* pSparse = (float*) calloc(pTable[nccol_k].bias,sizeof(float));  
   sprintf(name,"data/SRM/_srm_sp%.0f_vx%.1f/sparse_incre_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
   inINCRE = fopen(name,"rb");
   fread(pIncre,sizeof(INCREMENT_ZIP),pTable[nccol_k].bias,inINCRE);
   sprintf(name, "data/SRM/_srm_sp%.0f_vx%.1f/sparse_matrix_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
   inSparse = fopen(name,"rb");
   fread(pSparse,sizeof(float),pTable[nccol_k].bias,inSparse); 
   int iz,jy,kcz,kcy;
   int LOR_initial = pTable[nccol_k].bias;
   for(iz=0;iz<ncz;iz++)
   {
    int shift_z = (iz*4-ncz*2)/4;
    for(jy=0;jy<ncy;jy++)
    {
      int shift_y = (jy*4-ncy*2)/4; 
     for(kcz=0;kcz<ncz;kcz++)
     {
      int cz1=kcz+symz-shift_z;
      for(kcy=0;kcy<ncy;kcy++)
      {  
       int cy1=kcy+symy-shift_y;
       if(nboundary(cy1,cz1))
       {
        int cy2min=pTable[cz1*ncy_k+cy1].cymin+shift_y;
        int cz2min=pTable[cz1*ncy_k+cy1].czmin+shift_z;
        int c2total=pTable[cz1*ncy_k+cy1+1].bias;
        int ktotal;
        for(ktotal=pTable[cz1*ncy_k+cy1].bias;ktotal<c2total;ktotal++)
        {
         int cy2=pIncre[ktotal]._y+cy2min;
         int cz2=pIncre[ktotal]._z+cz2min;
         if(boundary(cy2-symy,cz2-symz))
         {      
          pRecon[rx_plane*nimgz*nimgy+(iz*nkernel+i)*nimgy+jy*nkernel+j]+=pRatio[(kcz)*ncy*ncrow+(kcy)*ncrow+(cz2-symz)*ncy+(cy2-symy)]*pSparse[ktotal];
          pNorm[rx_plane*nimgz*nimgy+(iz*nkernel+i)*nimgy+jy*nkernel+j]+=pSparse[ktotal];
         }
        }
       }
      }
     }
    }
   }
   fclose(inTAB);
   fclose(inINCRE);
   fclose(inSparse);
   free(pIncre);
   free(pSparse);
  }
 }
 free(pTable);
}

void update(float* pBwd,float* pRecon,float* pNorm,char* bin_name)
{
  int ix,i;
  for(ix=0;ix<nimgx;ix++) {
    for(i=0;i<nimgy*nimgz;i++) {
      if(pNorm[ix*nimgz*nimgy+i]) {
        pBwd[ix*nimgz*nimgy+i]=pBwd[ix*nimgz*nimgy+i]*pRecon[ix*nimgz*nimgy+i]/(pNorm[ix*nimgz*nimgy+i]);
      }
      else {
        pBwd[ix*nimgz*nimgy+i]=1;
      }
    }
  }
  char out_nameY[500];
  sprintf(out_nameY,"data/recon/3angle/OUTPUT/%s/GateSensitivityImageSchemeS3.DAT", bin_name);
  FILE *fp1Y;
  if((fp1Y=fopen(out_nameY,"wb"))==NULL)
  {
    printf("[Err] Can not open file: %s\n", out_nameY);
    exit(0);
  }
  fwrite(pNorm,sizeof(float)*nimgx*nimgy*nimgz,1,fp1Y);
  fclose(fp1Y);
}

// ====== ====== ====== ====== ====== Main ====== ====== ====== ====== ======
int main(int argc, char *argv[])
{
  // --- Print all input arguments ---
  while(argc--){ printf("argv[%d]: %s\n", argc, argv[argc]); }

  char bin_name[100];
  int iterations, xplanes;
  
  sprintf(bin_name, "%s", argv[1]);
  iterations = atoi(argv[2]);
  spacing = atof(argv[3]);
  vx = atof(argv[4]);
  xplanes = atoi(argv[5]);
  
/// - Remove previous output DAT files -
    char setting_cmd[300];
    sprintf(setting_cmd, "rm   -rf data/recon/3angle/OUTPUT/%s/  &&  mkdir -p data/recon/3angle/OUTPUT/%s/", bin_name, bin_name); system(setting_cmd);
    sprintf(setting_cmd, "mkdir -p data/recon/3angle/OUTPUT/%s/Aq1", bin_name); system(setting_cmd);
    sprintf(setting_cmd, "mkdir -p data/recon/3angle/OUTPUT/%s/Aq2", bin_name); system(setting_cmd);
    sprintf(setting_cmd, "mkdir -p data/recon/3angle/OUTPUT/%s/Aq3", bin_name); system(setting_cmd);

  config();
  
  char name[100];
  sprintf(name,"SRM/_srm_sp%.0f_vx%.1f/sparse_***_plane_%d_**", spacing, vx, x_plane);
  printf("\n[test] SRM path: %s",name);
  
  printf("\n[Info] bin_name = %s, iterations = %d, spacing = %.0f, vx = %.1f, xplanes = %d, nimgx = %d\n", bin_name, iterations, spacing, vx, xplanes, nimgx);
  
  float* pHist1= (float*) calloc(nccol*ncrow,sizeof(float));
  float* pHist2= (float*) calloc(nccol*ncrow,sizeof(float));
  float* pHist3= (float*) calloc(nccol*ncrow,sizeof(float));
  float* dHist1= (float*) calloc(nccol*ncrow,sizeof(float));
  float* dHist2= (float*) calloc(nccol*ncrow,sizeof(float));
  float* dHist3= (float*) calloc(nccol*ncrow,sizeof(float));
  
  float* pBwd= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd1= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd2= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd3= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd1n= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd2n= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd3n= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd13R= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd31R= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd32R= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd23R= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd12R= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pBwd21R= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  
  float* pFwd= (float*) calloc(nccol*ncrow,sizeof(float));
  double* pRatio= (double*) calloc(nccol*ncrow,sizeof(double));
  float* phantom= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pRecon= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  float* pNorm= (float*) calloc(nimgx*nimgy*nimgz,sizeof(float));
  
  int i, j, k, x, y, z, ix,iy,iz,ite,P1,P2;
  float sigma, *input, ***output, ***output_rot,T[2],***output_rot13, ***output_rot31,***output_rot23, ***output_rot32,***output_rot12, ***output_rot21;
  char guess_name[300], out_name[300];
  FILE *InGuess,*InGuessR,*iHist1,*iHist2,*iHist3;

  // Initial_guess
  for(ix=0; ix<nimgx; ix++)
  {
    for(i=0; i<nimgy*nimgz; i++)
    {	
      pBwd[ix*nimgz*nimgy+i]=.00005;	
      pBwd1[ix*nimgz*nimgy+i]=.00005;
      pBwd2[ix*nimgz*nimgy+i]=.00005;
      pBwd3[ix*nimgz*nimgy+i]=.00005;
      
      pBwd12R[ix*nimgz*nimgy+i]=.00005;
      pBwd21R[ix*nimgz*nimgy+i]=.00005;
      pBwd31R[ix*nimgz*nimgy+i]=.00005;
      pBwd13R[ix*nimgz*nimgy+i]=.00005;
      pBwd32R[ix*nimgz*nimgy+i]=.00005;
      pBwd23R[ix*nimgz*nimgy+i]=.00005;
      
      pBwd1n[ix*nimgz*nimgy+i]=.00005;
      pBwd2n[ix*nimgz*nimgy+i]=.00005;
      pBwd3n[ix*nimgz*nimgy+i]=.00005;
    }
  }
  sprintf(guess_name, "data/recon/3angle/OUTPUT/%s/InitGuess1_iter%d.DAT", bin_name, 0);
  if((InGuess=fopen(guess_name,"wb"))==NULL)
  {
    printf("[Err] Can not open file: %s\n", guess_name);
    exit(0);
  }
  fwrite(pBwd,sizeof(float)*nimgx*nimgy*nimgz,1,InGuess);
  fclose(InGuess);
  
  // ============================ INPUT Hist data ============================== ***
  char iHist1_name[300], iHist2_name[300], iHist3_name[300];
  sprintf(iHist1_name, "data/recon/3angle/INPUT/%s/Histogram_sorted_alongY_aq1.dat", bin_name);
    sprintf(iHist2_name, "data/recon/3angle/INPUT/%s/Histogram_sorted_alongY_aq2.dat", bin_name);
    sprintf(iHist3_name, "data/recon/3angle/INPUT/%s/Histogram_sorted_alongY_aq3.dat", bin_name);
  
  iHist1 = fopen(iHist1_name,"rb");
  iHist2 = fopen(iHist2_name,"rb");
  iHist3 = fopen(iHist3_name,"rb");
  
  //iHist1=fopen("3angle_recon/INPUT/HistDrenzo/Histogram_Derenzo_alongY_Aq2.dat","rb");
  //iHist2=fopen("3angle_recon/INPUT/HistDrenzo/Histogram_Derenzo_alongY_Aq1.dat","rb");
  //iHist3=fopen("3angle_recon/INPUT/HistDrenzo/Histogram_Derenzo_alongY_Aq3.dat","rb");
  
  fread(pHist1,sizeof(float),nccol*ncrow,iHist1);
  fread(pHist2,sizeof(float),nccol*ncrow,iHist2);
  fread(pHist3,sizeof(float),nccol*ncrow,iHist3);
  float sdhist1=0,sdhist2=0,sdhist3=0,shist1=0,shist2=0,shist3=0,alpha1=0.0,alpha2=0.0,alpha3=0.0,alpha1p=0.0,alpha2p=0.0,alpha3p=0.0;
  
  for (i=0; i<nccol*ncrow; i++)
  {
    dHist1[i]=pHist2[i]+pHist3[i];
    dHist2[i]=pHist1[i]+pHist3[i];
    dHist3[i]=pHist1[i]+pHist2[i];
    sdhist1=sdhist1+dHist1[i];
    sdhist2=sdhist2+dHist2[i];
    sdhist3=sdhist3+dHist3[i];
    shist1=shist1+pHist1[i];
    shist2=shist2+pHist2[i];
    shist3=shist3+pHist3[i];
  }
  
  alpha1=sdhist1/shist1;
  alpha2=sdhist2/shist2;
  alpha3=sdhist3/shist3;
  alpha1p=1/alpha1;
  alpha2p=1/alpha2;
  alpha3p=1/alpha3;
  
  printf("alpha1 = %.2f : alpha2 = %.2f : alpha3 = %.2f\n",alpha1,alpha2,alpha3);
  fclose(iHist1);
  fclose(iHist2);
  fclose(iHist3);

  sigma = .25;
  
  //////////////////////////////////// start EM ////////////////////////////////////
  
  //P1=-160, P2=160; // spacing: 256, vx: 0.8, xplanes: 320
  P1 = -xplanes/2, P2 = xplanes/2;

  for(ite=0; ite<iterations; ite++)
  {
    printf("\n[Run] Iteration: %d\n",ite+1);
    T[0]=s_time();
    memset(pFwd,0,nccol*ncrow*sizeof(float));
    memset(pRecon,0,nimgx*nimgy*nimgz*sizeof(float));
    memset(pNorm,0,nimgx*nimgy*nimgz*sizeof(float));

    // ====== ====== ====== AQ-1 ====== ====== ======
    for(x_plane=P1; x_plane<P2; x_plane++)
    {	
      //printf("Plane= %d \n",x_plane);  
      forward(pFwd,pBwd1);
    }
    ratio(pFwd,pHist1,pRatio,alpha1);
    for(x_plane=P1; x_plane<P2; x_plane++)
    {
      //printf("start backward\n");  
      backward(pRatio,pRecon,pNorm);
    }
    update(pBwd1,pRecon,pNorm,bin_name);
    
    memset(pFwd,0,nccol*ncrow*sizeof(float));
    memset(pRecon,0,nimgx*nimgy*nimgz*sizeof(float));
    memset(pNorm,0,nimgx*nimgy*nimgz*sizeof(float));

    // ====== ====== ====== AQ-2 ====== ====== ======
    for(x_plane=P1; x_plane<P2; x_plane++)
    {
      //printf("Plane= %d \n",x_plane);  
      forward(pFwd,pBwd2);
    }
    ratio(pFwd,pHist2,pRatio,alpha2);
    for(x_plane=P1;x_plane<P2;x_plane++)
    {
      //printf("start backward\n");  
      backward(pRatio,pRecon,pNorm);
    }
    update(pBwd2,pRecon,pNorm,bin_name);
    
    memset(pFwd,0,nccol*ncrow*sizeof(float));
    memset(pRecon,0,nimgx*nimgy*nimgz*sizeof(float));
    memset(pNorm,0,nimgx*nimgy*nimgz*sizeof(float));
    
    // ====== ====== ====== AQ-3 ====== ====== ======
    for(x_plane=P1;x_plane<P2;x_plane++)
    {
      forward(pFwd,pBwd3);
    }
    ratio(pFwd,pHist3,pRatio,alpha3);
    for(x_plane=P1;x_plane<P2;x_plane++)
    {
      backward(pRatio,pRecon,pNorm);
    }
    update(pBwd3,pRecon,pNorm,bin_name);
    
    // ====== ====== ====== ====== ====== ======
    sigma = 0.35;
    // float ***output_rot12;
    float angle = -120;
    reshape(pBwd1, nimgx, nimgy, nimgz, &output);
    rotate_y(&output,pBwd12R ,nimgx, nimgy, nimgz, angle, &output_rot12);
    // gaussian_smoothing(&output_rot12, &output_rot12, nimgx, nimgy, nimgz,sigma);
    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
          pBwd12R[i*nimgy*nimgz+j*nimgy + k] = output_rot12[i][j][k];
        }
      }
    }

    angle = 120;
    // float ***output_rot32;
    reshape(pBwd3, nimgx, nimgy, nimgz, &output);
    rotate_y(&output,pBwd32R,nimgx, nimgy, nimgz, angle, &output_rot32);
    // gaussian_smoothing(&output_rot32, &output_rot32, nimgx, nimgy, nimgz,sigma);

    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
          pBwd32R[i*nimgy*nimgz+j*nimgy + k] = output_rot32[i][j][k];
        }
      }
    }
    
    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
          pBwd2n[i*nimgy*nimgz+j*nimgy + k] = output_rot12[i][j][k]+ pBwd2[i*nimgy*nimgz+j*nimgy + k]+output_rot32[i][j][k];
        }
      }
    }
    
    // Middle Slice HHTotal
    sprintf(out_name, "data/recon/3angle/OUTPUT/%s/Aq2/ThreeAngleImage_aq2_iter%d.DAT", bin_name, ite+1);
    FILE *f2t;
    if((f2t=fopen(out_name,"wb"))==NULL)
    {
      printf("[Err] Can not open file: %s\n", out_name);
      exit(0);
    }
    fwrite(pBwd2n,sizeof(float)*nimgx*nimgy*nimgz,1,f2t);
    fclose(f2t);

    // preparing 1st total Image
    // float ***output_rot21;
    angle = 120;
    reshape(pBwd2, nimgx, nimgy, nimgz, &output);
    rotate_y(&output,pBwd21R ,nimgx, nimgy, nimgz, angle, &output_rot21);
    // gaussian_smoothing(&output_rot21, &output_rot21, nimgx, nimgy, nimgz,sigma);

    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
         pBwd21R[i*nimgy*nimgz+j*nimgy + k] = output_rot21[i][j][k];
        }
      }
    }

    angle = -120;
    // float ***output_rot31;
    reshape(pBwd3, nimgx, nimgy, nimgz, &output);
    rotate_y(&output,pBwd31R,nimgx, nimgy, nimgz, angle, &output_rot31);
    // gaussian_smoothing(&output_rot31, &output_rot31, nimgx, nimgy, nimgz,sigma);
    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
          pBwd31R[i*nimgy*nimgz+j*nimgy + k] = output_rot31[i][j][k];
        }
      }
    }

    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
          pBwd1n[i*nimgy*nimgz+j*nimgy + k] = output_rot21[i][j][k]+ pBwd1[i*nimgy*nimgz+j*nimgy + k]+output_rot31[i][j][k];
        }
      }
    }
    
    sprintf(out_name, "data/recon/3angle/OUTPUT/%s/Aq1/ThreeAngleImage_aq1_iter%d.DAT", bin_name, ite+1);
    FILE *f1t;
    if((f1t=fopen(out_name,"wb"))==NULL)
    {
      printf("[Err] Can not open file: %s\n", out_name);
      exit(0);
    }
    fwrite(pBwd1n,sizeof(float)*nimgx*nimgy*nimgz,1,f1t);
    fclose(f1t);

    // preparing 3rd Image
    // float ***output_rot13;
    angle = 120;
    reshape(pBwd1, nimgx, nimgy, nimgz, &output);
    rotate_y(&output,pBwd13R ,nimgx, nimgy, nimgz, angle, &output_rot13);
    // gaussian_smoothing(&output_rot13, &output_rot13, nimgx, nimgy, nimgz,sigma);
    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
          pBwd13R[i*nimgy*nimgz+j*nimgy + k] = output_rot13[i][j][k];
        }
      }
    }

    angle = -120;
    // float ***output_rot23;
    reshape(pBwd2, nimgx, nimgy, nimgz, &output);
    rotate_y(&output,pBwd23R,nimgx, nimgy, nimgz, angle, &output_rot23);
    // gaussian_smoothing(&output_rot23, &output_rot23, nimgx, nimgy, nimgz,sigma);
    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
          pBwd23R[i*nimgy*nimgz+j*nimgy + k] = output_rot23[i][j][k];
        }
      }
    }

    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
          pBwd3n[i*nimgy*nimgz+j*nimgy + k] = output_rot13[i][j][k]+ pBwd3[i*nimgy*nimgz+j*nimgy + k]+output_rot23[i][j][k];
        }
      }
    }
    
    sprintf(out_name, "data/recon/3angle/OUTPUT/%s/Aq3/ThreeAngleImage_aq3_iter%d.DAT", bin_name, ite+1);
    FILE *f3t;
    if((f3t=fopen(out_name,"wb"))==NULL)
    {
      printf("[Err] Can not open file: %s\n", out_name);
      exit(0);
    }
    fwrite(pBwd3n,sizeof(float)*nimgx*nimgy*nimgz,1,f3t);
    fclose(f3t);
    
    for (i = 0; i< nimgx; i++) {
      for (j = 0; j< nimgz; j++) {
        for (k = 0; k< nimgy; k++) {
          pBwd2[i*nimgy*nimgz+j*nimgy + k] =  pBwd2n[i*nimgy*nimgz+j*nimgy + k] ;
          pBwd1[i*nimgy*nimgz+j*nimgy + k] =  pBwd1n[i*nimgy*nimgz+j*nimgy + k] ;
          pBwd3[i*nimgy*nimgz+j*nimgy + k] =  pBwd3n[i*nimgy*nimgz+j*nimgy + k] ;
        }
      }
    }

    T[1]=s_time()-T[0];
    printf("time: %g sec \n",T[1]);
  }
  
  //////////////////////////////////// stop EM ////////////////////////////////////
  free(phantom); 
  free(pFwd);
  free(pBwd1);
  free(pHist1);
  free(pBwd2);
  free(pHist2);
  free(pBwd3);
  free(pHist3);
  free(pRatio);
  free(pRecon);
  free(pNorm);
}