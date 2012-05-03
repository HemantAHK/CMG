import FWCore.ParameterSet.Config as cms


from CommonTools.ParticleFlow.PFBRECO_cff import *
from CommonTools.ParticleFlow.ParticleSelectors.genericPFJetSelector_cfi import selectedPfJets
from PhysicsTools.PatAlgos.patSequences_cff import *
from CMGTools.Common.PAT.patLeptModifiedIsoDeposit_cff import *
from CMGTools.Common.analysis_cff import *

# FIXME : include type1 met corrections
# FIXME : check the taus in 44X and 52X
# FIXME : adapt pile-up jet id to 44X


# GEN              ---------------------------

# we want GenJets without neutrinos!
# will be added to the gen sequence later (see bottom of the file)
PATCMGGenJetSequence = cms.Sequence(
    genParticlesForJetsNoNu +
    ak5GenJetsNoNu 
)

# importing CMG gen sequence
from CMGTools.Common.gen_cff import *

PATCMGGenSequence = cms.Sequence(
    PATCMGGenJetSequence + 
    genSequence
    )
    

# TRIGGER          ---------------------------


from PhysicsTools.PatAlgos.triggerLayer1.triggerProducer_cff import *
patTrigger.processName = cms.string('*')
from CMGTools.Common.trigger_cff import *

PATCMGTriggerSequence = cms.Sequence(
    patTriggerDefaultSequence +
    triggerSequence
    ) 

# PU SUB AND PARTICLES FOR ISO ---------------

PATCMGPileUpSubtractionSequence = cms.Sequence(
    pfNoPileUpSequence +
    pfParticleSelectionSequence 
    )


# EVENT CLEANING   ----------------------------

from CMGTools.Common.eventCleaning.eventCleaning_cff import *
trackingFailureFilterCMG.JetSource = 'ak5PFJets'

# RHO's            ----------------------------

from RecoJets.JetProducers.kt4PFJets_cfi import kt4PFJets

kt6PFJets = kt4PFJets.clone( rParam = cms.double(0.6),
                             doAreaFastjet = cms.bool(True),
                             doRhoFastjet = cms.bool(True) )
kt6PFJetsForIso = kt6PFJets.clone( rParam = cms.double(0.6),
                                   Rho_EtaMax = cms.double(2.5),
                                   Ghost_EtaMax = cms.double(2.5) )
#NOTE: now taking the PU CHS subtracted PFCandidate collection in input,
# instead of pfNoElectron
#NOTE: cloning kt6PFJetsForIso, as the rho and ghost parameters are the same
kt6PFJetsCHSForIso = kt6PFJetsForIso.clone(
    # Rho_EtaMax = cms.double(2.5),
    # Ghost_EtaMax = cms.double(2.5),
    src = cms.InputTag("pfNoPileUpIso") )

PATCMGRhoSequence = cms.Sequence(
    # 44X needed (not present in AOD?)
    kt6PFJets + #NOTE: needed for V4
    kt6PFJetsForIso +
    kt6PFJetsCHSForIso  
    )


# MUONS           ----------------------------


patMuons.embedCaloMETMuonCorrs = False
patMuons.embedTcMETMuonCorrs = False
patMuons.embedTrack = True

# isolation (PF and custom detector based)

from CommonTools.ParticleFlow.Isolation.pfMuonIsolation_cff import *

sourceMuons = 'muons'

muPFIsoDepositCharged.src = sourceMuons
muPFIsoDepositChargedAll.src = sourceMuons
muPFIsoDepositNeutral.src = sourceMuons
muPFIsoDepositGamma.src = sourceMuons
muPFIsoDepositPU.src = sourceMuons

patMuons.isoDeposits = cms.PSet(
    pfChargedHadrons = cms.InputTag("muPFIsoDepositCharged" ),
    pfChargedAll = cms.InputTag("muPFIsoDepositChargedAll" ),
    pfPUChargedHadrons = cms.InputTag("muPFIsoDepositPU" ),
    pfNeutralHadrons = cms.InputTag("muPFIsoDepositNeutral" ),
    pfPhotons = cms.InputTag("muPFIsoDepositGamma" ),
    # 44X does not work. the name must have changed?
    # tracker = cms.InputTag("muons","muIsoDepositTk"),
    tracker = cms.InputTag("muIsoDepositTk"),
    # ecal    = cms.InputTag("muons","ecal"),
    # hcal    = cms.InputTag("muons","hcal"),
    )

