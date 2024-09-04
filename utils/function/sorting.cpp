#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <thread>
#include <fstream>
#include <iostream>
#include <typeinfo>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

int main (int argc, char** argv) { }


void coincidence_pairs (unsigned int** data, int index, int end, int total, vector<unsigned int>* coin_pairs)
{
    if (end > total) { end = total; }
    long int exist[total] = {0};
    long int highest = -1;
    
    for (int i = index; i < end; i++)
    {
        if (i < total - 1)
        {
            int j = i + 1;
            while ((data[j][0] - data[i][0]) < 1024)
            {
                if ((data[i][9] / 2) != (data[j][9] / 2))
                {
                    if (abs(int(data[i][2] - data[j][2])) < 10)
                    {
                        if (exist[i] == 0)
                        {
                            if (exist[j] == 0)
                            {
                                coin_pairs[i].push_back(j);
                                exist[i] = j; 
                                exist[j] = i;
                                break;
                            }
                        }
                        else if (exist[i] < i)
                        {
                            if (abs(int(data[i][0] - data[exist[i]][0])) <= abs(int(data[j][0] - data[i][0]))) break;
                            else
                            {
                                long int ind = exist[i];
                                exist[ind] = 0;
                                coin_pairs[ind].erase(coin_pairs[ind].begin());
                                coin_pairs[i].push_back(j);
                                exist[i] = j;
                                exist[j] = i;
                                i = ind - 1;
                                break;
                            }
                        }
                    }
                }
                ++j;
                if (j >= total) break;
            }
            highest++;
        }
        else break;
    }
}


void thread_func (char* frame, char* bin, int frame_id, int total)
{
    string myText;
    struct stat s = {0};

    char frame_fpath[100];
    sprintf(frame_fpath, "%s/%d.txt", frame, frame_id);
    
    if (stat(frame_fpath, &s)) return;
    cout << "Frame: " << frame_fpath << endl;
    
    // - initial array of frame data -
    unsigned int** frame_data = new unsigned int*[total];
    for (int i = 0; i < total; i++)
    {
        frame_data[i] = new unsigned int[12];
    }
    
    // ====== Read frame data into array ======
    ifstream txtfile(frame_fpath);
    int count = 0;
    while (count < total)
    {
        getline(txtfile, myText);
        for (int i = 0; i < 12; i++)
        {
            if (i == 11)
            {
                frame_data[count][i] = stoi(myText.substr(0, myText.find('\n')));
            }
            else
            {
                frame_data[count][i] = stoi(myText.substr(0, myText.find('\t')));
                myText.erase(0, myText.find('\t') + 1);
            }
        }
        count++;
    }
    txtfile.close();
    
    // ====== Create threads for sorting coincidence pairs ======
    vector<unsigned int>* coin_pairs = new vector<unsigned int>[total];
    if (total > 50000000)
    {
        vector<thread> threads;
        for (int i = 0; i < 5; i++)
        {
            threads.push_back(thread(coincidence_pairs, frame_data, (total/3) * i, (total/3) * (i+1), total, coin_pairs));
            if((i % 5) == 4) threads[i].join();
	        else threads[i].detach();
        }
    }
    else coincidence_pairs(frame_data, 0, total, total, coin_pairs);
    
    // ====== [Save] LOR B0B1 pair ======
    char LOR_B0B1_dir[120], cmd_mkdir[150];
    sprintf(LOR_B0B1_dir, "data/recon/LOR/%s", bin);
    
    // - create saving dir for LOR -
    if (stat(LOR_B0B1_dir, &s))
    {
        sprintf(cmd_mkdir, "mkdir -p %s", LOR_B0B1_dir);
        system(cmd_mkdir);
    }

    char LOR_B0B1_path[300];
    sprintf(LOR_B0B1_path, "%s/LOR_B0B1_%s.txt", LOR_B0B1_dir, bin);
    
    FILE* fLOR_B0B1;
    if (stat(LOR_B0B1_path, &s)) fLOR_B0B1 = fopen(LOR_B0B1_path, "w");
    else fLOR_B0B1 = fopen(LOR_B0B1_path, "a");

    for (int i = 0; i < total; i++)
    {
        if (coin_pairs[i].size() > 0)
        {
            unsigned int id_0 = frame_data[i][11] * 1024 + frame_data[i][9] * 256 + frame_data[i][10] * 64 + frame_data[i][7];
            unsigned int id_1 = frame_data[coin_pairs[i][0]][11] * 1024 + frame_data[coin_pairs[i][0]][9] * 256 + frame_data[coin_pairs[i][0]][10] * 64 + frame_data[coin_pairs[i][0]][7];
            
            if (id_0 < id_1) fprintf(fLOR_B0B1, "%d\t%d\n", id_0, id_1);
            else fprintf(fLOR_B0B1, "%d\t%d\n", id_1, id_0);
        }
    }
    fclose(fLOR_B0B1);
}


extern "C" void sorting (char* frame, char* bin, int* total)
{
    vector<thread> threads;
    
    for (int i = 0; i < 63; i++)
    {
        if ((i % 8) == 7)
        {
            threads.push_back(thread(thread_func, frame, bin, i, total[i]));
            threads[i].join();
        }
        else
        {
            threads.push_back(thread(thread_func, frame, bin, i, total[i]));
            threads[i].join();
        }
    }
}
