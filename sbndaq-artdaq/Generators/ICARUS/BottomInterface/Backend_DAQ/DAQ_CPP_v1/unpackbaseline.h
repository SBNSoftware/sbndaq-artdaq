#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <vector>
#include <cmath>

#define buf_size 1024

//using namespace std;
using std::cout;
using std::endl;
using std::string;

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

void unpackbaseline(string file,int pmt_board, string online_path)
{
    if(file.find(".csv")!=string::npos){
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
                vector<string> line;
                SplitString(s,line,",");
                if(line[1].c_str() != NULL)           //adc packet
                {
                    int len = 0;
                    for(int i = 1; i<=64; i++)
                    {
                        int d = atoi(line[3+i].c_str());
                        if(d)
                        {
                            cout << i << "," << d << "\n";
                            len++;
                        }
                    }
                    cout << "0," << len << "\n";
                }
            }
            IN.close();
        }
    }
    else{
        unsigned int otime = 0;
        unsigned long delta = 0;
        //string cmd = "/e/h.0/localdev/cpp_readout/decode/decode " + file;
        string cmd = online_path + "/DAQ_CPP_v1/decode/decode " + file;
	cout << "decode cmd: " << cmd << endl;
//        ifstream IN(cmd.c_str());
        FILE *IN = popen(cmd.c_str(), "r");
        string s;
        char buf[buf_size];
        if(!IN)
        {
            cout << "Can not open file\n";
            pclose(IN);
        }
        else
        {
            while(fgets(buf,buf_size,IN) != NULL)
            {
                s = buf;
                //  if(isspace(s))
                //     continue;
//                cout << s << "\n";
                vector<string> line;
                SplitString(s,line,",");
                if(strcmp(line[0].c_str(),"p") == 0)
                {
                    int tmp = atoi(line[1].c_str());
                    int mod = (tmp >> 8) & 127;
                //    if(pmt_board && (mod != pmt_board))
                    if(mod != pmt_board)
                        continue;
                    int len = (tmp & 255) - 4;
                    unsigned int time = (atoi(line[2].c_str()) << 16) + atoi(line[3].c_str());
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
            pclose(IN);
        }
    }
}
 
int dounpackbaseline(int argc, char *argv[])
{
    (void)argc;
    char file[512],online_path[512];
    int pmt_board;
    sscanf(argv[1],"%s",file);
    sscanf(argv[2],"%d",&pmt_board);
    sscanf(argv[3],"%s",online_path);

    unpackbaseline(file,pmt_board,online_path);
    return 0;
}