# needed in 44X:
muIsoFromDepsTkOptimized.deposits[0].src = 'muIsoDepositTk'

patMuons.isolationValues = cms.PSet(
    pfChargedHadrons = cms.InputTag("muPFIsoValueCharged04"),
    pfChargedAll = cms.InputTag("muPFIsoValueChargedAll04"),
    pfPUChargedHadrons = cms.InputTag("muPFIsoValuePU04" ),
    pfNeutralHadrons = cms.InputTag("muPFIsoValueNeutral04" ),
    pfPhotons = cms.InputTag("muPFIsoValueGamma04" ),
    user = cms.VInputTag( cms.InputTag("muIsoFromDepsTkOptimized") )
    )

selectedPatMuons.cut = 'pt()>3'

from CMGTools.Common.PAT.patMuonsWithTrigger_cff import * 

cmgMuon.cfg.inputCollection = 'patMuonsWithTrigger'

PATCMGMuonSequence = cms.Sequence(
    pfMuonIsolationSequence +
    detMuonIsoDepositSequence + 
    makePatMuons +
    selectedPatMuons +
    patMuonsWithTriggerSequence + 
    muonSequence
    )

# ELECTRONS      ----------------------------

# NOTE dunno why this is needed, but necessary to run on V5 PFAOD
# - Taejeong?
patElectrons.pfElectronSource = 'particleFlow'

# PF isolation

from CommonTools.ParticleFlow.Isolation.pfElectronIsolation_cff import *

# 44X need to remove spurious sequences from the path
pfElectronIsolationSequence.remove( pfElectronIsoDepositsSequence )
pfElectronIsolationSequence.remove( pfElectronIsolationFromDepositsSequence )

sourceElectrons = 'gsfElectrons'

elPFIsoDepositCharged.src = sourceElectrons
elPFIsoDepositChargedAll.src = sourceElectrons
elPFIsoDepositNeutral.src = sourceElectrons
elPFIsoDepositGamma.src = sourceElectrons
elPFIsoDepositPU.src = sourceElectrons

patElectrons.isoDeposits = cms.PSet(
    pfChargedHadrons = cms.InputTag("elPFIsoDepositCharged" ),
    pfChargedAll = cms.InputTag("elPFIsoDepositChargedAll" ),
    pfPUChargedHadrons = cms.InputTag("elPFIsoDepositPU" ),
    pfNeutralHadrons = cms.InputTag("elPFIsoDepositNeutral" ),
    pfPhotons = cms.InputTag("elPFIsoDepositGamma" ),
    tracker = cms.InputTag("eleIsoDepositTk")
    )

electronUserIsolation  = cms.PSet(
    user = cms.VPSet(
    cms.PSet( src = cms.InputTag("eleIsoFromDepsTkOptimized5") ),
    cms.PSet( src = cms.InputTag("eleIsoFromDepsTkOptimized7") ),
    )         
)

patElectrons.isolationValues = cms.PSet(
    pfChargedHadrons = cms.InputTag("elPFIsoValueCharged04PFId"),
    pfChargedAll = cms.InputTag("elPFIsoValueChargedAll04PFId"),
    pfPUChargedHadrons = cms.InputTag("elPFIsoValuePU04PFId" ),
    pfNeutralHadrons = cms.InputTag("elPFIsoValueNeutral04PFId" ),
    pfPhotons = cms.InputTag("elPFIsoValueGamma04PFId" ),
    )

patElectrons.userIsolation.user = electronUserIsolation.user

#NOTE the following should not be used for now, but we keep them just in case.
patElectrons.isolationValuesNoPFId = cms.PSet(
    pfChargedHadrons = cms.InputTag("elPFIsoValueCharged04NoPFId"),
    pfChargedAll = cms.InputTag("elPFIsoValueChargedAll04NoPFId"),
    pfPUChargedHadrons = cms.InputTag("elPFIsoValuePU04NoPFId" ),
    pfNeutralHadrons = cms.InputTag("elPFIsoValueNeutral04NoPFId" ),
    pfPhotons = cms.InputTag("elPFIsoValueGamma04NoPFId" )
    )

