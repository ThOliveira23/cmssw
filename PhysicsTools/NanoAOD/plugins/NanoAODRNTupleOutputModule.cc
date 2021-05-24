// -*- C++ -*-
//
// Package:     PhysicsTools/NanoAODOutput
// Class  :     NanoAODRNTupleOutputModule
//
// Implementation:
//     [Notes on implementation]
//
// Original Author:  Max Orok
//         Created:  Wed, 13 Jan 2021 14:21:41 GMT
//

#include <cstdint>
#include <string>

#include <ROOT/RNTuple.hxx>
#include <ROOT/RNTupleModel.hxx>
#include <ROOT/RNTupleOptions.hxx>
#include <ROOT/RPageStorageFile.hxx>
using ROOT::Experimental::RNTupleModel;
using ROOT::Experimental::RNTupleWriter;
using ROOT::Experimental::Detail::RPageSinkFile;
using ROOT::Experimental::RNTupleWriteOptions;

#include "TObjString.h"

#include "FWCore/Framework/interface/one/OutputModule.h"
#include "FWCore/Framework/interface/RunForOutput.h"
#include "FWCore/Framework/interface/LuminosityBlockForOutput.h"
#include "FWCore/Framework/interface/EventForOutput.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/MessageLogger/interface/JobReport.h"
#include "FWCore/Utilities/interface/Digest.h"
#include "FWCore/Utilities/interface/GlobalIdentifier.h"
#include "DataFormats/NanoAOD/interface/UniqueString.h"
#include "DataFormats/Provenance/interface/ProcessHistoryRegistry.h"

#include "PhysicsTools/NanoAOD/plugins/EventStringOutputFields.h"
#include "PhysicsTools/NanoAOD/plugins/NanoAODRNTuples.h"
#include "PhysicsTools/NanoAOD/plugins/TableOutputFields.h"
#include "PhysicsTools/NanoAOD/plugins/TriggerOutputFields.h"

class NanoAODRNTupleOutputModule : public edm::one::OutputModule<> {
public:
  NanoAODRNTupleOutputModule(edm::ParameterSet const& pset);
  ~NanoAODRNTupleOutputModule() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void openFile(edm::FileBlock const&) override;
  bool isFileOpen() const override;
  void write(edm::EventForOutput const& e) override;
  void writeLuminosityBlock(edm::LuminosityBlockForOutput const&) override;
  void writeRun(edm::RunForOutput const&) override;
  void reallyCloseFile() override;
  void writeProvenance();

  void initializeNTuple(edm::EventForOutput const& e);

  std::string m_fileName;
  std::string m_logicalFileName;
  std::string m_compressionAlgorithm;
  int m_compressionLevel;
  bool m_writeProvenance;
  edm::ProcessHistoryRegistry m_processHistoryRegistry;
  edm::JobReport::Token m_jrToken;

  std::unique_ptr<TFile> m_file;
  std::unique_ptr<RNTupleWriter> m_ntuple;
  TableCollections m_tables;
  std::vector<TriggerOutputFields> m_triggers;
  EventStringOutputFields m_evstrings;

  class CommonEventFields {
  public:
    void createFields(RNTupleModel& model) {
      model.AddField<UInt_t>("run", &m_run);
      model.AddField<UInt_t>("luminosityBlock", &m_luminosityBlock);
      model.AddField<std::uint64_t>("event", &m_event);
    }
    void fill(const edm::EventID& id) {
      m_run = id.run();
      m_luminosityBlock = id.luminosityBlock();
      m_event = id.event();
    }

  private:
    UInt_t m_run;
    UInt_t m_luminosityBlock;
    std::uint64_t m_event;
  } m_commonFields;

  LumiNTuple m_lumi;
  RunNTuple m_run;

  std::vector<std::pair<std::string, edm::EDGetToken>> m_nanoMetadata;
};

NanoAODRNTupleOutputModule::NanoAODRNTupleOutputModule(edm::ParameterSet const& pset)
    : edm::one::OutputModuleBase::OutputModuleBase(pset),
      edm::one::OutputModule<>(pset),
      m_fileName(pset.getUntrackedParameter<std::string>("fileName")),
      m_logicalFileName(pset.getUntrackedParameter<std::string>("logicalFileName")),
      m_compressionAlgorithm(pset.getUntrackedParameter<std::string>("compressionAlgorithm")),
      m_compressionLevel(pset.getUntrackedParameter<int>("compressionLevel")),
      m_writeProvenance(pset.getUntrackedParameter<bool>("saveProvenance", true)),
      m_processHistoryRegistry() {}

NanoAODRNTupleOutputModule::~NanoAODRNTupleOutputModule() {}

void NanoAODRNTupleOutputModule::writeLuminosityBlock(edm::LuminosityBlockForOutput const& iLumi) {
  edm::Service<edm::JobReport> jr;
  jr->reportLumiSection(m_jrToken, iLumi.id().run(), iLumi.id().value());
  m_lumi.fill(iLumi.id(), *m_file);
  m_processHistoryRegistry.registerProcessHistory(iLumi.processHistory());
}

