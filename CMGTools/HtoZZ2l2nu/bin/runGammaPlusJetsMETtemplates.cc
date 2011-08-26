#include <iostream>
#include <boost/shared_ptr.hpp>

#include "CMGTools/HtoZZ2l2nu/interface/ZZ2l2nuSummaryHandler.h"
#include "CMGTools/HtoZZ2l2nu/interface/ZZ2l2nuPhysicsEvent.h"
#include "CMGTools/HtoZZ2l2nu/interface/ReducedMETComputer.h"
#include "CMGTools/HtoZZ2l2nu/interface/setStyle.h"
#include "CMGTools/HtoZZ2l2nu/interface/plotter.h"
#include "CMGTools/HtoZZ2l2nu/interface/ObjectFilters.h"
#include "CMGTools/HtoZZ2l2nu/interface/SelectionMonitor.h"
#include "CMGTools/HtoZZ2l2nu/interface/JetEnergyUncertaintyComputer.h"
#include "CondFormats/JetMETObjects/interface/JetResolution.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"
#include "CMGTools/HtoZZ2l2nu/interface/EventCategory.h"

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

using namespace std;

//
int main(int argc, char* argv[])
{
 // load framework libraries
  gSystem->Load( "libFWCoreFWLite" );
  AutoLibraryLoader::enable();

  //check arguments
  if ( argc < 2 ) 
    {
      std::cout << "Usage : " << argv[0] << " parameters_cfg.py" << std::endl;
      return 0;
    }

  EventCategory eventClassifComp;
  
  //configure
  const edm::ParameterSet &runProcess = edm::readPSetsFrom(argv[1])->getParameter<edm::ParameterSet>("runProcess");
  TString url=runProcess.getParameter<std::string>("input");
  TString outdir=runProcess.getParameter<std::string>("outdir");
  bool isMC = runProcess.getParameter<bool>("isMC");
  int mctruthmode = runProcess.getParameter<int>("mctruthmode"); //1-ee/mumu;  3-tautau; inclusive otherwise
  if(mctruthmode!=0)
    std::cout << "Filtering sample for : " << (mctruthmode==1? "ee/mumu" : "tautau") << " events" << endl; 
  
  int evStart=runProcess.getParameter<int>("evStart");
  int evEnd=runProcess.getParameter<int>("evEnd");
  TString dirname = runProcess.getParameter<std::string>("dirName");

  TString gammaPtWeightsFile =  runProcess.getParameter<std::string>("weightsFile"); gSystem->ExpandPathName(gammaPtWeightsFile);
  TH1 *wgtsH=0;
  bool weightPtAndJetMult(false);
  TFile *fwgt=TFile::Open(gammaPtWeightsFile);
  if(fwgt)
    {
      //2d
      wgtsH = (TH1 *)fwgt->Get("gammaptweight_1");
      wgtsH->SetDirectory(0);
      weightPtAndJetMult = ((TClass*)wgtsH->IsA())->InheritsFrom("TH2");
      fwgt->Close();
      cout << "Gamma Pt will be reweighted using distribution found @ " << gammaPtWeightsFile << endl;
    }

  //control Histos
  SelectionMonitor controlHistos;
  double qtAxis[]={20,22.5,25,27.5,
		   30,32.5,35,37.5,40,42.5,45,47.5,
		   50,52.5,55,57.5,60,62.5,65,67.5,70,72.5,
		   75,80,85,90,95,100,105,110,115,120,
		   125,150,175,200,250,300,350,400,500,600,700,800,900,1000,2000};
  int nqtBins=sizeof(qtAxis)/sizeof(double)-1;

  controlHistos.addHistogram( new TH1D ("eta", ";#eta;Events", 100,-2.6,2.6) );
  controlHistos.addHistogram( new TH1F ("npu", ";Pileup;Events", 30,0,30) );
  TString reg[]={"eb","ee"};
  for(size_t ireg=0; ireg<2; ireg++)
    {
      controlHistos.addHistogram( new TH1D (reg[ireg]+"qt", ";p_{T}^{#gamma} [GeV/c];Events / (2.5 GeV/c)", nqtBins, qtAxis) );
      controlHistos.addHistogram( new TH2D (reg[ireg]+"qtvsnjets", ";p_{T}^{#gamma} [GeV/c];Jets;Events / (2.5 GeV/c)", nqtBins, qtAxis,6,0,6) );
      controlHistos.addHistogram( new TH1F (reg[ireg]+"metoverqt", ";type I E_{T}^{miss}/p_{T}^{#gamma};Events", 100,0,2.5) );
      controlHistos.addHistogram( new TH1F (reg[ireg]+"met", ";type-I E_{T}^{miss} [GeV];Events", 100,0,200) );  
      controlHistos.addHistogram( new TH1F (reg[ireg]+"redmet", ";red-E_{T}^{miss} [GeV];Events", 100,0,200) );  
      controlHistos.addHistogram( new TH1F (reg[ireg]+"njets", ";Jets;Events", 4,0,4) );  
      controlHistos.addHistogram( new TH1F (reg[ireg]+"ptjets", ";p_{T} [GeV/c];Jets", 100,0,250) );  
      controlHistos.addHistogram( new TH1F (reg[ireg]+"dphijetmet", ";min #Delta#phi(jet,MET);Events", 100,-3.2,3.2) );  
      controlHistos.addHistogram( new TH1F (reg[ireg]+"redmetL", ";red-E_{T}^{miss,#parallel};Events", 100,-250,250) );
      controlHistos.addHistogram( new TH1F (reg[ireg]+"redmetT", ";red-E_{T}^{miss,#perp};Events", 100,-250,250) );
      controlHistos.addHistogram( (TH1D *)(new TH2D ("reg[ireg]+redmetLvsredmetT", ";red-E_{T}^{miss,#parallel};red-E_{T}^{miss,#perp};Events", 100, -251.,249,100, -251.,249.) ) );
    }
  //trigger categories
  //  Int_t photoncats[]={0,20,30,50,75,90,125};
  Int_t photoncats[]={0,20,30,50,75,125};
  const size_t nPhotonCats=sizeof(photoncats)/sizeof(Int_t);
  for(size_t icat=1; icat<nPhotonCats; icat++)
    {
      TString subcat("photon"); subcat+=photoncats[icat];
      controlHistos.initMonitorForStep(subcat);
      controlHistos.initMonitorForStep(subcat+"eq0jets");
      controlHistos.initMonitorForStep(subcat+"eq1jets");
      controlHistos.initMonitorForStep(subcat+"geq2jets");
      controlHistos.initMonitorForStep(subcat+"vbf");
    }

  //start the met computers
  ReducedMETComputer rmetComp(1., 1., 1., 1., 1.);

  //open the file and get events tree
  TFile *file = TFile::Open(url);
  if(file==0) return -1;
  if(file->IsZombie()) return -1;
  ZZ2l2nuSummaryHandler evSummaryHandler;
  cout << url << endl;
  if( !evSummaryHandler.attachToTree( (TTree *)file->Get(dirname) ) ) 
    {
      file->Close();
      return -1;
    }

  //check run range
  float rescaleFactor( evEnd>0 ?  float(evSummaryHandler.getEntries())/float(evEnd-evStart) : -1 );
  if(evEnd<0 || evEnd>evSummaryHandler.getEntries() ) evEnd=evSummaryHandler.getEntries();
  if(evStart > evEnd ) 
    {
      file->Close();
       return -1;
    }
  //run the analysis
  std::set<int> trigList;
  LorentzVector nullP4(0,0,0,0);
  double puWeights[]={0.157604, 0.439638, 0.935174, 1.53779, 1.96657, 2.14567, 2.05675, 1.76878, 1.39373, 1.03143, 0.729504, 0.497438, 0.331889, 0.212978, 0.138625, 0.0864396, 0.0530587, 0.0328417, 0.0202008, 0.0216363, 0, 0, 0, 0, 0, };  
  for( int iev=evStart; iev<evEnd; iev++)
    {
      if(iev%1000==0) printf("\r [ %d/100 ] ",int(100*float(iev-evStart)/float(evEnd)));
      evSummaryHandler.getEntry(iev);
      ZZ2l2nuSummary_t &ev=evSummaryHandler.getEvent();
      PhysicsEvent_t phys=getPhysicsEventFrom(ev);

      int eventCategory = eventClassifComp.Get(phys);
      TString subcat    = eventClassifComp.GetLabel(eventCategory);
      
      bool isGammaEvent(ev.cat>3 && phys.gamma.pt()>0);
      float weight=puWeights[ev.ngenITpu]; //ev.weight;
      
      //event categories
      LorentzVector gamma(0,0,0,0);
      TString evcat("photon");
      int triggerThr(0);
      if(isGammaEvent)
	{
	  gamma=phys.gamma;
	  
	  triggerThr=(ev.cat-22)/1000;
	  trigList.insert(triggerThr);
	  
	  bool rejectEvent(false);
	  for(size_t icat=0; icat<nPhotonCats-1; icat++)
	    {
	      if(photoncats[icat]!=triggerThr) continue;
	      if(gamma.pt()<photoncats[icat] || gamma.pt()>=photoncats[icat+1]) rejectEvent=true;
	    }
	  if(rejectEvent) continue;
	}
      else
	{
	  gamma=phys.leptons[0]+phys.leptons[1];
	  if(fabs(gamma.mass()-91)>15) continue;
	  if(ev.cat != dilepton::MUMU && ev.cat!=dilepton::EE) continue;
	  if(mctruthmode==1 && ev.mccat!=gen::DY_EE && ev.mccat!=gen::DY_MUMU)  continue;
	  if(mctruthmode==2 && ev.mccat!=gen::DY_TAUTAU) continue;
	  
	  //find the trigger - threshold category (assume 100% efficiency...) 
	  if(gamma.pt()>=photoncats[nPhotonCats-1]) triggerThr=photoncats[nPhotonCats-1];
	  else
	    {
	      size_t icat=0;
	      for(; icat<nPhotonCats-1; icat++)	
		if(gamma.pt()>=photoncats[icat] &&
		   gamma.pt()<photoncats[icat+1]) break;
	      triggerThr=photoncats[icat];
	    }
	}

      //minimum threshold
      if(gamma.pt()<20) continue;
      TString region("eb");
      if(fabs(gamma.eta())>1.47) region="ee";

      evcat += triggerThr;

      int njets30(0),njets(0);
      for(size_t ijet=0; ijet<phys.jets.size(); ijet++)
	{
	  njets   += (phys.jets[ijet].pt()>15 && fabs(phys.jets[ijet].eta())<2.5);
	  njets30 += (phys.jets[ijet].pt()>30);
	}

      //reweight to reproduce pt weight
      double ptweight(1.0);
      if(isGammaEvent && wgtsH)
 	{
	  //take the last bin weight if pT>max available
	  for(int ibin=1; ibin<=wgtsH->GetXaxis()->GetNbins(); ibin++)
	    {
	      if(gamma.pt()<wgtsH->GetXaxis()->GetBinLowEdge(ibin) ) break;
	      if(weightPtAndJetMult)
		{
		  int jbin=njets+1;
		  if(jbin>wgtsH->GetYaxis()->GetNbins()) jbin=wgtsH->GetYaxis()->GetNbins();
		  ptweight = wgtsH->GetBinContent(ibin,jbin);
		}
	      else
		{
		  ptweight = wgtsH->GetBinContent(ibin);
		}
	    }
	}
      weight *= ptweight;
      
      //jet/met kinematics and systematic variations
      LorentzVector met=phys.met[0];
      LorentzVectorCollection jetsp4;
      double mindphijetmet(1000.);
      for(size_t ijet=0; ijet<phys.jets.size(); ijet++) 
	{
	  double dphi=deltaPhi(phys.jets[ijet].phi(),met.phi());
	  if(fabs(dphi)<fabs(mindphijetmet)) mindphijetmet=dphi;
	  jetsp4.push_back( phys.jets[ijet] );
	}

      //redmet
      rmetComp.compute(gamma,0,nullP4,0, jetsp4, met, true );
      double redmet = rmetComp.reducedMET(ReducedMETComputer::INDEPENDENTLYMINIMIZED);
      double redmetL = rmetComp.reducedMETComponents(ReducedMETComputer::INDEPENDENTLYMINIMIZED).second;
      double redmetT = rmetComp.reducedMETComponents(ReducedMETComputer::INDEPENDENTLYMINIMIZED).first;

      //fill control histograms
      TString cats[]={"all",evcat};
      TString subCats[]={"",subcat};
      for(size_t ic=0; ic<sizeof(cats)/sizeof(TString); ic++)
	{
	  for(size_t isc=0; isc<sizeof(subCats)/sizeof(TString); isc++)
	    {
	      TString ctf=cats[ic]+subCats[isc];
	      
	      controlHistos.fillHisto("npu",ctf, ev.ngenITpu,weight);
	      controlHistos.fillHisto("eta",ctf, gamma.eta(),weight,true);

	      if(jetsp4.size()) controlHistos.fillHisto(region+"dphijetmet",ctf, mindphijetmet,weight);
	      controlHistos.fillHisto(region+"njets",ctf, njets,weight);
	      controlHistos.fill2DHisto(region+"qtvsnjets",ctf, gamma.pt(), njets,weight,true);
	      controlHistos.fillHisto(region+"qt",ctf, gamma.pt(),weight,true);
	     
	      controlHistos.fillHisto(region+"met", ctf, met.pt(),weight);
	      controlHistos.fillHisto(region+"redmet", ctf, redmet,weight);
	      controlHistos.fillHisto(region+"metoverqt", ctf,met.pt()/gamma.pt(),weight);	      
 	      controlHistos.fillHisto(region+"redmetL", ctf,redmetL,weight);	      
	      controlHistos.fillHisto(region+"redmetT", ctf,redmetT,weight);	      
	      controlHistos.fill2DHisto(region+"redmetLvsredmetT", ctf,redmetL,redmetT,weight);	      

	      for(size_t ijet=0; ijet<phys.jets.size(); ijet++)
		{
		  if (phys.jets[ijet].pt()>15 && fabs(phys.jets[ijet].eta())<2.5)
		    controlHistos.fillHisto(region+"ptjets",ctf, phys.jets[ijet].pt(),weight);
		}
	    }
	}
    }
  
  
  
  //MC normalization (to 1/pb)
  float cnorm=1.0;
  if(isMC)
    {
      TH1F *cutflowH = (TH1F *) file->Get("evAnalyzer/h2zz/photon/photon_cutflow");
      if(cutflowH==0) cutflowH = (TH1F *) file->Get("evAnalyzer/h2zz/cutflow");
      if(cutflowH)
	cnorm=cutflowH->GetBinContent(1);
      cout << "The initial number of events was " << cnorm << endl;
      if(rescaleFactor>0) 
	{
	  cout << "Warning: rescaling by " << cnorm/rescaleFactor << " because we have run partially on this sample" << endl;
	  cnorm /= rescaleFactor;
	}
    }

  for(std::set<int>::iterator it = trigList.begin();
      it != trigList.end();
      it++)
    cout << *it << " ";
  cout << endl;

  //detach all histograms from the file
  SelectionMonitor::StepMonitor_t &mons=controlHistos.getAllMonitors();
  for(SelectionMonitor::StepMonitor_t::iterator it =mons.begin(); it!= mons.end(); it++)
    {
      for(SelectionMonitor::Monitor_t::iterator hit=it->second.begin(); hit!= it->second.end(); hit++)
        {
	  hit->second->SetDirectory(0);
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
  std::map<TString, TDirectory *> outDirs;
  for(SelectionMonitor::StepMonitor_t::iterator it =mons.begin(); it!= mons.end(); it++)
    {
      TString moncat=it->first;
      moncat.ReplaceAll("eq0jets","");
      moncat.ReplaceAll("eq1jets","");
      moncat.ReplaceAll("geq2jets","");
      if(outDirs.find(moncat)==outDirs.end()) outDirs[moncat]=baseOutDir->mkdir(moncat);
      outDirs[moncat]->cd();
      for(SelectionMonitor::Monitor_t::iterator hit=it->second.begin(); hit!= it->second.end(); hit++)
        {
	  if( !((TClass*)hit->second->IsA())->InheritsFrom("TH1") 
	      && !((TClass*)hit->second->IsA())->InheritsFrom("TH2") 
	      && !((TClass*)hit->second->IsA())->InheritsFrom("TGraph") ) 
	    continue;
	  
	  if(isMC && !((TClass*)hit->second->IsA())->InheritsFrom("TGraph") && cnorm>0)
	    hit->second->Scale(1./cnorm);
	  
	  if( !((TClass*)hit->second->IsA())->InheritsFrom("TH2") && !((TClass*)hit->second->IsA())->InheritsFrom("TGraph") )
	    fixExtremities(hit->second,true,true);
	  
	  hit->second->Write();
	}
    }
  ofile->Close();
}  
