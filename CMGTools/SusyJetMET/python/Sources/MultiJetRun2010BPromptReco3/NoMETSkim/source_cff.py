
import FWCore.ParameterSet.Config as cms

source = cms.Source(
	"PoolSource",

	noEventSort = cms.untracked.bool(True),
	duplicateCheckMode = cms.untracked.string("noDuplicateCheck"),
	fileNames = cms.untracked.vstring()
)
source.fileNames.extend([
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_0.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_1.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_10.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_11.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_12.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_13.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_14.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_15.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_16.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_17.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_18.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_19.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_2.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_20.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_21.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_22.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_23.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_24.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_25.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_26.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_27.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_28.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_29.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_3.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_30.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_31.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_32.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_33.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_34.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_35.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_36.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_37.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_38.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_39.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_4.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_40.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_41.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_42.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_43.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_44.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_45.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_46.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_47.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_48.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_49.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_5.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_50.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_51.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_52.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_53.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_54.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_55.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_56.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_57.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_58.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_59.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_6.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_60.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_61.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_62.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_63.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_64.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_65.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_66.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_67.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_68.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_69.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_7.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_70.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_71.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_72.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_73.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_8.root',
		'root://castorcms//castor/cern.ch/user/c/cbern/cmst3/RA2SusyJetMET/MultiJetRun2010BPromptReco3/NoMETSkim/susyJetMET_tree_RA2_CMG_9.root',
])