#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <typeinfo>
#include <sys/types.h>
#include <sys/stat.h>

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


void setup_file (char dirname[])
{
    char cmd[1000];
    struct stat s = { 0 };
    if (stat(dirname, &s))
    {
        sprintf(cmd, "mkdir -p %s", dirname);
        system(cmd);
    }	
}


extern "C" void extract_energy_info (char* input_file, char* data_info)
{
    char filename[1000];
    // char dirname[100];
    // char gmslname[100];
    // char sticname[100];
    char channelname[100];
    char energy[20];

    FILE *bin_file;
    sprintf(filename, "%s", input_file);
    sprintf(energy, "data/src/tmp/%s", data_info);
    // sprintf(energy, "%s", "data/energy");
    setup_file(energy);
    
    bin_file = fopen(filename,"rb");
    if (!bin_file) { cout << "Wrong path or file not exist!" << endl; }
    
    /// ====== ====== ====== LFSR DECODING ====== ====== ====== ///
    
    uint16_t m_lut[1 << 15], encodedLfsr[1 << 15];
    m_lut[ 0x7FFF ] = -1;  // invalid state
    uint16_t lfsr = 0x0000;

    for (uint16_t n = 0; n < (1 << 15)-1; ++n)
    {
        m_lut[ lfsr ] = n;
        encodedLfsr[n] = lfsr;
        const uint8_t bits13_14 = lfsr >> 13;
        uint8_t new_bit;
        
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

        lfsr = (lfsr << 1) | new_bit;  // - add the new bit to the right -
        lfsr &= 0x7FFF;  // - throw away the 16th bit from the shift -
    }
    
    // unsigned char buffer[sizeof(struct event)];
    unsigned char buffer[8];
    
    FILE* f;
    struct stat s = {0};
    uint32_t board_count[32] = {0};
    uint32_t gmsl_count[32*4] = {0};
    uint32_t stic_count[32*4*4] = {0};
    uint32_t channel_count[32*4*4*64] = {0};
    
    uint16_t board_energy[32][100] = {0};
    uint16_t gmsl_energy[32*4][100] = {0};
    uint16_t stic_energy[32*4*4][100] = {0};
    uint16_t channel_energy[32*4*4*64][100] = {0};
    int total = 0;
    // int zero = 0;
    uint32_t num = 0;

    while (fread(buffer, sizeof(buffer), 1, bin_file) != '\0')
    {
        num++;
        // data.ecoarse = m_lut[(buffer[1] % 16)*2048 + buffer[2] * 8 + (buffer[3] / 32)];
        // data.tcoarse = m_lut[(buffer[0] / 4) + buffer[7] * 64 + buffer[6] % 2 * 16384];
        // data.efine = buffer[3] % 32;
        // data.tfine = buffer[1] / 32 + (buffer[0] % 4) * 8;
        // data.ebad = (buffer[1] >> 4) % 2;
        // data.tbad = (buffer[6] >> 1) % 2;
        // data.channel = buffer[6] / 4;
        // data.frame = buffer[5] % 64;
        // data.gmslID = buffer[4] % 4;
        // data.sticID = buffer[5] / 64;
        // data.boardID = (buffer[4] >> 2) % 32;

        if ((buffer[5] == 255) && (buffer[6] == 255) && (buffer[7] == 255) && ((buffer[0] / 4) == 63))
        { /* - reset event - */ }
        else
        {
            uint16_t data;
            if (strcmp(data_info, "ecc") == 0) data = m_lut[(buffer[1] % 16) * 2048 + buffer[2] * 8 + (buffer[3] / 32)];
            else if (strcmp(data_info, "tcc") == 0) data = m_lut[(buffer[0] / 4) + buffer[7] * 64 + buffer[6] % 2 * 16384];
            else if (strcmp(data_info, "energy") == 0)
            {
                uint16_t ecc = m_lut[(buffer[1] % 16) * 2048 + buffer[2] * 8 + (buffer[3] / 32)];
                uint16_t tcc = m_lut[(buffer[0] / 4) + buffer[7] * 64 + buffer[6] % 2 * 16384];
                if (ecc > tcc) data = ecc - tcc;
                else data = 32767 + ecc - tcc;
            }
            
            if ((data < 1000) && (num > 2000))
            {
                uint8_t board   = (buffer[4] >> 2) % 32;
                uint8_t gmsl    = buffer[4] % 4;
                uint8_t stic    = buffer[5] / 64;
                uint8_t channel = buffer[6] / 4;
                
                // - Assign energy information to corresponding array -
                board_energy[board][(board_count[board]) % 100] = data;
                gmsl_energy[board*4 + gmsl][(gmsl_count[board*4 + gmsl]) % 100] = data;
                stic_energy[board*16 + gmsl*4 + stic][(stic_count[board*16 + gmsl*4 + stic]) % 100] = data;
                channel_energy[board*1024 + gmsl*256 + stic*64 + channel][(channel_count[board*1024 + gmsl*256 + stic*64 + channel] % 100)] = data;
                
                // - Calculate total amount of energy info in each level -
                board_count[board]++;
                gmsl_count[board*4 + gmsl]++;
                stic_count[board*16 + gmsl*4 + stic]++;
                channel_count[board*1024 + gmsl*256 + stic*64 + channel]++;

                // - Write channel energy information -
                if (channel_count[board*1024 + gmsl*256 + stic*64 + channel] % 100 == 0)
                {
                    sprintf(channelname, "%s/board%u/gmsl%u/stic%u", energy, board, gmsl, stic);
                    setup_file(channelname);
                    sprintf(filename, "%s/%u.txt", channelname, channel);
                    
                    f = new FILE;
                    if (stat(filename, &s)) f = fopen(filename,"wb");
                    else f = fopen(filename,"ab");
                    
                    for (int i = 0; i < 100; i++)
                    {
                        fprintf(f, "%u\n", channel_energy[board*1024 + gmsl*256 + stic*64 + channel][i]);
                    }
                    fclose(f);
                    total += 100;
                }
            }
        }
    }

    // - Channel -
    for (int channel = 0; channel < 32 * 4 * 4 * 64; channel++)
    {
        sprintf(channelname, "%s/board%u/gmsl%u/stic%u", energy, channel / 1024, (channel / 256) % 4, (channel / 64) % 4);
        sprintf(filename, "%s/%u.txt", channelname, channel % 64);
        f = new FILE;
        
        if ((channel_count[channel] % 100 != 0) && (channel_count[channel] / 100 == 0))
        {
            setup_file(channelname);
            f = fopen(filename,"wb");
            for (unsigned int i = 0; i < channel_count[channel] % 100; i++)
            {
                // fwrite(&channel_energy[channel][i], sizeof(uint16_t), 1, f);
                fprintf(f, "%u\n", channel_energy[channel][i]);
	        }
            fclose(f);
	        total += channel_count[channel] % 100;
        }
        else if ((channel_count[channel] / 100 > 0) && (channel_count[channel] % 100 != 0))
        {
            f = fopen(filename,"ab");
            for (unsigned int i = 0; i < channel_count[channel] % 100; i++)
            {
                // fwrite(&channel_energy[channel][i], sizeof(uint16_t), 1, f);
                fprintf(f, "%u\n", channel_energy[channel][i]);
	        }
            fclose(f);
	        total += channel_count[channel] % 100;
        }
    }
    cout << "[Info] total: " << total << endl;
}