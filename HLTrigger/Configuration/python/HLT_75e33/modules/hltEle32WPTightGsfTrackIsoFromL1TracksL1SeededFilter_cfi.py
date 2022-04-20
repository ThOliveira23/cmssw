import FWCore.ParameterSet.Config as cms

hltEle32WPTightGsfTrackIsoFromL1TracksL1SeededFilter = cms.EDFilter("HLTEgammaGenericQuadraticEtaFilter",
    absEtaLowEdges = cms.vdouble(0.0, 0.8, 1.479, 2.0),
    candTag = cms.InputTag("hltEle32WPTightBestGsfChi2L1SeededFilter"),
    doRhoCorrection = cms.bool(False),
    effectiveAreas = cms.vdouble(0.0, 0.0, 0.0, 0.0),
    energyLowEdges = cms.vdouble(0.0),
    etaBoundaryEB12 = cms.double(0.8),
    etaBoundaryEE12 = cms.double(2.0),
    l1EGCand = cms.InputTag("hltEgammaCandidatesL1Seeded"),
    lessThan = cms.bool(True),
    ncandcut = cms.int32(1),
    rhoMax = cms.double(99999999.0),
    rhoScale = cms.double(1.0),
    rhoTag = cms.InputTag("hltFixedGridRhoFastjetAllCaloForEGamma"),
    saveTags = cms.bool(True),
    thrOverE2EB1 = cms.vdouble(0.0),
    thrOverE2EB2 = cms.vdouble(0.0),
    thrOverE2EE1 = cms.vdouble(0.0),
    thrOverE2EE2 = cms.vdouble(0.0),
    thrOverEEB1 = cms.vdouble(0.0),
    thrOverEEB2 = cms.vdouble(0.0),
    thrOverEEE1 = cms.vdouble(0.0),
    thrOverEEE2 = cms.vdouble(0.0),
    thrRegularEB1 = cms.vdouble(5.5),
    thrRegularEB2 = cms.vdouble(8.0),
    thrRegularEE1 = cms.vdouble(5.5),
    thrRegularEE2 = cms.vdouble(5.5),
    useEt = cms.bool(True),
    varTag = cms.InputTag("hltEgammaEleL1TrkIsoL1Seeded")
)
