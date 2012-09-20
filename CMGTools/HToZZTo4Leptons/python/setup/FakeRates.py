from CMGTools.RootTools.fwlite.Config import CFG
from CMGTools.HToZZTo4Leptons.tools.FakeRateCalculator import FakeRateCalculator


fakeRate2011 = CFG(
    name='fR',
    muon = 'data/FR_muon_11.root',
    electron = 'data/FR_electron_11.root',    
    muonFSR = 'data/FR_muon_11.root',
    electronFSR = 'data/FR_electron_11.root'
    )


fakeRates2011 =[fakeRate2011] 



fakeRate2012 = CFG(
    name='fR',
    muon = 'data/FR_muon_12.root',
    electron = 'data/FR_electron_12.root',    
    muonFSR = 'data/FR_muon_12.root',
    electronFSR = 'data/FR_electron_12.root'
    )


fakeRates2012 =[fakeRate2012] 