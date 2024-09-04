#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "global_var_dec.h"
#include "psf_analytic.h"
#define PI 3.1415926
#define Tr 100.0        //timing resolution of detectors is 100 ps


float s_time()
{
    return (float) clock() / CLOCKS_PER_SEC;
}


int is_at_pitch(float a)
{
    a = a - pitch_half;
    if(a < 0) return 1;
    
    // a = a - cw * (int)(a / cw);
    a = fmodf(a, cw);

    if(a >= cw - pitch) return 1;
    else return 0;
}


int crystal_tracing(float in_p, float out_p, TRACING_CRY *F_alpha)
{   // one dimension raytracing without pitch. The original point is at left, NOT at center.
    int cry_bndery_start, cry_bndery_end;
    int k, i;

    if(out_p > in_p)
    {
        cry_bndery_start = (int)(in_p / cw) + 1;
        cry_bndery_end = (int)(out_p / cw);

        F_alpha[0].alpha = 0;
        F_alpha[1].alpha = 1;
        // F_alpha[2].alpha = 0.5;
        
        for(i = 2, k = cry_bndery_start; k <= cry_bndery_end; k++, i++)
        {
            F_alpha[i].alpha = (k * cw - in_p) / (out_p - in_p);
        }
        return i+1;
    }
    else if(out_p < in_p)
    {
        cry_bndery_start = (int)(in_p / cw);
        cry_bndery_end = (int)(out_p / cw) + 1;

        F_alpha[0].alpha = 0;
        F_alpha[1].alpha = 1;
        // F_alpha[2].alpha = 0.5;
        
        for(i = 2, k = cry_bndery_start; k >= cry_bndery_end; k--, i++)
        {
            F_alpha[i].alpha = (k * cw - in_p) / (out_p - in_p);
        }
        return i+1;
    }
    else
    {
        F_alpha[0].alpha = 0;
        F_alpha[1].alpha = 1;
        // F_alpha[2].alpha = 0.5;
        return 2;
    }
}


