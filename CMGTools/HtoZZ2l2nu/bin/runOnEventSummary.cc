#include <iostream>
#include <boost/shared_ptr.hpp>

#include "CMGTools/HtoZZ2l2nu/interface/ZZ2l2nuSummaryHandler.h"
#include "CMGTools/HtoZZ2l2nu/interface/ZZ2l2nuPhysicsEvent.h"
#include "CMGTools/HtoZZ2l2nu/interface/ReducedMETFitter.h"
#include "CMGTools/HtoZZ2l2nu/interface/ReducedMETComputer.h"
#include "CMGTools/HtoZZ2l2nu/interface/TransverseMassComputer.h"
#include "CMGTools/HtoZZ2l2nu/interface/ProjectedMETComputer.h"
#include "CMGTools/HtoZZ2l2nu/interface/setStyle.h"
#include "CMGTools/HtoZZ2l2nu/interface/plotter.h"
#include "CMGTools/HtoZZ2l2nu/interface/ObjectFilters.h"
#include "CMGTools/HtoZZ2l2nu/interface/SelectionMonitor.h"
#include "CMGTools/HtoZZ2l2nu/interface/JetEnergyUncertaintyComputer.h"

#include "CondFormats/JetMETObjects/interface/JetResolution.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

#include "FWCore/FWLite/interface/AutoLibraryLoader.h"
#include "FWCore/PythonParameterSet/interface/MakeParameterSets.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TProfile.h"
#include "TEventList.h"
 
using namespace std;

