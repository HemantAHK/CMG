import operator 
import itertools
import copy

from ROOT import TLorentzVector

from CMGTools.RootTools.fwlite.Analyzer import Analyzer
from CMGTools.RootTools.fwlite.Event import Event
from CMGTools.RootTools.statistics.Counter import Counter, Counters
from CMGTools.RootTools.fwlite.AutoHandle import AutoHandle
from CMGTools.RootTools.physicsobjects.PhysicsObjects import Jet, GenParticle

from CMGTools.LEP3.analyzers.DiObject import DiObject
from CMGTools.LEP3.analyzers.EMiss import EMiss, EVis

from CMGTools.RootTools.utils.DeltaR import deltaR
from math import pi, sqrt, acos
        
class EMissAnalyzer( Analyzer ):

    def declareHandles(self):
        super(EMissAnalyzer, self).declareHandles()

        self.handles['jets'] = AutoHandle ('cmgPFJetSel',
                                           'std::vector<cmg::PFJet>')

        self.mchandles['genParticles'] = AutoHandle( 'genParticlesPruned',
                                                     'std::vector<reco::GenParticle>' )



        
    def beginLoop(self):
        super(EMissAnalyzer,self).beginLoop()
        self.counters.addCounter('EMiss')
        self.counters.addCounter('EMissGen')
        self.counters.addCounter('WWHGen')
        counts = []
        counts.append(self.counters.counter('EMiss'))
        counts.append(self.counters.counter('EMissGen'))
        counts.append(self.counters.counter('WWHGen'))
        for count in counts : 
            count.register('All events')
            count.register('ptMiss > 10.')
            count.register('mVis < 180.')
            count.register('ctMiss > 1.00')
            count.register('mMiss > 0.')
            count.register('Two Good Jets')
            count.register('passing')
        
    def buildMiss(self, event):

        self.nunubb = False
        self.eleele = False
        self.mumu = False
        self.tautau = False
        self.wwh = False
        self.neutrinos = []
        self.bquarks = []
        self.electrons = []
        self.muons = []
        self.taus = []
        for ptc in self.mchandles['genParticles'].product():
            isNeutrino = abs(ptc.pdgId()) == 12 or abs(ptc.pdgId()) == 14 or abs(ptc.pdgId()) == 16
            isElectron = abs(ptc.pdgId()) == 11
            isMuon = abs(ptc.pdgId()) == 13
            isTau = abs(ptc.pdgId()) == 15
            isbquark = abs(ptc.pdgId()) == 5
            if isNeutrino : 
                if ptc.numberOfMothers() and ptc.mother(0).pdgId() != 23 : continue
                self.neutrinos.append(GenParticle(ptc))
            if isElectron : 
                if ptc.numberOfMothers() and ptc.mother(0).pdgId() != 23 : continue
                self.electrons.append(GenParticle(ptc))
            if isMuon : 
                if ptc.numberOfMothers() and ptc.mother(0).pdgId() != 23 : continue
                self.muons.append(GenParticle(ptc))
            if isTau : 
                if ptc.numberOfMothers() and ptc.mother(0).pdgId() != 23 : continue
                self.taus.append(GenParticle(ptc))
            elif isbquark : 
                if not(ptc.numberOfMothers()) : continue
                if ptc.mother(0).pdgId() != 25 : continue
                self.bquarks.append(GenParticle(ptc))

        if len(self.neutrinos) == 2 and len(self.bquarks) == 2 :
            self.nunubb = True
            self.wwh = not(self.neutrinos[0].numberOfMothers())

        if len(self.electrons) == 2 :
            self.eleele = True
        if len(self.muons) == 2 :
            self.mumu = True
        if len(self.taus) == 2 :
            self.taus = True
##            for ptc in self.mchandles['genParticles'].product():
##                print GenParticle(ptc)

        self.jets = []
        for ptj in self.handles['jets'].product():
            self.jets.append(Jet(ptj))

        self.eMiss = EMiss(self.jets)
        self.eVis = EVis(self.jets)
        #if self.nunubb : print self.eMiss

        


    def buildJetList(self, event):

        self.jets = []
        self.jet3 = []
        for ptj in self.handles['jets'].product():
            self.jets.append(Jet(ptj))
            #print jet, jet.nConstituents(), jet.component(1).number(), Jet(ptj).component(1).fraction()

        

        if len(self.jets) > 2 : 
            self.jets.sort(key=lambda a: a.energy(), reverse = True)
            tmpJets = set(self.jets)
            tmpJet3 = set(self.jets)
#            print 'Jets Avant : '
#            for jet in tmpJets:
#                print jet, jet.nConstituents(), jet.component(1).number(), jet.component(1).fraction(), jet.mass(), jet.btag(7)
            while len(tmpJets) != 2:
                # Keep the step with three jets
                if len(tmpJet3) == 3 :
                    for jet in tmpJet3 :
                        self.jet3.append(jet)
                # 
                dijets = self.findPairs(tmpJets)
                dijets.sort(key=lambda a: a.M())