int crystal_face_tracing(float in_p, float out_p, TRACING_CRY *F_alpha)
{   // one dimension raytracing with pitch. The original point is at left, NOT at center.
    int in_cry_end, in_cry_start;
    int out_cry_end, out_cry_start;
    // these are indice of crystal whose faces are traced by the ray.
    // e.g, in_cry_end = 5 is the  face of No.5 crystal which ray-tracing is incident on, "end" means the face of No.5 is last face traced by ray;
    // e.g, out_cry_start = 5 is the face of No.5 crystal which ray-tracing left, "start" means the face of No.5 the first face traced by ray;

    int k, i;
    
    if(out_p > in_p)
    {
        in_cry_start = (int)((in_p - pitch_half)  / cw) + 1;
        in_cry_end   = (int)((out_p - pitch_half) / cw);
        /*
        in_cry_start = (in_cry_end < in_cry_start) ? in_cry_start : in_cry_start-1;
        if(in_cry_end < in_cry_start && is_at_pitch(in_p))
        {
          in_cry_start=in_cry_start - 1;
        }
        */
        
        if(is_at_pitch(out_p))
        {
            // printf("out_p in pitch.\n");
            out_cry_end = in_cry_end;
            F_alpha[1].alpha = 1;
            F_alpha[1].io = 'o';
        }
        else
        {
            // printf("out_p in crystal.\n");
            out_cry_end = in_cry_end - 1;
            F_alpha[1].alpha = 1;
            F_alpha[1].io = 'i';
        }
        
        if(is_at_pitch(in_p)) 
        {
            // printf("in_p in pitch.\n");
            out_cry_start = in_cry_start;
            F_alpha[0].alpha = 0;
            F_alpha[0].io = 'o';
            
            for(i = 2, k = in_cry_start; k <= in_cry_end; k++, i+=2)
            {
                F_alpha[i].alpha = (k * cw + pitch_half - in_p) / (out_p - in_p);
                F_alpha[i].io = 'i';
            }
            
            for(i = 3, k = out_cry_start; k <= out_cry_end; k++, i+=2)
            {
                F_alpha[i].alpha = ((k + 1) * cw - pitch_half - in_p) / (out_p - in_p);
                F_alpha[i].io = 'o';
            }
        }
        else
        {
            // printf("in_p in crystal.\n");
            out_cry_start = in_cry_start - 1;
            F_alpha[0].alpha = 0;
            F_alpha[0].io = 'i';
            
            for(i = 3, k = in_cry_start; k <= in_cry_end; k++, i+=2)
            {
                F_alpha[i].alpha = (k * cw + pitch_half - in_p) / (out_p - in_p);
                F_alpha[i].io = 'i';
            }
            
            for(i = 2, k = out_cry_start; k <= out_cry_end; k++, i+=2)
            {
                F_alpha[i].alpha = ((k + 1) * cw - pitch_half - in_p) / (out_p - in_p);
                F_alpha[i].io = 'o';
            }
        }

        if(in_cry_start > in_cry_end && is_at_pitch(in_p) && is_at_pitch(out_p))
        {
            return 0;
        }
        else
        {
            return (out_cry_end - out_cry_start + 1) + (in_cry_end - in_cry_start + 1) + 2;
        }
    }
    else if(out_p < in_p)
    {
        out_cry_start = (int)((in_p - pitch_half)  / cw);
        out_cry_end   = (int)((out_p - pitch_half) / cw) + 1;
        
        if(is_at_pitch(out_p))
        {
            in_cry_end = out_cry_end;
            F_alpha[1].alpha = 1;
            F_alpha[1].io = 'o';
        }
        else
        {
            in_cry_end = out_cry_end - 1;
            F_alpha[1].alpha = 1;
            F_alpha[1].io = 'i';
        }
        
        if(is_at_pitch(in_p))
        {
            in_cry_start = out_cry_start;
            F_alpha[0].alpha = 0;
            F_alpha[0].io = 'o';
            
            for(i = 2, k = in_cry_start; k >= in_cry_end; k--, i+=2)
            {
                F_alpha[i].alpha = (in_p - ((k + 1) * cw - pitch_half)) / (in_p - out_p);
                F_alpha[i].io = 'i';
            }
            
            for(i = 3, k= out_cry_start; k >= out_cry_end; k--, i+=2)
            {
                F_alpha[i].alpha = (in_p - (k * cw + pitch_half)) / (in_p - out_p);
                F_alpha[i].io = 'o';
            }
        }
        else
        {
            in_cry_start = out_cry_start - 1;
            F_alpha[0].alpha = 0;
            F_alpha[0].io = 'i';

            for(i = 2, k = out_cry_start; k >= out_cry_end; k--, i+=2)
            {
                F_alpha[i].alpha = (in_p - (k * cw + pitch_half)) / (in_p - out_p);
                F_alpha[i].io = 'o';
            }

            for(i = 3, k = in_cry_start; k >= in_cry_end; k--, i+=2)
            {
                F_alpha[i].alpha = (in_p - ((k + 1) * cw - pitch_half)) / (in_p - out_p);
                F_alpha[i].io = 'i';
            }
        }
        
        if(out_cry_end > out_cry_start && is_at_pitch(in_p) && is_at_pitch(out_p))
        {
            return 0;
        }
        else
        {
            return (out_cry_start - out_cry_end + 1) + (in_cry_start - in_cry_end + 1) + 2;
        }
    }
    else
    {
        if(is_at_pitch(in_p))
        {
            return 0;
        }
        else
        {
            F_alpha[0].alpha = 0;F_alpha[1].alpha = 1;
            F_alpha[0].io = 'i';F_alpha[1].io = 'i';
            return 2;
        }
    }
}


int is_hit_panel(float y, float z)
{
	if(y < py_edge_up - pitch_half && y > py_edge_low && z < pz_edge_up - pitch_half && z > pz_edge_low) return 1;
	else return 0;
}