//
int main(int argc, char* argv[])
{
  // load framework libraries
  gSystem->Load( "libFWCoreFWLite" );
  AutoLibraryLoader::enable();

  //check arguments
  if ( argc < 2 ) {
    std::cout << "Usage : " << argv[0] << " parameters_cfg.py" << std::endl;
    return 0;
  }

  //configure
  const edm::ParameterSet &runProcess = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("runProcess");
  TString url=runProcess.getParameter<std::string>("input");
  TString outdir=runProcess.getParameter<std::string>("outdir");
  bool isMC = runProcess.getParameter<bool>("isMC");
  double xsec = runProcess.getParameter<double>("xsec");
  //bool useFitter = runProcess.getParameter<bool>("useFitter");

  int evStart=runProcess.getParameter<int>("evStart");
  int evEnd=runProcess.getParameter<int>("evEnd");
  TString dirname = runProcess.getParameter<std::string>("dirName");

  TString etaFileName = runProcess.getParameter<std::string>("etaResolFileName"); gSystem->ExpandPathName(etaFileName);
  JetResolution stdEtaResol(etaFileName.Data(),false);
  TString phiFileName = runProcess.getParameter<std::string>("phiResolFileName"); gSystem->ExpandPathName(phiFileName);
  JetResolution stdPhiResol(phiFileName.Data(),false);
  TString ptFileName  = runProcess.getParameter<std::string>("ptResolFileName");  gSystem->ExpandPathName(ptFileName);
  JetResolution stdPtResol(ptFileName.Data(),true);
  TString uncFile =  runProcess.getParameter<std::string>("jesUncFileName"); gSystem->ExpandPathName(uncFile);
  JetCorrectionUncertainty jecUnc(uncFile.Data());
  jet::UncertaintyComputer jcomp(&stdPtResol,&stdEtaResol,&stdPhiResol,&jecUnc);

  //
  //control histograms
  //
  SelectionMonitor controlHistos;
  
  TH1F *h=new TH1F ("eventflow", ";Step;Events", 6,0,6);
  h->GetXaxis()->SetBinLabel(2,"|M-M_{Z}|<15");
  h->GetXaxis()->SetBinLabel(3,"3^{rd}-lepton veto");
  h->GetXaxis()->SetBinLabel(4,"no SSVHEM tags");
  h->GetXaxis()->SetBinLabel(5,"red-E_{T}^{miss}>39");
  h->GetXaxis()->SetBinLabel(6,"red-E_{T}^{miss}>57");
  controlHistos.addHistogram( h );

  controlHistos.addHistogram( new TH1F ("nbtags", ";b-tag multiplicity (SSVHEM);Events", 4,0,4) );  

  controlHistos.addHistogram( new TProfile ("metprof", ";Pileup events;E_{T}^{miss}", 15,0,15) ); 
  controlHistos.addHistogram( new TProfile ("redmetprof", ";Pileup events;red-E_{T}^{miss}", 15,0,15) );  
  controlHistos.addHistogram( new TProfile ("superredmetprof", ";Pileup events;min red-( E_{T}^{miss},track-E_{T}^{miss} )", 15,0,15) );  
  controlHistos.addHistogram( new TProfile ("projmetprof", ";Pileup events;projected E_{T}^{miss}", 15,0,15) );  
  controlHistos.addHistogram( new TProfile ("minmetprof", ";Pileup events;min-E_{T}^{miss}", 15,0,15) );  
  controlHistos.addHistogram( new TH2F ("metvspu", ";Pileup events;E_{T}^{miss}", 15,0,15,100,0,500) );  
  controlHistos.addHistogram( new TH2F ("redmetvspu", ";Pileup events;red-E_{T}^{miss}", 15,0,15,100,0,500) );  
  controlHistos.addHistogram( new TH2F ("superredmetvspu", ";Pileup events;min red-( E_{T}^{miss},track-E_{T}^{miss} )", 15,0,15,100,0,500) );  
  controlHistos.addHistogram( new TH2F ("minmetvspu", ";Pileup events;min-E_{T}^{miss}", 15,0,15,100,0,500) );  
  controlHistos.addHistogram( new TH2F ("projmetvspu", ";Pileup events;projected E_{T}^{miss}", 15,0,15,100,0,500) );  

  controlHistos.addHistogram( new TH1F ("met", ";type-I E_{T}^{miss};Events", 100,0,500) );  
  controlHistos.addHistogram( new TH1F ("redmet", ";red-E_{T}^{miss};Events", 100,0,500) );
  controlHistos.addHistogram( new TH1F ("superredmet", ";min red-( E_{T}^{miss},track-E_{T}^{miss} );Events", 100,0,500) );
  controlHistos.addHistogram( new TH1F ("redmetL", ";red-E_{T}^{miss,#parallel};Events", 100,-250,250) );
  controlHistos.addHistogram( new TH1F ("redmetT", ";red-E_{T}^{miss,#perp};Events", 100,-250,250) );
  controlHistos.addHistogram( (TH1D *)(new TH2D ("redmetcomps", ";red-E_{T}^{miss,#parallel};red-E_{T}^{miss,#perp};Events", 100, -251.,249,100, -251.,249.) ) );

  controlHistos.addHistogram( new TH1F ("finalredmetL", ";red-E_{T}^{miss,#parallel};Events", 100,-250,250) );
  controlHistos.addHistogram( new TH1F ("finalredmetT", ";red-E_{T}^{miss,#perp};Events", 100,-250,250) );
  controlHistos.addHistogram( (TH1D *)(new TH2D ("finalredmetcomps", ";red-E_{T}^{miss,#parallel};red-E_{T}^{miss,#perp};Events", 100, -251.,249,100, -251.,249.) ) );

  controlHistos.addHistogram( new TH1F ("projmet", ";projected E_{T}^{miss};Events", 100,0,500) );
  controlHistos.addHistogram( new TH1F ("minmet", ";min projected E_{T}^{miss};Events", 100,0,500) );
  controlHistos.addHistogram(  new TH1D ("zpt", ";p_{T}^{ll};Events", 100,0,400) );
  controlHistos.addHistogram(  new TH1D ("zeta", ";#eta^{ll};Events", 100,-5,5) );

  controlHistos.addHistogram( new TH1F ("metoverzpt", ";type I E_{T}^{miss}/p_{T}(Z);Events", 100,-1,9) );
  controlHistos.addHistogram( new TH1F ("projmetoverzpt", ";projected E_{T}^{miss}/p_{T}(Z);Events", 100,-1,9) );
  controlHistos.addHistogram( new TH1F ("redmetoverzpt", ";red-E_{T}^{miss}/p_{T}(Z);Events", 100,-1,9) );

  controlHistos.addHistogram( (TH1D *)(new TH2D ("redmetvszpt", ";red-E_{T}^{miss};p_{T}(Z);Events", 100, -10.,250,100, -10,250) ) );
  controlHistos.addHistogram( (TH1D *)(new TH2D ("metvszpt", ";type I E_{T}^{miss};p_{T}(Z);Events", 100, -10.,250,100, -10,250) ) );
  controlHistos.addHistogram( (TH1D *)(new TH2D ("projmetvszpt", ";projected E_{T}^{miss};p_{T}(Z);Events", 100, -10.,250,100, -10,250) ) );
  
  controlHistos.addHistogram( new TH1D ("dphill", ";#Delta#phi(l^{(1)},l^{(2)});Events",100,-3.2,3.2) );
  controlHistos.addHistogram( new TH1D ("mindrlz", ";min #DeltaR(l,Z);Events",100,0,6) );
  controlHistos.addHistogram( new TH1D ("maxdrlz", ";max #DeltaR(l,Z);Events",100,0,6) );
  controlHistos.addHistogram( new TH1D ("drll", ";#DeltaR(l^{(1)},l^{(2)});Events",100,0,6) );
  controlHistos.addHistogram( new TH1D ("dphizz", ";#Delta#phi(ll,E_{T}^{miss});Events",100,-3.2,3.2) );
  controlHistos.addHistogram(  new TH1D ("mtsum", ";#sum M_{T}(l,E_{T}^{miss});Events", 100,0,1000) );

//   TString systVars[]={"jer","jesup","jesdown","nopu","flatpu"};
//   for(size_t ivar=0; ivar<sizeof(systVars)/sizeof(TString); ivar++)
//     {
//       controlHistos.addHistogram( new TH1F ("met"+systVars[ivar], ";type-I E_{T}^{miss};Events", 100,0,500) );  
//       controlHistos.addHistogram( new TH1F ("minmet"+systVars[ivar], ";min-E_{T}^{miss};Events", 100,0,500) );
//       controlHistos.addHistogram( new TH1F ("redmet"+systVars[ivar], ";red-E_{T}^{miss};Events", 100,0,500) );
//       controlHistos.addHistogram( new TH1F ("projmet"+systVars[ivar], ";projected E_{T}^{miss};Events", 100,0,500) );  
//     }

  //replicate monitor for categories
  TString cats[]={"ee","emu","mumu"};
  TString subcats[]={"","eq0jets","eq1jets","geq2jets"};
  TString topcats[]={""};//,"cat1","cat2"};
  for(size_t icat=0;icat<sizeof(cats)/sizeof(TString); icat++)
    {
      for(size_t isubcat=0;isubcat<sizeof(subcats)/sizeof(TString); isubcat++)
	{
	  for(size_t itopcat=0;itopcat<sizeof(topcats)/sizeof(TString); itopcat++)
	    {
	      controlHistos.initMonitorForStep(cats[icat]+subcats[isubcat]+topcats[itopcat]);
	    }
	}
    }
  
   
  //start the met computers
  ProjectedMETComputer pmetComp;
  ReducedMETComputer rmetComp(1., 1., 1., 1., 1.);
  //ReducedMETFitter rmetFitter(runProcess);
  TransverseMassComputer mtComp;

  //open the file and get events tree
  ZZ2l2nuSummaryHandler evSummaryHandler;
  TFile *file = TFile::Open(url);
  if(file==0) return -1;
  if(file->IsZombie()) return -1;
  if( !evSummaryHandler.attachToTree( (TTree *)file->Get(dirname) ) ) 
    {
      file->Close();
      return -1;
    }

  //check run range to compute scale factor (if not all entries are used)
  const Int_t totalEntries= evSummaryHandler.getEntries();
  float rescaleFactor( evEnd>0 ?  float(totalEntries)/float(evEnd-evStart) : -1 );
  if(evEnd<0 || evEnd>evSummaryHandler.getEntries() ) evEnd=totalEntries;
  if(evStart > evEnd ) 
    {
      file->Close();
      return -1;
    }

  //check PU normalized entries 
  evSummaryHandler.getTree()->Draw(">>elist","normWeight==1");
  TEventList *elist = (TEventList*)gDirectory->Get("elist");
  const Int_t normEntries = elist->GetN(); 
  if(normEntries==0) cout << "[Warning] Normalized PU entries is 0, check if the PU normalization producer was properly run" << endl;

  //MC normalization (to 1/pb)
  float cnorm=1.0;
  if(isMC)
    {
      TH1F *cutflowH = (TH1F *) file->Get("evAnalyzer/h2zz/mumu/mumu_cutflow");
      if(cutflowH) cnorm=cutflowH->GetBinContent(1);
      if(rescaleFactor>0) cnorm /= rescaleFactor;
    }
  cout << "xSec x Br=" << xsec << " total entries=" << totalEntries << " normalized PU entries=" << normEntries << " obtained from " << cnorm << " generated events" << endl; 
    
  // init summary tree (for unweighted events) 
  bool saveSummaryTree = runProcess.getParameter<bool>("saveSummaryTree");
  TFile *spyFile=0;
  TDirectory *spyDir=0;
  ofstream *outf=0;
  ZZ2l2nuSummaryHandler *spyHandler=0;
  float summaryWeight(1);
  if(saveSummaryTree && normEntries>0)
    {
      summaryWeight = xsec * float(totalEntries) / (cnorm * float(normEntries) );
      spyHandler = new ZZ2l2nuSummaryHandler;
      spyFile = TFile::Open("EventSummaries.root","UPDATE");
      TString evtag=gSystem->BaseName(url);
      evtag.ReplaceAll(".root","");
      spyFile->rmdir(evtag);
      spyDir = spyFile->mkdir(evtag);
      TTree *outT = evSummaryHandler.getTree()->CloneTree(0);
      outT->SetDirectory(spyDir);
      spyHandler->initTree(outT,false);
  
      if(!isMC) outf=new ofstream("highmetevents.txt",ios::app);  
    }
  
  //run the analysis
  for( int iev=evStart; iev<evEnd; iev++)
    {
      if(iev%1000==0) printf("\r [ %d/100 ] ",int(100*float(iev-evStart)/float(evEnd)));
      
      evSummaryHandler.getEntry(iev);
      ZZ2l2nuSummary_t &ev=evSummaryHandler.getEvent();
      PhysicsEvent_t phys=getPhysicsEventFrom(ev);
      float weight=ev.weight;
      if(!isMC) weight=1;

      //event categories
      TString evcat("");
      if(ev.cat==dilepton::EMU)  evcat="emu";
      if(ev.cat==dilepton::MUMU) evcat="mumu";
      if(ev.cat==dilepton::EE)   evcat="ee";

      TString subcat("eq0jets");
      if(phys.jets.size()==1) subcat="eq1jets";
      if(phys.jets.size()>1)  subcat="geq2jets";

      //z kinematics
      double dphill=deltaPhi(phys.leptons[0].phi(),phys.leptons[1].phi());
      double drll=deltaR(phys.leptons[0],phys.leptons[1]);
      LorentzVector zll=phys.leptons[0]+phys.leptons[1];
      double mindrlz = min( deltaR(phys.leptons[0],zll), deltaR(phys.leptons[1],zll) );
      double maxdrlz = max( deltaR(phys.leptons[0],zll), deltaR(phys.leptons[1],zll) );

      LorentzVector zvv=phys.met[0];
      LorentzVector chmet = phys.met[1];
      LorentzVector genzll(0,0,0,0), genzvv(0,0,0,0), higgs(0,0,0,0);
      if(phys.genleptons.size()) 
	{
	  genzll=phys.genleptons[0]+phys.genleptons[1];
	  genzvv=phys.genmet[0];
	  higgs = phys.genhiggs[0];	
	}

      //jet/met kinematics and systematic variations
      LorentzVectorCollection jetsp4;
      double mindphijmet(1000.);
      int nbtags(0);
      for(size_t ijet=0; ijet<phys.jets.size(); ijet++) 
	{
	  double dphi=deltaPhi(phys.jets[ijet].phi(),zvv.phi());
	  if(fabs(dphi)<fabs(mindphijmet)) mindphijmet=dphi;
	  jetsp4.push_back( phys.jets[ijet] );
	  if(phys.jets[ijet].btag3>1.74) nbtags++;
	}

      /*
	jcomp.compute(jetsp4,phys.met[0]);
	LorentzVector metJER=jcomp.getVariedMet(jet::UncertaintyComputer::JER);
	LorentzVectorCollection jetsJER = jcomp.getVariedJets(jet::UncertaintyComputer::JER);
	LorentzVector metJESdown=jcomp.getVariedMet(jet::UncertaintyComputer::JES_DOWN);
	LorentzVectorCollection jetsJESdown=jcomp.getVariedJets(jet::UncertaintyComputer::JES_DOWN);
	LorentzVector metJESup=jcomp.getVariedMet(jet::UncertaintyComputer::JES_UP);
	LorentzVectorCollection jetsJESup=jcomp.getVariedJets(jet::UncertaintyComputer::JES_UP);
      */

      //redmet
      //rmetComp.compute(phys.leptons[0],phys.leptons[0].ptErr,phys.leptons[1], phys.leptons[1].ptErr, jetsp4, zvv );
      rmetComp.compute(phys.leptons[0],0,phys.leptons[1], 0, jetsp4, zvv );
      double redmetL = rmetComp.reducedMETComponents(ReducedMETComputer::INDEPENDENTLYMINIMIZED).second;
      double redmetT = rmetComp.reducedMETComponents(ReducedMETComputer::INDEPENDENTLYMINIMIZED).first;
      double redmet = rmetComp.reducedMET(ReducedMETComputer::INDEPENDENTLYMINIMIZED);

      int rMetCateg = rmetComp.getEventCategory();
      double deltaDileptonL = rmetComp.dileptonPtCorrComponents().second;
      double deltaDileptonT = rmetComp.dileptonPtCorrComponents().first;

      //projected met
      double projMet = pmetComp.compute(phys.leptons[0], phys.leptons[1], zvv );
      double projChMet= pmetComp.compute(phys.leptons[0],phys.leptons[1], chmet);
      float minmet = min(fabs(projMet),fabs(projChMet));

      //super-redmet
      rmetComp.compute(phys.leptons[0],phys.leptons[0].ptErr,phys.leptons[1], phys.leptons[1].ptErr, jetsp4, chmet );
      double redchmet = rmetComp.reducedMET(ReducedMETComputer::INDEPENDENTLYMINIMIZED);
      float superredmet=min(redmet,redchmet);

      //ratios
      double metoverzpt     =zvv.pt()/zll.pt();
      double projmetoverzpt =projMet/zll.pt();
      double minmetoverzpt  =minmet/zll.pt();
      double redmetoverzpt  = redmet/zll.pt();

      //mt variables
      float mtsum=mtComp.compute(phys.leptons[0],zvv,false)+mtComp.compute(phys.leptons[1],zvv,false);
      float mt=mtComp.compute(zll,zvv,true);

      //systematic variations
      /*
      rmetComp.compute(phys.leptons[0],phys.leptons[0].ptErr,phys.leptons[1], phys.leptons[1].ptErr, jetsJER, metJER );
      double minmetJER = rmetComp.reducedMET(ReducedMETComputer::MINIMIZED);
      double redmetJER = rmetComp.reducedMET(ReducedMETComputer::INDEPENDENTLYMINIMIZED);
      double projMetJER = pmetComp.compute(phys.leptons[0], phys.leptons[1], metJER );

      rmetComp.compute(phys.leptons[0],phys.leptons[0].ptErr,phys.leptons[1], phys.leptons[1].ptErr, jetsJESup, metJESup );
      double minmetJESup = rmetComp.reducedMET(ReducedMETComputer::MINIMIZED);
      double redmetJESup = rmetComp.reducedMET(ReducedMETComputer::INDEPENDENTLYMINIMIZED);
      double projMetJESup = pmetComp.compute(phys.leptons[0], phys.leptons[1], metJESup );

      rmetComp.compute(phys.leptons[0],phys.leptons[0].ptErr,phys.leptons[1], phys.leptons[1].ptErr, jetsJESdown, metJESdown );
      double minmetJESdown = rmetComp.reducedMET(ReducedMETComputer::MINIMIZED);
      double redmetJESdown = rmetComp.reducedMET(ReducedMETComputer::INDEPENDENTLYMINIMIZED);
      double projMetJESdown = pmetComp.compute(phys.leptons[0], phys.leptons[1], metJESdown );
*/

//       bool passMedium(redmet>43.4);
//       bool passTight(redmet>62.7);

      //fill control histograms
      TString cats[]={"all",evcat};
      TString subCats[]={"",subcat};
      std::vector<TString> topCats;
      topCats.push_back("");
      //if(rMetCateg==ReducedMETComputer::COLLIMATED) topCats.push_back("cat1");
      //if(rMetCateg==ReducedMETComputer::OPENANGLE) topCats.push_back("cat2");
      //if(projMetCateg==ProjectedMETComputer::OPENANGLE) topCats.push_back("cat1");
      //if(projMetCateg==ProjectedMETComputer::COLLIMATED) topCats.push_back("cat2");
      for(size_t ic=0; ic<sizeof(cats)/sizeof(TString); ic++)
	{
	  for(size_t isc=0; isc<sizeof(subCats)/sizeof(TString); isc++)
	    {
	      for(size_t itc=0; itc<topCats.size(); itc++)
		{
		  TString ctf=cats[ic]+subCats[isc]+topCats[itc];

		  //all Z candidates
		  controlHistos.fillHisto("eventflow",ctf,1,weight);
		  controlHistos.fillHisto("zpt",ctf, zll.pt(),weight);
		  controlHistos.fillHisto("zeta",ctf, zll.eta(),weight);
		  controlHistos.fill2DHisto("zptvszeta", ctf,zll.pt(),zll.eta(),weight);
		  controlHistos.fillHisto("zmass",ctf, zll.mass(),weight);
		  controlHistos.fillHisto("deltazpt",ctf, zll.pt()-genzll.pt(),weight);
		  controlHistos.fillHisto("deltazphi",ctf, deltaPhi(zll.phi(),genzll.phi()),weight);

		  //3-rd lepton veto
		  if(ev.ln) continue;
		  controlHistos.fillHisto("eventflow",ctf,2,weight);
		  controlHistos.fillHisto("nbtags",ctf, nbtags,weight);
		  
		  //b veto
		  if(nbtags)  continue;
		  controlHistos.fillHisto("eventflow",ctf,3,weight);

// 		  controlHistos.fillHisto("deltadilLvszpt",ctf, zll.pt(), fabs(deltaDileptonL),weight);
// 		  controlHistos.fillHisto("deltadilTvszpt",ctf, zll.pt(), fabs(deltaDileptonT),weight);
// 		  controlHistos.fillHisto("metjer", ctf,metJER.pt(),weight);
// 		  controlHistos.fillHisto("metjesup", ctf,metJESup.pt(),weight);
// 		  controlHistos.fillHisto("metjesdown", ctf,metJESdown.pt(),weight);
// 		  controlHistos.fillHisto("metflatpu", ctf,zvv.pt());
// 		  if(ev.ngenpu<1) controlHistos.fillHisto("metnopu", ctf,zvv.pt());
		  
		  controlHistos.fillHisto("deltazvvpt",ctf, zvv.pt()-genzvv.pt(),weight);
		  controlHistos.fillHisto("deltazvvphi",ctf, deltaPhi(zvv.phi(),genzvv.phi()),weight);

// 		  controlHistos.fillHisto("projmetjer", ctf,projMetJER,weight);	      
// 		  controlHistos.fillHisto("projmetjesup", ctf,projMetJESup,weight);	      
// 		  controlHistos.fillHisto("projmetjesdown", ctf,projMetJESdown,weight);	     
// 		  controlHistos.fillHisto("projmetflatpu", ctf,projMet);	     
// 		  if(ev.ngenITpu<1) controlHistos.fillHisto("projmetnopu", ctf,projMet); 
		  
// 		  controlHistos.fillHisto("minmetjer", ctf,minmetJER,weight);	      
// 		  controlHistos.fillHisto("minmetjesup", ctf,minmetJESup,weight);	      
// 		  controlHistos.fillHisto("minmetjesdown", ctf,minmetJESdown,weight);	      
// 		  controlHistos.fillHisto("minmetflatpu", ctf,minmet);

		  controlHistos.fillHisto("met", ctf,zvv.pt(),weight);
		  controlHistos.fillProfile("metprof", ctf,ev.ngenITpu,zvv.pt());
		  controlHistos.fill2DHisto("metvspu", ctf,ev.ngenITpu,zvv.pt());
		  controlHistos.fillHisto("projmet", ctf,projMet,weight);	 
		  controlHistos.fillProfile("projmetprof", ctf,ev.ngenITpu,projMet);
		  controlHistos.fill2DHisto("projmetvspu", ctf,ev.ngenITpu,projMet);     
		  controlHistos.fillHisto("minmet", ctf,minmet,weight);	      
		  controlHistos.fillProfile("minmetprof", ctf,ev.ngenITpu,minmet);	      
		  controlHistos.fill2DHisto("minmetvspu", ctf,ev.ngenITpu,minmet);	      
 		  controlHistos.fillHisto("redmet", ctf,redmet,weight);	      
		  controlHistos.fillProfile("redmetprof", ctf,ev.ngenITpu,redmet);
		  controlHistos.fill2DHisto("redmetvspu", ctf,ev.ngenITpu,redmet);
 		  controlHistos.fillHisto("superredmet", ctf,superredmet,weight);	      
 		  controlHistos.fillHisto("superredmetprof", ctf,ev.ngenITpu,superredmet,weight);	      
 		  controlHistos.fillHisto("superredmetvspu", ctf,ev.ngenITpu,superredmet,weight);	      

 		  controlHistos.fillHisto("redmetL", ctf,redmetL,weight);	      
 		  controlHistos.fillHisto("redmetT", ctf,redmetT,weight);	      
		  controlHistos.fillHisto("redmetcomps", ctf,redmetL,redmetT,weight);	

		  //		  if(jetsp4.size() && passMedium) controlHistos.fillHisto("mindphimetjet", ctf,mindphijmet,weight);	      
		  // 		  controlHistos.fillHisto("redmetjer", ctf,redmetJER,weight);	      
		  // 		  controlHistos.fillHisto("redmetjesup", ctf,redmetJESup,weight);	      
		  // 		  controlHistos.fillHisto("redmetjesdown", ctf,redmetJESdown,weight);	      
		  // 		  controlHistos.fillHisto("redmetflatpu", ctf,redmet);
		  // 		  if(ev.ngenITpu<1)
		  // 		    {
		  // 		      controlHistos.fillHisto("minmetnopu", ctf,minmet);
		  // 		      controlHistos.fillHisto("redmetnopu", ctf,redmet);
		  // 		    }
		  
		  bool passMediumRedMet(redmet>39);
		  bool passTightRedMet(redmet>57);
		  
		  //save summary tree now
		  if(saveSummaryTree && ev.normWeight==1)
		    {
		      ev.pass=passMediumRedMet+passTightRedMet;
		      ev.weight=summaryWeight;		      
		      spyHandler->fillTree();
		    }
	  
		  //red-met cut
		  if(passMediumRedMet)  continue;
		  controlHistos.fillHisto("eventflow",ctf,4,weight);
		  if(passTightRedMet)  controlHistos.fillHisto("eventflow",ctf,5,weight);

		  controlHistos.fillHisto("metvszpt", ctf,zvv.pt(),zll.pt(),weight);
		  controlHistos.fillHisto("projmetvszpt", ctf,projMet,zll.pt(),weight);
		  controlHistos.fillHisto("minmetvszpt", ctf,minmet,zll.pt(),weight); 
		  controlHistos.fillHisto("redmetvszpt", ctf,redmet,zll.pt(),weight); 

		  controlHistos.fillHisto("metoverzpt", ctf,metoverzpt,weight);	
		  controlHistos.fillHisto("projmetoverzpt", ctf,projmetoverzpt,weight);
		  controlHistos.fillHisto("minmetoverzpt", ctf,minmetoverzpt,weight);		
		  controlHistos.fillHisto("redmetoverzpt", ctf,redmetoverzpt,weight);	

		  controlHistos.fillHisto("mtsum",ctf,mtsum,weight);
		  controlHistos.fillHisto("dphill",ctf, dphill,weight);
		  controlHistos.fillHisto("mindrlz",ctf, mindrlz,weight);
		  controlHistos.fillHisto("maxdrlz",ctf, maxdrlz,weight);
		  controlHistos.fillHisto("drll",ctf, drll,weight);
		  controlHistos.fillHisto("dphizz",ctf, deltaPhi(zll.phi(),zvv.phi()), weight);

 		  controlHistos.fillHisto("finalredmetL", ctf,redmetL,weight);	      
 		  controlHistos.fillHisto("finalredmetT", ctf,redmetT,weight);	      
		  controlHistos.fillHisto("finalredmetcomps", ctf,redmetL,redmetT,weight);	
    
		  //debug
		  if(ic==0 && isc==0 && itc==0 && !isMC && redmet>150)	
		    {
		      *outf << "<b>Selected event</b>"<< "<br/>" << std::endl;
		      *outf << "%$Run=" <<  ev.run << "$% %$Lumi=" << ev.lumi << "$% %$Event=" << ev.event <<"$%" << "<br/>" << std::endl;
  
		      *outf << "<i>Leptons</i>" << "<br/>" << std::endl;
		      for(size_t ilep=0; ilep<2; ilep++)
			*outf << "%$l_{" << ilep+1 << "}=" << phys.leptons[ilep].id << "$% "
			      << "%$p_T=" << phys.leptons[ilep].pt() << "$% "
			      << "%$\\eta=" << phys.leptons[ilep].eta() << "$% "
			      << "%$\\phi=" << phys.leptons[ilep].phi() << "$% "
			      << "%$I_{neut}=" << phys.leptons[ilep].iso1 << "$% "
			      << "%$I_{ch}=" << phys.leptons[ilep].iso2  << "$% "
			      << "%$I_{pho}=" << phys.leptons[ilep].iso3 << "$% " << "<br/>" << std::endl; 
 
		      *outf << "<i>Dilepton</i>" << "<br/>" << std::endl;
		      *outf  << "%$p_{T}^{ll}=" << zll.pt() << "$% "
			     << "%$\\eta^{ll}=" << zll.eta() << "$% "
			     << "%$\\phi^{ll}=" << zll.phi() << "$% "
			     << "%$m^{ll}=" << zll.mass() << "$% "   
			     << "%$\\Delta R(l,l)=" << drll << "$% "
			     << "%$\\Delta\\phi(l,l)=" << dphill << "$% " << "<br/>" << std::endl;
  
		      *outf << "<i>Missing transverse energy</i>" << "<br/>" << std::endl;
		      *outf << "%$E_{T}^{miss}=" << zvv.pt() << "$% "
			    << "%$\\phi=" << zvv.phi() << "$% " << "<br/>" << std::endl;
		      *outf << "%$red-E_{T}^{miss}=" << redmet << "$% "
			    << "%$l="<< redmetL << "$% "
			    << "%$t=" << redmetT << "$% " << "<br/>" << std::endl;
  
		      *outf << "<i>Transverse mass</i>" << "<br/>" << std::endl;
		      *outf << "%$\\sum M_T(l,E_{T}^{miss})=" << mtsum << "$% "
			    << "%$M_T(Z,E_{T}^{miss})=" << mt << "$% " << "<br/>" << std::endl; 
  
		      *outf << "<i>Jet activity</i>" << "<br/>" << std::endl;
		      *outf << "%$N {jets}(p_T>15)= " << ev.jn << "$% "
			    << "%$\\rho=" << ev.rho << "$% "  << "<br/>" << std::endl;

		      *outf << "------" << endl;
		    }}
	      
	    }
	}
    }

  //all done with the events file
  file->Close();

  //save to file
  TString outUrl( outdir );
  gSystem->Exec("mkdir -p " + outUrl);
  outUrl += "/";
  outUrl += gSystem->BaseName(url);
  
  TFile *ofile=TFile::Open(outUrl, "recreate");
  TDirectory *baseOutDir=ofile->mkdir("localAnalysis");
  SelectionMonitor::StepMonitor_t &mons=controlHistos.getAllMonitors();
  std::map<TString, TDirectory *> outDirs;
  outDirs["all"]=baseOutDir->mkdir("all");
  outDirs["ee"]=baseOutDir->mkdir("ee");
  outDirs["emu"]=baseOutDir->mkdir("emu");
  outDirs["mumu"]=baseOutDir->mkdir("mumu");
  for(SelectionMonitor::StepMonitor_t::iterator it =mons.begin(); it!= mons.end(); it++)
    {
      TString icat("all");
      if(it->first.BeginsWith("ee")) icat="ee";
      if(it->first.BeginsWith("emu")) icat="emu";
      if(it->first.BeginsWith("mumu")) icat="mumu";
      outDirs[icat]->cd();
      for(SelectionMonitor::Monitor_t::iterator hit=it->second.begin(); hit!= it->second.end(); hit++)
        {
	  if(isMC && cnorm>0) hit->second->Scale(1./cnorm);
	  if( !((TClass*)hit->second->IsA())->InheritsFrom("TH2")
	      && !((TClass*)hit->second->IsA())->InheritsFrom("TGraph") )
	    fixExtremities(hit->second,true,true);
	  hit->second->Write();

        }
    }
  ofile->Close();

  if(saveSummaryTree)
    {
      spyDir->cd();
      spyHandler->getTree()->Write();
      spyFile->Write();
      spyFile->Close();
      if(!isMC) outf->close();
    }
}  
