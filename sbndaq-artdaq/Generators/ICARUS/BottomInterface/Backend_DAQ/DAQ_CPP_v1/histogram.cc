
#include <string>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <vector>
#include <cmath>

using namespace std;
using std::cout;
using std::endl;
using std::string;

void SplitString2(string s, vector<string> &line, string c)
{
    size_t pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        line.push_back(s.substr(pos1,pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c,pos1);
    }
    if(pos1 != s.length())
        line.push_back(s.substr(pos1));
}

void histogram(string dir, string file,int col, string name)
{
   
    double data[64];
    double bins[64];
    
    // read input file
    string cmd = dir + "/" + file;
    ifstream IN(cmd.c_str());
    string s;
    int j = 0;
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
           // getline(IN, s);
            // if(isspace(s))
            //    continue;
            vector<string> line;
            SplitString2(s,line,",");
            double d = atof(line[col].c_str());
            data[j] = d;
            j++;
        }
        IN.close();
    }
    // generate histogram
    int n = sizeof(data)/sizeof(data[0]);
    if(!n)
        exit(0);
    double avg = 0;
    double dev = 0;
    FILE * OUT;
    string cmd1 = dir + "/histogram.dat";
    OUT = fopen(cmd1.c_str(),"w");
    if(OUT == NULL){
        printf("Can not open file\n");
        fclose(OUT);
    }
    else
    {
        if(n<3)
        {
            fprintf(OUT,"0,0\n0.5,1\n1,0\n");
        }
        else
        {
            double min = data[0];
            double max = data[0];
            for(int i = 0; i<n;i++)
            {
                if(data[i]<min)
                    min = data[i];
                if(data[i]>max)
                    max = data[i];
                avg += data[i];
            }
            avg = avg/n;
            int i;
            for(i=0;i<n;i++)
            {
                dev += pow((data[i]-avg),2);
            }
            dev = sqrt(dev/n);
            int nbins = 50;
            double wbins = (max-min)/nbins;
            if(wbins == 0)
                wbins = 1;
            min -= 0.0001 * wbins;
            max += 0.0001 * wbins;
            wbins = (max - min) / nbins;
            for(i=0;i<nbins;i++)
            {
                bins[i] = 0;
            }
            for(i=0;i<n;i++)
            {
                bins[int((data[i]-min)/wbins)]++;
            }
            min += 0.5 * wbins;
            for(i=0;i<nbins;i++)
            {
                fprintf(OUT,"%f,%f\n",min,bins[i]);
                min += wbins;
            }
        }
        fclose(OUT);
    }
    
    double per = 0;
    if(avg)
    {
        per = int(0.5 + 100 * dev/avg);
    }
    cout << per << "\n";
    string title;
    char titlebuf[128];
    sprintf(titlebuf,"%s    Mean = %.3f     Std Dev = %.3f (%.2f %%)   Total = %d",name.c_str(),avg,dev,per,n);
    title = titlebuf;
    
    FILE * PLOT;
    string cmd2 = dir + "/plot.gp";
    PLOT = fopen(cmd2.c_str(),"w");
    if(PLOT == NULL){
        printf("Can not open file\n");
        fclose(PLOT);
    }
    else
    {
        fprintf(PLOT,"set grid\n");
        fprintf(PLOT,"set datafile separator \",\"\n");
        fprintf(PLOT,"set term png size 1024,768\n");
        fprintf(PLOT,"set title \"%s\"\n",title.c_str());
        fprintf(PLOT,"set xlabel \"%s\"\n",name.c_str());
        fprintf(PLOT,"set ylabel \"Count\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"\n");
        
        size_t found = title.find("Gains");
        if(found!=string::npos)
            fprintf(PLOT,"set xrange [0:1000000]\n");
        fprintf(PLOT,"set yrange [0:*]\n");
        fprintf(PLOT,"set output \"%s/%s.png\"\n",dir.c_str(),name.c_str());
        fprintf(PLOT,"plot \"%s/histogram.dat\" using 1:2 title \"Histogram\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        fclose(PLOT);
    }
    string plotstr = "/usr/bin/gnuplot " + dir + "plot.gp";
    int status = system(plotstr.c_str());
    if(status != 0)
    {
        cout << "Error running gnuplot: " << status << endl;
    }
}


int dohistogram(int argc, char *argv[])
{
    (void)argc;
    char dir[128];
    char file[128];
    string name;
    int col;
    sscanf(argv[1],"%s",dir);
    sscanf(argv[2],"%s",file);
    sscanf(argv[3],"%d",&col);
    name = argv[4];
    
    histogram(dir,file,col,name);
    return 0;
}