void single_ray_tracing_panel(float yz_p[][4], TRACING_CRY trace_P[], int start_end[], TRACING_CRY** Y_alpha, TRACING_CRY** Z_alpha, TRACING_CRY** YZ_alpha, int nYZ[], float L, float trace_p_L[])
{
    int i, k;
    int numY_alpha, numZ_alpha;
    float y_in, y_out, z_in, z_out;
    float y_in_p, y_out_p, z_in_p, z_out_p;

    y_in_p  = yz_p[0][0];
    z_in_p  = yz_p[0][1];
    y_out_p = yz_p[0][2];
    z_out_p = yz_p[0][3];

    if(is_hit_panel(yz_p[0][2], yz_p[0][3]))
    {
        trace_P[0].alpha = 0;   trace_P[0].io = 'x';
        trace_P[1].alpha = 0.5; trace_P[1].io = 'm';
        trace_P[2].alpha = 1;   trace_P[2].io = 'X';
        
        start_end[0] = 0;
        start_end[1] = 2;
    }
    else
    {
        trace_P[0].alpha = 0; trace_P[0].io = 'x';
        trace_P[1].alpha = 1; trace_P[1].io = 'X';

        trace_P[2].alpha = (py_edge_low - y_in_p) / (y_out_p - y_in_p); trace_P[2].io = 'y';
        trace_P[3].alpha = (py_edge_up - pitch_half  - y_in_p) / (y_out_p - y_in_p); trace_P[3].io = 'Y';
        trace_P[4].alpha = (pz_edge_low - z_in_p) / (z_out_p - z_in_p); trace_P[4].io = 'z';
        trace_P[5].alpha = (pz_edge_up - pitch_half  - z_in_p) / (z_out_p - z_in_p); trace_P[5].io = 'Z';

        trace_P[6].alpha = 0.5; trace_P[6].io = 'm';

        quicksort_tracing_cry(7,trace_P);

        for(i = 0; i < 7; i++)
        {
            if(trace_P[i].alpha == 0)
            {
                start_end[0] = i;
                trace_P[i].io = 'x';
            }
        }

        for(i = start_end[0]; i < 7; i++)
        {             
            if(trace_P[i].io != 'x' && trace_P[i].io != 'm')
            {
                start_end[1] = i;
                break;
            }
        }
    }

    for(i = start_end[0]+1, k = 0; i <= start_end[1]; i++, k++)
    {
        y_in  = trace_P[i-1].alpha * (y_out_p - y_in_p) + y_in_p;
        z_in  = trace_P[i-1].alpha * (z_out_p - z_in_p) + z_in_p;
        y_out = trace_P[i].alpha * (y_out_p - y_in_p) + y_in_p;
        z_out = trace_P[i].alpha * (z_out_p - z_in_p) + z_in_p;

        yz_p[k][0] = y_in;
        yz_p[k][1] = z_in;
        yz_p[k][2] = y_out;
        yz_p[k][3] = z_out;

        trace_p_L[k] = (trace_P[i].alpha - trace_P[i-1].alpha) * L;

        if(pitch > 0)
        {
            numY_alpha = crystal_face_tracing(y_in - py_edge_low, y_out - py_edge_low, Y_alpha[k]);
            numZ_alpha = crystal_face_tracing(z_in - pz_edge_low, z_out - pz_edge_low, Z_alpha[k]);
        }
        else
        {
            numY_alpha = crystal_tracing(y_in - py_edge_low, y_out - py_edge_low, Y_alpha[k]);
            numZ_alpha = crystal_tracing(z_in - pz_edge_low, z_out - pz_edge_low, Z_alpha[k]);
        }
        
        memcpy(YZ_alpha[k], Y_alpha[k], numY_alpha * sizeof(TRACING_CRY));
        memcpy(YZ_alpha[k] + numY_alpha, Z_alpha[k], numZ_alpha * sizeof(TRACING_CRY));
    
        quicksort_tracing_cry(numY_alpha + numZ_alpha, YZ_alpha[k]);

        if(pitch > 0)
        {
            nYZ[k] = merge_with_pitch(numY_alpha + numZ_alpha, YZ_alpha[k]);
        }
        else
        {
            nYZ[k] = merge_without_pitch(numY_alpha + numZ_alpha, YZ_alpha[k]);
        }
    }
    return;
}


