#include <iostream>
#include <boost/shared_ptr.hpp>

#include "EGamma/EGammaAnalysisTools/interface/EGammaCutBasedEleId.h"

#include "CMGTools/HtoZZ2l2nu/interface/ZZ2l2nuSummaryHandler.h"
#include "CMGTools/HtoZZ2l2nu/interface/ZZ2l2nuPhysicsEvent.h"
#include "CMGTools/HtoZZ2l2nu/interface/METUtils.h"
#include "CMGTools/HtoZZ2l2nu/interface/GammaEventHandler.h"
#include "CMGTools/HtoZZ2l2nu/interface/setStyle.h"
#include "CMGTools/HtoZZ2l2nu/interface/plotter.h"
#include "CMGTools/HtoZZ2l2nu/interface/ObjectFilters.h"
#include "CMGTools/HtoZZ2l2nu/interface/SmartSelectionMonitor.h"
#include "CMGTools/HtoZZ2l2nu/interface/TMVAUtils.h"
#include "CMGTools/HtoZZ2l2nu/interface/MacroUtils.h"
#include "CMGTools/HtoZZ2l2nu/interface/EventCategory.h"
#include "CMGTools/HtoZZ2l2nu/interface/EfficiencyMap.h"

#include "CondFormats/JetMETObjects/interface/JetResolution.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "PhysicsTools/Utilities/interface/LumiReWeighting.h"

#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TEventList.h"
#include "TROOT.h"
 
using namespace std;

TString getJetRegion(float eta)
{
  TString reg("TK");
  if(fabs(eta)>2.5)  reg="HEin";
  if(fabs(eta)>2.75) reg="HEout";
  if(fabs(eta)>3)    reg="HF";
  return reg;
}


