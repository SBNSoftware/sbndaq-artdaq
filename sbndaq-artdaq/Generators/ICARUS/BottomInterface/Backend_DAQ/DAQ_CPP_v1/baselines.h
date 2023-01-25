#include <string>
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
using std::vector;
using std::to_string;

void SplitString1(string s, vector<string> &v, string c)
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

void baselines(string dir, string file, int pmt_board, string online_path)
{
    double min = 1000;
    
    string dcmacropath = online_path + "/DAQ_CPP_v1/unpackbaseline";
    string cmd = dir + "/" + file + " " + to_string(pmt_board) + " " + online_path;

        std::stringstream ss;
        auto old_buf = std::cout.rdbuf(ss.rdbuf());
        dodecode(cmd.c_str());
        std::cout.rdbuf(old_buf);
    
    double hist[65][4096]={0};
    
    string s;
    char buf[buf_size];
    int c,d;
    if(!ss)
    {
        cout << "Can not open file\n";
    }
    else
    {
    //    while(fgets(buf,buf_size,IN) != NULL)
      //  {
         //   if(isspace(s.c_str()))
         //       continue;
	    s = ss.str();
            vector<string> line;
            SplitString1(s,line,",");
            c = atoi(line[0].c_str());
            d = atoi(line[1].c_str());
            hist[c][d]++;
        //}
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

int dobaselines(string dir, string file, int pmt_board, string online_path)
{
   // char dir[512],file[512],online_path[512];
    //int pmt_board;
    //sscanf(strdir,"%s",dir);
    //sscanf(strfile,"%s",file);
    //sscanf(strpmt_board,"%d",pmt_board);
    //sscanf(stronline_path,"%s",online_path);
    std::cout<<"do baselines: dir = "<<dir<<" file = "<<file<<" online_path = "<<online_path<<std::endl; 
    baselines(dir,file,pmt_board,online_path);
    return 0;
}