void single_response(float yz_p[][4], float trace_p_L[], int nYZ[], TRACING_CRY** YZ_alpha, RESPONSE_CRY** P, int nP[],float fly[],float PHI)
{
    float travel_p_L;
    float track_cry;
    float medium;
    int   jlayer, itrack, jYZ;
    float d_angle = PI / 1440.0;
    float exp_order = 1.0;

    if(pitch > 0)
    {
        for(jlayer = 0, travel_p_L = 0; jlayer < 2; jlayer++)
        {
            for(jYZ = 0, itrack = 0; jYZ < nYZ[jlayer] - 2; jYZ++)
            {
                if(YZ_alpha[jlayer][jYZ].io == 'i' && YZ_alpha[jlayer][jYZ+1].io == 'o')
                {
                    track_cry = trace_p_L[jlayer] * (YZ_alpha[jlayer][jYZ+1].alpha - YZ_alpha[jlayer][jYZ].alpha);
                    medium = 0.5 * (YZ_alpha[jlayer][jYZ+1].alpha + YZ_alpha[jlayer][jYZ].alpha);
                    P[jlayer][itrack].cy = (int)((medium * (yz_p[jlayer][2] - yz_p[jlayer][0]) + yz_p[jlayer][0] - py_edge_low) / cw);
                    P[jlayer][itrack].cz = (int)((medium * (yz_p[jlayer][3] - yz_p[jlayer][1]) + yz_p[jlayer][1] - pz_edge_low) / cw);
                    // float dr=(fly[0]+travel_p_L);//*0.5*cos(PHI)*(tan(PHI+0.5*d_angle)-tan(PHI-0.5*d_angle));
                    P[jlayer][itrack].resp = exp(-mu_lso * travel_p_L) * (1 - exp(-mu_lso * track_cry));//*pow(dr,2)*d_angle*d_angle;
                    itrack++;
                    travel_p_L += track_cry;
                    // P[jlayer][itrack].resp = P[jlayer][itrack].resp * ((fly[0]+travel_p_L)*(fly[0]+travel_p_L)*d_angle);
                    
                    if(P[jlayer][itrack].cy<0 || P[jlayer][itrack].cz<0 || P[jlayer][itrack].cy>2*ncy || P[jlayer][itrack].cz>2*ncz)
                    {
                        /*
                        printf("cy=%d, ",P[jlayer][itrack].cy);
                        printf("cz=%d, ",P[jlayer][itrack].cz);
                        printf("alpha_0=%g, ",YZ_alpha[jlayer][jYZ].alpha);
                        printf("alpha_1=%g, ",YZ_alpha[jlayer][jYZ+1].alpha);
                        printf("y_in=%g, ",yz_p[jlayer][0]);
                        printf("y_out=%g, ",yz_p[jlayer][2]);
                        printf("z_in=%g, ",yz_p[jlayer][1]);
                        printf("z_out=%g, \n",yz_p[jlayer][3]);
                        printf("PHI=%g, \n",PHI);
                        printf("cz=%d, ",P[jlayer][itrack].cz);
                        */
                    }
                }
            }
            
            if(nYZ[jlayer] >= 2  && YZ_alpha[jlayer][nYZ[jlayer]-2].io == 'i' && YZ_alpha[jlayer][nYZ[jlayer]-1].io == 'i')
            {
                // printf("Hit here.\n");
                jYZ = nYZ[jlayer] - 2;
                track_cry = trace_p_L[jlayer] * (YZ_alpha[jlayer][jYZ+1].alpha - YZ_alpha[jlayer][jYZ].alpha);
                medium = 0.5 * (YZ_alpha[jlayer][jYZ+1].alpha + YZ_alpha[jlayer][jYZ].alpha);
                P[jlayer][itrack].cy = (int)((medium * (yz_p[jlayer][2] - yz_p[jlayer][0]) + yz_p[jlayer][0] - py_edge_low) / cw);
                P[jlayer][itrack].cz = (int)((medium * (yz_p[jlayer][3] - yz_p[jlayer][1]) + yz_p[jlayer][1] - pz_edge_low) / cw);
                // float dr=(fly[0]+travel_p_L);//*0.5*cos(PHI)*(tan(PHI+0.5*d_angle)-tan(PHI-0.5*d_angle));
                P[jlayer][itrack].resp = exp(-mu_lso * travel_p_L) * (1 - exp(-mu_lso * track_cry));//*pow(dr,2)*d_angle*d_angle;
                itrack++;
                travel_p_L += track_cry;
                // P[jlayer][itrack].resp = P[jlayer][itrack].resp * ((fly[0]+travel_p_L)*(fly[0]+travel_p_L)*d_angle);
            }
            nP[jlayer] = itrack;
        }
    }
    else
    {
        float icry,seg_area,r,dfly;
        for(jlayer = 0, travel_p_L = 0; jlayer < 2; jlayer++)
        {
            for(jYZ = 0, itrack = 0; jYZ < nYZ[jlayer] - 1; jYZ++)
            {
                track_cry = trace_p_L[jlayer] * (YZ_alpha[jlayer][jYZ+1].alpha - YZ_alpha[jlayer][jYZ].alpha);
                medium = 0.5 * (YZ_alpha[jlayer][jYZ+1].alpha + YZ_alpha[jlayer][jYZ].alpha);
                P[jlayer][itrack].cy = (int)((medium * (yz_p[jlayer][2] - yz_p[jlayer][0]) + yz_p[jlayer][0] - py_edge_low) / cw);
                P[jlayer][itrack].cz = (int)((medium * (yz_p[jlayer][3] - yz_p[jlayer][1]) + yz_p[jlayer][1] - pz_edge_low) / cw);

                // exp_order=exp_order*(exp(-mu_lso*travel_p_L-cry[icry-1])-exp(-mu_lso*travel_p_L));
                // exp_order=(icry>1) ? exp_order : (exp_order+1.0);
                // P[jlayer][itrack].resp = exp(-mu_lso * travel_p_L) * (track_cry - (exp(-mu_lso * travel_p_L)-exp(-mu_lso * (travel_p_L+track_cry))) / mu_lso);

                P[jlayer][itrack].resp = exp(-mu_lso * travel_p_L) * (1 - exp(-mu_lso * track_cry));
                // r = sqrt(pow((yz_p[jlayer][2]-yz_p[jlayer][0]),2)+pow((yz_p[jlayer][3]-yz_p[jlayer][1]),2));
                
                itrack++;
                travel_p_L += track_cry;
                icry=track_cry;
            }
            nP[jlayer] = itrack;
        }
    }
}


