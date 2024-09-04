#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <typeinfo>
#include <iostream>
#include <fstream>

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


extern "C" void extract_entries_distribution (char* input_file)
{
    char filename[1000];
    FILE *bin_file;
    sprintf(filename, "%s", input_file);

    bin_file = fopen(filename,"rb");
    if (!bin_file) { cout << "[Err] Wrong path or file not exist!" << endl; }

    unsigned char buffer[sizeof(struct event)];
    board boards[32];
    int count = 0;
    
    while (fread(buffer, sizeof(buffer), 1, bin_file) != '\0')
    {
	    count++;
        boards[buffer[12]].total++;
        boards[buffer[12]].gmsls[buffer[10]].total++;
        boards[buffer[12]].gmsls[buffer[10]].stics[buffer[11]].total++;
        boards[buffer[12]].gmsls[buffer[10]].stics[buffer[11]].channels[buffer[8]]++;
    }
    cout << "[Info] count: " << count << endl;
    // cout << boards[31].total << ' ' << boards[31].gmsls[1].total << ' ' << boards[31].gmsls[1].stics[1].total << ' ' << boards[31].gmsls[1].stics[1].channels[63] << endl;

    FILE* f = fopen("data/src/tmp/entries_distribution.bin","wb");
    for (size_t i = 0; i < sizeof(boards) / sizeof(struct board); i++)
    {
	    fwrite(&boards[i], sizeof(struct board), 1, f);
    }
}