#                print dijets[0],dijets[0].M()
                
                tmpJets.remove(dijets[0].leg1)
                tmpJets.remove(dijets[0].leg2)
                tmpJets.add(dijets[0])

 #           print 'Jets apres : '
            self.jets = []
            for jet in tmpJets:
 #               print jet,jet.nConstituents(), jet.mass(), jet.btag(7)
 #               print jet,jet.nConstituents(), jet.component(1).number(), jet.component(1).fraction(), jet.mass(), jet.btag(7)
                self.jets.append(jet)
                
        self.jets.sort(key=lambda a: a.btag(7), reverse = True)


    def process(self, iEvent, event):
        self.readCollections( iEvent )

        eventNumber = iEvent.eventAuxiliary().id().event()
        #print 'Event ',eventNumber
        # creating a "sub-event" for this analyzer
        myEvent = Event(event.iEv)
        setattr(event, self.name, myEvent)
        event = myEvent
        
        self.buildMiss( event )

        event.ptMiss = self.eMiss.Pt()
        event.pMiss = self.eMiss.P()
        event.eMiss = self.eMiss.E()
        event.ctMiss = 1.
        if self.eMiss.P() > 0. : 
            event.ctMiss = self.eMiss.Pz()/self.eMiss.P()
        event.mMiss = self.eMiss.M()
        event.ptVis = self.eVis.Pt()
        event.pVis = self.eVis.P()
        event.eVis = self.eVis.E()
        event.ctVis = -1.
        if self.eVis.P() > 0. : 
            event.ctVis = self.eVis.Pz()/self.eVis.P()
        event.mVis = self.eVis.M()
        event.nunubb = self.nunubb
        event.wwh = self.wwh
        event.eleele = self.eleele
        event.mumu = self.mumu
        event.tautau = self.tautau

        event.allJets = self.jets
        event.jetPairs = self.findPairs( event.allJets )

        #for ptc in self.mchandles['genParticles'].product():
        #    print GenParticle(ptc)
        #for jet in self.jets :
        #    print jet, jet.nConstituents(), jet.component(1).number(), jet.component(1).fraction(), jet.mass(),jet.btag(7),jet.btag(0)

        self.counters.counter('EMiss').inc('All events')
        if self.nunubb : self.counters.counter('EMissGen').inc('All events')
        if self.wwh : self.counters.counter('WWHGen').inc('All events')
        event.step = 0

        if self.eMiss.Pt() < self.cfg_ana.ptmiss :
            return 0
        else:
            event.step +=1
        self.counters.counter('EMiss').inc('ptMiss > 10.')
        if self.nunubb : self.counters.counter('EMissGen').inc('ptMiss > 10.')
        if self.wwh : self.counters.counter('WWHGen').inc('ptMiss > 10.')
            
        if self.eVis.M() > self.cfg_ana.mvis :
            return 0
        else:
            event.step +=1
        self.counters.counter('EMiss').inc('mVis < 180.')
        if self.nunubb : self.counters.counter('EMissGen').inc('mVis < 180.')
        if self.wwh : self.counters.counter('WWHGen').inc('mVis < 180.')
            
        if self.eMiss.P() == 0. or abs(self.eMiss.Pz()/self.eMiss.P()) > self.cfg_ana.ctmiss  :
            return 0
        else:
            event.step +=1
        self.counters.counter('EMiss').inc('ctMiss > 1.00')
        if self.nunubb : self.counters.counter('EMissGen').inc('ctMiss > 1.00')
        if self.wwh : self.counters.counter('WWHGen').inc('ctMiss > 1.00')
            
        if self.eMiss.M() < self.cfg_ana.mmiss :
            return 0
        else:
            event.step +=1

        self.counters.counter('EMiss').inc('mMiss > 0.')
        if self.nunubb : self.counters.counter('EMissGen').inc('mMiss > 0.')
        if self.wwh : self.counters.counter('WWHGen').inc('mMiss > 0.')

        delta = self.eVis.E()*self.eVis.E() * 91.2 * 91.2 + self.eVis.p()*self.eVis.p()*(240.*240.-91.2*91.2)
        event.alpha = (self.eVis.E()*240. - sqrt(delta)) / (self.eVis.M()*self.eVis.M())

        # Force the event into two jets.
        self.buildJetList( event )        

        if len(self.jets) != 2 : 
            return 0
        else:
            event.step +=1
        self.counters.counter('EMiss').inc('Two Good Jets')
        if self.nunubb : self.counters.counter('EMissGen').inc('Two Good Jets')
        if self.wwh : self.counters.counter('WWHGen').inc('Two Good Jets')

        acol = self.jets[0].px() * self.jets[1].px() + \
               self.jets[0].py() * self.jets[1].py() + \
               self.jets[0].pz() * self.jets[1].pz()
        acol /= self.jets[0].p() * self.jets[1].p()
        acol = acos(acol)*180./pi

        acop = self.jets[0].px() * self.jets[1].px() + \
               self.jets[0].py() * self.jets[1].py() 
        acop /= self.jets[0].pt() * self.jets[1].pt()
        acop = acos(acop)*180./pi

        sumtet = 0.
        if len(self.jet3) == 3 :
            jp = self.findPairs( self.jet3 )
            for j in jp :
                angle = j.leg1.px() * j.leg2.px() + \
                        j.leg1.py() * j.leg2.py() + \
                        j.leg1.pz() * j.leg2.pz() 
                angle /= j.leg1.p() * j.leg2.p()
                angle = acos(angle)*180./pi
                sumtet += angle

        event.acol = acol
        event.acop = acop
        event.sumtet = sumtet

#        self.counters.counter('EMiss').inc('passing')
#        if self.nunubb : self.counters.counter('EMissGen').inc('passing')
#        if self.wwh : self.counters.counter('WWHGen').inc('passing')
#        event.step += 1


        return True
 

    def findPairs(self, jets):
        out = []

        for j1, j2 in itertools.combinations(jets, 2):
            out.append( DiObject(j1, j2) )

        return out
    
 
 
    
##    def testTwoJets(self, jets) :
    
##          if len(jets) != 2 :
##             #print 'NJets = ', len(jets)
##             return False
        