/*
int tof_information(float x, float y, float z, float yz_p1[][4], float yz_p2[][4])
{	 
    float xc,yc,zc;  //middle point of LOR
    float distance,distance_x,distance_y,distance_z;
    float time;
    int t_inf=0;
    xc=0;
    yc=( yz_p1[0][0] + yz_p1[0][2] + yz_p2[0][0] + yz_p2[0][2] )*0.25;
    zc=( yz_p1[0][1] + yz_p1[0][3] + yz_p2[0][1] + yz_p2[0][3] )*0.25;

    distance_x = fabs(x-xc);
    distance_y = fabs(y-yc);
    distance_z = fabs(z-zc);
    distance = sqrt(distance_x*distance_x+distance_y*distance_y+distance_z*distance_z);
    time = 2.0*(distance/3.0)*10.0;
    time = 2 x s / c
    t_inf = floor(time / Tr);

    return t_inf;
}
*/


void coincidence_response(float* pCOIN, /* int* tCOIN, */ RESPONSE_CRY** P1, RESPONSE_CRY** P2, int nP1[], int nP2[], unsigned char tag_layers, float PHI, float THETA,float yz_p1[][4],float yz_p2[][4],int nYZ_1[],TRACING_CRY** YZ_alpha_1, float x, float y ,float z)
{
    int   jlayer1, jlayer2;
    int   jtrack1, jtrack2;
    float resp1, resp2;
    int   index;
    int   cy1, cy2, cz1, cz2;
    float yz_c1[3], yz_c2[3];
    float tmp_coinresp = 0;
    /////////////////////////////////////////
    // jlayer1 = (tag_layers & 0xf0) >> 4;
    // jlayer2 =  tag_layers & 0x0f;
    /////////////////////////////////////////
    // float seg_area_1,seg_area_2;

    jlayer1 = 0;
    jlayer2 = 0;

    for(jtrack1 = 0; jtrack1 < nP1[jlayer1]; jtrack1++)
    {        
        for(jtrack2 = 0; jtrack2 < nP2[jlayer2]; jtrack2++)
        {
            yz_c1[0] = (P1[0][jtrack1].cz + 0.5) * cw + pz_edge_low;
            yz_c1[1] = (P1[0][jtrack1].cy + 0.5) * cw + py_edge_low;
            yz_c2[0] = (P2[0][jtrack1].cz + 0.5) * cw + pz_edge_low;
            yz_c2[0] = (P2[0][jtrack1].cz + 0.5) * cw + py_edge_low;
            
            if(pCOIN[(P2[0][jtrack1].cz * ncy_k + P2[0][jtrack1].cy) * nccol_k + P1[0][jtrack2].cz * ncy_k + P2[0][jtrack2].cy]==0)
            {
                // index = tof_information(x, y, z, yz_p1, yz_p2);
                // tCOIN[(P2[0][jtrack1].cz * ncy_k + P2[0][jtrack1].cy) * nccol_k + P1[0][jtrack2].cz * ncy_k + P1[0][jtrack2].cy]=index;
                pCOIN[(P2[0][jtrack1].cz * ncy_k + P2[0][jtrack1].cy) * nccol_k + P1[0][jtrack2].cz * ncy_k + P1[0][jtrack2].cy] += P1[0][jtrack2].resp * P2[0][jtrack1].resp;
            }
            else
            {
                pCOIN[(P2[0][jtrack1].cz * ncy_k + P2[0][jtrack1].cy) * nccol_k + P1[0][jtrack2].cz * ncy_k + P1[0][jtrack2].cy] += P1[0][jtrack2].resp * P2[0][jtrack1].resp;
            }
        }
    }
    return;
}