# identification

from CMGTools.Common.PAT.patElectronID_cff import *
patElectrons.addElectronID = True
from CMGTools.Common.PAT.electronIDs_cfi import electronIDs
patElectrons.electronIDSources  = electronIDs.clone()

patElectrons.embedTrack = True

selectedPatElectrons.cut = 'pt()>5'

from CMGTools.Common.PAT.patElectronsWithTrigger_cff import * 

cmgElectron.cfg.inputCollection = 'patElectronsWithTrigger'

# conversions
from CMGTools.Common.PAT.patConversions_cfi import patConversions

PATCMGElectronSequence = cms.Sequence(
    pfElectronIsolationSequence +
    detElectronIsoDepositSequence + 
    patElectronIDSequence + 
    makePatElectrons +
    selectedPatElectrons +
    patElectronsWithTriggerSequence +
    patConversions +
    electronSequence
    )



# JETS NO PU CHS ----------------------------

#FIXME: is this cut really necessary? does it play well with Phil's stuff?
# NOTE cutting on uncorrected jets, but no bias for corrected jet pT>7
ak5PFJetsSel = selectedPfJets.clone( src = 'ak5PFJets',
                                     cut = 'pt()>5' )

jetSource = 'ak5PFJetsSel'

# corrections 
from PhysicsTools.PatAlgos.recoLayer0.jetCorrFactors_cfi import *
patJetCorrFactors.src = jetSource
# will need to add L2L3 corrections in the cfg
patJetCorrFactors.levels = ['L1FastJet', 'L2Relative', 'L3Absolute']
patJetCorrFactors.payload = 'AK5PF'
patJetCorrFactors.useRho = True

patJets.jetSource = jetSource
patJets.addJetCharge = False
patJets.embedCaloTowers = False
patJets.embedPFCandidates = False
patJets.addAssociatedTracks = False

# b tagging 
from RecoJets.JetAssociationProducers.ak5JTA_cff import *
ak5JetTracksAssociatorAtVertex.jets = jetSource
from RecoBTag.Configuration.RecoBTag_cff import * # btagging sequence
softMuonTagInfos.jets = jetSource
softElectronTagInfos.jets = jetSource

# parton and gen jet matching

from CommonTools.ParticleFlow.genForPF2PAT_cff import * 

from PhysicsTools.PatAlgos.mcMatchLayer0.jetMatch_cfi import *
patJetPartonMatch.src = jetSource
patJetGenJetMatch.src = jetSource
patJetGenJetMatch.matched = 'ak5GenJetsNoNu'

from PhysicsTools.PatAlgos.mcMatchLayer0.jetFlavourId_cff import *
patJetPartonAssociation.jets = jetSource

# FIXME is it ok for jet studies?
selectedPatJets.cut = 'pt()>10'

cmgPFJet.cfg.inputCollection = 'selectedPatJets'
cmgPFBaseJet.cfg.inputCollection = 'selectedPatJets'

# Pile-up jet ID
from  CMGTools.External.pujetidsequence_cff import puJetId
cmgPUJetMva.jets = 'selectedPatJets'

jetMCSequence = cms.Sequence(
    patJetPartonMatch +
    patJetGenJetMatch
    )

PATCMGJetSequence = cms.Sequence(
    ak5PFJetsSel + 
    jetMCSequence +
    ak5JetTracksAssociatorAtVertex + 
    btagging + 
    patJetCorrFactors +
    patJetFlavourId +
    patJets +
    selectedPatJets +
    puJetId +
    jetSequence
    )


# TAUS NO PU CHS  ----------------------------

from RecoTauTag.Configuration.RecoPFTauTag_cff import *
PFTau.remove( recoTauClassicShrinkingConeSequence )
PFTau.remove( recoTauClassicShrinkingConeMVASequence )
PFTau.remove( recoTauHPSTancSequence )

# will insert a jet selector before tau ID to speed up processing
jetSelectionForTaus = 'pt()>15.0 && abs(eta())<3.0'
pfJetsForHPSTau = selectedPfJets.clone( src = 'ak5PFJets',
                                        cut = jetSelectionForTaus )
