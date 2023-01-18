//
//  decode.cpp
//  

#include <string>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <vector>
#include <cmath>

using namespace std;


int datasize = 0;
int extrasize = 0;
int check_debug(int);
void got_word(int);
void check_data();
void flush_extra();

int buf_size = 16 * 1024;

int word;                          // holds 24-bit word being built
int EXP = 0;                       // expecting this type next
char buffer[16*1024];
unsigned char bytes[16*1024];

unsigned long data[1024];                     // 11
double extra[16*1024];                    // leftovers
int words = 0;                     // word counter

int time_hi;
int got_hi = 0;                    // hee hee
int word_count[4] = {0,0,0,0};
int word_index = 0;



void got_word(int d)
{
    words++;
    int type = (d >> 22) & 3;
    if(type == 1)                         // command word
    {
        int b1 = (d >> 16) & 63;
        int b2 = (d >> 8) & 255;
        int b3 = d & 255;
        flush_extra();
        cout << "c," << b1 << "," << b2 << "," << b3 << "\n";
    }
    else if(type == 3)
    {
        if(check_debug(d))
            return;
        data[datasize] = d & 65535;
        datasize++;
        check_data();
    }
    else
    {
        printf("?,%06x\n",d);
    }
}

void check_data()
{
    while(1)
    {
        int got_packet = 0;
        int got = datasize;
        if(!got)
            break;
        if(data[0] == 65535)                    // check header
        {
            if(!(got >= 2))
                return;
            int len = data[1] & 255;
            if(len > 1)
            {
                if(!(got >= (len + 1)))
                    return;
                int par = 0;
                for(int i = 1; i <= len; i++)
                {
                    par ^= data[i];
                }
                if(!par)                        // check parity
                {
                    got_packet = 1;
                    flush_extra();
                    cout << "p,";
                    for(int j = 1; j<len; j++)
                    {
                        cout << data[j] << ",";
                    }
                    cout << "\n";
                    for(int t = 0; t <= len + 1; t++)
                    {
                        data[t] = data[t+len+1];
                    }
                    datasize = datasize - len - 1;
                }
            }
        }
        if(!got_packet)
        {
            extra[extrasize] = data[0];
            extrasize++;
            for(int j = 0;j<datasize; j++)
            {
                data[j] = data[j+1];
            }
            datasize--;
        }
    }
}


int check_debug(int d)
{
    int a = (d >> 16) & 255;
    d = d & 65535;
    
    if(a == 200)
    {
        time_hi = d;
        got_hi = 1;
        return 1;
    }
    else if(a == 201)
    {
        if(got_hi)
        {
            int time = (time_hi << 16) | d;
            got_hi = 0;
            flush_extra();
            cout << "t," << time << "\n";
        }
        return 1;
    }
    else if(a == 197)
    {
        word_index = 0;
        return 1;
    }
    else if(a == 198)
    {
        word_count[word_index++] = d;
        if(word_index == 4)
        {
            int t = (word_count[0] << 16) + word_count[1];
            int v = (word_count[2] << 16) + word_count[3];
            unsigned int diff = t - v -words;
            if(diff < 0)
            {
                diff += (1UL << 32);
            }
            flush_extra();
            cout << "n," << t << "," << v << "," << words << "," << diff << "\n";
        }
        return 1;
    }
    else if(a == 193)
    {
        flush_extra();
        cout << "dac," << d << "\n";
        return 1;
    }
    else if(a == 194)
    {
        flush_extra();
        cout << "delay," << d << "\n";
        return 1;
    }
    else
    {
        return 0;
    }
}



void flush_extra()
{
    if(extrasize)
    {
        cout << "x,";
        for(int i = 0;i<extrasize;i++)
        {
            cout << extra[i] << ",";
            extra[i] = 0;
        }
        extrasize= 0;
        cout << "\n";
    }
}









void decode(string file)
{
    
    ifstream IN(file.c_str());
    if(!IN.good())
    {
        cout << "Can not open file\n";
        IN.close();
    }
    else
    {
        cout << "Open file sucessfully!\n";
        while(1)
        {
            IN.get(buffer,buf_size);
            for(int i = 0;i<(16*1024);i++)
            {
                bytes[i] = buffer[i];
            }
            for(int i=0;bytes[i];i++)
            {
                int payload = int(bytes[i]) & 63;
                int type = int(bytes[i]) >> 6;
                if(type == 0)
                {
                    EXP = 1;
                    word = payload;
                }
                else
                {
                    if(type == EXP)
                    {
                        word = (word << 6) | payload;
                        if(++EXP == 4)
                        {
                            EXP = 0;
                            got_word(word);
                        }
                    }
                    else
                    {
                        EXP = 0;
                    }
                }
            } 
            if(IN.eof())
                break;
        }
        for(int i = 0; i<datasize; i++)
        {
            extra[i+extrasize] = data[i];
        }
        extrasize = extrasize + datasize;
        flush_extra();
        IN.close();
    }
}

int main(int argc, char *argv[])
{
    char file[128];
    sscanf(argv[1],"%s",file);
    
    decode(file);
    return 0;
}