void NanoAODRNTupleOutputModule::writeRun(edm::RunForOutput const& iRun) {
  edm::Service<edm::JobReport> jr;
  jr->reportRunNumber(m_jrToken, iRun.id().run());

  m_run.fill(iRun, *m_file);

  edm::Handle<nanoaod::UniqueString> hstring;
  for (const auto& p : m_nanoMetadata) {
    iRun.getByToken(p.second, hstring);
    TObjString* tos = dynamic_cast<TObjString*>(m_file->Get(p.first.c_str()));
    if (tos && hstring->str() != tos->GetString()) {
      throw cms::Exception("LogicError", "Inconsistent nanoMetadata " + p.first +
          " (" + hstring->str() + ")");
    } else {
      auto ostr = std::make_unique<TObjString>(hstring->str().c_str());
      m_file->WriteTObject(ostr.release(), p.first.c_str());
    }
  }
  m_processHistoryRegistry.registerProcessHistory(iRun.processHistory());
}

bool NanoAODRNTupleOutputModule::isFileOpen() const {
  return nullptr != m_ntuple.get();
}

void NanoAODRNTupleOutputModule::openFile(edm::FileBlock const&) {
  m_file = std::make_unique<TFile>(m_fileName.c_str(), "RECREATE", "", m_compressionLevel);
  // todo check if m_file is valid?
  edm::Service<edm::JobReport> jr;
  cms::Digest branchHash;
  m_jrToken = jr->outputFileOpened(m_fileName,
                                   m_logicalFileName,
                                   std::string(),
                                   // TODO check if needed
                                   //m_fakeName ? "PoolOutputModule" : "NanoAODOutputModule",
                                   "NanoAODRNTupleOutputModule",
                                   description().moduleLabel(),
                                   edm::createGlobalIdentifier(),
                                   std::string(),
                                   branchHash.digest().toString(),
                                   std::vector<std::string>());

  if (m_compressionAlgorithm == "ZLIB") {
    m_file->SetCompressionAlgorithm(ROOT::kZLIB);
  } else if (m_compressionAlgorithm == "LZMA") {
    m_file->SetCompressionAlgorithm(ROOT::kLZMA);
  } else {
    throw cms::Exception("Configuration")
      << "NanoAODOutputModule configured with unknown compression algorithm '"
      << m_compressionAlgorithm << "'\n" << "Allowed compression algorithms are ZLIB and LZMA\n";
  }

  const auto& keeps = keptProducts();
  for (const auto& keep : keeps[edm::InRun]) {
    if (keep.first->className() == "nanoaod::MergeableCounterTable") {
      m_run.register_token(keep.second);
    }
    else if (keep.first->className() == "nanoaod::UniqueString" &&
             keep.first->moduleLabel() == "nanoMetadata")
    {
      m_nanoMetadata.emplace_back(keep.first->productInstanceName(), keep.second);
    }
    else {
      throw cms::Exception("Configuration", "NanoAODRNTupleOutputModule cannot handle class " +
          keep.first->className() + " in Run branch");
    }
  }
}

void NanoAODRNTupleOutputModule::initializeNTuple(edm::EventForOutput const& iEvent) {
  // set up RNTuple schema
  auto model = RNTupleModel::Create();
  m_commonFields.createFields(*model);

  //m_tables.clear();
  const auto& keeps = keptProducts();
  for (const auto& keep: keeps[edm::InEvent]) {
    //std::cout << "branch name: " << keep.first->branchName() << "\n";
    if (keep.first->className() == "nanoaod::FlatTable") {
      edm::Handle<nanoaod::FlatTable> handle;
      const auto& token = keep.second;
      iEvent.getByToken(token, handle);
      m_tables.add(token, *handle);
    } else if (keep.first->className() == "edm::TriggerResults") {
      m_triggers.emplace_back(TriggerOutputFields(keep.first->processName(), keep.second));
    } else if (keep.first->className() == "std::basic_string<char,std::char_traits<char> >" &&
               keep.first->productInstanceName() == "genModel") {
      m_evstrings.register_token(keep.second);
    } else {
      throw cms::Exception("Configuration", "NanoAODOutputModule cannot handle class " + keep.first->className());
    }
  }
  m_tables.createFields(iEvent, *model);
  for (auto& trigger: m_triggers) {
    trigger.createFields(iEvent, *model);
  }
  m_tables.print();
  m_evstrings.createFields(*model);
  // todo use Append
  RNTupleWriteOptions options;
  options.SetCompression(m_file->GetCompressionSettings());
  m_ntuple = std::make_unique<RNTupleWriter>(std::move(model),
    std::make_unique<RPageSinkFile>("Events", *m_file, options)
  );
}

