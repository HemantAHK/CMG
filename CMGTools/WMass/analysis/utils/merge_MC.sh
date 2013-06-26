#!/bin/sh

echo 'EWK ONLY (EWK)'
# EWK ONLY
mkdir ${1}/test_numbers_EWK
echo 'Z analysis (W+Jets sig+fake, DY+Jets fake, ZZ, WZ, WW)'
hadd -f ${1}/test_numbers_EWK/ZanalysisOnDATA.root ${1}/test_numbers_WJetsSig/ZanalysisOnDATA.root ${1}/test_numbers_WJetsFake/ZanalysisOnDATA.root ${1}/test_numbers_DYJetsFake/ZanalysisOnDATA.root ${1}/test_numbers_ZZJets/ZanalysisOnDATA.root ${1}/test_numbers_WZJets/ZanalysisOnDATA.root ${1}/test_numbers_WWJets/ZanalysisOnDATA.root 
echo 'W analysis (W+Jets fake, DY+Jets sig+fake, ZZ, WZ, WW)'
hadd -f ${1}/test_numbers_EWK/WanalysisOnDATA.root ${1}/test_numbers_WJetsFake/WanalysisOnDATA.root ${1}/test_numbers_DYJetsSig/WanalysisOnDATA.root ${1}/test_numbers_DYJetsFake/WanalysisOnDATA.root ${1}/test_numbers_ZZJets/WanalysisOnDATA.root ${1}/test_numbers_WZJets/WanalysisOnDATA.root ${1}/test_numbers_WWJets/WanalysisOnDATA.root 

echo 'EWK + TT (EWKTT)'
# EWK + TT
mkdir ${1}/test_numbers_EWKTT
echo 'Z analysis'
hadd -f ${1}/test_numbers_EWKTT/ZanalysisOnDATA.root ${1}/test_numbers_EWK/ZanalysisOnDATA.root ${1}/test_numbers_TTJets/ZanalysisOnDATA.root
echo 'W analysis'
hadd -f ${1}/test_numbers_EWKTT/WanalysisOnDATA.root ${1}/test_numbers_EWK/WanalysisOnDATA.root ${1}/test_numbers_TTJets/WanalysisOnDATA.root

echo 'EWK + TT + SIG (MCDATALIKE)'
# EWK + TT + SIG
mkdir ${1}/test_numbers_MCDATALIKE
echo 'Z analysis'
hadd -f ${1}/test_numbers_MCDATALIKE/ZanalysisOnDATA.root ${1}/test_numbers_EWKTT/ZanalysisOnDATA.root ${1}/test_numbers_DYJetsSig/ZanalysisOnDATA.root
echo 'W analysis'
hadd -f ${1}/test_numbers_MCDATALIKE/WanalysisOnDATA.root ${1}/test_numbers_EWKTT/WanalysisOnDATA.root ${1}/test_numbers_WJetsSig/WanalysisOnDATA.root

########################################################
echo 'EWK ONLY (EWK)'
# EWK ONLY
# mkdir ${1}/test_numbers_EWK
echo 'Z analysis (W+Jets sig+fake, DY+Jets fake, ZZ, WZ, WW)'
hadd -f ${1}/test_numbers_EWK/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_WJetsSig/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_WJetsFake/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_DYJetsFake/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_ZZJets/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_WZJets/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_WWJets/Zanalysis_controlplotsOnDATA.root.root 
echo 'W analysis (W+Jets fake, DY+Jets sig+fake, ZZ, WZ, WW)'
hadd -f ${1}/test_numbers_EWK/WanalysisOnDATA.root ${1}/test_numbers_WJetsFake/WanalysisOnDATA.root ${1}/test_numbers_DYJetsSig/WanalysisOnDATA.root ${1}/test_numbers_DYJetsFake/WanalysisOnDATA.root ${1}/test_numbers_ZZJets/WanalysisOnDATA.root ${1}/test_numbers_WZJets/WanalysisOnDATA.root ${1}/test_numbers_WWJets/WanalysisOnDATA.root 

echo 'EWK + TT (EWKTT)'
# EWK + TT
# mkdir ${1}/test_numbers_EWKTT
echo 'Z analysis'
hadd -f ${1}/test_numbers_EWKTT/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_EWK/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_TTJets/Zanalysis_controlplotsOnDATA.root.root
echo 'W analysis'
hadd -f ${1}/test_numbers_EWKTT/WanalysisOnDATA.root ${1}/test_numbers_EWK/WanalysisOnDATA.root ${1}/test_numbers_TTJets/WanalysisOnDATA.root

echo 'EWK + TT + SIG (MCDATALIKE)'
# EWK + TT + SIG
# mkdir ${1}/test_numbers_MCDATALIKE
echo 'Z analysis'
hadd -f ${1}/test_numbers_MCDATALIKE/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_EWKTT/Zanalysis_controlplotsOnDATA.root.root ${1}/test_numbers_DYJetsSig/Zanalysis_controlplotsOnDATA.root.root
echo 'W analysis'
hadd -f ${1}/test_numbers_MCDATALIKE/WanalysisOnDATA.root ${1}/test_numbers_EWKTT/WanalysisOnDATA.root ${1}/test_numbers_WJetsSig/WanalysisOnDATA.root

# hadd -f ${1}/test_numbers_EWK/ZanalysisOnDATA.root ${1}/test_numbers_WJetsSig/ZanalysisOnDATA.root ${1}/test_numbers_WJetsFake/ZanalysisOnDATA.root ${1}/test_numbers_DYJetsSig/ZanalysisOnDATA.root ${1}/test_numbers_DYJetsFake/ZanalysisOnDATA.root ${1}/test_numbers_TTJets/ZanalysisOnDATA.root ${1}/test_numbers_ZZJets/ZanalysisOnDATA.root ${1}/test_numbers_WZJets/ZanalysisOnDATA.root ${1}/test_numbers_WWJets/ZanalysisOnDATA.root 