/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : make_photonlibrary.cc
 * @created     : 2025-09-02 17:38
 */

#include <iostream>
#include <functional>
#include <vector>
#include <fstream>
#include "TFile.h"
#include "TChain.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

#include "event/SLArGenRecords.hh"
#include "event/SLArEventAnode.hh"
#include "event/SLArEventSuperCellArray.hh"

const int N_CRU = 2;
const int N_CRU_TILE = 30;
const int N_CRU_SIPM = 160;

const int N_CRU_EDGE = 1;
const int N_EDGE_TILE = 10;
const int N_EDGE_SIPM = 60;

int get_sipm_index_main(const int& mt_idx, const int& t_idx, const int& sipm_idx) 
{
  int idx = sipm_idx + N_CRU_SIPM*(t_idx + N_CRU_TILE*(mt_idx)); 
  return idx;
}

int get_sipm_index_lat(const int& mt_idx, const int& t_idx, const int& sipm_idx) 
{
  int idx = sipm_idx + N_EDGE_SIPM*(t_idx + N_EDGE_TILE*(mt_idx)); 
  return idx;
}

int get_tile_index_main(const int& mt_idx, const int& t_idx) {
  return t_idx + N_CRU_TILE*(mt_idx);
}

int get_tile_index_lat(const int& mt_idx, const int& t_idx) {
  return t_idx + N_EDGE_TILE*(mt_idx);
}