int sparse_matrix_wr(TABLE_ZIP* pTable, INCREMENT_ZIP* pIncre, float* pSparse,/* int* tSparse,*/ float* TOP_cone_resp /*, int* TOP_cone_resp_tof */, FILE *outf_incre, FILE *outf_matrix /* , FILE *outf_TOF */)
{
    int cymin, cymax, czmin, czmax;
    int cy2, cz2, y, z;
    int irow, jcol;
    int total;

    cymin = ncy_k - 1;
    cymax = 0;
    czmin = ncz_k - 1;
    czmax = 0;

    for(jcol = 0; jcol < nccol_k; jcol++)
    {
        if(TOP_cone_resp[jcol] > 0)
        {
            cz2 = jcol / ncy_k;  // ncy_k = ncz_k
            cy2 = jcol % ncz_k;
            czmin = (czmin > cz2) ? cz2 : czmin;
            czmax = (czmax < cz2) ? cz2 : czmax;
            cymin = (cymin > cy2) ? cy2 : cymin;
            cymax = (cymax < cy2) ? cy2 : cymax;
        }
    }

    pTable->cymin = cymin;
    pTable->czmin = czmin;

    for(z = czmin, total = 0; z <= czmax; z++)
    {
        for(y = cymin, jcol = z * ncy_k + cymin; y <= cymax; y++, jcol++)
        {
            if(TOP_cone_resp[jcol] > 0)
            {
                pIncre[total]._y = y - cymin;
                pIncre[total]._z = z - czmin;
                pSparse[total] = TOP_cone_resp[jcol] / (ngrid*ngrid*ngrid*720*2880);
		        // tSparse[total] = TOP_cone_resp_tof[jcol];
                total++; 
            }
        }
    }

    int nw_incre, nw_matrix;  //, nw_tof
    
    if(total)
    {
        nw_incre=fwrite(pIncre, sizeof(INCREMENT_ZIP), total, outf_incre);
        // printf("total = %d, nw = %d, %s\n",total, nw_incre, strerror(ferror(outf_incre)));
        nw_matrix=fwrite(pSparse, sizeof(float), total, outf_matrix);
	    // nw_tof=fwrite(tSparse, sizeof(int), total, outf_TOF);
    }

    return total;
}


