import FWCore.ParameterSet.Config as cms

from PhysicsTools.PatAlgos.tools.jetTools import *
from PhysicsTools.PatAlgos.tools.trigTools import *
from PhysicsTools.PatAlgos.tools.coreTools import *

def myPatSequence( process, runOnData=True  ): 

    if runOnData:
        switchOnTrigger( process )
        removeMCMatching(process, ['All'])
        #RunOnData(process, ['All'])


    addJetCollection(process,cms.InputTag('ak7CaloJets'),
                 'AK7', 'Calo',
                 doJTA        = True,
                 doBTagging   = False,
                 jetCorrLabel = ('AK7', 'Calo'),
                 doType1MET   = True,
                 doL1Cleaning = True,                 
                 doL1Counters = False,
                 genJetCollection=cms.InputTag("ak7GenJets"),
                 doJetID      = True,
                 jetIdLabel   = "ak7"
                 )


    addJetCollection(process,cms.InputTag('ak7PFJets'),
                  'AK7', 'PF',
                  doJTA        = True,
                  doBTagging   = False,
                  jetCorrLabel = ('AK7', 'PF'),
                  doType1MET   = True,
                  doL1Cleaning = True,                 
                  doL1Counters = False,
                  genJetCollection=cms.InputTag("ak7GenJets"),
                  doJetID      = True,
                  jetIdLabel   = "ak7"
                  )

    addJetCollection(process,cms.InputTag('ak5CaloJets'),
                 'AK5', 'Calo',
                 doJTA        = True,
                 doBTagging   = False,
                 jetCorrLabel = ('AK5', 'Calo'),
                 doType1MET   = True,
                 doL1Cleaning = True,                 
                 doL1Counters = False,
                 genJetCollection=cms.InputTag("ak5GenJets"),
                 doJetID      = True,
                 jetIdLabel   = "ak5"
                 )


    addJetCollection(process,cms.InputTag('ak5PFJets'),
                  'AK5', 'PF',
                  doJTA        = True,
                  doBTagging   = False,
                  jetCorrLabel = ('AK5', 'PF'),
                  doType1MET   = True,
                  doL1Cleaning = True,                 
                  doL1Counters = False,
                  genJetCollection=cms.InputTag("ak5GenJets"),
                  doJetID      = True,
                  jetIdLabel   = "ak5"
                  )

    process.selectedPatJetsAK7Calo.cut = "pt()>30"
    process.selectedPatJetsAK7PF.cut = "pt()>10"
    process.selectedPatJetsAK5Calo.cut = "pt()>30"
    process.selectedPatJetsAK5PF.cut = "pt()>10"

