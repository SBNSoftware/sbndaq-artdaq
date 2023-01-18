#include <cstdio>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <cstring>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;


int main()
{
    unsigned int data = 4418816;
    unsigned char x[4];
    memset(x,'\0',4);
  //  memcpy(x,&data,sizeof(unsigned int));
    x[0] = data & 0xff;
    x[1] = (data>>8) & 0xff;
    x[2] = (data>>16) & 0xff;
    x[3] = (data>>24) & 0xff;
    for(int i =0;i<4;i++)
        printf("%d\n",x[i]);
    printf("\n");

 
}    








