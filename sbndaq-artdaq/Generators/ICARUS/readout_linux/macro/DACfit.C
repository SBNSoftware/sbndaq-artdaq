#include<fstream>

int DACfit(TString filename){

const int n = 10;
fstream f(filename, fstream::in);
double DAC[n];
double ADC[n];
int j=0;
// f.open(filename);
while(f.good()){
f >> DAC[j] >> ADC[j];
j++;
}
f.close();
double par[2]={};
TGraph *gr = new TGraph(j,ADC,DAC);
TF1 *fit = new TF1("fit","pol1");
gr->Fit("fit");
fit->GetParameters(par);
fstream fout(filename, fstream::out | fstream::app);
fout << "\n# " << par[0] << " " << par[1] << endl;
fout.close();

return 0;
}