int make_photonlibrary(
    const TString& input_list_path, 
    const TString& output_file_path)
{
  TChain EvChain("EventTree");
  TChain GenChain("GenTree");

  std::ifstream file_list; 
  file_list.open( input_list_path ); 
  if (file_list.is_open() == false) {
    printf("ERROR: Unable to open input list %s\n", input_list_path.Data());
    exit(1);
  }

  std::string line;
  int nfiles = 0;
  while (std::getline(file_list, line)) {
    printf("Adding file %s\n", line.c_str());
    EvChain.AddFile(line.c_str());
    GenChain.Add(line.c_str());
    nfiles++;
  }
  
  EvChain.AddFriend(&GenChain, "GenTree");
  
  TTreeReader reader(&EvChain);
  TTreeReaderValue<SLArListEventAnode> evAnodeList(reader, "EventAnode");
  TTreeReaderValue<SLArListEventPDS> evPDSList(reader, "EventPDS");
  TTreeReaderValue<SLArGenRecordsVector> genRecords(reader, "GenTree.GenRecords");

  const float num_photons = 1e7; 
  float coords[3] = {0.0, 0.0, 0.0};

  TFile* output_file = new TFile(
      "/eos/user/g/guffantd/DUNE/SoLAr_ProtoDUNE3/solar-plibrary.root", 
      "recreate",
      "SoLAr-ProtoDUNE3 Photon Library", 
      1); // minimal compression for fast access

  TTree* plib = new TTree("photonLib", "SoLAr@ProtoDUNE3 Photon Library"); 

  float  vis_tot = 0.0; 
  float  vis_dir = 0.0; 
  float  vis_wls = 0.0; 

  float  vis_tot_tile_main[60] = {0.0};
  float  vis_tot_tile_lat0[10] = {0.0};
  float  vis_tot_tile_lat1[10] = {0.0};
  
  float  vis_dir_tile_main[60] = {0.0};
  float  vis_dir_tile_lat0[10] = {0.0};
  float  vis_dir_tile_lat1[10] = {0.0};
  
  float  vis_wls_tile_main[60] = {0.0};
  float  vis_wls_tile_lat0[10] = {0.0};
  float  vis_wls_tile_lat1[10] = {0.0};
  
  float  vis_sipm_main[9600] = {0}; 
  float  vis_sipm_lat0[600] = {0};
  float  vis_sipm_lat1[600] = {0};
  
  plib->Branch("x", &coords[0]);
  plib->Branch("y", &coords[1]);
  plib->Branch("z", &coords[2]); 

  plib->Branch("vis_tot", &vis_tot); 
  plib->Branch("vis_dir", &vis_dir); 
  plib->Branch("vis_wls", &vis_wls);
  
  plib->Branch("vis_tot_tile_main" , &vis_tot_tile_main, "vis_tot_tile_main[60]/F");
  plib->Branch("vis_tot_tile_edge0", &vis_tot_tile_lat0, "vis_tot_tile_edge0[10]/F");
  plib->Branch("vis_tot_tile_edge1", &vis_tot_tile_lat1, "vis_tot_tile_edge1[10]/F");
  
  plib->Branch("vis_dir_tile_main" , &vis_dir_tile_main, "vis_dir_tile_main[60]/F");
  plib->Branch("vis_dir_tile_edge0", &vis_dir_tile_lat0, "vis_dir_tile_edge0[10]/F");
  plib->Branch("vis_dir_tile_edge1", &vis_dir_tile_lat1, "vis_dir_tile_edge1[10]/F");
  
  plib->Branch("vis_wls_tile_main" , &vis_wls_tile_main, "vis_wls_tile_main[60]/F");
  plib->Branch("vis_wls_tile_edge0", &vis_wls_tile_lat0, "vis_wls_tile_edge0[10]/F");
  plib->Branch("vis_wls_tile_edge1", &vis_wls_tile_lat1, "vis_wls_tile_edge1[10]/F");
  
  plib->Branch("vis_sipm_main", &vis_sipm_main, "vis_sipm_main[9600]/F");
  plib->Branch("vis_sipm_edge00", &vis_sipm_lat1, "vis_sipm_edge0[600]/F");
  plib->Branch("vis_sipm_edge11", &vis_sipm_lat1, "vis_sipm_edge1[600]/F");

  float* vis_tot_tile[3] = {vis_tot_tile_main, vis_tot_tile_lat0, vis_tot_tile_lat1};
  float* vis_dir_tile[3] = {vis_dir_tile_main, vis_dir_tile_lat0, vis_dir_tile_lat1};
  float* vis_wls_tile[3] = {vis_wls_tile_main, vis_wls_tile_lat0, vis_wls_tile_lat1};
  float* vis_sipm    [3] = {vis_sipm_main, vis_sipm_lat0, vis_sipm_lat1};

  std::vector<std::function<int(const int&, const int&, const int&)>> sipm_mapper = {
    get_sipm_index_main,
    get_sipm_index_lat,
    get_sipm_index_lat
  };

  std::vector<std::function<int(const int&, const int&)>> tile_mapper = {
    get_tile_index_main,
    get_tile_index_lat,
    get_tile_index_lat
  };

  auto get_anode_idx = [](const Int_t& tpc_id) {
    if (tpc_id == 11) return 0;
    else if (tpc_id == 12) return 1;
    else if (tpc_id == 13) return 2;
    else return -1;
  };

  while (reader.Next()) {
    // 1. Access generator information
    const auto& genRecord = genRecords->GetRecordsVector().at(0);
    const auto& genStatus = genRecord.GetGenStatus();
    coords[0] = genStatus.at(0);
    coords[1] = genStatus.at(1); 
    coords[2] = genStatus.at(2); 

    // 2. Reset counters
    vis_tot = 0.0; 
    vis_dir = 0.0; 
    vis_wls = 0.0; 
    std::memset(vis_sipm_main, 0.0, sizeof(vis_sipm_main));
    std::memset(vis_sipm_lat0, 0.0, sizeof(vis_sipm_lat0));
    std::memset(vis_sipm_lat1, 0.0, sizeof(vis_sipm_lat1));

    std::memset(vis_tot_tile_main, 0.0, sizeof(vis_tot_tile_main));
    std::memset(vis_tot_tile_lat0, 0.0, sizeof(vis_tot_tile_lat0));
    std::memset(vis_tot_tile_lat1, 0.0, sizeof(vis_tot_tile_lat1));
    
    std::memset(vis_dir_tile_main, 0.0, sizeof(vis_dir_tile_main));
    std::memset(vis_dir_tile_lat0, 0.0, sizeof(vis_dir_tile_lat0));
    std::memset(vis_dir_tile_lat1, 0.0, sizeof(vis_dir_tile_lat1));
    
    std::memset(vis_wls_tile_main, 0.0, sizeof(vis_wls_tile_main));
    std::memset(vis_wls_tile_lat0, 0.0, sizeof(vis_wls_tile_lat0));
    std::memset(vis_wls_tile_lat1, 0.0, sizeof(vis_wls_tile_lat1));

    // 3. Process anode events (skip the top TPC)
    for (const auto& evAnode_itr : evAnodeList->GetConstAnodeMap()) {
      if (evAnode_itr.first == 10) continue; // Skip top TPC

      //printf("---------------------------------------------------------------------------\n");
      //printf("Processing Anode %i\n", evAnode_itr.first);

      const int anode_idx = get_anode_idx( evAnode_itr.first ); 

      for (const auto& evMT_itr : evAnode_itr.second.GetConstMegaTilesMap()) {
        //printf("  MegaTile %i\n", evMT_itr.first);

        for (const auto& evT_itr : evMT_itr.second.GetConstTileMap()) {
          //printf("    Tile %i\n", evT_itr.first);
          const int tile_idx = tile_mapper[anode_idx](evMT_itr.first, evT_itr.first);
          auto& vtot_tile = vis_tot_tile[anode_idx][tile_idx];
          auto& vdir_tile = vis_dir_tile[anode_idx][tile_idx];
          auto& vwls_tile = vis_wls_tile[anode_idx][tile_idx];
        
          for (const auto& evSiPM_itr : evT_itr.second.GetConstSiPMEvents()) {
            //printf("      SiPM %i: %i hits\n", evSiPM_itr.first, evSiPM_itr.second.GetNhits());

            const int sipm_idx = 
              sipm_mapper[anode_idx](evMT_itr.first, evT_itr.first, evSiPM_itr.first);
          
            const auto& evSiPM = evSiPM_itr.second;
            const auto& backtrackerColl = evSiPM.GetBacktrackerRecordCollection(); 
            int nHitsPerProc[6] = {0, 0, 0, 0, 0, 0};

            for (const auto& hit : evSiPM.GetConstHits()) {
              const auto& backtrackers = backtrackerColl.at(hit.first);
              const auto& bktrkProc = backtrackers.GetConstRecords().at(0); 
              for (const auto& proc : bktrkProc.GetConstCounter()) {
                nHitsPerProc[proc.first] += proc.second;
              }
            }

            nHitsPerProc[0] = evSiPM.GetNhits();
            vis_tot += nHitsPerProc[0];
            vis_dir += nHitsPerProc[4];
            vis_wls += nHitsPerProc[3];
            vtot_tile += nHitsPerProc[0];
            vdir_tile += nHitsPerProc[4];
            vwls_tile += nHitsPerProc[3];
            vis_sipm[anode_idx][sipm_idx] = nHitsPerProc[0] / num_photons;
          }

          vtot_tile /= num_photons;
          vdir_tile /= num_photons;
          vwls_tile /= num_photons;
        }
      }
    }

    vis_tot /= num_photons;
    vis_dir /= num_photons;
    vis_wls /= num_photons;


    plib->Fill();
  }

  output_file->cd();
  plib->Write(); 
  output_file->Close();

  return 0;
}


