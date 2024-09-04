//200720
//BINARY READ SCRIPT
//Reads the binary data and converts it into events and values.
//Single large buffer reads the file contents in one go.
//Prints the values of channel, energy and time coarse and fine, frame number, stic id etc
//next is to make this a histogram
//next is to make this a root tree

//200902

//BINARY READING WITH A SMALL BUFFER OVER MANY LOOPS
//201112
//BINARY READING - NO DEBUGGING; CHANNEL/FPGA/BOARD SPECIFIC DATA

//201122 - BINARY READ4
// Arguments: file name, Emin and Emax,
// Assigns histograms to each channel x64
// Generates  4x4 canvasses for each 16 channel segment to monitor
// For now start with E histogram, and deltaT histogram
// channel activity on a colZ map (next)


#include "TCanvas.h"
#include "TStyle.h"
#include "TH1.h"
#include "TGaxis.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TFile.h"
#include "TROOT.h"
#include "TRint.h"
#include "TObject.h"
#include "TPad.h"
#include "TVirtualFitter.h"
#include "TMultiGraph.h"
#include "TGraphErrors.h"
#include "TStyle.h"
#include "TAxis.h"
#include "TGraph.h"
#include "TLine.h"

#include <string.h>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <math.h>
#include <complex>
#include <TApplication.h>
#include <TMath.h>

using namespace std;

std::string toBinary(int n)
{
    std::string r; int count=15;
    //while(n!=0) {
    while(count>0) {
        r=(n%2==0 ?"0":"1")+r; n/=2;
        count--;
    }
    return r;
}

//File name:  //test/PET/test/2020-11-19_DCR/DCR_DET-ON_HV-ON-3000_B3F1S3_ch48-63_E0-VNVCO9/190222.653_DCR_DET-ON_HVON3000_B3F1S3_ch48-63_T5-E0-VNVCO9.bin
    //test/DCR_201122
