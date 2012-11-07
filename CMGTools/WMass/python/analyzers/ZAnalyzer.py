import operator
import copy
import math
from CMGTools.RootTools.fwlite.Analyzer import Analyzer
from CMGTools.RootTools.statistics.Counter import Counter, Counters
from CMGTools.RootTools.fwlite.AutoHandle import AutoHandle
from CMGTools.RootTools.physicsobjects.PhysicsObjects import Muon, Jet, GenParticle
from CMGTools.RootTools.utils.TriggerMatching import triggerMatched
from CMGTools.RootTools.utils.DeltaR import bestMatch, deltaR, deltaR2

class ZAnalyzer( Analyzer ):

    ZMuonClass = Muon 
    ZJetClass = Jet 
    
    def beginLoop(self):
        super(ZAnalyzer,self).beginLoop()
        self.counters.addCounter('ZAna')
        count = self.counters.counter('ZAna')
        count.register('Z all triggered events')
        count.register('Z >= 2 lepton')
        count.register('Z at least 1 lep trig matched')
        count.register('Z good muon pair found')
        count.register('Z Inv Mass>50')
        count.register('Z pos Mu is MuIsTightAndIso')
        count.register('Z pos Mu_eta<2.1 && Mu_pt>30*MZ/MW')
        count.register('Z pfmet>25*MZ/MW')
        count.register('Z pt<20*MZ/MW')
        count.register('Z Jet_leading_pt<30')

    def buildLeptons(self, cmgMuons, event):
        '''Creates python Leptons from the muons read from the disk.
        to be overloaded if needed.'''
        return map( self.__class__.ZMuonClass, cmgMuons )

    def buildJets(self, cmgJets, event):
        '''Creates python Jets from the Jets read from the disk.
        to be overloaded if needed.'''
        return map( self.__class__.ZJetClass, cmgJets )
        
    def buildGenParticles(self, cmgGenParticles, event):
        '''Creates python GenParticles from the di-leptons read from the disk.
        to be overloaded if needed.'''
        return map( GenParticle, cmgGenParticles )
        
    def declareVariables(self):
      tr = self.tree
      var( tr, 'pfmet')
    
    def process(self, iEvent, event):
        # access event
        self.readCollections( iEvent )
        # access muons
        event.Zmuons = self.buildLeptons( self.handles['Zmuons'].product(), event )
        # access jets
        event.Zjets = self.buildJets( self.handles['Zjets'].product(), event )
        # access MET
        event.pfmet = self.handles['pfmet'].product()[0]
        # access genP
        event.genParticles = []
        if self.cfg_comp.isMC :
          event.genParticles = self.buildGenParticles( self.mchandles['genpart'].product(), event )
        # define good event bool
        event.ZGoodEvent = False
          # select event
        return self.selectionSequence(event, fillCounter=True)

        
    def selectionSequence(self, event, fillCounter):

        if fillCounter: self.counters.counter('ZAna').inc('Z all triggered events')
        
        # retrieve collections of interest (muons and jets)
        event.ZallMuons = copy.copy(event.Zmuons)
        event.ZselTriggeredMuons = []
        event.ZselNoTriggeredMuons = []
        event.ZallJets = copy.copy(event.Zjets)
        event.ZselJets = []
        
        # check if the event is MC and if genp must be saved
        event.savegenpZ=True
        if not (self.cfg_ana.savegenp and self.cfg_comp.isMC):
          event.savegenpZ=False

        # save genp only for signal events
        # i.e. only one Z is present and daughters are muons
        genZ_dummy = [ genp for genp in event.genParticles if \
                             math.fabs(genp.pdgId())==23 
                             ]
        if len(genZ_dummy)==1:
          event.genZ = [ genp for genp in genZ_dummy if \
                               math.fabs(genp.daughter(0).pdgId())==13
                               ]
          event.genMuPos = []
          event.genMuNeg = []
          if len(event.genZ)==1:
          # if the genp event is selected, associate gen muons
            if(event.genZ[0].daughter(0).pdgId()==13):
              event.genMuPos.append(event.genZ[0].daughter(0))
              event.genMuNeg.append(event.genZ[0].daughter(1))
            else:
              event.genMuPos.append(event.genZ[0].daughter(1))
              event.genMuNeg.append(event.genZ[0].daughter(0))
            
            event.genZ_mt = self.mT(event.genZ[0].daughter(0).p4() , event.genZ[0].daughter(1).p4())
            event.muPosGenDeltaRgenP=1e6
            event.muNegGenDeltaRgenP=1e6
            
          else:
            # if the genp is not signal, don't save genp but do not exit 
            # -----> events which will pass the reconstruction but are not signal
            # can be considered as background (for example, in Z+Jets, from Z decaying into electrons, taus)
            event.savegenpZ=False
        else:
          event.savegenpZ=False

        # store event MET and jets in all gen events (necessary to make cuts in genp studies...)
        event.ZpfmetNoMu = event.pfmet.p4()
        # clean jets by removing muons
        event.ZselJets = [ jet for jet in event.ZallJets if not \
                                (bestMatch( jet , event.ZallMuons ))[1] <0.5
                          ]

        # reco events must have good reco vertex and trigger fired...                          
        if not (event.HasGoodVertices and event.HasTriggerFired):
          return True
        # ...and at lest two reco muons...
        if len(event.ZallMuons) < 2 :
            return True
        if fillCounter : self.counters.counter('ZAna').inc('Z >= 2 lepton')
        
        # check if the event is triggered according to cfg_ana
        # store separately muons that fired the trigger
        if len(self.cfg_comp.triggers)>0:
            # muon object trigger matching
            event.ZselTriggeredMuons = [lep for lep in event.ZallMuons if \
                            self.trigMatched(event, lep)]
            if len(event.ZselTriggeredMuons) == 0 :
                return True, 'trigger matching failed'
            else:
                if fillCounter : self.counters.counter('ZAna').inc('Z at least 1 lep trig matched')

        # store muons that did not fire the trigger
        event.ZselNoTriggeredMuons = [lep for lep in event.ZallMuons if \
                        not self.trigMatched(event, lep)]
        
        # check wether there are muons that did not fire the trigger, if so print some info
        # if len(event.ZselNoTriggeredMuons)>0:
        # print 'len(event.ZallMuons)= ',len(event.ZallMuons),' len(event.ZselTriggeredMuons)= ',len(event.ZselTriggeredMuons),' len(event.ZselNoTriggeredMuons)= ', len(event.ZselNoTriggeredMuons)
        
        # check for muon pair (with at least 1 triggering muon) which give invariant mass closest to Z pole
        event.BestZMuonPairList = self.BestZMuonPair(event.ZselTriggeredMuons,event.ZselNoTriggeredMuons)
        #                                            mZ                   mu1 (always firing trigger)              mu2                     mu2 has fired trigger?
        # print 'event.BestZMuonPairList= ',event.BestZMuonPairList[0],' ',event.BestZMuonPairList[1],' ',event.BestZMuonPairList[2],' ',event.BestZMuonPairList[3]
                  
        # check that a good muon pair exists, otherwise reject reco event
        if event.BestZMuonPairList[0] > 1e6 :
          return True, 'good muon pair not found (probably same charge)'
        else:
            if fillCounter : self.counters.counter('ZAna').inc('Z good muon pair found')

        # associate properly positive and negative muons
        if(event.BestZMuonPairList[1].charge()>0):
          event.BestZPosMuon = event.BestZMuonPairList[1]
          event.BestZPosMuonHasTriggered = 1
          event.BestZNegMuon = event.BestZMuonPairList[2]
          event.BestZNegMuonHasTriggered = event.BestZMuonPairList[3]
        else:
          event.BestZPosMuon = event.BestZMuonPairList[2]
          event.BestZPosMuonHasTriggered = event.BestZMuonPairList[3]
          event.BestZNegMuon = event.BestZMuonPairList[1]
          event.BestZNegMuonHasTriggered = 1
                                    
        # if the genp are saved, compute dR between gen and reco muons
        if event.savegenpZ :
          event.muPosGenDeltaRgenP = deltaR( event.BestZPosMuon.eta(), event.BestZPosMuon.phi(), event.genMuPos[0].eta(), event.genMuPos[0].phi() ) 
          event.muNegGenDeltaRgenP = deltaR( event.BestZNegMuon.eta(), event.BestZNegMuon.phi(), event.genMuNeg[0].eta(), event.genMuNeg[0].phi() ) 
            
        # associate lepton to good vertex to muons to compute dxy
        event.BestZPosMuon.associatedVertex = event.goodVertices[0]
        event.BestZNegMuon.associatedVertex = event.goodVertices[0]
        
        # testing offline muon cuts (tight+iso, no kinematical cuts)
        event.BestZPosMuonIsTightAndIso=0
        if self.testLeg( event.BestZPosMuon ):
            event.BestZPosMuonIsTightAndIso=1
        event.BestZNegMuonIsTightAndIso=0
        if self.testLeg( event.BestZNegMuon ):
            event.BestZNegMuonIsTightAndIso=1
        event.BestZPosMuonIsTight=0
        if self.testLegID( event.BestZPosMuon ):
            event.BestZPosMuonIsTight=1
        event.BestZNegMuonIsTight=0
        if self.testLegID( event.BestZNegMuon ):
            event.BestZNegMuonIsTight=1
                
        # if event.BestZPosMuon.sourcePtr().innerTrack():
          # event.BestZPosMuon_dxy = event.BestZPosMuon.dxy()
        # if event.BestZNegMuon.sourcePtr().innerTrack():
          # event.BestZNegMuon_dxy = event.BestZNegMuon.dxy()

        # assign negative lepton to MET to build W+
        event.ZpfmetWpos = event.ZpfmetNoMu + event.BestZNegMuon.p4()
        # assign positive lepton to MET to build W-
        event.ZpfmetWneg = event.ZpfmetNoMu + event.BestZPosMuon.p4()        

        # define a positive W from positive lepton and MET
        event.Wpos4VfromZ = event.BestZPosMuon.p4() + event.ZpfmetWpos
        event.Wpos4VfromZ_mt = self.mT(event.BestZPosMuon.p4() , event.ZpfmetWpos)
        # define a negative W from negative lepton and MET
        event.Wneg4VfromZ = event.BestZNegMuon.p4() + event.ZpfmetWneg
        event.Wneg4VfromZ_mt = self.mT(event.BestZNegMuon.p4() , event.ZpfmetWneg)
        # define a Z from the two leptons
        event.Z4V = event.BestZPosMuon.p4() + event.BestZNegMuon.p4()
        event.Z4V_mt = self.mT(event.BestZPosMuon.p4() , event.BestZNegMuon.p4())
        
        # Code to study the recoil
        metVect = event.ZpfmetNoMu.Vect()
        metVect.SetZ(0.) # use only transverse info
        ZVect = event.Z4V.Vect()
        ZVect.SetZ(0.) # use only transverse info
        recoilVect = copy.deepcopy(metVect)
        recoilVect -= ZVect
        
        uZVect = ZVect.Unit()
        zAxis = type(ZVect)(0,0,1)
        uZVectPerp = ZVect.Cross(zAxis).Unit()

        u1 = - recoilVect.Dot(uZVect) # recoil parallel to Z pt
        u2 = recoilVect.Dot(uZVectPerp) # recoil perpendicular to Z pt

        event.Zu1 = u1
        event.Zu2 = u2

        if fillCounter:
          if event.Wpos4VfromZ.M() > 50: 
            self.counters.counter('ZAna').inc('Z Inv Mass>50')
            if event.BestZPosMuonIsTightAndIso : 
              self.counters.counter('ZAna').inc('Z pos Mu is MuIsTightAndIso')
              if self.testLegKine( event.BestZPosMuon , 30*91.1876/80.385 , 2.1 ) : 
                self.counters.counter('ZAna').inc('Z pos Mu_eta<2.1 && Mu_pt>30*MZ/MW')
                if event.ZpfmetWpos.Pt() >25*91.1876/80.385: 
                  self.counters.counter('ZAna').inc('Z pfmet>25*MZ/MW')
                  if event.Wpos4VfromZ.Pt() < 20*91.1876/80.385: 
                    self.counters.counter('ZAna').inc('Z pt<20*MZ/MW')
                    if len(event.ZselJets) > 0: 
                      if event.ZselJets[0].pt() < 30: 
                        self.counters.counter('ZAna').inc('Z Jet_leading_pt<30')
                    else:
                      self.counters.counter('ZAna').inc('Z Jet_leading_pt<30')

        
        # event is fully considered as good
        # self.counters.counter('ZAna').inc('Z pass')
        event.ZGoodEvent = True
        return True
        
        
    def declareHandles(self):        
        super(ZAnalyzer, self).declareHandles()
        self.handles['cmgTriggerObjectSel'] =  AutoHandle(
            'cmgTriggerObjectSel',
            'std::vector<cmg::TriggerObject>'
            )
        self.handles['Zmuons'] = AutoHandle(
            'cmgMuonSel',
            'std::vector<cmg::Muon>'
            )
        self.handles['Zjets'] = AutoHandle(
            'cmgPFJetSel',
            'std::vector<cmg::PFJet>'
            )
        self.handles['pfmet'] = AutoHandle(
          'cmgPFMET',
          'std::vector<cmg::BaseMET>' 
          )
        self.mchandles['genpart'] =  AutoHandle(
            'genParticlesPruned',
            'std::vector<reco::GenParticle>'
            )


    def testJet(self, jet):
        '''returns testjetID && testjetIso && testjetKine for jet'''
        return jet.pt() > self.cfg_ana.jetptcut

    def testJetIsNotSelLepton(self, jet, lep):
        '''returns testjetID && testjetIso && testjetKine for jet'''
        return deltaR( lep.p4().Eta(),lep.p4().Phi(), jet.p4().Eta(),jet.p4().Phi() )  > 0.2

    def testLeg(self, leg):
        '''returns testLegID && testLegIso && testLegKine for leg'''
        return self.testLegID(leg) and \
               self.testLegIso(leg, self.cfg_ana.iso)
               # and self.testLegKine(leg, self.cfg_ana.pt, self.cfg_ana.eta)


    def testLegID(self, leg):
        '''Always return true by default, overload in your subclass'''
        # return True
        return (leg.tightId() and \
                leg.dxy() < 0.2 and \
                leg.dz() < 0.5 )
                
                
    def testLegIso(self, leg, isocut):
        '''If isocut is None, the iso value is taken from the iso1 parameter.
        Checks the standard dbeta corrected isolation.
        '''
        if isocut is None:
            isocut = self.cfg_ana.iso
        return leg.relIso(0.5) < isocut
        
        
    def testLegKine(self, leg, ptcut, etacut ):
        '''Tests pt and eta.'''
        return leg.pt() > ptcut and \
               abs(leg.eta()) < etacut 
       

    def BestZMuonPair(self, ZselTriggeredMuons, ZselNoTriggeredMuons):
      mZpole = 91.1876
      mZ=1e10
      bestmu1=0
      bestmu2=0
      mu2hastriggered=0
      for lep1 in ZselTriggeredMuons:
        for lep2 in ZselTriggeredMuons:
          if( lep1 != lep2 and lep1.charge() != lep2.charge() ):
            if(math.fabs((lep1.p4()+lep2.p4()).M()-mZpole) < math.fabs(mZ-mZpole) ):
              mZ=(lep1.p4()+lep2.p4()).M()
              bestmu1=lep1
              bestmu2=lep2
              mu2hastriggered=1
      for lep1 in ZselTriggeredMuons:
        for lep2 in ZselNoTriggeredMuons:
          if( lep1 != lep2 and lep1.charge() != lep2.charge() ):
            if(math.fabs((lep1.p4()+lep2.p4()).M()-mZpole) < math.fabs(mZ-mZpole) ):
              mZ=(lep1.p4()+lep2.p4()).M()
              bestmu1=lep1
              bestmu2=lep2
              mu2hastriggered=0

      return mZ, bestmu1, bestmu2, mu2hastriggered
    
    
    
    def mT(self, leg1, leg2):
        return math.sqrt( 2*leg1.Pt()*leg2.Pt()*( 1 - (leg1.Px()*leg2.Px() + leg1.Py()*leg2.Py()) / ( leg1.Pt()*leg2.Pt() ) ) )

    def trigMatched(self, event, leg):
        '''Returns true if the leg is matched to a trigger object as defined in the
        triggerMap parameter'''
        if not hasattr( self.cfg_ana, 'triggerMap'):
            return True
        path = event.hltPath
        triggerObjects = event.triggerObjects
        filters = self.cfg_ana.triggerMap[ path ]
        filter = filters[0]
        # if legName == 'leg1':
            # filter = filters[0]
        # elif legName == 'leg2':
            # filter = filters[1]
        # else:
            # raise ValueError( 'legName should be leg1 or leg2, not {leg}'.format(
                # leg=legName )  )
        # the dR2Max value is 0.3^2
        pdgIds = None
        if len(filter) == 1:
            filter, pdgIds = filter[0], filter[1]
        return triggerMatched(leg, triggerObjects, path, filter,
                              # dR2Max=0.089999,
                              dR2Max=0.25,
                              pdgIds=pdgIds )