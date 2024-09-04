#include <vector>
#include <string>
#include <ctype.h>
#include <stdio.h>
#include <sstream>
#include <fstream>
#include <string.h>
#include <iostream>
#include <typeinfo>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

int main(int argc, char** argv) { }

void mapping_id (uint8_t gmsl, uint8_t stic, uint8_t channel) { }


extern "C" void read_data (uint8_t *channel_sets, uint32_t *resol, uint32_t *recon_img)
{
    vector<string> rows;
    ifstream ifs("./data/src/tmp/coin_data.txt", ios::in);
    
    if (!ifs.is_open()) { cout << "[Err] failed to open coin_data.txt. \n"; }
    
    else
    {
        string s;
        int begin = 0, end;
        uint16_t first, second; 
        uint8_t board_0 = 0, gmsl_0, stic_0, channel_0, board_1 = 0, gmsl_1, stic_1, channel_1, loc_x, loc_y;
        uint8_t mapping_x_0, mapping_y_0, mapping_x_1, mapping_y_1;
        uint8_t sum_x, sum_y;
        uint16_t start;
        
        while (getline(ifs, s))
        {
	        end = s.find('\t');  // - split row -
            
            // - first id mapping -
            first  = stoi(s.substr(begin, end - begin));
            gmsl_0 = (first / 64) / 4;
            stic_0 = (first / 64) % 4;
            channel_0 = first % 64;
            start = board_0 * 1024 + gmsl_0 * 256 + stic_0 * 64;
            
            for (int i = 0; i < 64; i++)
            {
                if (channel_sets[start + i] == channel_0)
                {
                    loc_x = i % 8;
                    loc_y = i / 8;
                    break;
                }
            }
            mapping_x_0 = (gmsl_0 % 2) * 16 + (stic_0 % 2) * 8 + loc_x;
            mapping_y_0 = (1 - (stic_0 / 2)) * 8 + loc_y;
    
            // - second id mapping -
            second = stoi(s.substr(end + 1, s.length()));
	        gmsl_1 = (second / 64) / 4;
            stic_1 = (second / 64) % 4;
            channel_1 = second % 64;
            start = board_1 * 1024 + gmsl_1 * 256 + stic_1 * 64;
            
            for (int i = 0; i < 64; i++)
            {
                if (channel_sets[start + i] == channel_1)
                {
                    loc_x = i % 8;
                    loc_y = i / 8;
                    break;
                }
            }
            mapping_x_1 = (gmsl_1 % 2) * 16 + (stic_1 % 2) * 8 + loc_x;
            mapping_y_1 = (stic_1 / 2) * 8 + loc_y;
            
            // - get sum x & y -
            sum_x = mapping_x_0 + mapping_x_1;
            sum_y = mapping_y_0 + mapping_y_1;   
            
            // recon_img[(sum_y / 2) * 32 + (sum_x / 2)] += 1;
            
            if (((sum_x % 2) != 0) &&  ((sum_y % 2) != 0))
            {
                for(int i = 0; i < 16; i++) recon_img[((sum_y / 2) * 4 + 2 + (i / 4)) * 32 * 4 + ((sum_x / 2) * 4 + 2 + (i % 4))] += resol[i];
            }
            else if ((sum_x % 2) != 0)
            {
                for(int i = 0; i < 16; i++) recon_img[((sum_y / 2) * 4 + (i / 4)) * 32 * 4 + (sum_x / 2 * 4 + 2 + (i % 4))] += resol[i];
            }
            else if ((sum_y % 2) != 0)
            {
                for(int i = 0; i < 16; i++) recon_img[((sum_y / 2) * 4 + 2 + (i / 4)) * 32 * 4 + (sum_x / 2 * 4 + (i % 4))] += resol[i];
            }
            else
            {
                for(int i = 0; i < 16; i++) recon_img[((sum_y / 2) * 4 + (i / 4)) * 32 * 4 + (sum_x / 2 * 4 + (i % 4))] += resol[i];
            }
        }
    }
    ifs.close();
    
    // return recon_img;
}
