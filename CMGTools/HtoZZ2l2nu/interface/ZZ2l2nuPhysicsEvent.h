#ifndef zz2l2nuphysicsevent_h
#define zz2l2nuphysicsevent_h

#include <vector>

#include "Math/LorentzVector.h"
#include "TVector2.h"
#include "CMGTools/HtoZZ2l2nu/interface/ZZ2l2nuSummaryHandler.h"
#include "CMGTools/HtoZZ2l2nu/interface/ObjectFilters.h"

typedef ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > LorentzVector;
typedef std::vector<LorentzVector> LorentzVectorCollection;

class PhysicsObject : public LorentzVector
{
public :
  PhysicsObject(LorentzVector vec, Int_t id_):
    LorentzVector(vec), id(id_){ }
    Int_t id;
};


//
class PhysicsObject_Lepton : public LorentzVector
{
 public :
  PhysicsObject_Lepton(LorentzVector vec, Int_t id_,Int_t genid_=0, Float_t ptErr_=0, 
		       Float_t ecalIso_=0, Float_t hcalIso_=0, Float_t trkIso_=0, 
		       Float_t gIso_=0,     Float_t chIso_=0,    Float_t puchIso_=0, Float_t nhIso_=0,
		       Float_t pid_=0):
    LorentzVector(vec), id(id_), genid(genid_), ptErr(ptErr_), 
    ecalIso(ecalIso_), hcalIso(hcalIso_), trkIso(trkIso_), gIso(gIso_), chIso(chIso_), puchIso(puchIso_), nhIso(nhIso_),
    pid(pid_) { }

    Int_t id,genid;
    Float_t ptErr, ecalIso, hcalIso, trkIso, gIso, chIso, puchIso, nhIso;
    Int_t  pid;
};


class PhysicsObject_Jet : public LorentzVector
{
public :
  PhysicsObject_Jet(LorentzVector vec, Int_t genid_=0, Float_t btag1_=0, Float_t btag2_=0, Float_t neutHadFrac_=0, Float_t neutEmFrac_=0, Float_t chHadFrac_=0, Int_t pid_=0):
    LorentzVector(vec), genid(genid_), btag1(btag1_), btag2(btag2_), neutHadFrac(neutHadFrac_), neutEmFrac(neutEmFrac_), chHadFrac(chHadFrac_), pid(pid_) 
    {
      genPt=0;
      pumva=-9999.;
    }
    void setGenPt(float val) { if(val<0 || val>1000) genPt=0; else genPt=val; }
    void setPUmva(float val) { pumva=val; }
    Int_t genid;
    Float_t btag1, btag2, neutHadFrac, neutEmFrac, chHadFrac, genPt,pumva;
    Int_t pid;
};
bool JetPtOrdering (PhysicsObject_Jet i,PhysicsObject_Jet j);


class PhysicsObject_Gamma : public LorentzVector
{ 
public : 
  PhysicsObject_Gamma(LorentzVector vec, Float_t ptErr_=0, Float_t iso1_=0, Float_t iso2_=0, Float_t iso3_=0, Float_t sihih_=0, Float_t r9_=0, Float_t hoe_=0):
    LorentzVector(vec), iso1(iso1_), iso2(iso2_), iso3(iso3_), sihih(sihih_), r9(r9_), hoe(hoe_) 
    { 
      hasCtfTrkVeto=false;
      scEnSF=1.0; scEnSFerr=0;
    }
    inline void setSCcorrections(float scEnCorrected,float scEnCorrectedError)
      {
	scEnSF    = scEnCorrected/this->energy();
	scEnSFerr = scEnCorrectedError/this->energy();
      }
    inline void setID(int id)
      {
	hasCtfTrkVeto=false;
      }
    Bool_t hasCtfTrkVeto;
    Bool_t isConv,convMatchesPrimVertex;
    LorentzVector convP4;
    Float_t iso1, iso2, iso3, sihih, r9, hoe;
    Float_t scEnSF,scEnSFerr;
};

typedef std::vector<PhysicsObject>        PhysicsObjectCollection;
typedef std::vector<PhysicsObject_Lepton> PhysicsObjectLeptonCollection;
typedef std::vector<PhysicsObject_Jet>    PhysicsObjectJetCollection;
typedef std::vector<PhysicsObject_Gamma>  PhysicsObjectGammaCollection;

//
struct PhysicsEvent_t
{
  float puWeight;
  int cat;
  int nvtx;
  LorentzVectorCollection met;
  LorentzVector vtx;
  PhysicsObjectGammaCollection gammas;
  PhysicsObjectJetCollection jets, ajets;
  PhysicsObjectLeptonCollection leptons;

  PhysicsObjectCollection genjets, genleptons, genmet, genhiggs, gengammas;
};

//
PhysicsEvent_t getPhysicsEventFrom(ZZ2l2nuSummary_t &ev);
std::pair<int,int> getWZdecayMode(ZZ2l2nuSummary_t &ev);
int getNgenLeptons(int mcChannelCode, int pdgId);
int getGenProcess(int mcChannelCode);
bool isDYToLL(int mcChannelCode);
bool isDYToTauTau(int mcChannelCode);
bool isZZ2l2nu(int mcChannelCode);
bool isW(int mcChannelCode);
bool isWW(int mcChannelCode);

#endif