# from CMGTools.Common.Tools.visitorUtils import replaceSrc
# replaceSrc( PFTau, 'ak5PFJets', 'pfJetsForHPSTau')
tauSource =  'pfJetsForHPSTau'
recoTauAK5PFJets08Region.src = tauSource #NOTE this guy has a pfSrc
ak5PFJetTracksAssociatorAtVertex.jets = tauSource
ak5PFJetsLegacyHPSPiZeros.jetSrc = tauSource
combinatoricRecoTaus.jetSrc = tauSource
tauMatch.src = 'hpsPFTauProducer'
tauGenJetMatch.src = 'hpsPFTauProducer'

tauIsoDepositPFCandidates.ExtractorPSet.tauSource = 'hpsPFTauProducer'
tauIsoDepositPFCandidates.src = 'hpsPFTauProducer'
tauIsoDepositPFChargedHadrons.ExtractorPSet.tauSource = 'hpsPFTauProducer'
tauIsoDepositPFChargedHadrons.src = 'hpsPFTauProducer'
tauIsoDepositPFNeutralHadrons.ExtractorPSet.tauSource = 'hpsPFTauProducer'
tauIsoDepositPFNeutralHadrons.src = 'hpsPFTauProducer'
tauIsoDepositPFGammas.ExtractorPSet.tauSource = 'hpsPFTauProducer'
tauIsoDepositPFGammas.src = 'hpsPFTauProducer'

# 44X : HPS taus are not yet the default. 
patTaus.tauSource = 'hpsPFTauProducer'
patTaus.tauIDSources = cms.PSet(
    # configure many IDs as InputTag <someName> = <someTag>
    # you can comment out those you don't want to save some
    # disk space
    decayModeFinding = cms.InputTag("hpsPFTauDiscriminationByDecayModeFinding"),
    byVLooseCombinedIsolationDeltaBetaCorr = cms.InputTag("hpsPFTauDiscriminationByVLooseCombinedIsolationDBSumPtCorr"),
    byLooseCombinedIsolationDeltaBetaCorr = cms.InputTag("hpsPFTauDiscriminationByLooseCombinedIsolationDBSumPtCorr"),
    byMediumCombinedIsolationDeltaBetaCorr = cms.InputTag("hpsPFTauDiscriminationByMediumCombinedIsolationDBSumPtCorr"),
    byTightCombinedIsolationDeltaBetaCorr = cms.InputTag("hpsPFTauDiscriminationByTightCombinedIsolationDBSumPtCorr"),
    againstElectronLoose = cms.InputTag("hpsPFTauDiscriminationByLooseElectronRejection"),
    againstElectronMedium = cms.InputTag("hpsPFTauDiscriminationByMediumElectronRejection"),
    againstElectronTight = cms.InputTag("hpsPFTauDiscriminationByTightElectronRejection"),
    againstElectronMVA = cms.InputTag("hpsPFTauDiscriminationByMVAElectronRejection"),
    againstMuonLoose = cms.InputTag("hpsPFTauDiscriminationByLooseMuonRejection"),
    againstMuonMedium = cms.InputTag("hpsPFTauDiscriminationByMediumMuonRejection"),
    againstMuonTight = cms.InputTag("hpsPFTauDiscriminationByTightMuonRejection")
    )


patTaus.embedLeadTrack                        = True
patTaus.embedSignalTracks                     = True
patTaus.embedIsolationTracks                  = True

patTaus.embedLeadPFCand                       = False
patTaus.embedLeadPFChargedHadrCand            = False
patTaus.embedLeadPFNeutralCand                = False
patTaus.embedSignalPFCands                    = False
patTaus.embedSignalPFChargedHadrCands         = False
patTaus.embedSignalPFNeutralHadrCands         = False
patTaus.embedSignalPFGammaCands               = False
patTaus.embedIsolationPFCands                 = False
patTaus.embedIsolationPFChargedHadrCands      = False
patTaus.embedIsolationPFNeutralHadrCands      = False
patTaus.embedIsolationPFGammaCands            = False

# need to deal with tau matching
patTaus.addGenJetMatch = True
patTaus.addGenMatch = True

# for simplicity, now selecting at the end. check if this ok in terms of speed
selectedPatTaus.cut = 'pt()>15 && tauID("decayModeFinding")'

cmgTau.cfg.inputCollection = 'selectedPatTaus'

