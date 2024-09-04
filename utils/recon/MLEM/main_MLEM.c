#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>
#include <getopt.h>
#include <sys/stat.h>
#include "psf_analytic.h"
#include "global_var_dec.h"
#define _GNU_SOURCE
#define OUTFILE1 "DHAPET_original_forward.DAT"
#define OUTFILE2 "DHAPET_original_backward.DAT"
 

float s_time () { return (float) clock() / CLOCKS_PER_SEC; }


int nboundary (int cy, int cz)
{
   return cy < ncy_k && cy >= 0 &&
          cz < ncz_k && cz >= 0   ;
}


int boundary (int cy, int cz)
{
   return cy < ncy && cy >= 0 &&
          cz < ncz && cz >= 0   ;
}


int boundary_z (int cz) { return cz < ncz && cz >= 0; }


int boundary_y (int cy) { return cy < ncy && cy >= 0; }


/// ====== ====== ====== ====== ====== ///
/// ======  Function: config()  ====== ///
/// ====== ====== ====== ====== ====== ///

void config ()
{
    // int ncy = 16;
    // int ncz = 32;
    // int n_crsl2pxl = 4;

    nimgx   = spacing / vx;  // e.g. spacing 60 / vx 0.8 = 75 xplanes
    nimgy   = ncy * n_crsl2pxl;
    nimgz   = ncz * n_crsl2pxl;
    ncrow   = ncy * ncz;
    nccol   = ncy * ncz;
    ncy_k   = ncz * 2;
    ncz_k   = ncz * 2;
    nkernel = 4;
    ncrow_k = ncy_k * ncz_k;
    nccol_k = ncrow_k;
    
    py_edge_low = -ncy_k * cw / 2.;
    py_edge_up  =  ncy_k * cw / 2.;
    pz_edge_low = -ncz_k * cw / 2.;
    pz_edge_up  =  ncz_k * cw / 2.;
    vy = cw / (n_crsl2pxl);
    vz = cw / (n_crsl2pxl);
    
    kvx_edge_low = vx * (x_plane - 0.5);
    kvx_edge_up  = vx * (x_plane + 0.5);
    
    /*
    printf(" ncy=%d, ncz=%d\n", ncy,ncz); 
    printf(" pitch=%.4f\n", pitch); 
    printf(" nimgy=%d, nimgz=%d, nimgx=%d\n", nimgy,nimgz,nimgx); 
    printf(" ngrid=%d\n", ngrid); 
    printf(" spacing=%4.0fmm\n", spacing);
    printf(" index of xplane=%d\n", x_plane);
    printf(" the object data will be saved with description: %s\n", description_name);
    */
}


/// ====== ====== ====== ====== ====== ///
/// ====== Function: forward()  ====== ///
/// ====== ====== ====== ====== ====== ///