int main(int argc, char *argv[])
//int BinaryRead4(int argc, char *argv[])
//int main()
{
   
    
    
    cout<<"Start1"<<endl;
    
    cout<<"argc="<<argc<<endl;
    cout<<"argv[0]="<<argv[0]<<"\t argv[1]="<<argv[1]<<"\n"<<endl;
    
    ////Variables define
    char argName[1000], fileName[1000], file[10][1000], filenameroot[1000];//file name string
    const int N=20000;
    double xmin=0, xmax=32800;
    if(argc>3){xmin=strtod(argv[2],NULL); xmax=strtod(argv[3],NULL);}
    
    
    sprintf(argName,"%s",argv[1]);
    sprintf(fileName,"%s.bin",argName);
    cout<<"Opening file from the argument..\t"<<fileName<<endl;
    
    
    //ifstream infile1*;
    //sprintf(fileName,"test/PET/test/143611.156.bin");
    const int nevents=1;//100000
    unsigned char buffer[8*nevents];
    unsigned int a[8];
    
    unsigned int ecoarse[nevents], eccOld=0; int decctcc=0;
    unsigned int tcoarse[nevents], tccOld=0; int dtcc=0;
    unsigned int energy[nevents];
    unsigned int efine[nevents];
    unsigned int tfine[nevents];
    unsigned int time[nevents], timeOld=0; int dT=0;
    unsigned int ebad[nevents];
    unsigned int tbad[nevents];
    
    unsigned int channel[nevents];
    unsigned int frame[nevents], frameOld=0; int dframe=0;
    unsigned int gmslID[nevents];
    unsigned int sticID[nevents];
    unsigned int boardID[nevents];
    unsigned int Data0_Sync1[nevents];
    
    //HISTOGRAMS
    TH1F *hEc=new TH1F("hEc","E coarse",32770,-1,32769);
    //TH1F *hTc=new TH1F("hTc","time coarse",32770,-1,32769);//32768
    TH1F *hTc=new TH1F("hTc","time coarse",40000,-1,39999);//32768
    //TH1F *hE=new TH1F("hE","Energy",32768,0,32768);
    //TH1F *hE=new TH1F("hE","Energy",8192,0,32768);
    TH1F *hE[64];
    TH1F *hTf=new TH1F("hTf","time fine",32,0,32);
    TH1F *hEf=new TH1F("hEf","energy fine",32,0,32);
    TH1F *hT=new TH1F("hT","Time",10000000,-5000000,5000000);
    TH1F *hEbad=new TH1F("hEbad","E-bad",2,0,2);
    TH1F *hTbad=new TH1F("hTbad","T-bad",2,0,2);
    
    TH1F *hFrame=new TH1F("hFrame","Frames",256,0,256);
    TH1F *hCh=new TH1F("hCh","Channels",64,0,64);
    TH1F *hSticID=new TH1F("hSticID","STiC ID",8,0,8);
    
    //DIFFERENCE HISTOGRAMS
    TH1F *hdT=new TH1F("hdT","Time difference",1050000,-1000,1049000);//1048576 bins ideally
    TH1F *hdF=new TH1F("hdF","delta Frames",128,-64,64);
    TH1F *hdTcc=new TH1F("hdTcc","delta time coarse",42000,-2000,40000); //32768 bins
    TH1F *hdEccTcc=new TH1F("hdEccTcc","delta energy coarse - time coarse",42000,-2000,40000); //32768
    
    //2D HISTOGRAMS
    TH2F *h2ChTbad = new TH2F("h2ChTbad", "Tbad hits vs Ch", 64,0,64,2,0,2 );
    TH2F *h2TccTbad = new TH2F("h2TccTbad", "Tbad hits vs Tcc", 40000,-1,39999,2,0,2 );
    //TH2F *h2ECh = new TH2F("h2ECh", "Ch vs Energy", 40000,-1,39999,64,0,64 );
    TH2F *h2ECh = new TH2F("h2ECh", "Ch vs Energy", 10000,-1,39999,64,0,64 );
    TH2F *h2ETf = new TH2F("h2ETf", "Tfine vs Energy", 10000,-1,39999,32,0,32 );
    
    char hname[10];
    for(int i=0; i<64; i++)
    {
        sprintf(hname,"%d",i);
        hE[i]=new TH1F(hname,"E",8192,0,32768);
    }

    
    //char path[200],tag[300], date[50];
    FILE *ptr;
    
    //OPENING FILE

    
    cout<<"Opening file.."<<fileName<<endl;
    ptr=fopen(fileName,"rb");
    if(!ptr){cout<<"Disaster!"<<endl; return 0;}
    else{cout<<"opening succesful.."<<fileName<<endl;}
    
    
    /////////////////////////////////////
    /////////////////////////////////////
    //LFSR DECODING****
    int16_t m_lut[ 1 << 15 ], encodedLfsr[1 << 15];
    m_lut[ 0x7FFF ] = -1; // invalid state
    uint16_t lfsr = 0x0000;
    for ( int16_t n = 0; n < ( 1 << 15 ) - 1; ++n )
    {
        m_lut[ lfsr ] = n;
        encodedLfsr[n]=lfsr;
        const uint8_t bits13_14 = lfsr >> 13;
        uint8_t new_bit;
        switch ( bits13_14 )
        { // new_bit = !(bit13 ^ bit14)
            case 0x00:
            case 0x03:
                new_bit = 0x01;
                break;
            case 0x01:
            case 0x02:
                new_bit = 0x00;
                break;
        }// switch
        lfsr = ( lfsr << 1 ) | new_bit; // add the new bit to the right
        lfsr &= 0x7FFF; // throw away the 16th bit from the shift
    }// for
    /////////////////////////////////////
    /////////////////////////////////////
 
    
    //READING FILE THROUGH THE BUFFER
    int ncount=0; // stores the number of events
    while(std::fread(buffer, sizeof(buffer),1,ptr)!='\0')
    {
        if(ncount==0){timeOld=0; frameOld=0;}
        for(int j=0; j<nevents; j++)
        {
            for(int i=0; i<8; i++)
            {
                a[i]=int(buffer[j*8+i]);
                //cout<<int(buffer[i])<<"\t";
            }
            ecoarse[j]=m_lut[ buffer[j*8+3]/32+buffer[j*8+2]*256/32+(buffer[j*8+1]%16)*256*256/32 ];
            tcoarse[j]=m_lut[  (buffer[j*8+0]/4+buffer[j*8+7]*256/4+(buffer[j*8+6]%2)*256*256/4) ];
            efine[j]= buffer[j*8+3]%32;
            tfine[j]= buffer[j*8+1]/32+(buffer[j*8+0]%4)*256/32;
            channel[j]=buffer[j*8+6]/4;
            frame[j]=buffer[j*8+5]%64;
            tbad[j]=(buffer[j*8+6]/2)%2;
            ebad[j]=(buffer[j*8+1]/16)%2;
            sticID[j]=buffer[j*8+5]/64;
            gmslID[j]=buffer[j*8+4]%4;
            boardID[j]=(buffer[j*8+4]/4)%32;
            Data0_Sync1[j]=buffer[j*8+4]/128;
            if(ecoarse[j]>tcoarse[j]){energy[j]=ecoarse[j]-tcoarse[j];}
            else {energy[j]=32767+ecoarse[j]-tcoarse[j];}
            time[j]=32*tcoarse[j]+tfine[j];
            
            ///////ENERGY CALCULATION AND FILLING HISTOGRAMS
            if(tbad[j]>=0)
            //if(channel[j]==8) //29 good in det2
            //if(channel[j]==33&&tbad[j]==0)//&&tbad[j]==0
            {
                dtcc=tcoarse[j]-tccOld; if(dtcc<0){dtcc +=32767;}
                decctcc=tcoarse[j]-eccOld; if(decctcc<0){decctcc +=32767;}
                dframe=frame[j]-frameOld; if(dframe<0){dframe+=64;}
                dT=time[j]-timeOld; if(dT<0){dT+=1048544;} //1048544=(2^15-1)x32
                hEc->Fill(ecoarse[j]);
                hTc->Fill(tcoarse[j]);
                
                hE[channel[j]]->Fill(energy[j]);
                hTf->Fill(tfine[j]);
                hEf->Fill(efine[j]);
                hT->Fill(time[j]);
                
                hEbad->Fill(ebad[j]);
                hTbad->Fill(tbad[j]);
                
                hFrame->Fill(frame[j]);
                    
                hCh->Fill(channel[j]);
                
                hdTcc->Fill(dtcc); //tcoarse[j]-tccOld
                hdEccTcc->Fill(decctcc);
                hdT->Fill(dT);//time[j]-timeOld
                hdF->Fill(dframe);//frame[j]-frameOld
                
                h2ChTbad->Fill(channel[j],tbad[j]);
                h2TccTbad->Fill(tcoarse[j],tbad[j]);
                //h2ECh->Fill(tcoarse[j], channel[j]);
                h2ECh->Fill(energy[j], channel[j]);
                h2ETf->Fill(energy[j],tfine[j]);
                
                timeOld=time[j];
                tccOld=tcoarse[j];
                eccOld=ecoarse[j];
                frameOld=frame[j];
            }
        }
        ncount++;
    }
    fclose(ptr);
    
    
    sprintf(fileName,"%s_output.root",argName);
    TFile* fileRoot = new TFile(fileName, "RECREATE");
    fileRoot->cd();
    cout<<"Root file save.."<<fileName<<endl;
    
    hEc->Write();
    hTc->Write();
    //hE->Write();
    for(int i=0; i<64; i++){hE[i]->Write();}
    hTf->Write();
    hEf->Write();
    hT->Write();
    hEbad->Write();
    hTbad->Write();
    hCh->Write();
    hFrame->Write();
    
    hdT->Write();
    hdF->Write();
    hdTcc->Write();
    hdEccTcc->Write();
    h2ChTbad->Write();
    h2TccTbad->Write();
    h2ECh->Write();
    h2ETf->Write();
    
  
    
    TCanvas *c1 = new TCanvas("c1","Energy coarse",500,500);
    hEc->Draw(); c1->SetLogy();
    TCanvas *c2 = new TCanvas("c2","Time coarse",500,500);
    hTc->Draw(); //c2->SetLogy();
    
    TCanvas *c3[4];
    for(int j=0; j<4; j++)
    {
        sprintf(hname,"E-ChSet%d",j);
        c3[j] = new TCanvas(hname,"Energy",500,500);
        int nx=4, ny=4;
        int number=0;
        c3[j]->Divide(nx,ny,0,0);
        for(int i=0; i<nx*ny; i++)
        {
            number++;
            c3[j]->cd(number);
            //h1->FillRandom("gaus",1000);
            //hE[47+i]->GetXaxis()->SetXLimits();
            //double xmin=0, xmax=800;
            hE[16*j+i]->SetAxisRange(xmin, xmax,"X");
            hE[16*j+i]->GetXaxis()->SetLabelFont(53);
            hE[16*j+i]->GetXaxis()->SetLabelSize(10);
            hE[16*j+i]->GetYaxis()->SetLabelFont(53);
            hE[16*j+i]->GetYaxis()->SetLabelSize(10);
            hE[16*j+i]->DrawCopy();
        }
        sprintf(hname,"%s.pdf",hname);
        c3[j]->Write(); c3[j]->SaveAs(hname);
    }
    
    //hE->Draw(); //c3->SetLogy();
    TCanvas *c4 = new TCanvas("c4","Channel activity",500,500);
    hCh->Draw(); c4->SetLogy();
    TCanvas *c5 = new TCanvas("c5","Time fine",500,500);
    hTf->Draw(); c5->SetLogy();
    TCanvas *c6 = new TCanvas("c6","Energy fine",500,500);
    hEf->Draw(); c6->SetLogy();
    //DIFFERENCE HISTOGRAMS
    TCanvas *c7 = new TCanvas("c7","Period",500,500);
    hdT->Draw(); //c7->SetLogy();
    TCanvas *c8 = new TCanvas("c8","Delta Frames",500,500);
    //hdF->Draw(); //c8->SetLogy();
    hFrame->Draw();
    TCanvas *c9 = new TCanvas("c9","Delta Time coarse",500,500);
    hdTcc->Draw(); //c9->SetLogy();
    
    //TCanvas *c10 = new TCanvas("c10","Tbad vs Channel",500,500);
    //h2ChTbad->Draw();
    //TCanvas *c11 = new TCanvas("c11","Tbad vs Tcc",500,500);
    //h2TccTbad->Draw();
    
    
    TCanvas *c12 = new TCanvas("c12","Ch vs Energy",500,500);
    h2ECh->Draw("colz");
    c12->Write();
    
    TCanvas *c13 = new TCanvas("c13","Tfine vs Energy",500,500);
    h2ETf->Draw("colz");
    c13->Write();
    
    
    fileRoot->Close();
    
    cout<<"Data events="<<ncount*5<<endl;
    cout<<"The End"<<endl;
    
    
    
    
    //return 0;
    
    ///////////THE END
    
    
}



//LFSR VALUES FOR REF
//for(int i=0; i<( 1 << 15 ) - 1; i++)
//{
//    cout<<i<<"\tLFSR[i]"<<encodedLfsr[i]<<"\tDecodedLFSR="<<m_lut[i]<<endl;
//}
