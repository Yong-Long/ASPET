#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <typeinfo>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>

using namespace std;

int main (int argc, char** argv) { }


struct event
{
    uint16_t  ecoarse;
    uint16_t  tcoarse;
    uint8_t   efine;
    uint8_t   tfine;
    uint8_t   ebad;
    uint8_t   tbad;

    uint8_t   channel;
    uint8_t   frame;
    uint8_t   gmslID;
    uint8_t   sticID;
    uint8_t   boardID;
};

struct stic
{
    uint32_t total = 0;
    uint32_t channels[64] = { 0 };
};

struct gmsl
{
    uint32_t total = 0;
    stic     stics[4];
};

struct board
{
    uint32_t total = 0;
    gmsl     gmsls[4];
};


extern "C" void extract_data (char* input_file)
{
    char filename[500];
    sprintf(filename, "%s", input_file);
    cout << "\n[File] input binary file: " << filename << endl;
    
    // - open bin file -
    FILE *bin_file;
    bin_file = fopen(filename,"rb");
    if (!bin_file) { cout << "[Err] file " << filename << " not exist!" << endl; }
    
    /// ====== ====== LFSR DECODING ====== ====== ///
    
    uint16_t m_lut[1 << 15], encodedLfsr[1 << 15];
    m_lut[ 0x7FFF ] = -1;  // - invalid state -
    uint16_t lfsr = 0x0000;
    
    for (uint16_t n = 0; n < (1 << 15)-1; ++n)
    {
        uint8_t new_bit;
        m_lut[ lfsr ] = n;
        encodedLfsr[n] = lfsr;
        const uint8_t bits13_14 = lfsr >> 13;
        
        switch (bits13_14)
        {   // new_bit = !(bit13 ^ bit14)
            case 0x00:
            case 0x03:
                new_bit = 0x01;
                break;
            case 0x01:
            case 0x02:
                new_bit = 0x00;
                break;
        }

        lfsr = (lfsr << 1) | new_bit;  // Add new bit to the right
        lfsr &= 0x7FFF;  // throw away 16th bit from the shift
    }
    
    /// ====== ====== READING FILE THROUGH THE BUFFER ====== ====== ///
    
    uint16_t ecc;
    uint16_t tcc;
    int count = 0;
    uint16_t energy;
    board boards[32];
    uint32_t num = 0;
    char frame_id[100];
    char cmd_mkdir[100];
    struct stat s = {0};
    int frame[64] = {0};
    // int reset_count = 0;
    unsigned char buffer[8];
    // FILE* events = fopen("data/src/tmp/events.txt", "w");
    
    // - make dir of tmp/frame -
    if (stat("data/src/tmp/frame/", &s))
    {
        sprintf(cmd_mkdir, "mkdir -p %s", "data/src/tmp/frame/");
        system(cmd_mkdir);
    }
    
    // - initialize frame data matrix -
    int*** frame_data = new int**[64];
    for (int i = 0; i < 64; i++)
    {
	    frame_data[i] = new int*[100];
        for (int j = 0; j < 100; j++)
        {
	        frame_data[i][j] = new int[12];
        }
    }
    
    while (fread(buffer, sizeof(buffer), 1, bin_file) != '\0')
    { 
        num++;
        if ((buffer[5] == 255) && (buffer[6] == 255) && (buffer[7] == 255) && ((buffer[0] / 4) == 63))
        { /* - reset event - */ }
        else
        {
            ecc = m_lut[(buffer[1] % 16) * 2048 + buffer[2] * 8 + (buffer[3] / 32)];
            tcc = m_lut[(buffer[0] / 4) + buffer[7] * 64 + buffer[6] % 2 * 16384];
            
            if (ecc > tcc) { energy = ecc - tcc; }
            else { energy = 32767 + ecc - tcc; }

            if ((energy < 1000) && (num > 2000))
            {
                // - calculate total num of Board/GMSL/STiC/Channel -
                boards[(buffer[4] >> 2) % 32].total++;
                boards[(buffer[4] >> 2) % 32].gmsls[buffer[4] % 4].total++;
                boards[(buffer[4] >> 2) % 32].gmsls[buffer[4] % 4].stics[buffer[5] / 64].total++;
                boards[(buffer[4] >> 2) % 32].gmsls[buffer[4] % 4].stics[buffer[5] / 64].channels[buffer[6] / 4]++;
                frame[buffer[5] % 64]++;
                
                // - main content of each frame -
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][0]  = count;
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][1]  = m_lut[(buffer[1] % 16) * 2048 + buffer[2] * 8 + (buffer[3] / 32)];
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][2]  = m_lut[(buffer[0] / 4) + buffer[7] * 64 + buffer[6] % 2 * 16384];
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][3]  = buffer[3] % 32;
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][4]  = buffer[1] / 32 + (buffer[0] % 4) * 8;
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][5]  = (buffer[1] >> 4) % 2;
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][6]  = (buffer[6] >> 1) % 2;
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][7]  = buffer[6] / 4;
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][8]  = buffer[5] % 64;
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][9]  = buffer[4] % 4;
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][10] = buffer[5] / 64;
                frame_data[buffer[5] % 64][(frame[buffer[5] % 64]) % 100][11] = (buffer[4] >> 2) % 32;
                
                // - write data into frame files every 100 events -
                if ((frame[buffer[5] % 64] % 100) == 0)
                {
                    sprintf(frame_id, "data/src/tmp/frame/%d.txt", buffer[5] % 64);
                    FILE* frame_file;
                    
                    if (stat(frame_id, &s)) frame_file = fopen(frame_id,"w");
                    else frame_file = fopen(frame_id,"a");
                    
                    for (int i = 0; i < 100; i++)
                    {
                        for(int j = 0; j < 12; j++)
                        {
                            if (j == 11) fprintf(frame_file, "%d\n", frame_data[buffer[5] % 64][i][j]);
                            else fprintf(frame_file, "%d\t", frame_data[buffer[5] % 64][i][j]);
                        }
                    }
                    fclose(frame_file);
                }
                count++;
            }
            /*
            fprintf(events, "%d\t", count);
            fprintf(events, "%d\t", m_lut[(buffer[1] % 16)*2048 + buffer[2] * 8 + (buffer[3] / 32)]);
            fprintf(events, "%d\t", buffer[1] / 32 + (buffer[0] % 4) * 8);
            fprintf(events, "%d\t", (buffer[1] >> 4) % 2);
            fprintf(events, "%d\t", buffer[5] / 64);
            fprintf(events, "%d\n", (buffer[4] >> 2) % 32);
            
            // - key info of all parameters -
            frameID = buffer[5] % 64;
            eventID = (frame[buffer[5] % 64]) % 100;
            
            event data;
            data.ecoarse = m_lut[(buffer[1] % 16) * 2048 + buffer[2] * 8 + (buffer[3] / 32)];
            data.tcoarse = m_lut[(buffer[0] / 4) + buffer[7] * 64 + buffer[6] % 2 * 16384];
            data.efine   = buffer[3] % 32;
            data.tfine   = buffer[1] / 32 + (buffer[0] % 4) * 8;
            data.ebad    = (buffer[1] >> 4) % 2;
            data.tbad    = (buffer[6] >> 1) % 2;
            data.channel = buffer[6] / 4;
            data.frame   = buffer[5] % 64;
            data.gmslID  = buffer[4] % 4;
            data.sticID  = buffer[5] / 64;
            data.boardID = (buffer[4] >> 2) % 32;
            if (fwrite(&data, sizeof(struct event), 1, f))
            */
        }
    }
    
    // - save frame data -
    for (int i = 0; i < 64; i++)
    {
        if ((frame[i] % 100) > 0)
        {
            sprintf(frame_id, "data/src/tmp/frame/%d.txt", i);
            FILE* frame_file;
            
            if (stat(frame_id, &s)) frame_file = fopen(frame_id,"w");
            else frame_file = fopen(frame_id,"a");
            
            for (int j = 0; j < (frame[i] % 100); j++)
            {
                for (int k = 0; k < 12; k++)
                {
                    if (k == 11) fprintf(frame_file, "%d\n", frame_data[i][(i / 100) + j][k]);
                    else fprintf(frame_file, "%d\t", frame_data[i][(i / 100) + j][k]);
                }
            }
            fclose(frame_file);
        }
    }
    
    // - save active boards info -
    FILE* f = fopen("data/src/tmp/active.txt","w");
    for (int i = 0; i < 32; i++)
    {
        fprintf(f, "%d\n", boards[i].total);

        for (int j = 0; j < 4; j++)
        {
            if (j == 3) fprintf(f, "%d\n", boards[i].gmsls[j].total);
            else fprintf(f, "%d\t", boards[i].gmsls[j].total);
        }

        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                if ((j == 3) && (k == 3)) fprintf(f, "%d\n", boards[i].gmsls[j].stics[k].total);
                else fprintf(f, "%d\t", boards[i].gmsls[j].stics[k].total);
            }
        }

        for (int j = 0; j < 4; j++)
        {
            for (int k = 0; k < 4; k++)
            {
                for (int l = 0; l < 64; l++)
                {
                    if ((j == 3) && (k == 3) && (l == 63)) fprintf(f, "%d\n", boards[i].gmsls[j].stics[k].channels[l]);
                    else fprintf(f, "%d\t", boards[i].gmsls[j].stics[k].channels[l]);
                }
            }
        }
    }
    fclose(f);
    
    // - save frame info -
    FILE* frame_info = fopen("data/src/tmp/frame_info.txt","w");
    for (int i = 0; i < 64; i++)
    {
        fprintf(frame_info, "%d\t", frame[i]);
    }
    fclose(frame_info);
    cout << "[Info] Total counts: " << count << endl;
}