void forward (float* pFwd, float* phantom, double* board0, double* board1)
{
    int i, j;
    int jlayer1 = 1;
    int jlayer2 = 1;
    int fx_plane = x_plane + (int)(nimgx / 2);  // [Previous] x_plane + spacing;
    
    FILE *inTAB, *inINCRE, *inSparse;
    TABLE_ZIP *pTable = (TABLE_ZIP *)calloc(nccol_k+1, sizeof(TABLE_ZIP));
    
    /* - Testing files for loading data -
    FILE* dat3 = fopen("Dat3.txt", "w");
    FILE* dat4 = fopen("Dat4.txt", "w");
    FILE* dat5 = fopen("Dat6.txt", "w");
    FILE* dat6 = fopen("Dat6.txt", "w");
    */
    
    for (i = 0; i < nkernel; i++)
    {
        for (j = 0; j < nkernel; j++)
        {
            char name[500];
            sprintf(kernel_name[i*nkernel+j], "%d_%d", j, i);
            
            sprintf(name, "data/SRM/_srm_sp%.0f_vx%.1f/sparse_table_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
            inTAB = fopen(name,"rb");
            fread(pTable, sizeof(TABLE_ZIP), nccol_k+1, inTAB);
            
            INCREMENT_ZIP *pIncre = (INCREMENT_ZIP *)calloc(pTable[nccol_k].bias, sizeof(INCREMENT_ZIP));
            float *pSparse = (float *)calloc(pTable[nccol_k].bias, sizeof(float)); 
            
            sprintf(name, "data/SRM/_srm_sp%.0f_vx%.1f/sparse_incre_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
            inINCRE = fopen(name,"rb");
            fread(pIncre, sizeof(INCREMENT_ZIP), pTable[nccol_k].bias, inINCRE);
            
            sprintf(name, "data/SRM/_srm_sp%.0f_vx%.1f/sparse_matrix_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
            inSparse = fopen(name,"rb");
            fread(pSparse, sizeof(float), pTable[nccol_k].bias, inSparse);

            // [Debug] print some info
            // printf("[test] SRM file: %s \n", name);
            // printf("- pTable[nccol_k].bias = %d \n", pTable[nccol_k].bias);

            int iz, jy, kcz, kcy;
            int LOR_initial = pTable[nccol_k].bias;
            
            for (iz = 0; iz < ncz; iz++)
            {
                int shift_z = (iz*4 - ncz*2) / 4;
                for (jy = 0; jy < ncy; jy++)
                {
                    int shift_y = (jy*4 - ncy*2) / 4;  
                    for (kcz = 0; kcz < ncz; kcz++)
                    {
                        int cz1 = kcz + symz - shift_z;
                        for (kcy = 0; kcy < ncy; kcy++)
                        {
                            int  cy1 = kcy + symy - shift_y;
                            float h1 = board1[kcy*ncz + kcz];
                            if (nboundary(cy1, cz1))
                            {
                                int ktotal;
                                int cy2min  = pTable[cz1*ncy_k + cy1].cymin + shift_y;
                                int cz2min  = pTable[cz1*ncy_k + cy1].czmin + shift_z;
                                int c2total = pTable[cz1*ncy_k + cy1+1].bias;
                                
                                // fprintf(dat5, "%d\n", cy1-symy);
                                // fprintf(dat6, "%d\n", cz1-symz);
                                // if (c2total > 0) { printf("c2total=%d\n",c2total); }
                                // if (pTable[cz1*ncy_k/4+cy1].bias > 0) { printf("ktotal=%d\n",pTable[cz1*ncy_k/4+cy1].bias); }
                                
                                for (ktotal = pTable[cz1*ncy_k+cy1].bias; ktotal < c2total; ktotal++)
                                {
                                    int cy2 = pIncre[ktotal]._y + cy2min;
                                    int cz2 = pIncre[ktotal]._z + cz2min;
                                    // [Debug] check range of ktotal
                                    // if (ktotal > 63900) printf("- ktotal: %d, cy2= %d, cz2= %d \n", ktotal, cy2, cz2);
                                    
                                    if (boundary(cy2-symy, cz2-symz))
                                    {
                                        float h0 = board1[(cy2-symy)*ncz + cz2-symz];
                                        
                                        pFwd[(kcz)*ncy*ncrow + (kcy)*ncrow + (cz2-symz)*ncy + (cy2-symy)] +=
                                              pSparse[ktotal] * phantom[fx_plane*nimgz*nimgy + (iz*nkernel+i)*nimgy + jy*nkernel+j];
                                        
                                        // fprintf(dat3, "%d\n", cy2-symy);
                                        // fprintf(dat4, "%d\n", cz2-symz);
                                        // printf(" p: %f \n", phantom[59*nimgz*nimgy + (iz*4+i)*nimgy + jy*4+j]);
                                        // printf(" pSparse_values = %f \n", ngrid*ngrid*ngrid*720*2880*pSparse[ktotal]);
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
            // fclose(dat3);
            // fclose(dat4);
            // fclose(dat5);
            // fclose(dat6);
        }
    }
    free(pTable);
}


/// ====== ====== ====== ====== ====== ///
/// ======   Function: ratio()  ====== ///
/// ====== ====== ====== ====== ====== ///

void ratio (float *pFW, float *pHist, double *pRatio)
{
    int i;
    float loss = 0;
    
    for (i = 0; i < ncrow * nccol; i++)
    {
        if (pHist[i] >= 0)
        {
            if (pFW[i] > 0)
            {
                pRatio[i] =  pHist[i] / pFW[i];
                // printf("First Condition: index= %d, Ratio= %f\n", i, pRatio[i]);
            }
            else
            {
                if (pHist[i] > 0) { pRatio[i] = 0; }  // non_zero / zero
                else { pRatio[i] = 1.0; }  // zero / zero
            }
            loss = loss + (pFW[i] - pHist[i]);
        }
        /*
        if (pRatio[i] > 1024.0)
        {
            pRatio[i] = 1024.0;
        }
        if (pFW[i] != pFW[i]) { printf("- This is NaN at LOR = %d, with pHist = %f. \n", i, pHist[i]); }
        */
    }
    printf("[LOSS] inside ratio function: %f \n", loss);
}


/// ====== ====== ====== ====== ====== ///
/// ====== Function: backward() ====== ///
/// ====== ====== ====== ====== ====== ///

void backward (double* pRatio, float* pRecon, float* pNorm, double* board0, double* board1)
{
    int i, j;
    int jlayer1 = 1;
    int jlayer2 = 1;
    int rx_plane = x_plane + (int)(nimgx / 2);  // [Previous] x_plane + spacing
    
    FILE *inTAB, *inINCRE, *inSparse;
    TABLE_ZIP* pTable = (TABLE_ZIP *)calloc(nccol_k+1, sizeof(TABLE_ZIP));
    
    for (i = 0; i < nkernel; i++)
    {
        for (j = 0; j < nkernel; j++)
        {
            char name[500];
            sprintf(kernel_name[i*nkernel+j], "%d_%d", j, i);
            
            sprintf(name, "data/SRM/_srm_sp%.0f_vx%.1f/sparse_table_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
            inTAB = fopen(name,"rb");
            fread(pTable, sizeof(TABLE_ZIP), nccol_k+1, inTAB);
            
            INCREMENT_ZIP* pIncre = (INCREMENT_ZIP *)calloc(pTable[nccol_k].bias, sizeof(INCREMENT_ZIP));
            float* pSparse = (float *)calloc(pTable[nccol_k].bias, sizeof(float));  
            
            sprintf(name, "data/SRM/_srm_sp%.0f_vx%.1f/sparse_incre_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
            inINCRE = fopen(name,"rb");
            fread(pIncre, sizeof(INCREMENT_ZIP), pTable[nccol_k].bias, inINCRE);
            
            sprintf(name, "data/SRM/_srm_sp%.0f_vx%.1f/sparse_matrix_plane_%d_%s", spacing, vx, x_plane, kernel_name[i*nkernel+j]);
            inSparse = fopen(name,"rb");
            fread(pSparse, sizeof(float), pTable[nccol_k].bias, inSparse);
            
            int iz, jy, kcz, kcy;
            int LOR_initial = pTable[nccol_k].bias;
            
            for (iz = 0; iz < ncz; iz++)
            {
                int shift_z = (iz*4 - ncz*2) / 4;
                for (jy = 0; jy < ncy; jy++)
                {
                    int shift_y = (jy*4 - ncy*2) / 4;
                    for (kcz = 0; kcz < ncz; kcz++)
                    {
                        int cz1 = kcz + symz - shift_z;
                        for (kcy = 0; kcy < ncy; kcy++)
                        {
                            float h1 = board1[kcz*ncy + kcy];
                            int cy1 = kcy + symy - shift_y;
                            if (nboundary(cy1, cz1))
                            {
                                int cy2min  = pTable[cz1*ncy_k + cy1].cymin + shift_y;
                                int cz2min  = pTable[cz1*ncy_k + cy1].czmin + shift_z;
                                int c2total = pTable[cz1*ncy_k + cy1+1].bias;
                                
                                int ktotal;
                                for(ktotal = pTable[cz1*ncy_k+cy1].bias; ktotal < c2total; ktotal++)
                                {
                                    int cy2 = pIncre[ktotal]._y + cy2min;
                                    int cz2 = pIncre[ktotal]._z + cz2min;
                                    if (boundary(cy2-symy, cz2-symz))
                                    {
                                        float h0 = board1[(cz2-symz)*ncy + cy2-symy];
                                        
                                        pRecon[rx_plane*nimgz*nimgy + (iz*nkernel+i)*nimgy + jy*nkernel+j] += 
                                               pRatio[(kcz)*ncy*ncrow+(kcy)*ncrow+(cz2-symz)*ncy+(cy2-symy)] * pSparse[ktotal];
                                        
                                        pNorm[rx_plane*nimgz*nimgy + (iz*nkernel+i)*nimgy + jy*nkernel+j] += pSparse[ktotal];
                                        // printf("h0: %f, h1: %f \n", h0, h1);
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


/// ====== ====== ====== ====== ====== ///
/// ======  Function: update()  ====== ///
/// ====== ====== ====== ====== ====== ///

void update (float* pBwd, float* pRecon, float* pNorm)
{
    int ix, i;
    for (ix = 0; ix < nimgx; ix++)
    {
        for (i = 0; i < nimgy*nimgz; i++)
        {
            if (pNorm[ix*nimgz*nimgy + i])
            {
                pBwd[ix*nimgz*nimgy + i] = pBwd[ix*nimgz*nimgy + i] * pRecon[ix*nimgz*nimgy + i] / (pNorm[ix*nimgz*nimgy + i]);
                // printf("pNorm[ix*nimgz*nimgy+i] %f\n", pNorm[ix*nimgz*nimgy+i]);
            }
            else
            {
                pBwd[ix*nimgz*nimgy + i] = 1;
                // printf("pNorm[ix*nimgz*nimgy+i] %f\n", pNorm[ix*nimgz*nimgy+i]);
            }
        }
    }
}


/// ====== ====== ====== ====== ====== ///
/// ======   Function: main()   ====== ///
/// ====== ====== ====== ====== ====== ///

int main (int argc, char *argv[])
{    
    // - Print all input arguments -
    while (argc--) { printf("argv[%d]: %s \n", argc, argv[argc]); }
    
    char bin_name[100];
    int iterations, xplanes;
    
    // - Parse external arguments -
    sprintf(bin_name, "%s", argv[1]);
    iterations = atoi(argv[2]);
    spacing    = atof(argv[3]);
    vx         = atof(argv[4]);
    xplanes    = atoi(argv[5]);
    
    // - Remove previous DAT output folder and create new one -
    char rm_cmd[300];
    sprintf(rm_cmd, "rm -rf data/recon/MLEM/OUTPUT/%s/ && mkdir data/recon/MLEM/OUTPUT/%s/", bin_name, bin_name);
    system(rm_cmd);
    
    // - Load configuration parameters -
    config();
    printf("\n[Info] bin_name = %s, iterations = %d, spacing = %.2f, vx = %.2f, xplanes = %d, nimgx = %d\n", bin_name, iterations, spacing, vx, xplanes, nimgx);

    float  *pHist   = (float  *)calloc(nccol*ncrow,       sizeof(float));
    float  *pFwd    = (float  *)calloc(nccol*ncrow,       sizeof(float));
    double *pRatio  = (double *)calloc(nccol*ncrow,       sizeof(double));
    float  *phantom = (float  *)calloc(nimgx*nimgy*nimgz, sizeof(float));
    float  *pBwd    = (float  *)calloc(nimgx*nimgy*nimgz, sizeof(float));
    float  *pRecon  = (float  *)calloc(nimgx*nimgy*nimgz, sizeof(float));
    float  *pNorm   = (float  *)calloc(nimgx*nimgy*nimgz, sizeof(float));
    
    // FILE *fPhantom;
    int    i, ix, iy, iz;
    // char   phname[100];
    // float* phantom_yz = (float *) calloc(nimgy*nimgz, sizeof(float));
    
    // - Initial array -
    for (ix = 0; ix < nimgx; ix++)
    {
        for (i = 0; i < nimgy*nimgz; i++)
        {
            pBwd[ix*nimgz*nimgy+i] = .5;
        }
    }
    memset(pFwd, 0, nccol*ncrow*sizeof(float));
    
    double board0[512];
    double board1[512];
    double sensitivity;
    int  bufferLength = 512;
    char buffer[bufferLength];
    int  ID = 0;
    
    FILE *fBoard0;
    fBoard0 = fopen("data/recon/MLEM/INPUT/Board0.dat","r");
    
    while (fgets(buffer, bufferLength, fBoard0))
    {
        if (1 == sscanf(buffer, "%lf", &sensitivity))
        {
            board0[ID] = sensitivity;
            // printf("a:%d, b:%d, c:%d, d:%lf \n", BoardID, Z, Y, sensitivity);
        }
        ID++;
    }
    fclose(fBoard0);
    
    ID = 0;
    FILE *fBoard1;
    fBoard1 = fopen("data/recon/MLEM/INPUT/Board1.dat","r");
    
    while (fgets(buffer, bufferLength, fBoard1))
    {
        if (1 == sscanf(buffer, "%lf", &sensitivity))
        {
            board1[ID] = sensitivity;
            // printf("a:%d, b:%d, c:%d, d:%lf \n", BoardID, Z, Y, sensitivity);
        }
        ID++;
    }
    fclose(fBoard1);
    
    char hist_name[300];
    sprintf(hist_name, "data/recon/MLEM/INPUT/Hist_sort_%s.dat", bin_name);
    printf("[File] Hist data: %s \n", hist_name);
    
    FILE *fHist;
    fHist = fopen(hist_name, "rb");
    fread(pHist, sizeof(float), nccol*ncrow, fHist);
    fclose(fHist);
    
    /// ====== ====== ====== Start ML-EM ====== ====== ====== ///
    int ite;
    float T[2];
    char out_name[300];
    // float pLoss = 0.0;
    
    for (ite = 0; ite < iterations; ite++)
    {
        printf("\n[Stat] Iteration = %d \n", ite+1);
        T[0] = s_time();

        memset(pFwd,   0, nccol*ncrow*sizeof(float));
        memset(pRecon, 0, nimgx*nimgy*nimgz*sizeof(float));
        memset(pNorm,  0, nimgx*nimgy*nimgz*sizeof(float));
        
        /// ====== ====== Forward stage ====== ====== ///
        for (x_plane = -xplanes/2; x_plane < xplanes/2; x_plane++)
        {
            // printf("[Forward] at plane %d\n", x_plane);  //[Debug]
            forward(pFwd, pBwd, board0, board1);
        }
        ratio(pFwd, pHist, pRatio);
        
        /*
        pLoss = 0.0;
        for(int m=0; m < ncrow*nccol; m++)
        {
            pLoss = pLoss + abs((pFwd[m]-pHist[m] ));
        }
        printf("Loss function = %f\n", (pLoss));
        */
        
        /// ====== ====== Backward stage ====== ====== ///
        for (x_plane = -xplanes/2; x_plane < xplanes/2; x_plane++)
        { 
            backward(pRatio, pRecon, pNorm, board0, board1);
        }
        update(pBwd, pRecon, pNorm);
        
        T[1] = s_time() - T[0];
        printf("[Time] %g sec \n", T[1]);
        
        /// ====== ====== Output recon file ====== ====== ///
        sprintf(out_name, "data/recon/MLEM/OUTPUT/%s/MLEM_ite%d_%s.DAT", bin_name, ite+1, bin_name);
        FILE *fMLEM_DAT;
        if ((fMLEM_DAT = fopen(out_name,"wb")) == NULL)
        {
            printf("[Err] Can not open file: %s\n", out_name);
            exit(0);
        }
        fwrite(pBwd, sizeof(float)*nimgx*nimgy*nimgz, 1, fMLEM_DAT);
        fclose(fMLEM_DAT);
    }
    free(pFwd);
    free(pBwd);
    free(phantom);
    free(pHist);
    free(pRatio);
    free(pRecon);
    free(pNorm);
}