void NanoAODRNTupleOutputModule::write(edm::EventForOutput const& iEvent) {
  if (!m_ntuple) {
    initializeNTuple(iEvent);
  }

  edm::Service<edm::JobReport> jr;
  jr->eventWrittenToFile(m_jrToken, iEvent.id().run(), iEvent.id().event());

  m_commonFields.fill(iEvent.id());
  m_tables.fill(iEvent);
  for (auto& trigger: m_triggers) {
    trigger.fill(iEvent);
  }
  m_evstrings.fill(iEvent);
  m_ntuple->Fill();
  m_processHistoryRegistry.registerProcessHistory(iEvent.processHistory());
}

void NanoAODRNTupleOutputModule::reallyCloseFile() {
  if (m_writeProvenance) {
    writeProvenance();
  }
  // write ntuple to disk by calling the RNTupleWriter destructor
  m_ntuple.reset();
  m_lumi.finalizeWrite();
  m_run.finalizeWrite();
  m_file->Write();
  m_file->Close();

  auto ntuple = ROOT::Experimental::RNTupleReader::Open("Events", m_fileName);
  ntuple->PrintInfo();
  ntuple->PrintInfo(ROOT::Experimental::ENTupleInfo::kStorageDetails);

  auto muons = ntuple->GetViewCollection("Muon");
  auto muon_pt = muons.GetView<float>("pt");
  auto num_entries = 0;
  for (auto e: ntuple->GetEntryRange()) {
    std::cout << "entry " << num_entries++ << ":\n";
    auto num_muons = 0;
    for (auto m: muons.GetCollectionRange(e)) {
      std::cout << "pt: " << muon_pt(m) << "\n";
      num_muons++;
    }
    std::cout << "... total " << num_muons << " muons\n";
  }

  auto lumi_ntuple = ROOT::Experimental::RNTupleReader::Open("LuminosityBlocks", m_fileName);
  lumi_ntuple->PrintInfo();
  lumi_ntuple->PrintInfo(ROOT::Experimental::ENTupleInfo::kStorageDetails);

  auto run_ntuple = ROOT::Experimental::RNTupleReader::Open("Runs", m_fileName);
  run_ntuple->PrintInfo();
  run_ntuple->PrintInfo(ROOT::Experimental::ENTupleInfo::kStorageDetails);

  auto pset_ntuple = ROOT::Experimental::RNTupleReader::Open(
    edm::poolNames::parameterSetsTreeName(), m_fileName);
  pset_ntuple->PrintInfo();
  pset_ntuple->PrintInfo(ROOT::Experimental::ENTupleInfo::kStorageDetails);

  auto md_ntuple = ROOT::Experimental::RNTupleReader::Open(
    edm::poolNames::metaDataTreeName(), m_fileName);
  md_ntuple->PrintInfo();
  md_ntuple->PrintInfo(ROOT::Experimental::ENTupleInfo::kStorageDetails);

  edm::Service<edm::JobReport> jr;
  jr->outputFileClosed(m_jrToken);
}

void NanoAODRNTupleOutputModule::writeProvenance() {
  PSetNTuple pntuple;
  pntuple.fill(edm::pset::Registry::instance(), *m_file);
  pntuple.finalizeWrite();

  MetadataNTuple mdntuple;
  mdntuple.fill(m_processHistoryRegistry, *m_file);
  mdntuple.finalizeWrite();
}

void NanoAODRNTupleOutputModule::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.addUntracked<std::string>("fileName");
  desc.addUntracked<std::string>("logicalFileName", "");
  desc.addUntracked<int>("compressionLevel", 9)->setComment("ROOT compression level of output file.");
  desc.addUntracked<std::string>("compressionAlgorithm", "ZLIB")->setComment("Algorithm used to "
    "compress data in the ROOT output file, allowed values are ZLIB and LZMA");
  desc.addUntracked<bool>("saveProvenance", true)
      ->setComment("Save process provenance information, e.g. for edmProvDump");
  const std::vector<std::string> keep = {"drop *",
                                         "keep nanoaodFlatTable_*Table_*_*",
                                         "keep edmTriggerResults_*_*_*",
                                         "keep String_*_genModel_*",
                                         "keep nanoaodMergeableCounterTable_*Table_*_*",
                                         "keep nanoaodUniqueString_nanoMetadata_*_*"};
  edm::one::OutputModule<>::fillDescription(desc, keep);

  //Used by Workflow management for their own meta data
  edm::ParameterSetDescription dataSet;
  dataSet.setAllowAnything();
  desc.addUntracked<edm::ParameterSetDescription>("dataset", dataSet)
      ->setComment("PSet is only used by Data Operations and not by this module.");

  edm::ParameterSetDescription branchSet;
  branchSet.setAllowAnything();
  desc.add<edm::ParameterSetDescription>("branches", branchSet);

  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(NanoAODRNTupleOutputModule);