int main(int argc, char* argv[])
{
  //##############################################
  //########    GLOBAL INITIALIZATION     ########
  //##############################################

  // check arguments
  if(argc<2){ std::cout << "Usage : " << argv[0] << " parameters_cfg.py" << std::endl; exit(0); }
  
  // load framework libraries
  gSystem->Load( "libFWCoreFWLite" );
  AutoLibraryLoader::enable();
  
  // configure the process
  const edm::ParameterSet &runProcess = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("runProcess");

  bool use2011Id = runProcess.getParameter<bool>("is2011");
  cout << "Note: will apply " << (use2011Id ? 2011 : 2012) << " version of the id's" << endl;

  bool isMC = runProcess.getParameter<bool>("isMC");
  bool runBlinded = runProcess.getParameter<bool>("runBlinded"); 
  int mctruthmode=runProcess.getParameter<int>("mctruthmode");

  TString url=runProcess.getParameter<std::string>("input");
  TString outdir=runProcess.getParameter<std::string>("outdir");
  TString outUrl( outdir );
  gSystem->Exec("mkdir -p " + outUrl);
  int fType(0);
  if(url.Contains("DoubleEle")) fType=EE;
  if(url.Contains("DoubleMu"))  fType=MUMU;
  if(url.Contains("MuEG"))      fType=EMU;
  if(url.Contains("SingleMu"))  fType=MUMU;
  bool isSingleMuPD(!isMC && url.Contains("SingleMu"));  
  
  TString outTxtUrl= outUrl + "/" + gSystem->BaseName(url) + ".txt";
  FILE* outTxtFile = NULL;
  if(!isMC)outTxtFile = fopen(outTxtUrl.Data(), "w");
  printf("TextFile URL = %s\n",outTxtUrl.Data());

  //tree info
  int evStart=runProcess.getParameter<int>("evStart");
  int evEnd=runProcess.getParameter<int>("evEnd");
  TString dirname = runProcess.getParameter<std::string>("dirName");

  //jet energy scale uncertainties
  TString uncFile =  runProcess.getParameter<std::string>("jesUncFileName"); gSystem->ExpandPathName(uncFile);
  JetCorrectionUncertainty jecUnc(uncFile.Data());

  //systematics
  bool runSystematics                        = runProcess.getParameter<bool>("runSystematics");
  TString varNames[]={"",
		      "_jerup","_jerdown",
		      "_jesup","_jesdown",
		      "_umetup","_umetdown",
		      "_lesup","_lesdown",
		      "_puup","_pudown",
		      "_btagup","_btagdown"};
  size_t nvarsToInclude(1);
  if(runSystematics)
    {
      cout << "Systematics will be computed for this analysis" << endl;
      nvarsToInclude=sizeof(varNames)/sizeof(TString);
    }




  //##############################################
  //########    INITIATING HISTOGRAMS     ########
  //##############################################
  SmartSelectionMonitor mon;

  TH1F* Hcutflow  = (TH1F*) mon.addHistogram(  new TH1F ("cutflow"    , "cutflow"    ,6,0,6) ) ;
  TH1 *h=mon.addHistogram( new TH1F ("eventflow", ";;Events", 10,0,10) );
  h->GetXaxis()->SetBinLabel(1,"#geq 2 leptons");
  h->GetXaxis()->SetBinLabel(2,"|M-M_{Z}|<7");
  h->GetXaxis()->SetBinLabel(3,"p_{T}^{ll}>30");
  h->GetXaxis()->SetBinLabel(4,"b-veto"); 
  h->GetXaxis()->SetBinLabel(5,"Jet veto"); 
  h->GetXaxis()->SetBinLabel(6,"red-E_{T}^{miss}>70");
  h->GetXaxis()->SetBinLabel(7,"0.4<B<1.8");
  h->GetXaxis()->SetBinLabel(8,"#Delta #phi(jet,E_{T}^{miss})>0.5");
  h->GetXaxis()->SetBinLabel(9,"3^{rd}-lepton veto");

  //pileup control
  mon.addHistogram( new TH1F( "nvtx",";Vertices;Events",50,0,50) ); 
  mon.addHistogram( new TH1F( "nvtxraw",";Vertices;Events",50,0,50) ); 
  mon.addHistogram( new TH1F( "rho",";#rho;Events",50,0,25) ); 
  mon.addHistogram( new TH1F( "rho25",";#rho(#eta<2.5);Events",50,0,25) ); 

  //lepton control
  Double_t effptAxis[]={0,10,20,30,40,50,75,100,150,200,500};
  const size_t nEffptAxis=sizeof(effptAxis)/sizeof(Double_t)-1;
  for(size_t ilep=0; ilep<2; ilep++)
    {
      TString lepStr(ilep==0? "mu" :"e");
      mon.addHistogram(new TH1F(lepStr+"genpt",   ";p_{T} [GeV/c];Leptons",nEffptAxis,effptAxis) );
      mon.addHistogram(new TH1F(lepStr+"geneta",   ";#eta;Leptons",50,-5,5) );
      mon.addHistogram(new TH1F(lepStr+"genpu",   ";Pileup;Leptons",50,0,50) );
      for(int iid=0; iid<4; iid++)
	{
	  TString idctr(""); idctr+=iid;
	  mon.addHistogram(new TH1F(lepStr+idctr+"pt",   ";p_{T} [GeV/c];Leptons",nEffptAxis,effptAxis) );
	  mon.addHistogram(new TH1F(lepStr+idctr+"eta",   ";#eta;Leptons",50,-5,5) );
	  mon.addHistogram(new TH1F(lepStr+idctr+"pu",   ";Pileup;Leptons",50,0,50) );
	  mon.addHistogram(new TH1F(lepStr+idctr+"isopt",   ";p_{T} [GeV/c];Leptons",nEffptAxis,effptAxis) );
	  mon.addHistogram(new TH1F(lepStr+idctr+"isoeta",   ";#eta;Leptons",50,-5,5) );
	  mon.addHistogram(new TH1F(lepStr+idctr+"isopu",   ";Pileup;Leptons",50,0,50) );
	}
      if(ilep==1)
	{
	  for(size_t ireg=0; ireg<2; ireg++)
	    { 
	      TString reg(ireg==0?"eb":"ee");
	      mon.addHistogram(new TH1F(lepStr+reg+"detain",   ";#Delta#eta_{in};Leptons",50,0,0.01) );
	      mon.addHistogram(new TH1F(lepStr+reg+"dphiin",   ";#Delta#phi_{in};Leptons",50,0,0.1) );
	      mon.addHistogram(new TH1F(lepStr+reg+"sihih",    ";#sigma_{i#eta i#eta};Leptons",50,0,0.05) );
	      mon.addHistogram(new TH1F(lepStr+reg+"sipip",    ";#sigma_{i#phi i#phi};Leptons",50,0,0.05) );
	      mon.addHistogram(new TH1F(lepStr+reg+"r9",       ";R_{9};Leptons",50,0,1.) );
	      mon.addHistogram(new TH1F(lepStr+reg+"hoe",      ";h/e;Leptons",50,0,0.2) );
	      mon.addHistogram(new TH1F(lepStr+reg+"ooemoop",  ";1/E-1/p;Leptons",100,0,0.05) );
	      mon.addHistogram(new TH1F(lepStr+reg+"eopin",    ";E/p;Leptons",100,0,2) );
	      mon.addHistogram(new TH1F(lepStr+reg+"fbrem",    ";f_{brem};Leptons",100,0,2) );
	    }
	}
      else
	{
	  mon.addHistogram(new TH1F(lepStr+"nmatches", ";Muon matches;Leptons",15,0,15) );
	  mon.addHistogram(new TH1F(lepStr+"nmuonhits", ";Muon hits;Leptons",30,0,30) );
	}
      mon.addHistogram(new TH1F(lepStr+"d0",            ";d_{0};Leptons",50,0,0.05) );
      mon.addHistogram(new TH1F(lepStr+"dZ",            ";d_{Z};Leptons",50,0,0.1) );
      mon.addHistogram(new TH1F(lepStr+"trkchi2",       ";#chi^{2};Leptons",50,0,10) );
      mon.addHistogram(new TH1F(lepStr+"trkvalidpixel",  ";Valid pixel hits;Leptons",20,0,20) );
      mon.addHistogram(new TH1F(lepStr+"trkvalidtracker",  ";Valid tracker hits;Leptons",50,0,50) );
      mon.addHistogram(new TH1F(lepStr+"losthits",         ";Lost hits;Leptons",4,0,4) );
      mon.addHistogram(new TH1F(lepStr+"reliso",           ";RelIso;Leptons",50,0,2) );
      mon.addHistogram(new TH1F(lepStr+"reliso2011",        ";RelIso(#rho);Leptons",50,0,2) );
      mon.addHistogram(new TH1F(lepStr+"truereliso",           ";RelIso;Leptons",50,0,2) );
      mon.addHistogram(new TH1F(lepStr+"truereliso2011",        ";RelIso(#rho);Leptons",50,0,2) );
      mon.addHistogram(new TH1F(lepStr+"fakereliso",           ";RelIso;Fake leptons",50,0,2) );
      mon.addHistogram(new TH1F(lepStr+"fakereliso2011",        ";RelIso(#rho);Fake leptons",50,0,2) );
    }
  mon.addHistogram( new TH1F( "leadpt", ";p_{T}^{l};Events", 50,0,500) );
  mon.addHistogram( new TH1F( "leadeta", ";#eta^{l};Events", 50,-2.6,2.6) );
  mon.addHistogram( new TH1F( "trailerpt", ";p_{T}^{l};Events", 50,0,500) );
  mon.addHistogram( new TH1F( "trailereta", ";#eta^{l};Events", 50,-2.6,2.6) );
  mon.addHistogram( new TH1F( "deltaleptonpt", ";|p_{T}^{1}-p_{T}^{2}|;Events", 50,0,250) );
  mon.addHistogram( new TH1F( "deltazpt", ";p_{T}^{ll}-E_{T}^{miss};Events", 50,-250,250) );
  mon.addHistogram( new TH1F( "zpt", ";p_{T}^{ll};Events", 50,0,500) );
  mon.addHistogram( new TH1F( "zeta", ";#eta^{ll};Events", 50,-10,10) );
  mon.addHistogram( new TH1F( "zmass", ";M^{ll};Events", 100,40,250) );
  mon.addHistogram( new TH1F( "mindphilmet", ";min(#Delta#phi(lepton,E_{T}^{miss});Events",40,0,4) );
  mon.addHistogram( new TH1F( "maxdphilmet", ";max(#Delta#phi(lepton,E_{T}^{miss});Events",40,0,4) );
  mon.addHistogram( new TH1D( "balance", ";E_{T}^{miss}/q_{T};Events", 25,0,2.5) );

  //3rd lepton control 
  mon.addHistogram( new TH1F( "thirdleptonpt", ";p_{T}^{l};Events", 50,0,500) );
  mon.addHistogram( new TH1F( "thirdleptoneta", ";#eta^{l};Events", 50,-2.6,2.6) );
  mon.addHistogram( new TH1F( "nleptons", ";Leptons;Events", 3,2,4) );
  mon.addHistogram( new TH1F( "mt3" , ";M_{T}^{3rd lepton} [GeV/c^{2}];Events",50,0,200) );


  //jet control
  mon.addHistogram( new TH1F("jetpt"       , ";p_{T} [GeV/c];Events",50,0,250) );
  mon.addHistogram( new TH1F("jeteta"       , ";|#eta|;Events",25,0,5) );
  mon.addHistogram( new TH1F("btags", ";CSV;Events",50,-0.5,1.2) );
  h=mon.addHistogram( new TH1F("njets",  ";Jet multiplicity (p_{T}>30 GeV/c);Events",5,0,5) );
  TH1 *hb=mon.addHistogram( new TH1F("nbtags",   ";b-tag multiplicity (CSV);Events",5,0,5) );
  for(int ibin=1; ibin<=h->GetXaxis()->GetNbins(); ibin++)
    {
      TString label("");
      if(ibin==h->GetXaxis()->GetNbins()) label +="#geq";
      else                                label +="=";
      label += (ibin-1);
      h->GetXaxis()->SetBinLabel(ibin,label);
      hb->GetXaxis()->SetBinLabel(ibin,label);
    } 
  mon.addHistogram( new TH1F( "mindphijmet", ";min #Delta#phi(jet,E_{T}^{miss});Events",40,0,4) );
  
  //MET control
  mon.addHistogram( new TH1F( "met_redMetSB"  , ";E_{T}^{miss};Events", 50,0,500) );
  mon.addHistogram( new TH1F( "met_redMet"  , ";red(E_{T}^{miss},clustered-E_{T}^{miss});Events", 50,0,500) );
  mon.addHistogram( new TH1F( "met_redMetL"  , ";red(E_{T}^{miss},clustered-E_{T}^{miss}) - longi.;Events", 50,-250,250) );
  mon.addHistogram( new TH1F( "met_redMetT"  , ";red(E_{T}^{miss},clustered-E_{T}^{miss}) - perp.;Events", 50,-250,250) );
  mon.addHistogram( new TH1F( "mt"  , ";M_{T};Events", 100,0,1000) );

  //FIXME
  //optimization
  //   std::vector<double> optim_Cuts1_met; 
  //   std::vector<double> optim_Cuts1_zpt;
  //   std::vector<double> optim_Cuts1_zmass;
  //   for(double met=50;met<100;met+=2.5){
  //     for(double pt=30;pt<60;pt+=2.5){
  //       for(double zm=5;zm<20;zm+=2.5){
  // 	optim_Cuts1_met    .push_back(met);
  // 	optim_Cuts1_zpt    .push_back(pt);
  // 	optim_Cuts1_zmass  .push_back(zm);
  //       }
  //     }
  //   }
  
  //    TH1F* Hoptim_cuts1_met    =  (TH1F*) mon.addHistogram( new TH1F ("optim_cut1_met"    , ";cut index;met"    ,optim_Cuts1_met.size(),0,optim_Cuts1_met.size()) ) ;
  //    TH1F* Hoptim_cuts1_zpt    =  (TH1F*) mon.addHistogram( new TH1F ("optim_cut1_zpt"  , ";cut index;zpt"  ,optim_Cuts1_met.size(),0,optim_Cuts1_met.size()) ) ;
  //    TH1F* Hoptim_cuts1_zmass  =  (TH1F*) mon.addHistogram( new TH1F ("optim_cut1_zm"  , ";cut index;zmass"  ,optim_Cuts1_met.size(),0,optim_Cuts1_met.size()) ) ;
  //    for(unsigned int index=0;index<optim_Cuts1_met.size();index++){
  //       Hoptim_cuts1_met    ->Fill(index, optim_Cuts1_met[index]);    
  //       Hoptim_cuts1_mtmin  ->Fill(index, optim_Cuts1_mtmin[index]);
  //       Hoptim_cuts1_mtmax  ->Fill(index, optim_Cuts1_mtmax[index]);
  //    }
  
  TH1F* Hoptim_systs     =  (TH1F*) mon.addHistogram( new TH1F ("optim_systs"    , ";syst;", nvarsToInclude,0,nvarsToInclude) ) ;
  for(size_t ivar=0; ivar<nvarsToInclude; ivar++)
    {
      Hoptim_systs->GetXaxis()->SetBinLabel(ivar+1, varNames[ivar]);
      //FIXME
      //      mon.addHistogram( new TH2F (TString("mt_shapes")+varNames[ivar],";cut index;M_{T} [GeV/c^{2}];#events (/10GeV)",optim_Cuts1_met.size(),0,optim_Cuts1_met.size(), 160,150,950) );
      //      TH2F *h=(TH2F *) mon.addHistogram( new TH2F ("nonresbckg_ctrl"+varNames[ivar],";cut index;Selection region;Events",optim_Cuts1_met.size(),0,optim_Cuts1_met.size(),6,0,6) );
      //      h->GetYaxis()->SetBinLabel(1,"M_{in}^{ll}/=0 b-tags");
      //      h->GetYaxis()->SetBinLabel(2,"M_{out}^{ll}/=0 b-tags");
      //      h->GetYaxis()->SetBinLabel(3,"M_{out+}^{ll}/=0 b-tags");
      //      h->GetYaxis()->SetBinLabel(4,"M_{in}^{ll}/#geq 1 b-tag");
      //      h->GetYaxis()->SetBinLabel(5,"M_{out}^{ll}/#geq 1 b-tag");
      //     h->GetYaxis()->SetBinLabel(6,"M_{out+}^{ll}/#geq 1 b-tag");
    } 
  
  
  //##############################################
  //######## GET READY FOR THE EVENT LOOP ########
  //##############################################

  //open the file and get events tree
  ZZ2l2nuSummaryHandler evSummaryHandler;
  TFile *file = TFile::Open(url);
  printf("Looping on %s\n",url.Data());
  if(file==0) return -1;
  if(file->IsZombie()) return -1;
  if( !evSummaryHandler.attachToTree( (TTree *)file->Get(dirname) ) ){
      file->Close();
      return -1;
  }


  //check run range to compute scale factor (if not all entries are used)
  const Int_t totalEntries= evSummaryHandler.getEntries();
  float rescaleFactor( evEnd>0 ?  float(totalEntries)/float(evEnd-evStart) : -1 );
  if(evEnd<0 || evEnd>evSummaryHandler.getEntries() ) evEnd=totalEntries;
  if(evStart > evEnd ){
      file->Close();
      return -1;
  }

  //MC normalization (to 1/pb)
  float cnorm=1.0;
  if(isMC){
      TH1F* cutflowH = (TH1F *) file->Get("evAnalyzer/h2zz/cutflow");
      if(cutflowH) cnorm=cutflowH->GetBinContent(1);
      if(rescaleFactor>0) cnorm /= rescaleFactor;
      printf("cnorm = %f\n",cnorm);
  }
  Hcutflow->SetBinContent(1,cnorm);


  //pileup weighting: based on vtx for now...
  std::vector<double> dataPileupDistributionDouble = runProcess.getParameter< std::vector<double> >("datapileup");
  std::vector<float> dataPileupDistribution; for(unsigned int i=0;i<dataPileupDistributionDouble.size();i++){dataPileupDistribution.push_back(dataPileupDistributionDouble[i]);}
  std::vector<float> mcPileupDistribution;
  bool useObservedPU(true);
  if(isMC){
    TString puDist("evAnalyzer/h2zz/pileuptrue");
    if(useObservedPU) puDist="evAnalyzer/h2zz/pileup";
    TH1F* histo = (TH1F *) file->Get(puDist);
    if(!histo)std::cout<<"pileup histogram is null!!!\n";
    for(int i=1;i<=histo->GetNbinsX();i++){mcPileupDistribution.push_back(histo->GetBinContent(i));}
    delete histo;
    if(dataPileupDistribution.size()==0) dataPileupDistribution=mcPileupDistribution;
  }
  while(mcPileupDistribution.size()<dataPileupDistribution.size())  mcPileupDistribution.push_back(0.0);
  while(mcPileupDistribution.size()>dataPileupDistribution.size())dataPileupDistribution.push_back(0.0);

  gROOT->cd();  //THIS LINE IS NEEDED TO MAKE SURE THAT HISTOGRAM INTERNALLY PRODUCED IN LumiReWeighting ARE NOT DESTROYED WHEN CLOSING THE FILE
  edm::LumiReWeighting *LumiWeights=0;
  PuShifter_t PuShifters;
  //  reweight::PoissonMeanShifter *PShiftUp=0, *PShiftDown=0;
  if(isMC){
      LumiWeights= new edm::LumiReWeighting(mcPileupDistribution,dataPileupDistribution);
      PuShifters=getPUshifters(dataPileupDistribution,0.05);
  }

  //event Categorizer
  EventCategory eventCategoryInst(0); //inclusive analysis
  //EventCategory eventCategoryInst(1); //jet binning
  //EventCategory eventCategoryInst(2); //vbf binning
  //EventCategory eventCategoryInst(3); //jet+vbf binning

  //##############################################
  //########           EVENT LOOP         ########
  //##############################################
  //loop on all the events
  printf("Progressing Bar     :0%%       20%%       40%%       60%%       80%%       100%%\n");
  printf("Scanning the ntuple :");
  int treeStep = (evEnd-evStart)/50;if(treeStep==0)treeStep=1;
  DuplicatesChecker duplicatesChecker;
  int nDuplicates(0);
  for( int iev=evStart; iev<evEnd; iev++){
      if((iev-evStart)%treeStep==0){printf(".");fflush(stdout);}

      //##############################################   EVENT LOOP STARTS   ##############################################
      //load the event content from tree
      evSummaryHandler.getEntry(iev);
      ZZ2l2nuSummary_t &ev=evSummaryHandler.getEvent();
      if(!isMC && duplicatesChecker.isDuplicate( ev.run, ev.lumi, ev.event) ) { nDuplicates++; continue; }
      PhysicsEvent_t phys=getPhysicsEventFrom(ev);
      
      //event category
      bool isSameFlavor(ev.cat==MUMU || ev.cat==EE);
      TString tag_cat;
      switch(ev.cat){
         case MUMU : tag_cat = "mumu";  break;
         case EE   : tag_cat = "ee";    break;
         case EMU  : tag_cat = "emu";   break;
         default   : continue;
      }
      //      if(isMC && mctruthmode==1 && !isDYToLL(ev.mccat) && !isZZ2l2nu(ev.mccat) ) continue;
      if(isMC && mctruthmode==1 && !isDYToLL(ev.mccat) ) continue;
      if(isMC && mctruthmode==2 && !isDYToTauTau(ev.mccat) ) continue;

      //require compatibilitiy of the event with the PD
      bool hasEEtrigger = ev.triggerType & 0x1;
      bool hasMMtrigger = (ev.triggerType >> 1 ) & 0x1;
      bool hasEMtrigger = (ev.triggerType >> 2 ) & 0x1;
      bool hasMtrigger  = (ev.triggerType >> 3 ) & 0x1;
      if(!isMC){
	  if(ev.cat!=fType) continue;

	  if(ev.cat==EE   && !hasEEtrigger) continue;
	  if(ev.cat==MUMU && !(hasMMtrigger||hasMtrigger) ) continue;
	  if(ev.cat==EMU  && !hasEMtrigger) continue;

	  //this is a safety veto for the single mu PD
	  if(isSingleMuPD) {
	    if(!hasMtrigger) continue;
	    if(hasMtrigger && hasMMtrigger) continue;
	  }
      }
      
      //prepare the tag's vectors for histo filling
      std::vector<TString> tags_full(1,"all");
      
      //pileup weight
      float weight = 1.0;
      float signalWeight=1.0;
      double TotalWeight_plus = 1.0;
      double TotalWeight_minus = 1.0;
      if(isMC){
        weight            = LumiWeights->weight(useObservedPU ? ev.ngenITpu : ev.ngenTruepu);
        TotalWeight_plus  = PuShifters[PUUP]->Eval(useObservedPU ? ev.ngenITpu : ev.ngenTruepu);
        TotalWeight_minus = PuShifters[PUDOWN]->Eval(useObservedPU ? ev.ngenITpu : ev.ngenTruepu);
      }
      Hcutflow->Fill(1,1);
      Hcutflow->Fill(2,weight);
      Hcutflow->Fill(3,weight*TotalWeight_minus);
      Hcutflow->Fill(4,weight*TotalWeight_plus);
      Hcutflow->Fill(5,signalWeight);

      //apply JER base corrections to jets (and compute associated variations)
      std::vector<PhysicsObjectJetCollection> variedAJets;
      LorentzVectorCollection zvvs;
      METUtils::computeVariation(phys.ajets, phys.leptons, phys.met[0], variedAJets, zvvs, &jecUnc);

      //
      //
      // BELOW FOLLOWS THE ANALYSIS OF THE MAIN SELECTION WITH N-1 PLOTS
      //
      //
      
      //
      // LEPTON ANALYSIS
      //
      LorentzVector lep1=phys.leptons[0];
      LorentzVector lep2=phys.leptons[1];
      LorentzVector zll(lep1+lep2);
      bool passIdAndIso(true);
      bool passZmass(fabs(zll.mass()-91)<7);
      bool passZsideBand( (zll.mass()>40 && zll.mass()<70) || (zll.mass()>110 && zll.mass()<200));
      bool passZpt(zll.pt()>30);
      
      //check alternative selections for the dilepton
      double llScaleFactor(1.0),llTriggerEfficiency(1.0);
      for(int ilep=0; ilep<2; ilep++)
	{
	  TString lepStr( fabs(phys.leptons[ilep].id)==13 ? "mu" : "e");
	  
	  //generator level matching
	  int matchid(0);
	  LorentzVector genP4(0,0,0,0);
	  for(size_t igl=0;igl<phys.genleptons.size(); igl++) 
	    {
	      if(deltaR(phys.genleptons[igl],phys.leptons[ilep])>0.1) continue;
	      genP4=phys.genleptons[igl];
	      matchid=phys.genleptons[igl].id;
	    }
	  
	  //id and isolation
	  int lpid=phys.leptons[ilep].pid;
	  float relIso2011    = phys.leptons[ilep].relIsoRho(ev.rho);
	  float relIso = (lepStr=="mu") ? 
	    phys.leptons[ilep].pfRelIsoDbeta(): //muPFRelIsoCorrected2012(ev.rho25Neut):
	    phys.leptons[ilep].ePFRelIsoCorrected2012(ev.rho);
	  std::vector<int> passIds;
	  std::map<int,bool> passIsos;
	  bool hasGoodId(false), isIso(false);
	  if(fabs(phys.leptons[ilep].id)==13)
	    {
	      if( hasObjectId(ev.mn_idbits[lpid], MID_LOOSE) )    { passIds.push_back(0); passIsos[0]=(relIso<0.2); if(!use2011Id) { hasGoodId=true; isIso=passIsos[0]; } }
	      if( hasObjectId(ev.mn_idbits[lpid], MID_TIGHT) )    { passIds.push_back(1); passIsos[1]=(relIso<0.2); }
	      if( hasObjectId(ev.mn_idbits[lpid], MID_VBTF2011) ) { passIds.push_back(2); passIsos[2]=(relIso2011<0.15); if(use2011Id) {hasGoodId=true; isIso=passIsos[2];} }
	      if( hasObjectId(ev.mn_idbits[lpid], MID_SOFT) )     { passIds.push_back(3); passIsos[3]=true;}
	      if(use2011Id) 
		{
		  try{
		    llScaleFactor *= muonScaleFactor(phys.leptons[ilep].pt(),fabs(phys.leptons[ilep].eta()));
		    llTriggerEfficiency *= muonTriggerEfficiency(phys.leptons[ilep].pt(),fabs(phys.leptons[ilep].eta()));
		  }catch(std::string &e){
		  }
		}
	      else
		{
		  llScaleFactor *= 1;
		  llTriggerEfficiency *= muonTriggerEfficiency(phys.leptons[ilep].pt(),fabs(phys.leptons[ilep].eta()),2012);
		}
	    }
	  else
	    {
	      int wps[]={EgammaCutBasedEleId::LOOSE,EgammaCutBasedEleId::MEDIUM, EID_VBTF2011, EgammaCutBasedEleId::VETO};
	      for(int iwp=0; iwp<4; iwp++)
		{
		  if(iwp==2 && hasObjectId(ev.en_idbits[lpid], EID_VBTF2011)) 
		    { 
		      passIds.push_back(2); passIsos[2]=(relIso2011<0.10); 
		      if(use2011Id) 
			{ 
			  hasGoodId=true; isIso=passIsos[2]; 
			  try{
			    llScaleFactor *= electronScaleFactor(phys.leptons[ilep].pt(),fabs(phys.leptons[ilep].eta()));
			    llTriggerEfficiency *= electronTriggerEfficiency(phys.leptons[ilep].pt(),fabs(phys.leptons[ilep].eta()));
			  } catch(std::string &e){
			  }
			}
		    }
		  else
		    {
		      bool passWp = EgammaCutBasedEleId::PassWP(EgammaCutBasedEleId::WorkingPoint(wps[iwp]),
								(fabs(phys.leptons[ilep].eta())<1.4442),
								phys.leptons[ilep].pt(), phys.leptons[ilep].eta(),
								ev.en_detain[lpid],  ev.en_dphiin[lpid], ev.en_sihih[lpid], ev.en_hoe[lpid],
								ev.en_ooemoop[lpid], phys.leptons[ilep].d0, phys.leptons[ilep].dZ,
								0., 0., 0.,
								!hasObjectId(ev.en_idbits[lpid], EID_CONVERSIONVETO),0,ev.rho);
		      if(passWp) { 
			passIds.push_back(iwp); 
			passIsos[iwp]=(relIso<0.15);
			if(wps[iwp]==EgammaCutBasedEleId::MEDIUM && !use2011Id){  hasGoodId=true; isIso=passIsos[iwp]; }
		      }
		      if(!use2011Id)
			{
			  llScaleFactor *= 1;
			  llTriggerEfficiency *= electronTriggerEfficiency(phys.leptons[ilep].pt(),fabs(phys.leptons[ilep].eta()),2012);
			}
		    }
		}
	    }
	  if(!hasGoodId)  passIdAndIso=false;
	  else if(!isIso) passIdAndIso=false;     
	
	  //fill control histograms (constrained to the Z mass)
	  if(passZmass && isSameFlavor){
	      if(matchid!=0){
		  mon.fillHisto(lepStr+"genpt",tags_full, genP4.pt(), weight,true);
		  mon.fillHisto(lepStr+"geneta",tags_full, genP4.eta(), weight);
		  mon.fillHisto(lepStr+"genpu",tags_full,ev.ngenITpu, weight);
		  for(size_t iid=0; iid<passIds.size(); iid++){
		      TString idStr(lepStr);  idStr += passIds[iid];
		      mon.fillHisto(idStr+"pt",tags_full, genP4.pt(), weight,true);
		      mon.fillHisto(idStr+"eta",tags_full, genP4.eta(), weight);
		      mon.fillHisto(idStr+"pu",tags_full,ev.ngenITpu, weight);
		      if(!passIsos[ passIds[iid] ]) continue;
		      mon.fillHisto(idStr+"isopt",tags_full,  genP4.pt(), weight,true);
		      mon.fillHisto(idStr+"isoeta",tags_full, genP4.eta(), weight);
		      mon.fillHisto(idStr+"isopu",tags_full,ev.ngenITpu, weight);
		    }
	      }
	  
	      mon.fillHisto(lepStr+"d0",              tags_full,fabs(phys.leptons[ilep].d0),weight);
	      mon.fillHisto(lepStr+"dZ",              tags_full,fabs(phys.leptons[ilep].dZ),weight);
	      mon.fillHisto(lepStr+"trkchi2",         tags_full,fabs(phys.leptons[ilep].trkchi2),weight);
	      mon.fillHisto(lepStr+"trkvalidpixel",   tags_full,fabs(phys.leptons[ilep].trkValidPixelHits),weight);
	      mon.fillHisto(lepStr+"trkvalidtracker", tags_full,fabs(phys.leptons[ilep].trkValidTrackerHits),weight);
	      mon.fillHisto(lepStr+"losthits",        tags_full,fabs(phys.leptons[ilep].trkLostInnerHits),weight);

	      if(lepStr=="e"){
		  TString reg="ee";
		  if(fabs(phys.leptons[ilep].eta())<1.442) reg="eb";
		  mon.fillHisto(lepStr+reg+"detain",  tags_full,fabs(ev.en_detain[lpid]),weight);
		  mon.fillHisto(lepStr+reg+"dphiin",  tags_full,fabs(ev.en_dphiin[lpid]),weight);
		  mon.fillHisto(lepStr+reg+"sihih",   tags_full,fabs(ev.en_sihih[lpid]),weight);
		  mon.fillHisto(lepStr+reg+"sipip",   tags_full,fabs(ev.en_sipip[lpid]),weight);
		  mon.fillHisto(lepStr+reg+"r9",      tags_full,fabs(ev.en_r9[lpid]),weight);
		  mon.fillHisto(lepStr+reg+"hoe",     tags_full,fabs(ev.en_hoe[lpid]),weight);
		  mon.fillHisto(lepStr+reg+"eopin",   tags_full,fabs(ev.en_eopin[lpid]),weight);
		  mon.fillHisto(lepStr+reg+"fbrem",   tags_full,fabs(ev.en_fbrem[lpid]),weight);
		  mon.fillHisto(lepStr+reg+"ooemoop", tags_full,fabs(ev.en_ooemoop[lpid]),weight);
	      }else{
		  mon.fillHisto(lepStr+"nmuonhits",  tags_full,fabs(ev.mn_validMuonHits[lpid]),weight);
		  mon.fillHisto(lepStr+"nmatches",   tags_full,fabs(ev.mn_nMatches[lpid]),weight);
	      }
	      
	      if(hasGoodId){
		  mon.fillHisto(lepStr+"reliso",     tags_full, relIso,   weight);
		  mon.fillHisto(lepStr+"reliso2011", tags_full, relIso2011, weight);
		  TString lepType(matchid!=0 ? "true":"fake");
		  if(isMC){
		      mon.fillHisto(lepStr+lepType+"reliso",     tags_full, relIso,   weight);
		      mon.fillHisto(lepStr+lepType+"reliso2011", tags_full, relIso2011, weight);
		  }
	      }
          }
      }
      //select dilepton
      if(!passIdAndIso) continue;
      if(isMC && use2011Id) weight *= llScaleFactor*llTriggerEfficiency;

      //
      // 3rd LEPTON ANALYSIS
      //
      int nextraleptons(0);
      std::vector<LorentzVector> extraLeptonsP4;
      for(size_t ilep=2; ilep<phys.leptons.size(); ilep++){
	//lepton type
	bool isGood(false);
	int lpid=phys.leptons[ilep].pid;
	if(fabs(phys.leptons[ilep].id)==13){
	  if(!use2011Id){
	    isGood = (hasObjectId(ev.mn_idbits[lpid], MID_LOOSE) && phys.leptons[ilep].pfRelIsoDbeta()<0.2);
	    isGood |= (hasObjectId(ev.mn_idbits[lpid], MID_SOFT) && phys.leptons[ilep].pt()>3);
	  }else{
	    isGood = (hasObjectId(ev.mn_idbits[lpid], MID_VBTF2011) && phys.leptons[ilep].relIsoRho(ev.rho)<0.15 && phys.leptons[ilep].pt()>10);
	  }
	}else{
	  if(!use2011Id){
	    isGood = ( hasObjectId(ev.en_idbits[lpid],EID_VETO) && phys.leptons[ilep].ePFRelIsoCorrected2012(ev.rho)<0.15 && phys.leptons[ilep].pt()>10);
	  }else{
	    isGood = ( hasObjectId(ev.en_idbits[lpid],EID_VBTF2011) && phys.leptons[ilep].relIsoRho(ev.rho)<0.1 && phys.leptons[ilep].pt()>10);
	  }
	    }
	nextraleptons += isGood;
	
	if(!isGood) continue;
	extraLeptonsP4.push_back( phys.leptons[ilep] );
      }
      bool pass3dLeptonVeto(nextraleptons==0);
      
      //
      //JET ANALYSIS
      //
      PhysicsObjectJetCollection aJets= variedAJets[0];
      PhysicsObjectJetCollection aGoodIdJets;
      LorentzVector aClusteredMetP4(zll); aClusteredMetP4 *= -1;
      int nAJetsLoose(0), nABtags(0);
      float mindphijmet(999999.),mindphijmet15(999999.);
      PhysicsObjectJetCollection recoilJets;
      for(size_t ijet=0; ijet<aJets.size(); ijet++) 
	{
	  float idphijmet( fabs(deltaPhi(aJets[ijet].phi(),zvvs[0].phi()) ) );
	  if(idphijmet<mindphijmet15)  mindphijmet15=idphijmet;
	  if(aJets[ijet].pt()>30) if(idphijmet<mindphijmet)  mindphijmet=idphijmet;
	      
	  aClusteredMetP4 -= aJets[ijet];	  
	  if(fabs(deltaPhi(aJets[ijet].phi(),zll.phi()))>2) recoilJets.push_back( aJets[ijet] );
	  
	  bool isGoodJet    =hasObjectId(aJets[ijet].pid,JETID_LOOSE);//TIGHT);
	  if(isGoodJet)
	    {
	      aGoodIdJets.push_back(aJets[ijet]);
	      if(passZmass && passZpt)
		{
		  mon.fillHisto("jetpt",  tags_full, aJets[ijet].pt(),weight);
		  mon.fillHisto("jeteta",  tags_full, fabs(aJets[ijet].eta()),weight);
		}
	    }
	  
	  if(aJets[ijet].pt()<30) continue;
	  if(fabs(aJets[ijet].eta())<2.5) 
	    {
	      nABtags += (aJets[ijet].btag2>0.244);
	      if(passZmass && passZpt)	mon.fillHisto("btags",  tags_full, aJets[ijet].btag2,weight);
		  
	      if(aJets[ijet].pt()<30 ) continue;
	      nAJetsLoose      += hasObjectId(aJets[ijet].pid,JETID_LOOSE);
	    }
	}
      bool passBveto(nABtags==0);
      bool passJetVeto(nAJetsLoose==0);
      
      //
      // MET ANALYSIS
      //
      METUtils::stRedMET aRedMetOut; 
      LorentzVector aRedMet=METUtils::redMET(METUtils::INDEPENDENTLYMINIMIZED, lep1, 0, lep2, 0, aClusteredMetP4, zvvs[0],false,&aRedMetOut);
      double aRedMetL=aRedMetOut.redMET_l;
      double aRedMetT=aRedMetOut.redMET_t;
      float balance=zvvs[0].pt()/zll.pt();
      bool passBalance(balance>0.4 && balance<1.8);
      bool passDphijmet(mindphijmet>0.5);
      if(nAJetsLoose==0) passDphijmet=(mindphijmet15>0.5);
      bool passRedMet(aRedMet.pt()>70);
      
      //now fill control distributions
      tags_full.push_back(tag_cat);
      mon.fillHisto("eventflow",tags_full,0,weight);
      mon.fillHisto("zmass",       tags_full, zll.mass(), weight);  
 
      //
      // EVENT SELECTION
      //
      if(passZmass){

	//basic Z and pu control
	mon.fillHisto("eventflow",   tags_full, 1,            weight);
	mon.fillHisto("zpt"      ,   tags_full, zll.pt(),     weight);      
	mon.fillHisto("zeta"     ,   tags_full, zll.eta(),    weight);
	mon.fillHisto("nvtx"     ,   tags_full, ev.nvtx,      weight);
	mon.fillHisto("nvtxraw"  ,   tags_full, ev.nvtx,      1);
	mon.fillHisto("rho"      ,   tags_full, ev.rho,       weight);
	mon.fillHisto("rho25"    ,   tags_full, ev.rho25Neut, weight);
	
	if(passZpt){
	  mon.fillHisto  ("eventflow",tags_full,2,weight);
	  
	  //analyze dilepton kinematics
	  LorentzVector leadingLep(phys.leptons[0].pt()>phys.leptons[1].pt() ? phys.leptons[0]: phys.leptons[1]);
	  LorentzVector trailerLep(phys.leptons[0].pt()>phys.leptons[1].pt() ? phys.leptons[1]: phys.leptons[0]);
	  mon.fillHisto("leadeta"     ,   tags_full, leadingLep.eta()   ,weight);
	  mon.fillHisto("leadpt"      ,   tags_full, leadingLep.pt()    ,weight);
	  mon.fillHisto("trailereta"  ,   tags_full, trailerLep.eta()   ,weight);
	  mon.fillHisto("trailerpt"   ,   tags_full, trailerLep.pt()    ,weight);

	  //analyze also 3 lepton category
	  mon.fillHisto("nleptons",tags_full,2+nextraleptons,weight);
	  if(nextraleptons==1) 
	    {
	      mon.fillHisto("thirdleptoneta",   tags_full,extraLeptonsP4[0].eta()   ,weight);
	      mon.fillHisto("thirdleptonpt" ,   tags_full,extraLeptonsP4[0].pt()     ,weight);
	      mon.fillHisto("mt3",tags_full,METUtils::transverseMass(extraLeptonsP4[0],zvvs[0],false),weight);
	    }

	  mon.fillHisto("nbtags",  tags_full, nABtags,weight);
	  if(passBveto)
	    {
	      mon.fillHisto("eventflow",tags_full,3,weight);
	      mon.fillHisto("njets",          tags_full, nAJetsLoose,weight);
	      if(passJetVeto)
		{
		  mon.fillHisto("eventflow",tags_full,4,weight);
		  mon.fillHisto("met_redMet",tags_full,aRedMet.pt(),weight);
		  mon.fillHisto("met_redMetL",tags_full,aRedMetT,weight);
		  mon.fillHisto("met_redMetT",tags_full,aRedMetL,weight);		  

		  if(passRedMet)
		    {
		      mon.fillHisto("eventflow",tags_full,5,weight);
		      mon.fillHisto("balance",tags_full, balance, weight);
	      
		      if(passBalance)
			{
			  mon.fillHisto("eventflow",tags_full,6,weight);
			  mon.fillHisto("mindphijmet",tags_full,nAJetsLoose==0 ? mindphijmet15:mindphijmet,weight);
			  mon.fillHisto("eventflow",tags_full,6,weight);
			  
			  if(passDphijmet)
			    {
			      mon.fillHisto("eventflow",tags_full,7,weight);
			      if(pass3dLeptonVeto)
				{
				  mon.fillHisto("eventflow",tags_full,8,weight);
				}
			    }
			}
		    }
		}
	    }
	}
      }
      

      //
      // HISTOS FOR STATISTICAL ANALYSIS (include systematic variations, selection is repeated for each variation)
      //
      //FIXME
//       for(size_t ivar=0; ivar<nvarsToInclude; ivar++){
//         float iweight = weight;                                               //nominal
//         if(ivar==9)                         iweight *=TotalWeight_plus;        //pu up
//         if(ivar==10)                        iweight *=TotalWeight_minus;       //pu down

// 	//recompute MET/MT if JES/JER was varied
// 	LorentzVector zvv    = zvvs[ivar>8 ? 0 : ivar];
// 	float mt3(0);
// 	if(nextraleptons==1) 
// 	PhysicsObjectJetCollection &varJets=variedAJets[ivar>4 ? 0  : ivar];
// 	PhysicsObjectJetCollection tightVarJets;
// 	LorentzVector clusteredMetP4(zll); clusteredMetP4 *= -1;
// 	bool passLocalBveto(passBveto);
// 	for(size_t ijet=0; ijet<varJets.size(); ijet++){
// 	    clusteredMetP4 -= varJets[ijet];
// 	    if(!hasObjectId(varJets[ijet].pid,JETID_LOOSE)) continue;
// 	    tightVarJets.push_back( varJets[ijet] );
// 	    if(varJets[ijet].pt()<30 || fabs(varJets[ijet].eta())>2.5)continue;
// 	    if(ivar==11)      passLocalBveto &= (varJets[ijet].btag3<0.285);
// 	    else if(ivar==12) passLocalBveto &= (varJets[ijet].btag3<0.265);
//         }
// 	bool passPreselection             (passZmass && passZpt && pass3dLeptonVeto && passDphijmet && passLocalBveto);
// 	bool passPreselectionMbvetoMzmass (             passZpt && pass3dLeptonVeto && passDphijmet                  );          
// 	bool passPreselectionM3dlep       (passZmass && passZpt                     && passDphijmet && passLocalBveto);
	
// 	float mt = METUtils::transverseMass(zll,zvv,true);
// 	LorentzVector nullP4(0,0,0,0);
// 	LorentzVector redMet = METUtils::redMET(METUtils::INDEPENDENTLYMINIMIZED, zll, 0, nullP4, 0, clusteredMetP4, zvv,true);

// 	//re-assign the event category if jets were varied
// 	int eventSubCat  = eventCategoryInst.Get(phys,&tightVarJets);
// 	TString tag_subcat = eventCategoryInst.GetLabel(eventSubCat);
// 	tags_full.clear();
// 	tags_full.push_back(tag_cat);
// 	tags_full.push_back(tag_cat+tag_subcat);
// 	if(tag_subcat!="vbf") tags_full.push_back(tag_cat + "novbf");
//         if(tag_subcat=="eq1jets" || tag_subcat=="geq2jets")tags_full.push_back(tag_cat + "geq1jets");
//         if(tag_subcat=="geq2jets" || tag_subcat=="vbf")tags_full.push_back(tag_cat + "geq2jetsInc");
	
// 	bool hasVbfBlinding(!isMC && runBlinded && tag_subcat=="vbf" && zvvs[0].pt()>70);
// 	if(runBlinded && (mustBlind || hasVbfBlinding) ) continue;
	
// 	if(passPreselection && zvv.pt()>50) mon.fillHisto("mtvar"+varNames[ivar],tags_full,mt,iweight);
	

//         if(ivar==0 && outTxtFile && tag_subcat=="vbf" && zvv.pt()>70 && passPreselection){
//            fprintf(outTxtFile,"X----------------------------\nCat: %s - %s\n",tag_cat.Data(),tag_subcat.Data());
//            fprintf(outTxtFile,"inputFile = %s\n",url.Data());
//            fprintf(outTxtFile,"run/lumi/event = %i/%i/%i\n",ev.run, ev.lumi, ev.event);
//            fprintf(outTxtFile,"mt = %f met = %f -redMet = %f\n",mt, zvv.pt(), redMet.pt());
//         }


//         if(ivar==0 && outTxtFile && tag_subcat=="geq2jets" && zvv.pt()>100 && mt<250 && passPreselection){
//            fprintf(outTxtFile,"DEBUG----------------------------\nCat: %s - %s\n",tag_cat.Data(),tag_subcat.Data());
//            fprintf(outTxtFile,"subcat = %s inputFile = %s\n",tag_subcat.Data(), url.Data());
//            fprintf(outTxtFile,"run/lumi/event = %i/%i/%i\n",ev.run, ev.lumi, ev.event);
//            fprintf(outTxtFile,"mt = %f met = %f -redMet = %f\n",mt, zvv.pt(), redMet.pt());
//            fprintf(outTxtFile,"nvtx = %i rho=%f rho25 = %f\n",ev.nvtx,ev.rho, ev.rho25Neut);
//            fprintf(outTxtFile,"zll  pt=%f mass=%f eta=%f phi=%f\n",zll.pt(), zll.mass(), zll.eta(), zll.phi());
//            for(unsigned int j=0;j<phys.ajets.size();j++){
//               fprintf(outTxtFile,"jet %i  pt=%f eta=%f phi=%f\n",j, phys.ajets[j].pt(), phys.ajets[j].eta(), phys.ajets[j].phi());
//            }
//         }


// 	//fill shapes
// 	for(unsigned int index=0;index<optim_Cuts1_met.size();index++){
	  
// 	  if(redMet.pt()>optim_Cuts1_met[index] && mt>optim_Cuts1_mtmin[index] && mt<optim_Cuts1_mtmax[index])
// 	    if(passPreselection                                        )          mon.fillHisto(TString("mt_redMet_shapes")+varNames[ivar],tags_full,index, mt,iweight);
	  
// 	  if(zvv.pt()>optim_Cuts1_met[index] && mt>optim_Cuts1_mtmin[index] && mt<optim_Cuts1_mtmax[index]){
// 	      if(passPreselection                                                    )   mon.fillHisto(TString("mt_shapes")+varNames[ivar],tags_full,index, mt,iweight);
// 	      if(passPreselection && passZmass10                                     )   mon.fillHisto(TString("mt_shapesZ10")+varNames[ivar],tags_full,index, mt,iweight);
// 	      if(passPreselection && passZmass5                                      )   mon.fillHisto(TString("mt_shapesZ5")+varNames[ivar],tags_full,index, mt,iweight);
	      
// 	      //                   if(passPreselectionM3dlep       && !pass3dLeptonVeto && nExtraLep==1 && passZmass3dLepton && ivar==0)   mon.fillHisto(TString("mt_shapes_3rdLepton")+varNames[ivar],tags_full,index, mt,iweight);
// //	      if(passPreselectionM3dlep       && !pass3dLeptonVeto && nextraleptons==1 && ivar==0)   mon.fillHisto(TString("mt_shapes_3rdLepton")+varNames[ivar],tags_full,index, mt,iweight);
	      
// //	      if(passPreselectionMbvetoMzmass && passZmass   && !passLocalBveto && ivar==0)   mon.fillHisto(TString("mt_shapesBTagSB")+varNames[ivar],tags_full,index, mt,iweight);
// //	      if(passPreselectionM3dlep       && !pass3dLeptonVeto && nextraleptons==1   )   mon.fillHisto(TString("mt3")+varNames[ivar],tags_full,index, mt3,iweight);
// 	      if(passPreselectionMbvetoMzmass && passZmass         && passLocalBveto      )   mon.fillHisto("nonresbckg_ctrl"+varNames[ivar],tags_full,index,0,iweight);
// 	      if(passPreselectionMbvetoMzmass && isZsideBand       && passLocalBveto      )   mon.fillHisto("nonresbckg_ctrl"+varNames[ivar],tags_full,index,1,iweight);
// 	      if(passPreselectionMbvetoMzmass && isZsideBandPlus   && passLocalBveto      )   mon.fillHisto("nonresbckg_ctrl"+varNames[ivar],tags_full,index,2,iweight);
// 	      if(passPreselectionMbvetoMzmass && passZmass         && !passLocalBveto     )   mon.fillHisto("nonresbckg_ctrl"+varNames[ivar],tags_full,index,3,iweight);
// 	      if(passPreselectionMbvetoMzmass && isZsideBand       && !passLocalBveto     )   mon.fillHisto("nonresbckg_ctrl"+varNames[ivar],tags_full,index,4,iweight);
// 	      if(passPreselectionMbvetoMzmass && isZsideBandPlus   && !passLocalBveto     )   mon.fillHisto("nonresbckg_ctrl"+varNames[ivar],tags_full,index,5,iweight);

// //              if(passPreselection && index==78 && (ivar==0 || ivar==5 || ivar==6)){printf("SYst=%10s  Weight=%6.3E Integral=%6.3E\n",varNames[ivar].Data(),iweight, ((TH2*)mon.getHisto(TString("mt_shapes")+varNames[ivar],tag_cat))->ProjectionY("tmp",79,79)->Integral());}

// 	    }         
//  	 }
//       }
  }

  printf("\n"); 
  file->Close();

  //##############################################
  //########     SAVING HISTO TO FILE     ########
  //##############################################
  //save control plots to file
  outUrl += "/";
  outUrl += gSystem->BaseName(url);
  printf("Results save in %s\n", outUrl.Data());

  //save all to the file
  TFile *ofile=TFile::Open(outUrl, "recreate");
  mon.Write();
  ofile->Close();

  if(outTxtFile)fclose(outTxtFile);
}  




