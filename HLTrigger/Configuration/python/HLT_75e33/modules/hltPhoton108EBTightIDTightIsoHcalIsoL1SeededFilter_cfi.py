import FWCore.ParameterSet.Config as cms

hltPhoton108EBTightIDTightIsoHcalIsoL1SeededFilter = cms.EDFilter("HLTEgammaGenericQuadraticEtaFilter",
    absEtaLowEdges = cms.vdouble(0.0, 0.8, 1.479, 2.0),
    candTag = cms.InputTag("hltPhoton108EBTightIDTightIsoEcalIsoL1SeededFilter"),
    doRhoCorrection = cms.bool(False),
    effectiveAreas = cms.vdouble(0.2, 0.2, 0.4, 0.5),
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
    thrOverEEB1 = cms.vdouble(0.02),
    thrOverEEB2 = cms.vdouble(0.02),
    thrOverEEE1 = cms.vdouble(0.0),
    thrOverEEE2 = cms.vdouble(0.0),
    thrRegularEB1 = cms.vdouble(3.8),
    thrRegularEB2 = cms.vdouble(6.0),
    thrRegularEE1 = cms.vdouble(0),
    thrRegularEE2 = cms.vdouble(0),
    useEt = cms.bool(True),
    varTag = cms.InputTag("hltEgammaHcalPFClusterIsoL1Seeded")
)
