#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>




using namespace std;

#define buf_size 1024
char buf[buf_size];


int main(int argc, char *argv[])
{
    string file;
    file = argv[1];
    cout << file << "\n";
    string cmd = "/e/h.0/localdev/readout/cppscript/decode/decode " + file;
    FILE *IN = popen(cmd.c_str(), "r");
    if(!IN){
        cout << "Error to open\n";
        pclose(IN);
    }
    else{
        while(fgets(buf,buf_size,IN) != NULL){
            cout << buf;
        }
        pclose(IN);
    }
    return 0;
}