void ray_tracing_panel(char* save_dir)
{
    float y_in_p, y_out_p, z_in_p, z_out_p; 
    float yz_p1[2][4], yz_p2[2][4];  // recode the position that trace comes in and leaves out each layer of each panel
    //yz_p*[0][0] = y_in_p ;
    //yz_p*[0][1] = z_in_p ;
    //yz_p*[0][2] = y_out_p;
    //yz_p*[0][3] = z_out_p; * = 1 or 2

    // The above: Start point and end point between ray and panel.
    TRACING_CRY **Y_alpha = (TRACING_CRY**) calloc(2, sizeof(TRACING_CRY*));
    TRACING_CRY **Z_alpha = (TRACING_CRY**) calloc(2, sizeof(TRACING_CRY*));
    Y_alpha[0] = (TRACING_CRY*) calloc(2 * ncy_k, sizeof(TRACING_CRY));
    Y_alpha[1] = (TRACING_CRY*) calloc(2 * ncy_k, sizeof(TRACING_CRY));
    Z_alpha[0] = (TRACING_CRY*) calloc(2 * ncz_k, sizeof(TRACING_CRY));
    Z_alpha[1] = (TRACING_CRY*) calloc(2 * ncz_k, sizeof(TRACING_CRY));

    TRACING_CRY **YZ_alpha_1 = (TRACING_CRY**) calloc(2, sizeof(TRACING_CRY*));
    TRACING_CRY **YZ_alpha_2 = (TRACING_CRY**) calloc(2, sizeof(TRACING_CRY*));
    YZ_alpha_1[0] = (TRACING_CRY*) calloc(2 * ncy_k + 2 * ncz_k, sizeof(TRACING_CRY));
    YZ_alpha_1[1] = (TRACING_CRY*) calloc(2 * ncy_k + 2 * ncz_k, sizeof(TRACING_CRY));
    YZ_alpha_2[0] = (TRACING_CRY*) calloc(2 * ncy_k + 2 * ncz_k, sizeof(TRACING_CRY));
    YZ_alpha_2[1] = (TRACING_CRY*) calloc(2 * ncy_k + 2 * ncz_k, sizeof(TRACING_CRY));

    RESPONSE_CRY **P1 = (RESPONSE_CRY**) calloc(2, sizeof(RESPONSE_CRY*));
    RESPONSE_CRY **P2 = (RESPONSE_CRY**) calloc(2, sizeof(RESPONSE_CRY*));
    P1[0] = (RESPONSE_CRY*) calloc(ncy_k + ncz_k, sizeof(RESPONSE_CRY));
    P1[1] = (RESPONSE_CRY*) calloc(ncy_k + ncz_k, sizeof(RESPONSE_CRY));
    P2[0] = (RESPONSE_CRY*) calloc(ncy_k + ncz_k, sizeof(RESPONSE_CRY));
    P2[1] = (RESPONSE_CRY*) calloc(ncy_k + ncz_k, sizeof(RESPONSE_CRY));

    float PHI, THETA;
    float cPHI, sPHI, cTHETA, sTHETA;
    int iPHI, iTHETA, nTHETA, nPHI;
    float fly1_L[2], fly2_L[2];
    nPHI = 720;

    TRACING_CRY trace_P[7];
    int start_end[2];
    int nYZ_1[2], nYZ_2[2];  // recode how many intersections at which trace is cut by crystal boundary.
                             // they used with YZ_alpha_*[][].
    int nP1[2], nP2[2];      // recode how many crystal in each layer of each panel in the ray-tracing.
    float trace_p1_L[2], trace_p2_L[2];  // recode how long the length that trace travels in the each layer of each panel
    float L_bw_x;
    
    unsigned char tag_layers = 0;  // layer index, front layer is 0, back layer is 1
                        	       // panel 1, layer index is (0xf0 & tag_layers)>>4
                        	       // panel 2, layer index is (0x0f & tag_layers)
    ////////////////////////////////////////////
    int jlayer1, jlayer2;
    jlayer1 = 1;  //(tag_layers & 0xf0) >> 4;
    jlayer2 = 1;  //tag_layers & 0x0f;
    ////////////////////////////////////////////
    FILE *outf, *outf_incre, *outf_matrix, *outf_table, *outf_DOI, *outf_TOF;
    int irow_, irow, bias;
    char name[500];

    float* TOP_cone_resp     = (float*) calloc(nccol_k, sizeof(float));
    int*   TOP_cone_resp_tof = (int*)   calloc(nccol_k, sizeof(int));
    TABLE_ZIP* pTable      = (TABLE_ZIP*) calloc(ncrow_k + 1, sizeof(TABLE_ZIP));
    INCREMENT_ZIP* pIncre  = (INCREMENT_ZIP*) calloc(nccol_k, sizeof(INCREMENT_ZIP));
    float *pCOIN = (float*) calloc(ncrow_k * nccol_k, sizeof(float));
    int   *tCOIN = (int*)   calloc(ncrow_k * nccol_k, sizeof(int));
    float* pSparse = (float*) calloc(nccol_k, sizeof(float));
    int*   tSparse = (int*)   calloc(nccol_k, sizeof(int));
    
    float y0, z0, x0;
    float y, z, x;
    float lgridy = vy / ngrid, lgridz = vz / ngrid, lgridx = vx / ngrid;
    int igx, igy, igz, k;
    int ikernel;
    int count;
    
    // x0 = (x_plane - 0.5) * vx + lgridx / 2;
    x0 = x_plane * vx + lgridx / 2;
    float testing = 0;
    int jtrack1, jtrack2;
    float resp1, resp2;
    int cy1, cz1, cy2, cz2;
    // int yy, zz;

    // int tmp_kernel[2]={2,7};
    // int itmp;
    float T[2];
    T[0]=s_time();
    
    // char save_dir[100];
    // sprintf(save_dir, "SRM/_data");
    
    for(ikernel = 0; ikernel < nkernel*nkernel; ikernel++)
    {
        printf("\n- Computing the analytic PSF of kernel %s\n", kernel_name[ikernel]);

        sprintf(name, "%s/sparse_incre_plane_%d_%s",  save_dir, x_plane, kernel_name[ikernel]);  outf_incre = fopen(name, "wb");
        sprintf(name, "%s/sparse_matrix_plane_%d_%s", save_dir, x_plane, kernel_name[ikernel]);  outf_matrix = fopen(name, "wb");
        sprintf(name, "%s/sparse_table_plane_%d_%s",  save_dir, x_plane, kernel_name[ikernel]);  outf_table = fopen(name, "wb");

        y0 = kernel_corner[ikernel][0] + lgridy / 2;
        z0 = kernel_corner[ikernel][1] + lgridz / 2;
        
        bias = 0;
        memset(pCOIN, 0, sizeof(float) * ncrow_k * nccol_k);
        // memset(tCOIN, 0, sizeof(int) * ncrow_k * nccol_k);

        ///////////////////
        float T[2];
        T[0] = s_time();
        ///////////////////

        for(igx=0, x=x0; igx < ngrid; igx++, x+=lgridx)
        { 
            for(igz=0, z=z0; igz < ngrid; igz++, z+=lgridz)
            {
                for(igy=0, y=y0; igy < ngrid; igy++, y+=lgridy)
                {
                    for(iPHI=0, count=0; iPHI < nPHI; iPHI++)
                    { 
                        nTHETA = (iPHI>0) ? 2880 : 0;
                        PHI = iPHI * PI / (2 * nPHI);  // PHI is sampled at 0.125 degree.
                        cPHI = cos(PHI);
                        sPHI = sin(PHI);
                        fly1_L[0] = fabs((spacing / 2 - x) / cPHI);            // distance bwn voxel and upper in
                        fly1_L[1] = fabs((spacing / 2 + 2 * cl - x) / cPHI);   // distance bwn voxel and upper out
                        fly2_L[0] = fabs((-spacing / 2 - x) / cPHI);           // distance bwn voxel and lower in
                        fly2_L[1] = fabs((-spacing / 2 - 2 * cl - x) / cPHI);  // distance bwn voxel and lower out
                        
                        L_bw_x = 2 * cl / cPHI;
                        
                        for(iTHETA = 0; iTHETA < nTHETA; iTHETA++)
                        {
                            THETA  = iTHETA * PI * 2 / nTHETA;  // THETA is sampled at 0.125 degree.
                            cTHETA = cos(THETA);
                            sTHETA = sin(THETA);
                            
                            y_in_p   = y + sPHI * cTHETA * fly1_L[0];
                            z_in_p   = z + sPHI * sTHETA * fly1_L[0];
                            y_out_p  = y + sPHI * cTHETA * fly1_L[1];
                            z_out_p  = z + sPHI * sTHETA * fly1_L[1];
                            yz_p1[0][0] = y_in_p ;
                            yz_p1[0][1] = z_in_p ;
                            yz_p1[0][2] = y_out_p ;
                            yz_p1[0][3] = z_out_p ;
                            
                            y_in_p   = y - sPHI * cTHETA * fly2_L[0];
                            z_in_p   = z - sPHI * sTHETA * fly2_L[0];
                            y_out_p  = y - sPHI * cTHETA * fly2_L[1];
                            z_out_p  = z - sPHI * sTHETA * fly2_L[1];
                            yz_p2[0][0] = y_in_p ;
                            yz_p2[0][1] = z_in_p ;
                            yz_p2[0][2] = y_out_p ;
                            yz_p2[0][3] = z_out_p ;

                            if(is_hit_panel(yz_p1[0][0], yz_p1[0][1]) && is_hit_panel(yz_p2[0][0], yz_p2[0][1]))
                            {
                                nYZ_1[0] = 0; nYZ_1[1] = 0;
                                nYZ_2[0] = 0; nYZ_2[1] = 0;
                 
                                trace_p1_L[0] = 0; trace_p1_L[1] = 0;
                                trace_p2_L[0] = 0; trace_p2_L[1] = 0;
                 
                                single_ray_tracing_panel(yz_p1, trace_P, start_end, Y_alpha, Z_alpha, YZ_alpha_1, nYZ_1, L_bw_x, trace_p1_L);
                                single_ray_tracing_panel(yz_p2, trace_P, start_end, Y_alpha, Z_alpha, YZ_alpha_2, nYZ_2, L_bw_x, trace_p2_L);
                                
                                single_response(yz_p1,trace_p1_L,nYZ_1,YZ_alpha_1,P1, nP1,fly1_L,PHI);
                                single_response(yz_p2,trace_p2_L,nYZ_2,YZ_alpha_2,P2, nP2,fly2_L,PHI);
                        	    coincidence_response(pCOIN/*, tCOIN*/, P1, P2, nP1, nP2, tag_layers, PHI, THETA, yz_p1, yz_p2, nYZ_1,YZ_alpha_1, x, y , z);
                            }
                        }
                    }
                    // printf("resp=%.10f\n", testing);
                }
            }
        }

        for(irow = 0; irow < ncrow_k; irow++)
        {
            pTable[irow].bias = bias;
        
            memcpy(TOP_cone_resp, pCOIN + irow * nccol_k, nccol_k * sizeof(float));
	        memcpy(TOP_cone_resp_tof, tCOIN + irow * nccol_k, nccol_k * sizeof(int));
	    
	        bias += sparse_matrix_wr(pTable + irow, pIncre, pSparse/*, tSparse*/, TOP_cone_resp/*, TOP_cone_resp_tof*/, outf_incre, outf_matrix/*, outf_TOF*/);
        }
        
        pTable[irow].bias = bias;
        fwrite(pTable, sizeof(TABLE_ZIP), ncrow_k + 1, outf_table);

        ///////////////////////////////
        T[1]=s_time()-T[0];
        printf("time: %g sec \n",T[1]);
        ///////////////////////////////

        fclose(outf_incre);
        fclose(outf_matrix);
        fclose(outf_table);
        // fclose(outf_TOF);
    }
    free(Y_alpha[0]); free(Y_alpha[1]); free(Y_alpha);
    free(Z_alpha[0]); free(Z_alpha[1]); free(Z_alpha);
    free(YZ_alpha_1[0]); free(YZ_alpha_1[1]); free(YZ_alpha_1);
    free(YZ_alpha_2[0]); free(YZ_alpha_2[1]); free(YZ_alpha_2);
    free(P1[0]);free(P1[1]); free(P1);
    free(P2[0]);free(P2[1]); free(P2);
    free(pCOIN);
    // free(tCOIN);
    free(TOP_cone_resp);
    // free(TOP_cone_resp_tof);
    free(pTable);
    free(pIncre);
    free(pSparse);
    // free(tSparse);
}