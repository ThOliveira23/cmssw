import FWCore.ParameterSet.Config as cms

from IOMC.EventVertexGenerators.VtxSmearedParameters_cfi import *
VtxSmeared = cms.EDProducer("BetafuncEvtVtxGenerator",
    Realistic25ns13TeVEarly2018CollisionVtxSmearingParameters,
    VtxSmearedCommon
)
