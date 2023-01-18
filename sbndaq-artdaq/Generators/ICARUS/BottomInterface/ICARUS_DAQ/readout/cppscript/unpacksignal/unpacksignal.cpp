#include <string>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <vector>
#include <cmath>

using namespace std;

void SplitString(string s, vector<string> &v, string c)
{
    size_t pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1,pos2-pos1));
        
        pos1 = pos2 + c.size();
        pos2 = s.find(c,pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

void unpacksignal(string file,int pmt_board)
{
    int otime = 0;
    unsigned long delta = 0;
    ifstream IN(file.c_str());
    string s;
    if(!IN.good())
    {
        cout << "Can not open file\n";
        IN.close();
    }
    else
    {
        getline(IN,s);
        while(getline(IN,s))
        {
          //  if(isspace(s))
           //     continue;
            vector<string> line;
            SplitString(s,line,",");
            if(line[0] == "p")
            {
                int tmp = atoi(line[1].c_str());
                int mod = (tmp >> 8) & 127;
                if(pmt_board && (mod != pmt_board))
                    continue;
                int len = (tmp & 255) - 4;
                int time = (atoi(line[2].c_str()) << 16) + atoi(line[3].c_str());
                if(time<otime)
                    delta++;
                otime = time;
                time = (delta << 32) + time;
                int type = tmp >> 15;
                if(type)                //adc packet
                {
                    len >>= 1;
                    for(int i = 0; i<=(len-1); i++)
                    {
                        double data = atof(line[3 + 2 * i + 1].c_str());
                        int channel = atoi(line[4 + 2 * i + 1].c_str()) + 1;
                        cout << channel << "," << data << "," << time << "\n";
                    }
                    cout << "0," << len << "," << time << "\n";
                }
            }
        }
        IN.close();
    }
}

int main(int argc, char *argv[])
{
    char file[128];
    int pmt_board;
    sscanf(argv[1],"%s",file);
    sscanf(argv[2],"%d",&pmt_board);
    
    unpacksignal(file,pmt_board);
    return 0;
}
