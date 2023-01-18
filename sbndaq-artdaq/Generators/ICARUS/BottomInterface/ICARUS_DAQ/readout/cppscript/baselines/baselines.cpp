#include <string>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <vector>
#include <cmath>

#define buf_size 1024

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

void baselines(string dir, string file, int pmt_board)
{
    double min = 1000;
    
    string dcmacropath = "/e/h.0/localdev/readout/cppscript/unpackbaseline";
    
    double hist[65][2048]={0};
    
    string cmd = dcmacropath + "/unpackbaseline " + dir + "/" + file + " " + to_string(pmt_board);
    FILE *IN = popen(cmd.c_str(), "r");
    string s;
    char buf[buf_size];
    int c,d;
    if(!IN)
    {
        cout << "Can not open file\n";
        pclose(IN);
    }
    else
    {
        while(fgets(buf,buf_size,IN) != NULL)
        {
         //   if(isspace(s.c_str()))
         //       continue;
            s = buf;
            vector<string> line;
            SplitString(s,line,",");
            c = atoi(line[0].c_str());
            d = atoi(line[1].c_str());
            hist[c][d]++;
        }
        pclose(IN);
    }
    
    FILE * OUT;
    string cmd1 = dir + "/baselines.dat";
    OUT = fopen(cmd1.c_str(),"w");
    if(OUT == NULL){
        printf("Can not open file\n");
        fclose(OUT);
    }
    else
    {
        for(int i = 1; i<=64; i++)
        {
            int x;
            int j;
            double f;
            double n = 0;
            double dev = 0;
            double avg = 0;
            for(j = 0; j<2048; j++){
                if(hist[i][j])
                {
                    x = j;
                    f = hist[i][j];
                    avg += f * x;
                    n += f;
                }
            }
            if(n)
            {
                avg = avg / n;
                int t = 0;
                for(t = 0;t<2048; t++){
                    if(hist[i][t])
                    {  
                        x = t;
                        f = hist[i][t];
                        dev += f * pow((x-avg),2);
                    }
                }
                dev = sqrt(dev/n);
            }
            fprintf(OUT,"%d,%f,%f,%f\n",i,avg,dev,n);
            min = n;
        }
        fclose(OUT);
        cout << "PMT " << pmt_board << ":\t hits: " << min << "\n";
    }
}

int main(int argc, char *argv[])
{
    char dir[128],file[128];
    int pmt_board;
    sscanf(argv[1],"%s",dir);
    sscanf(argv[2],"%s",file);
    sscanf(argv[3],"%d",&pmt_board);
    
    baselines(dir,file,pmt_board);
    return 0;
}
