#include "interpolatingDetection.h"

namespace SpkDslowFilter {

Detection::Detection() {}   

int* SetInitialParams (long nFrames, double nSec, int sf, double sfd, int NCh, int* Indices) {
    dfTI = new int[3];
    Aglobal = new int[tInc];
    Aglobaldiff = new int[tInc];
    AglobalSdiff = new int[tInc];
    for (int i = 0; i < tInc; i++) {
        Aglobal[i] = 4094;
        Aglobaldiff[i] = 0;
        AglobalSdiff[i] = 0;
    }

    NChannels = NCh;
    //NFrames = nFrames;
    sfi = sf / 1670;
    FrameDt = (decimal)(1000.0 / sfd);
    int* SInd = new int[4096];
    int* SInd4 = new int[4096];
    int* SInd5 = new int[4096];
    ChInd4a = new int*[NCh];
    ChInd4b = new int*[NCh];
    ChInd4c = new int*[NCh];
    //ChInd4d = new int*[NCh];
    ChInd5 = new int[NCh];
    ChInd5a = new int*[NCh];
    ChInd5b = new int*[NCh];
    ChInd5c = new int*[NCh];
    //ChInd5d = new int*[NCh];
    for (int i = 0; i < NCh; i++) {//fillvalues
        ChInd4a[i] = new int[4];
        ChInd4b[i] = new int[8];
        ChInd4c[i] = new int[4];
        //ChInd4d[i] = new int[4];
        ChInd5a[i] = new int[4];
        ChInd5b[i] = new int[4];
        ChInd5c[i] = new int[4];
        //ChInd5d[i] = new int[12];
        SInd[i] = -1;
        SInd4[i] = 0;
        SInd5[i] = 0;
        for (int j = 0; j < 8; j++) {
            ChInd4b[i][j] = -1;
        }
        for (int j = 0; j < 4; j++) {
            ChInd4a[i][j] = -1;
            ChInd4c[i][j] = -1;
            ChInd5a[i][j] = -1;
            ChInd5b[i][j] = -1;
            ChInd5c[i][j] = -1;
        }
        ChInd5[i] = -1;
    }
    for (int i = 0; i < NCh; i++) {//find active channels and number of neighbors
        SInd[Indices[i]] = i;
        SInd4[Indices[i]] += 1;
        SInd5[Indices[i]] += 1;
        if ((Indices [i] % 64) >0) {//channel to the left
            SInd4 [Indices [i] - 1] += 1;
            SInd5 [Indices [i] - 1] += 1;
            if ((Indices [i] / 64) >0) {//up and left
                SInd4 [Indices [i] - 65] += 1;
            }
        }
        if ((Indices [i] % 64) <63) {//right
            SInd5 [Indices [i] + 1] += 1;
        }
        if ((Indices [i] / 64) >0) {//up
            SInd4 [Indices [i] - 64] += 1;
            SInd5 [Indices [i] - 64] += 1;
        }
        if ((Indices [i] / 64) <63) {//down
            SInd5 [Indices [i] + 64] += 1;
        }
    }//channels with (all) neighbors have SInd4=4 or SInd5=5
    int ChInd4aN = 0;
    int ChInd5N = 0;
    for (int i=0; i<4096; i++) {
        if (SInd4 [i] == 4) {
            ChInd4a [SInd [i]][0] = SInd [i];
            ChInd4aN++;
            ChInd4a [SInd [i]][1] = SInd [i + 1];
            ChInd4a [SInd [i]][2] = SInd [i + 65];
            ChInd4a [SInd [i]][3] = SInd [i + 64];
            if (SInd5 [i] == 5) {
                ChInd4c [SInd [i]][0] = SInd [i];
            }
            if (SInd5 [(i + 1)%4096] == 5) {
                ChInd4c [SInd [i]][1] = SInd [(i + 1) % 4096];
            }
            if (SInd5 [(i + 64) % 4096] == 5) {
                ChInd4c [SInd [i]][3] = SInd [(i +64) % 4096];
            }
            if (SInd5 [(i + 65) % 4096] == 5) {
                ChInd4c [SInd [i]][2] = SInd [(i + 65) % 4096];
            }
            if (SInd4 [(i + 4032) % 4096] == 4) {
                ChInd4b [SInd [i]][0] = SInd [(i + 4032) % 4096];
                ChInd4b [SInd [i]][1] = SInd [(i + 4033) % 4096];
            }
            if (SInd4 [(i + 1) % 4096] == 4) {
                ChInd4b [SInd [i]][2] = SInd [(i + 2) % 4096];
                ChInd4b [SInd [i]][3] = SInd [(i + 66) % 4096];
            }
            if (SInd4 [(i + 64) % 4096] == 4) {
                ChInd4b [SInd [i]][4] = SInd [(i + 129) % 4096];
                ChInd4b [SInd [i]][5] = SInd [(i + 128) % 4096];
            }
            if (SInd4 [(i + 4095) % 4096] == 4) {
                ChInd4b [SInd [i]][6] = SInd [(i + 63) % 4096];
                ChInd4b [SInd [i]][7] = SInd [(i + 4095) % 4096];
            }
        }
        if (SInd5 [i] == 5) {
            ChInd5 [SInd [i]] = SInd [i];
            ChInd5N++;
            ChInd5a [SInd [i]][0] = SInd [i - 64];
            ChInd5a [SInd [i]][1] = SInd [i + 1];
            ChInd5a [SInd [i]][2] = SInd [i + 64];
            ChInd5a [SInd [i]][3] = SInd [i - 1];
            if (SInd4 [i] == 4) {
                ChInd5c [SInd [i]][2] = SInd [i];
                ChInd5b [SInd [i]][2] = SInd [(i + 65) % 4096];
            }
            if (SInd4 [(i + 4095)%4096] == 4) {
                ChInd5c [SInd [i]][3] = SInd [(i + 4095) % 4096];
                ChInd5b [SInd [i]][3] = SInd [(i + 63) % 4096];
            }
            if (SInd4 [(i + 4032) % 4096] == 4) {
                ChInd5c [SInd [i]][1] = SInd [(i + 4032) % 4096];
                ChInd5b [SInd [i]][1] = SInd [(i + 4033) % 4096];
            }
            if (SInd4 [(i + 4031)%4096] == 4) {
                ChInd5c [SInd [i]][0] = SInd [(i + 4031)%4096];
                ChInd5b [SInd [i]][0] = SInd [(i + 4031)%4096];
            }
        }
    }
    //ChInd5[ChInd4[i,ii],ii+5]

    //count -1 in ChInd4a[i][0]
    ChInd4List= new int[ChInd4aN];
    int iiii = 0;
    for (int i=0; i<NCh; i++) {
        if (ChInd4a [i] [0] != -1) {
            ChInd4List [iiii] = ChInd4a [i] [0];
            iiii++;
        }
    }
    ChInd5List= new int[ChInd5N];
    iiii = 0;
    for (int i=0; i<NCh; i++) {
        if (ChInd5 [i] != -1) {
            ChInd5List [iiii] = ChInd5 [i];
            iiii++;
        }
    }
    Qd = new int[NChannels];//noise amplitude
    Qm = new int[NChannels];//median
    QmPre = new int[NChannels];
    QmPreD = new int[NChannels];
    Qdiff = new int[NChannels][dtMx];
    Qmax = new int[NChannels][2];
    QmaxE = new int[NChannels];
    SqIv = new long[NChannels];//sum of squared channel increments
    SIprod = new long[NChannels][13];//sum of product of global and channel voltage increments
    //SIp = new int[NChannels];
    Vbias = new int[NChannels];
    FVbias = new int[NChannels];
    FVsbias = new long[NChannels];
    Vsqbias = new long[NChannels];
    A = new int[NChannels];//control parameter for amplifier effects
    Sl4 = new int[NChannels];//counter for spike length
    Sl4x = new bool[NChannels];
    Z4next = new int[NChannels];
    Z5next = new int[NChannels];
    AHP4 = new bool[NChannels];//counter for repolarizing current
    Amp4 = new int[NChannels];//buffers spike amplitude
    //Qd5 = new int[NChannels];//noise amplitude
    Sl5 = new int[NChannels];//counter for spike length
    Sl5x = new bool[NChannels];
    AHP5 = new bool[NChannels];//counter for repolarizing current
    Amp5 = new int[NChannels];//buffers spike amplitude
    Slice = new int[NChannels];
    Avgs1= new int[NChannels][3];
    //sortAvg= new int[NChannels][2];
    //Avgs1b= new int[NChannels][];
    Avgs3= new int[NChannels];
    
    // Parameters    
    float detection_threshold = 6.0;
    float repolarization_threshold = 0.0;
    bool recalibration = true;
    int increment = 1;
    float cutoutPrePeak = 2*sfi*FrameDt; // 1.14
    float cutoutPostPeak = 3*sfi*FrameDt; // 1.71
    float smoothing_kernel = (sf/5000+2)*FrameDt; // 0.43
    bool measure_autocorrelation = false;
    
    // Set the parameters
    threshold = (int)(detection_threshold*AmpScale);
    //thr5 = (threshold + 2) / 2;
    df = (int) increment;
    AHPthr = (int)(repolarization_threshold*AmpScale);
    CutPre = (int)(cutoutPrePeak/FrameDt+0.1);
    CutPost = (int)(cutoutPostPeak/FrameDt+0.1);
    tSmth = (int)(smoothing_kernel/FrameDt+0.1);
    tSmth1 = tSmth - 1;
    ACF = measure_autocorrelation;

    if (sf / 5000 > 3) {
        NFblocks = sf / 5000;
    }
    Avgs2 = new int[NChannels][NFblocks];
    FiltNorm = 2*(8 * (NFblocks - 1) + 2);
    HF = (sf > 12000);
    if (HF) {
        TauFiltOffset = TauFilt / 2 + 4 * TauFilt1 *(NFblocks);
    } else {
        TauFiltOffset = TauFilt / 2 + 2 * TauFilt1*(NFblocks+1);
    }
    if (df >= 0) {
        if (df == 0) {
            df += 1;
        }
        dfAbs = df;
        dfSign = 1;
        Slmax = (sf / 1000)/df +1;
        CutOffset = (sf / 1000 + CutPre)/df-tSmth/2;//from where to start cutting...
        //CutAfter = (sf / 1002)/df;
        //CutAfterLong = (sf / 501)/df;
        tCut = (CutPre + 1 + CutPost) / df;//6 +(sf / 1002 + sf / 1000)/df;//cutout length
        tCutLong = tCut + sfi / df;//6 +(sf / 501 + sf / 1000)/df;// long cutout length
        Sln0= (sf / 2334)/df;//3340//835*12/5
        Sampling = sf/df;
        SqIglobal = 0;
        tx = 0;
        ti = df * std::max(CutOffset,8);//8 needed for correlation estimate!
        tf =  df * (tInc -std::max(tCutLong-CutOffset,std::max(8,TauFiltOffset+2*TauFilt1)));
        tm=(tInc-1)*df-tf+ti;
        tdSmth1=-(tSmth-1);
        tdSmth=-tSmth;
        tms=(tInc-tSmth1-1)*df-tf+ti;
        tfiA = (tf - ti) / dfAbs;
        ty=df*(tInc-1);
        t0x = 0;
        dfTI [1] = tf-ti;
    }
    else {
        dfAbs = -df;
        dfSign = -1;
        Slmax = -(sf / 1000)/df+1;
        CutOffset =(sf / 1000 - CutPre)/df+(tSmth-1)/2;// 4 +(sf/835)/df;//should be ok if negative
        //CutAfter = 2 -(sf / 1002 + sf/835 + sf / 1000)/df;
        //CutAfterLong = 2 -(sf / 1002 + sf/835 + sf / 1000)/df;
        tCut=-(CutPre + 1 + CutPost) / df;//6 -(sf / 1002 + sf / 1000)/df;
        tCutLong=-(CutPre + 1 + CutPost) / df;//same as tCut here; long spikes do not make sense
        //tCutLong0=-(CutPre + 1 + CutPost + sfi) / df;//6 -(sf / 501 + sf / 1000)/df;
        Sln0= -(sf / 2334)/df;
        Sampling = -sf/df;
        SqIglobal = 0;
        tx = (-tInc+1)*df;
        ti =  df *(std::max(tCutLong-CutOffset,9)-tInc);//backward starts with -1...
        tf = std::max(std::max(7,TauFiltOffset+2*TauFilt1-1),CutOffset)*dfAbs;//-df * CutOffset;
        tm=ti-tf;
        tdSmth1=(tSmth-1);
        tdSmth=tSmth;
        tms=-(tSmth1)*df-tf+ti;
        tfiA = (tf - ti) / dfAbs;
        ty=0;
        t0x = nFrames + df*tInc;
        dfTI [1] = ti-tf;//positive
    }
    TauFiltOffset *= dfSign;
    Subsample = (dfAbs > 1);
    tQm0 = Sampling / 2000;
    tQmi = (Sampling + 1000) / 2000+tQm0;
    tQmf = (Sampling * 6) / 5000+tQmi;
    //tQml = tQmf + tQmi+tQm0;
    tQm= new int[tQmf];
    tQmLong= new int[tQmf];
    tQmX= new int[tQmf];
    tQmXLong= new int[tQmf];
    for (int i=tQm0; i<tQmi; i++) {
        tQm [i] = -CutOffset + i-tQm0;//-8:-4
        tQmLong [i] = -CutOffset + i-tQm0;
        tQmX [i] = i-tQm0;//-8:-4
        tQmXLong [i] = i-tQm0;
    }
    for (int i=tQmi; i<tQmf; i++) {
        tQm [i] = -CutOffset+tCut-tQmf+ i;//4:12
        tQmLong [i] = -CutOffset+tCutLong-tQmf + i;
        tQmX [i] = tCut +i-tQmf;//-8:-4
        tQmXLong [i] = tCutLong+i-tQmf;
    }
    tQmm = (tQmf) / 2;
    tQmA= new int[tQmf];
    Lspike = Sampling / 5000 + 3;
    Lsw = (Lspike - 1);
    Li = CutPre - CutPre / 3 - Lspike / 2;//-4:4 how likely Overlap?
    Lf = CutPre + CutPre / 3 - Lspike / 2 +1;
    //Ll = Lf - Li + Lspike - 1;
    //LiX = - CutPre / 3 - Lspike / 2;//-4:4 how likely Overlap?
    //LfX = CutPre / 3 - Lspike / 2 +1;
    Lw=new int[Lspike];
    Lw [0] = 1;
    Lw [Lspike - 1] = 1;
    for (int i =1; i<Lspike-1; i++) {
        Lw [i] = 2;
    }
    Lmx= 0;
    tShape= new int[tCutLong];
    dfTI [0] = df;
    dfTI [2] = tInc*dfAbs;//positive
    Ascale = (Ascale0 / tSmth / tSmth1/2) *2* tSmth * tSmth1;
    AscaleV = Ascale / tSmth;
    AscaleG = Ascale / tSmth1/2;
    AglobalSmth=new int[tInc];
    for (int i=0; i<tInc; i++) {
        AglobalSmth [i] = 2047*Ascale;
    }
    vSmth=new int[NChannels];
    //TauF1 = 2 * TauFilt1 * dfAbs;

    // Log    
    std::cout << "Detecting spikes..." << std::endl;

    for (int i=0;i<NChannels;i++){
        Qd[i]=600;
        Qm[i]=0;
        for (int ij=0; ij<dtMx; ij++) {
            Qdiff [i][ij] = 0;
            Qmax[i][ij]=0;
        }
        QmaxE[i]=0;
        SqIv[i]=0;//sum of squared channel increments
        //SIp[i]=0;
        for (int ii=0; ii<13; ii++) {
            SIprod[i][ii]=0;//sum of product of global and channel voltage increments
        }
        for (int iii=0; iii<3; iii++) {
            Avgs1 [i][iii] = 4094;//sum of product of global and channel voltage increments
        }
        for (int iii=0; iii<NFblocks; iii++) {
            Avgs2 [i][iii] = 4094;//sum of product of global and channel voltage increments
        }
        QmPreD [i] = 0;
        Avgs3 [i] = 2047*Ascale;
        FVbias[i]=0;
        Vbias[i]=0;
        FVsbias[i]=0;
        Vsqbias[i]=0;
        A[i]=0;
        Sl4[i]=0;
        Sl4x[i]=false;
        Z4next[i]=0;
        Z5next[i]=0;
        AHP4[i]=false;
        Amp4[i]=0;
        Sl5[i]=0;
        Sl5x[i]=false;
        AHP5[i]=false;
        Amp5[i]=0;
    }
    return dfTI;
}

static std::ofstream w; //for spikes
static std::ofstream wShapes; //for raw data
static std::ofstream wX; //for spikes
static std::ofstream wShapesX; //for raw data
static std::ofstream wInfo; //for other stuff
static std::ofstream wMean; //for avg. Voltage
        
void AvgVoltageDefault(short** vm, long t0, int t) { //want to compute an approximate 33 percentile
    //can average over 2 consecutive frames
    //each time called, I should take the next 4 frames of vm (all channels)
    //would need to correct for Aglobal
    //iterates over 8 frames
    int P1;
    int P2;
    int Plow = 0;//lowest value to discard
    int Plowx = 0;//temporary variable
    int k;
    int kk;
    bool Px;
    if (Subsample) {
        if (HF) {
            k = (int)((t + t0) / (8 * TauFilt1)) % NFblocks;
            kk = (int)((t0 + t) / (TauFilt1 * 2)) % 4;
        } else {
            k = (int)((t + t0) / (TauFilt1 * 8)) % NFblocks;
            if ((((t0 + t) / (TauFilt1 * 2)) % 2) != 0) {
                kk = 2;
            } else {
                kk = (int)(((t0 + t) / (4 * TauFilt1)) % 2);
            }
        }
        int knext = (k + (NFblocks - 1)) % NFblocks;
        int klast = (k + 1) % NFblocks;
        for (int i=1; i<NChannels; i++) {//loop across channels
            //want something that works in most cases
            Px = false;
            //assume that I already have the right value
            P1 = 2*vm [i] [t]-Aglobal[t/dfAbs];
            for (int tt=1; tt<TauFilt1; tt++) {//this function wastes most of the time
                Plowx = 2*vm [i] [t + tt * df]-Aglobal[(t+tt*df)/dfAbs];//factor of 3!
                if (Plowx < P1) {
                    if (not Px) {
                        Px = true;
                        Plow = Plowx;
                    } else {
                        P1 = Plow;
                        Plow = Plowx;
                    }
                }
            }
            if (not Px) {//P1 was the lowest value
                P1 = Plowx;
                for (int tt=TauFilt1-2; tt>0; tt--) {
                    Plowx = 2*vm [i] [t + tt * df]-Aglobal[(t+tt*df)/dfAbs];
                    if (Plowx < P1) {
                        P1 = Plowx;
                    }
                }
            }
            //outliers
            if (((P1 + 13000) % 16000) < 10000) {
                P1 = 0;
            }
            //how about average over four frames, take weighted average (first and last weighted by 1/2 or so)
            //i.e. for intermediate values, just change the weighting
            //(advantage: less memory wastage and only need to compute median every fourth frame)
            //have one block of four values to estimate (make area to average shorter), take weighted average of first and last block
            if (kk < 2) {
                Avgs3 [i] += (Avgs2 [i][knext] - Avgs2 [i][klast]) * Ascale / FiltNorm;
                Avgs1 [i][kk] = 2*P1;
            } else if (HF & (kk<3)) {
                Avgs3 [i] += (Avgs2 [i][knext] - Avgs2 [i][klast]) * Ascale / FiltNorm;
                Avgs1 [i][kk] = 2*P1;
            } else {
                //assume that I already have the right value
                Px = false;
                P2 = 2*P1;
                for (int tt=0; tt<3; tt++) {//this function wastes most of the time
                    Plowx = Avgs1 [i][tt];
                    if (Plowx < P2) {
                        if (not Px) {
                            Px = true;
                            Plow = Plowx;
                        } else {
                            P2 = Plow;
                            Plow = Plowx;
                        }
                    }
                }
                if (not Px) {//P1 was the lowest value
                    P2 = Plowx;
                    for (int tt=2; tt>=0; tt--) {
                        Plowx = Avgs1 [i][tt];
                        if (Plowx < P2) {
                            P2 = Plowx;
                        }
                    }
                }
                Avgs2 [i][klast] = P2;
                if (not HF) {
                    Avgs1 [i][2] = 2*P1;
                }
                //to avoid accumulating numerical errors (not sure whether necessary)
                if (NFblocks == 3) {
                    Avgs3 [i] = (P2 + 4 * (Avgs2 [i][knext] + Avgs2 [i][k])) * Ascale/ FiltNorm;
                } else {
                    Avgs3 [i] = -3*P2;
                    for (int tt=0; tt<NFblocks; tt++) {
                        Avgs3 [i] += 4 * Avgs2 [i][tt];
                    }
                    Avgs3 [i] *= Ascale;
                    Avgs3 [i] /=FiltNorm;
                }
            }
        }
    } else {
        if (df > 0) {
            if (HF) {
                k = (int)((t + t0) / (8 * TauFilt1)) % NFblocks;
                kk = (int)((t0 + t) / (2 * TauFilt1)) % 4;
            } else {
                k = (int)((t + t0) / (4 * TauFilt1)) % NFblocks;//overlapping blocks
                if ((((t0 + t) / (2 * TauFilt1)) % 2) != 0) {
                    kk = 2;
                } else {
                    kk = (int)(((t0 + t) / (4 * TauFilt1)) % 2);
                }
            }
            int knext = (k + (NFblocks - 1)) % NFblocks;
            int klast = (k + 1) % NFblocks;
            //Console.WriteLine ("{0} {1} {2}", k, knext, klast);
            //int FiltNorm = 18;
            //int Pl = 0;//lowest value to discard
            //int Plx = 0;//temporary variable
            //int[] Px1= new int[TauFilt1];
            for (int i=1; i<NChannels; i++) {//loop across channels
                //want something that works in most cases
                Px = false;
                //assume that I already have the right value
                P1 = 2*(vm [i] [t] + vm [i] [t + 1])-Aglobal[t]-Aglobal[t+1];
                for (int tt=1; tt<TauFilt1; tt++) {//this function wastes most of the time
                    Plowx = 2*(vm [i] [t + 2 * tt] + vm [i] [t + 2 * tt + 1])-Aglobal[t+2*tt]-Aglobal[t+2*tt+1];
                    if (Plowx < P1) {
                        if (not Px) {
                            Px = true;
                            Plow = Plowx;
                        } else {
                            P1 = Plow;
                            Plow = Plowx;
                        }
                    }
                }
                if (!Px) {//P1 was the lowest value
                    P1 = Plowx;
                    for (int tt=TauFilt1-2; tt>0; tt--) {
                        Plowx = 2*(vm [i] [t + 2 * tt] + vm [i] [t + 2 * tt + 1])-Aglobal[t+2*tt]-Aglobal[t+2*tt+1];
                        if (Plowx < P1) {
                            P1 = Plowx;
                        }
                    }
                }
                //outliers
                if (((P1 + 26000) % 32000) < 20000) {
                    P1 = 0;
                }
                //how about average over four frames, take weighted average (first and last weighted by 1/2 or so)
                //i.e. for intermediate values, just change the weighting
                //(advantage: less memory wastage and only need to compute median every fourth frame)
                //have one block of four values to estimate (make area to average shorter), take weighted average of first and last block
                //Avgs1 [i][kk] = P1;
                if (kk < 2) {
                    Avgs3 [i] += (Avgs2 [i][knext] - Avgs2 [i][klast]) * Ascale / FiltNorm;
                    Avgs1 [i][kk] = P1;
                } else if (HF & (kk < 3)) {
                    Avgs3 [i] += (Avgs2 [i][knext] - Avgs2 [i][klast]) * Ascale / FiltNorm;
                    Avgs1 [i][kk] = P1;
                } else {
                    //assume that I already have the right value
                    P2 = P1;
                    Px = false;
                    for (int tt=0; tt<3; tt++) {//this function wastes most of the time
                        Plowx = Avgs1 [i][tt];
                        if (Plowx < P2) {
                            if (not Px) {
                                Px = true;
                                Plow = Plowx;
                            } else {
                                P2 = Plow;
                                Plow = Plowx;
                            }
                        }
                    }
                    if (not Px) {//P1 was the lowest value
                        P2 = Plowx;
                        for (int tt=1; tt>=0; tt--) {
                            Plowx = Avgs1 [i][tt];
                            if (Plowx < P2) {
                                P2 = Plowx;
                            }
                        }
                    }
                    Avgs2 [i][klast] = P2;//klast will be knext after one iteration
                    if (not HF) {
                        Avgs1 [i][2] = P1;
                    }
                    //to avoid accumulating numerical errors (not sure whether necessary)
                    if (NFblocks == 3) {
                        Avgs3 [i] = (P2 + 4 * (Avgs2 [i][knext] + Avgs2 [i][k])) * Ascale / FiltNorm;
                    } else {
                        Avgs3 [i] = -3 * P2;
                        for (int tt=0; tt<NFblocks; tt++) {
                            Avgs3 [i] += 4 * Avgs2 [i][tt];
                        }
                        Avgs3 [i] *= Ascale;
                        Avgs3 [i] /= FiltNorm;
                    }
                }
            }
        } else {
            if (HF) {
                k = (int)((t + t0) / (8 * TauFilt1)) % NFblocks;
                kk = (int)((t0 + t) / (2 * TauFilt1)) % 4;
            } else {
                k = (int)((t + t0) / (4 * TauFilt1)) % NFblocks;//overlapping blocks
                if ((((t0 + t) / (2 * TauFilt1)) % 2) != 0) {
                    kk = 2;
                } else {
                    kk = (int)(((t0 + t) / (4 * TauFilt1)) % 2);
                }
            }
            int knext = (k + (NFblocks - 1)) % NFblocks;
            int klast = (k + 1) % NFblocks;
            //Console.WriteLine ("{0} {1} {2}", k, knext, klast);
            //int FiltNorm = 18;
            //int Pl = 0;//lowest value to discard
            //int Plx = 0;//temporary variable
            //int[] Px1= new int[TauFilt1];
            for (int i=1; i<NChannels; i++) {//loop across channels
                //want something that works in most cases
                Px = false;
                //assume that I already have the right value
                P1 = 2*(vm [i] [t] + vm [i] [t - 1])-Aglobal[t]-Aglobal[t-1];
                for (int tt=1; tt<TauFilt1; tt++) {//this function wastes most of the time
                    Plowx = 2*(vm [i] [t - 2 * tt] + vm [i] [t - 2 * tt - 1])-Aglobal[t-2*tt]-Aglobal[t-2*tt-1];
                    if (Plowx < P1) {
                        if (not Px) {
                            Px = true;
                            Plow = Plowx;
                        } else {
                            P1 = Plow;
                            Plow = Plowx;
                        }
                    }
                }
                if (not Px) {//P1 was the lowest value
                    P1 = Plowx;
                    for (int tt=TauFilt1-2; tt>0; tt--) {
                        Plowx = 2*(vm [i] [t - 2 * tt] + vm [i] [t - 2 * tt - 1])-Aglobal[t-2*tt]-Aglobal[t-2*tt-1];
                        if (Plowx < P1) {
                            P1 = Plowx;
                        }
                    }
                }
                //outliers
                if (((P1 + 26000) % 32000) < 20000) {
                    P1 = 0;
                }
                //how about average over four frames, take weighted average (first and last weighted by 1/2 or so)
                //i.e. for intermediate values, just change the weighting
                //(advantage: less memory wastage and only need to compute median every fourth frame)
                //have one block of four values to estimate (make area to average shorter), take weighted average of first and last block
                //Avgs1 [i][kk] = P1;
                if (kk < 2) {
                    Avgs3 [i] += (Avgs2 [i][knext] - Avgs2 [i][klast]) * Ascale / FiltNorm;
                    Avgs1 [i][kk] = P1;
                } else if (HF & (kk < 3)) {
                    Avgs3 [i] += (Avgs2 [i][knext] - Avgs2 [i][klast]) * Ascale / FiltNorm;
                    Avgs1 [i][kk] = P1;
                } else {
                    //assume that I already have the right value
                    P2 = P1;
                    Px = false;
                    for (int tt=0; tt<3; tt++) {//this function wastes most of the time
                        Plowx = Avgs1 [i][tt];
                        if (Plowx < P2) {
                            if (not Px) {
                                Px = true;
                                Plow = Plowx;
                            } else {
                                P2 = Plow;
                                Plow = Plowx;
                            }
                        }
                    }
                    if (not Px) {//P1 was the lowest value
                        P2 = Plowx;
                        for (int tt=1; tt>=0; tt--) {
                            Plowx = Avgs1 [i][tt];
                            if (Plowx < P2) {
                                P2 = Plowx;
                            }
                        }
                    }
                    Avgs2 [i][klast] = P2;//klast will be knext after one iteration
                    if (not HF) {
                        Avgs1 [i][2] = P1;
                    }
                    //to avoid accumulating numerical errors (not sure whether necessary)
                    if (NFblocks == 3) {
                        Avgs3 [i] = (P2 + 4 * (Avgs2 [i][knext] + Avgs2 [i][k])) * Ascale / FiltNorm;
                    } else {
                        Avgs3 [i] = -3 * P2;
                        for (int tt=0; tt<NFblocks; tt++) {
                            Avgs3 [i] += 4 * Avgs2 [i][tt];
                        }
                        Avgs3 [i] *= Ascale;
                        Avgs3 [i] /= FiltNorm;
                    }
                }
            }
        }
    }
    //Console.WriteLine ("{0} {1} {2}",Avgs2[2120,0],Avgs2[2120,1], Avgs2[2120,2]);
}

void InitialEstimation(short** vm, long t0) { //use this to get a better initial estimate of Qd. only fast transients.
    int tA;
    if (t0 == t0x) {
        //estimate Aglobal
        for (int t=tx; dfSign*t<dfSign*ty; t+=df) {//loop over data, will be removed for an online algorithm
            tA = t / dfAbs;
            for (int i=1; i<NChannels; i++) {//loop across channels
                Slice [i] = (vm [i] [t]) % 4095 + (vm [i] [t + df]) % 4095;
            }
            std::sort(std::begin(Slice), std::end(Slice));
            Aglobal [tA] = Slice [NChannels / 2];
        }
        for (int t=tx/dfAbs+dfSign; dfSign*t<ty/df; t+=dfSign) {
            Aglobaldiff [t] = Aglobal [t] - Aglobal [t - dfSign];
        }
        for (int t=tx/dfAbs; dfSign*t<ty/df-dfSign*tSmth1; t+=dfSign) {
            //tA = t / dfAbs;
            AglobalSmth [t] = Aglobal [t];
            for (int ii=1; ii<tSmth1; ii++) {
                AglobalSmth [t] += Aglobal [t + ii*dfSign];
            }
            AglobalSmth [t] *= AscaleG;
        }
        for (int t=tx/dfAbs+dfSign; dfSign*t<ty/df-dfSign*tSmth1; t+=dfSign) {
            AglobalSdiff [t] = AglobalSmth [t] - AglobalSmth [t - dfSign];
        }
        //initialize slow filter
        if (HF) {
            for (int t=tx; dfSign*t<dfSign*tx+NFblocks*4*TauFilt1*2; t+=2*TauFilt1*dfSign) {
                AvgVoltageDefault (vm, t0, t);//-4+4*dfSign
            }
        } else {
            for (int t=tx; dfSign*t<dfSign*tx+(NFblocks+1)*4*TauFilt1; t+=2*TauFilt1*dfSign) {
                AvgVoltageDefault (vm, t0, t);
            }
        }
        for (int t=tx; dfSign*t<dfSign*ti; t+=df) {
            tA = t / dfAbs;
            for (int i=1-recalibTrigger; i<NChannels; i++) {//loop across channels
                vSmth [i] = vm [i] [t];
                for (int ii=1; ii<tSmth; ii++) {
                    vSmth [i] += vm [i] [t + ii * df];
                }
                vSmth [i] *= AscaleV;
                //CHANNEL OUT OF LINEAR REGIME
                if (((vm [i] [t + 2*df] + 4) % 4096) < 10) {
                    if (A [i] < artT) {//reset only when it starts leaving the linear regime
                        A [i] = artT;
                    }
                } else {
                    Qm [i] = (2 * (Qm [i] + Qd [i]) + vSmth [i] - AglobalSmth[tA]) / 3;//update Qm
                }
            }
        }
        //shift Aglobal entries
        for (int t=tx/dfAbs; dfSign*t<tm/df; t+=dfSign) {
            Aglobal [t + tfiA] = Aglobal [t];
            Aglobaldiff [t + tfiA] = Aglobaldiff [t];
        }
        for (int t=tx/dfAbs; dfSign*t<tms/df; t+=dfSign) {
            AglobalSmth [t + tfiA] = AglobalSmth [t];
            AglobalSdiff [t + tfiA] = AglobalSdiff [t];
        }
    }
    //shift Aglobal entries
    for (int t=tx/dfAbs; dfSign*t<tm/df; t+=dfSign) {
        Aglobal [t] = Aglobal [t + tfiA];
        Aglobaldiff [t] = Aglobaldiff [t + tfiA];
    }
    for (int t=tx/dfAbs; dfSign*t<tms/df; t+=dfSign) {
        AglobalSmth [t] = AglobalSmth [t + tfiA];
        AglobalSdiff [t] = AglobalSdiff [t + tfiA];
    }
    //new Aglobal entries
    for (int t=tm; dfSign*t<dfSign*ty; t+=df) {
        tA = t / dfAbs;
        for (int i=1; i<NChannels; i++) {//loop across channels
            Slice[i]=(vm [i][t])%4095+(vm[i][t+df])%4095;
        }
        std::sort(std::begin(Slice), std::end(Slice));
        Aglobal[tA]=Slice[NChannels / 2];
        Aglobaldiff [tA] = Aglobal [tA] - Aglobal [tA - dfSign];
        AglobalSmth [tA+tdSmth1] = Aglobal [tA+tdSmth1];
        for (int ii=1; ii<tSmth1; ii++) {
            AglobalSmth [tA+tdSmth1] += Aglobal [tA+tdSmth1 + ii*dfSign];
        }
        AglobalSmth [tA+tdSmth1] *= AscaleG;
        AglobalSdiff[tA+tdSmth1]=AglobalSmth[tA+tdSmth1]-AglobalSmth[t+tdSmth];
    }
    //avoid cumulation of numerical errors
    for (int i=1; i<NChannels; i++) {//loop across channels
        vSmth [i] = vm [i] [ti - df];
        for (int ii=0; ii<tSmth1; ii++) {
            vSmth [i] += vm [i] [ti + ii * df];
        }
        vSmth [i] *= AscaleV;
    }
    for (int t=ti; dfSign*t<dfSign*tf; t+=df) {//loop over data, will be removed for an online algorithm
        dt = (dt+1) % dtMx;
        tA = t / dfAbs;
        if ((t0+t)%(2*TauFilt1)==0) {
            //update Avgs3
            AvgVoltageDefault (vm, t0, t+TauFiltOffset);
        }
        for (int i=1; i<NChannels; i++) {//update vSmth
            vSmth [i] += (vm [i] [t+ tSmth1 * df]-vm [i] [t - df]) *AscaleV;
            QmPre[i]=vSmth[i]-AglobalSmth[tA];
        }
        // SPIKE DETECTION
        for (int i=1-recalibTrigger; i<NChannels; i++) {//loop across channels
            //CHANNEL OUT OF LINEAR REGIME
            if (((vm [i] [t + df] + 4) % 4096) < 10) {
                if (A [i] < artT) {//reset only when it starts leaving the linear regime
                    A [i] = artT;
                }
            }
            //DEFAULT OPERATIONS
            else if (A [i] == 0) {
                QmPreD [i] += (Avgs3[i] - QmPre [i]-QmPreD [i]) /(TauFilt);
                Vbias[i]=FVbias[i]*AglobalSdiff[tA]/Sampling;
                //vm[i][t-df]+vm[i][t]+vm[i][t+df]
                Qdiff [i][dt]= (QmPre [i]+QmPreD [i])-Qm[i]-Vbias[i];//difference between ADC counts and Qm
                if ((AglobalSdiff[tA] * (Qdiff [i][dt] - Qdiff [i][dtPre])) != 0) {
                    if ((AglobalSdiff[tA] > 0) == ((Qdiff [i][dt] - Qdiff [i][dtPre]) > 0)) {//(SIp[i]>0) {
                        FVbias [i]++;
                    } else {
                        FVbias [i]--;
                    }//Qdiff negative-->Ascale!!!;
                }
                //Qdiff [i][dt] = (vm [i] [t - df] + vm [i] [t] + vm [i] [t + df] - Aglobal[tA]) * Ascale - Qm [i];//difference between ADC counts and Qm
                //UPDATE Qm and Qd
                if (Qdiff [i][dt] > 0) {
                    if (Qdiff [i][dt] > Qd [i]) {
                        Qm [i] += Qd [i] / Tau_m0;
                        if (Qdiff [i][dt] < (5 * Qd [i])) {
                            Qd [i]++;
                        } else if ((Qd [i] > Qdmin) & (Qdiff [i][dt] > (6 * Qd [i]))) {
                            Qd [i]--;
                        }
                    } else if (Qd [i] > Qdmin) {//set a minimum level for Qd
                        Qd [i]--;
                    }
                } else if (Qdiff [i][dt] < -Qd [i]) {
                    Qm [i] -= Qd [i] / Tau_m0 / 2;

                }
            }
            //AFTER CHANNEL WAS OUT OF LINEAR REGIME
            else {
                Qm [i] = (2 * (Qm [i] + Qd [i]) + vSmth [i] - AglobalSmth[tA]) / 3;//update Qm
                A [i]--;
            }
        }
    }
    if (df > 0) {
        if (t0 >=199*(dfTI[2] - dfTI[1])) {
            for (int i=0; i<NChannels; i++) {
                Qm [i] = 0;
                for (int ij=0; ij<dtMx; ij++) {
                    Qdiff [i][ij] = 0;
                }
                A [i] = 0;
            }
        }
    } else {
        if (t0 <=t0x-199*(dfTI[2] - dfTI[1])) {
            for (int i=0; i<NChannels; i++) {
                Qm [i] = 0;
                for (int ij=0; ij<dtMx; ij++) {
                    Qdiff [i][ij] = 0;
                }
                A [i] = 0;
            }
        }
    }
}

void StartDetection(short** vm, long t0, long nFrames, double nSec, double sfd, int* Indices) {
    w.BaseStream.Seek(0, SeekOrigin.Begin);   // Set the file pointer to the start.
    wShapes.BaseStream.Seek(0, SeekOrigin.Begin);   // Set the file pointer to the start.
    wX.BaseStream.Seek(0, SeekOrigin.Begin);   // Set the file pointer to the start.
    wShapesX.BaseStream.Seek(0, SeekOrigin.Begin);   // Set the file pointer to the start.
    wInfo.BaseStream.Seek(0, SeekOrigin.Begin);   // Set the file pointer to the start.
    wMean.BaseStream.Seek(0, SeekOrigin.Begin);   // Set the file pointer to the start.
    //write some info
    
    wInfo << "# Number of frames:" << nFrames/dfAbs << "\n";
    
    // ...    
    wInfo.WriteLine("# Number of frames:\n{0}", nFrames/dfAbs);
    wInfo.WriteLine("# Duration (s):\n{0}", nSec);
    wInfo.WriteLine("# Sampling rate:\n{0}", sfd/dfAbs);
    wInfo.WriteLine("# Threshold scaling:\n{0}", AmpScale);
    wInfo.WriteLine("# Amplitude scaling:\n{0}", Ascale);
    wInfo.WriteLine("# Detection threshold*{0}:\n{1}", AmpScale, threshold);
    wInfo.WriteLine("# Repolarization threshold*{0}:\n{1}", AmpScale, AHPthr);
    wInfo.WriteLine("# Recalibration trigger:\n{0}", recalibTrigger);
    wInfo.WriteLine("# Cutouts:\n{0} {1} {2} {3} {4}", CutPre, CutPost, tCut, tCutLong, df);
    wInfo.WriteLine("# Smoothing window (detection):\n{0}", tSmth);
    wInfo.WriteLine("# Smoothing window (amplitudes):\n{0}", Lspike);
    wInfo.WriteLine ("# Recording channels:");
    for (int i=0; i<Indices.Length; i++) {
        wInfo.WriteLine ("{0}", Indices [i]);
    }
    wInfo.WriteLine ("# Recording channels4:");
    for (int i=0; i<NChannels; i++) {
        for (int j=0; j<4;j++){
            wInfo.Write("{0} ", ChInd4a[i][j]);
        }
        for (int j=0; j<8;j++){
            wInfo.Write("{0} ", ChInd4b[i][j]);
        }
        wInfo.WriteLine ();
    }
    wInfo.WriteLine ("# Recording channels5:");
    for (int i=0; i<NChannels; i++) {
        wInfo.Write("{0} ", ChInd5[i]);
        for (int j=0; j<4;j++){
            wInfo.Write("{0} ", ChInd5a[i][j]);
        }
        for (int j=0; j<4;j++){
            wInfo.Write("{0} ", ChInd5b[i][j]);
        }
        wInfo.WriteLine ();
    }
    wInfo.WriteLine("# Recalibration events:");
    int tA;
    //estimate Aglobal
    for (int t=tx; dfSign*t<dfSign*ty; t+=df) {//loop over data, will be removed for an online algorithm
        tA = t / dfAbs;
        for (int i=1; i<NChannels; i++) {//loop across channels
            Slice [i] = (vm [i] [t]) % 4095 + (vm [i] [t+df]) % 4095;
        }
        Array.Sort (Slice);
        Aglobal[tA] = Slice [NChannels / 2];
    }
    for (int t=tx/dfAbs+dfSign; dfSign*t<ty/df; t+=dfSign) {
        Aglobaldiff [t] = Aglobal [t] - Aglobal [t - dfSign];
    }
    for (int t=tx/dfAbs; dfSign*t<ty/df-dfSign*tSmth1; t+=dfSign) {
        //tA = t / dfAbs;
        AglobalSmth [t] = Aglobal [t];
        for (int ii=1; ii<tSmth1; ii++) {
            AglobalSmth [t] += Aglobal [t + ii*dfSign];
        }
        AglobalSmth [t] *= AscaleG;
    }
    for (int t=tx/dfAbs+dfSign; dfSign*t<ty/df-dfSign*tSmth1; t+=dfSign) {
        AglobalSdiff [t] = AglobalSmth [t] - AglobalSmth [t - dfSign];
    }
    //initialize slow filter
    if (HF) {
        for (int t=tx; dfSign*t<dfSign*tx+NFblocks*4*TauFilt1*2; t+=2*TauFilt1*dfSign) {
            AvgVoltageDefault (vm, t0, t);//-4+4*dfSign
        }
    } else {
        for (int t=tx; dfSign*t<dfSign*tx+(NFblocks+1)*4*TauFilt1; t+=2*TauFilt1*dfSign) {
            AvgVoltageDefault (vm, t0, t);
        }
    }
    for (int t=tx; dfSign*t<dfSign*ti; t+=df) {
        tA = t / dfAbs;
        //Console.Write("{0} ", t+t0);
        dtPre = dt;
        dt = (dt+1) % dtMx;
        dtEx = dtE;
        dtE = (dtE + 1) % dtEMx;
        if ((t0 + t) % (2 * TauFilt1) == 0) {
            AvgVoltageDefault (vm, t0, t + TauFiltOffset);
        }
        //now have to change voltage
        //can use Qm?
        for (int i=1; i<NChannels; i++) {//loop across channels
            vSmth [i] = vm [i] [t];
            for (int ii=1; ii<tSmth; ii++) {
                vSmth [i] += vm [i] [t + ii * df];
            }
            vSmth [i] *= AscaleV;
            QmPre[i]=vSmth[i]-AglobalSmth[tA];
            //QmPre[i]=(vm[i][t]+vm [i][t+df]+vm[i][t+2*df]-Aglobal[tA])*Ascale;
        //	//QmPreD [i] += (Avgs3[i] - QmPre [i]-QmPreD [i]) /(TauFilt);
        //	Slice [i] = QmPre [i]+QmPreD [i];//((vm[i][t])%4095+(vm[i][t+df])%4095+(vm[i][t+2*df])%4095);
        }

        //Array.Sort(Slice);
        //Aglobaldiff=Slice[NChannels / 2]-Aglobal;
        //Aglobal=Slice[NChannels / 2];
        wMean.WriteLine("{0}", Aglobal[tA]);
        for (int i=1-recalibTrigger; i<NChannels; i++) {//loop across channels
            //CHANNEL OUT OF LINEAR REGIME
            if (((vm[i][t+2*df]+4)%4096)<10) {
                if (A[i]<artT) {//reset only when it starts leaving the linear regime
                    A[i]=artT;
                    QmPreD[i] = 0;
                }
            }
            else {
                QmPreD[i]  += (Avgs3[i] - QmPre [i]-QmPreD [i]) /(TauFilt);
                Qm[i]=(2*(Qm[i]+Qd[i])+QmPre [i]+QmPreD [i])/3;//update Qm vm[i][t]+vm[i][t+df]+vm[i][t+2*df]
            }
        }
    }
    //shift Aglobal entries
    for (int t=tx/dfAbs; dfSign*t<tm/df; t+=dfSign) {
        Aglobal [t + tfiA] = Aglobal [t];
        Aglobaldiff [t + tfiA] = Aglobaldiff [t];
    }
    for (int t=tx/dfAbs; dfSign*t<tms/df; t+=dfSign) {
        AglobalSmth [t + tfiA] = AglobalSmth [t];
        AglobalSdiff [t + tfiA] = AglobalSdiff [t];
    }
}
void skipLastReverse(int skipLast);
void Iterate(short** vm, long t0);
void FinishDetection(short** vm, int skipLast);

}

int main(int argc,  char** argv) {
    
}