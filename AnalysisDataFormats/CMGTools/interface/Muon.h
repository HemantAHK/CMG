#ifndef MUON_H_
#define MUON_H_

#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

#include "AnalysisDataFormats/CMGTools/interface/TriBool.h"
#include "AnalysisDataFormats/CMGTools/interface/UnSet.h"

#include "AnalysisDataFormats/CMGTools/interface/PhysicsObject.h"
#include "AnalysisDataFormats/CMGTools/interface/Lepton.h"
#include "AnalysisDataFormats/CMGTools/interface/PatTypes.h"


#include <vector>

namespace cmg
{

//forward def needed
class Muon;
 class MuonFactory;

 class Muon : public cmg::Lepton<pat::MuonPtr>{
 public:
	
      Muon(){
      }
      Muon(const value& m):
	   cmg::Lepton<value>::Lepton(m),
	   isGlobal_(cmg::unset(cmg::TriBool())),
	   isTracker_(cmg::unset(cmg::TriBool())),
	   muonID_(cmg::unset(cmg::TriBool())),
	   pixelHits_(UnSet(int)),
	   trackerHits_(UnSet(int)),
	   globalNormChi2_(UnSet(double)),
	   dBS2D_(UnSet(double)),
	   muonHits_(UnSet(int)),
	   nMatches_(UnSet(int)){
      }
      virtual ~Muon(){
	  }
	
     //getters for VBTF selection
     cmg::TriBool isGlobal() const{
       return isGlobal_;
     }
     cmg::TriBool isTracker() const{
       return isTracker_;
     }
     cmg::TriBool muonID() const{
       return muonID_;
     }
     Int_t numberOfValidPixelHits() const{
       return pixelHits_;
     }
     Int_t numberOfValidTrackerHits() const{
       return trackerHits_;
     }
     Double_t normalizedChi2() const{
       return globalNormChi2_;
     }
     Double_t dBS2D() const{
       return dBS2D_;
     }
     Int_t numberOfValidMuonHits() const{
       return muonHits_;
     }
     Int_t numberOfMatches() const{
       return nMatches_;
     }
     // from LeafCandidate 
     virtual bool isMuon() const {
       return true;
     }
	
     friend class cmg::MuonFactory;
	
 private:

     //@see https://twiki.cern.ch/twiki/bin/viewauth/CMS/VbtfWmunuBaselineSelection
     cmg::TriBool isGlobal_;
     cmg::TriBool isTracker_;
     cmg::TriBool muonID_;
     Int_t pixelHits_;
     Int_t trackerHits_;
     //Dxy...
     Double_t globalNormChi2_;
     Double_t dBS2D_;
     Int_t muonHits_;
     Int_t nMatches_;
     //eta...

 };

}

#endif /*MUON_H_*/