PATCMGTauSequence = cms.Sequence(
    # add jet selection
    pfJetsForHPSTau + 
    PFTau + 
    makePatTaus +
    selectedPatTaus +
    tauSequence
    )



# JETS CHS ----------------------------

# modifying PU charged hadron subtraction according to:
# https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#JetEnCorPFnoPU
pfPileUp.checkClosestZVertex = False
pfPileUp.Vertices = 'goodOfflinePrimaryVertices'
from CommonTools.ParticleFlow.goodOfflinePrimaryVertices_cfi import goodOfflinePrimaryVertices
pfNoPileUpSequence.insert(0, goodOfflinePrimaryVertices)

# taking all PFCandidates which are not PU charged hadron
from RecoJets.JetProducers.ak5PFJets_cfi import *
ak5PFJetsCHS = ak5PFJets.clone()
ak5PFJetsCHS.src = 'pfNoPileUp'

ak5PFJetsCHS.doAreaFastjet = True
ak5PFJetsCHS.doRhoFastjet = False

#NOTE: please refer to the cfg, where the cloning of the PAT jet sequence is done
# could be rewritten here, but painful...


# JETS CHS PRUNED ----------------------------

# 44X Does not work 
# from CMGTools.Common.PAT.jetSubstructure_cff import *

# MET      ----------------------------

# 44X does not work - both METs are identical for now
#produce type 1 corrected MET
# patMETs.metSource = 'pfType1CorrectedMet'
# dammit! the PAT MET sequence contains jet clustering modules... 
# producePFMETCorrections.remove( kt6PFJets )
# producePFMETCorrections.remove( ak5PFJets )
patMETs.metSource = 'pfMet'
patMETs.addMuonCorrections = False
#now the cmg part
cmgPFMET.cfg.inputCollection = 'patMETs'

# adding raw PFMET also
patMETsRaw = patMETs.clone()
patMETsRaw.addMuonCorrections = False
patMETsRaw.metSource = 'pfMet'
#now the cmg part
cmgPFMETRaw = cmgPFMET.clone()
cmgPFMETRaw.cfg.inputCollection = 'patMETsRaw'

PATCMGMetSequence = cms.Sequence(
    # 44X does not work
    # producePFMETCorrections + 
    patMETs + 
    cmgPFMET +
    patMETsRaw +
    cmgPFMETRaw 
    )

# MET  SIGNIFICANCE    ----------------------------

from CMGTools.Common.miscProducers.metSignificance_cfi import metSignificance
pfMetSignificance = metSignificance.clone()
pfMetSignificance.inputPATElectrons = cms.InputTag('')   
pfMetSignificance.inputPATMuons = cms.InputTag('')
# pfMetSignificance.inputPFJets = cms.InputTag('') #Jose: previously used pfJets here and pfNoJet below but now pfNoJet is no loger available 
# pfMetSignificance.inputPFCandidates = cms.InputTag('particleFlow')
pfMetSignificance.inputPFJets = cms.InputTag('ak5PFJets') #Jose: previously used pfJets here and pfNoJet below but now pfNoJet is no loger available 

from CommonTools.ParticleFlow.TopProjectors.pfNoJet_cfi import *
pfNoJet.topCollection = 'ak5PFJets'
pfNoJet.bottomCollection = 'particleFlow'
pfMetSignificance.inputPFCandidates = cms.InputTag('pfNoJet')

PATCMGMetSignificanceSequence = cms.Sequence(
    pfNoJet + 
    pfMetSignificance
    )


# COUNTERS  ----------------------------
prePathCounter = cms.EDProducer("EventCountProducer")
postPathCounter = cms.EDProducer("EventCountProducer")



####  FULL SEQUENCE  ####

# NOTE: object sequences are defined so that they can be easily removed from the path
# contrary to PAT layer-wise sequences

PATCMGSequence = cms.Sequence(
    PATCMGGenSequence +
    PATCMGTriggerSequence +
    eventCleaningSequence + 
    PATCMGPileUpSubtractionSequence +
    PATCMGRhoSequence +
    PATCMGMuonSequence +
    PATCMGElectronSequence +
    PATCMGJetSequence +
    PATCMGTauSequence +
    PATCMGMetSequence +
    PATCMGMetSignificanceSequence
    )





# curing a configuration bug from PAT / EGamma
eIdSequence = cms.Sequence()
