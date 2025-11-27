/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : make_vis_tree.cc
 * @created     : 2025-09-02 17:38
 */

#include <iostream>
#include <getopt.h>
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

int make_vis_tree(
    const TString& input_file_path, 
    TString output_file_path = "")
{
  TFile* input_file = TFile::Open(input_file_path); 
  if (input_file == nullptr || input_file->IsZombie()) {
    fprintf(stderr, "make_photonlibrary ERROR: Unable to open input file %s\b", 
        input_file_path.Data());
    exit(EXIT_FAILURE);
  }

  TTree* tree_event = input_file->Get<TTree>("EventTree");
  TTree* tree_gen = input_file->Get<TTree>("GenTree");
  tree_event->AddFriend(tree_gen, "GenTree");
  
  TTreeReader reader(tree_event);
  TTreeReaderValue<SLArListEventAnode> evAnodeList(reader, "EventAnode");
  TTreeReaderValue<SLArListEventPDS> evPDSList(reader, "EventPDS");
  TTreeReaderValue<SLArGenRecordsVector> genRecords(reader, "GenTree.GenRecords");

  const float num_photons = 1e7; 
  float coords[3] = {0.0, 0.0, 0.0};
  UInt_t n_events_per_point = 0;

  if (output_file_path.IsNull()) {
    output_file_path = input_file_path;
    output_file_path.Resize( output_file_path.Index(".root") ); 
    output_file_path.Append("_ntuple.root"); 
  }
  TFile* output_file = new TFile(
      output_file_path, 
      "recreate");

  TTree* plib = new TTree("photonLib", "SoLAr@ProtoDUNE3 Photon Library"); 

  float  vis_tot = 0.0; 
  float  vis_dir = 0.0; 
  float  vis_wls = 0.0; 

  const Int_t NTILE_MAIN = 60;
  const Int_t NTILE_LAT  = 10; 
  float  vis_tot_tile_main[NTILE_MAIN] = {0.0};
  float  vis_tot_tile_lat0[NTILE_LAT ] = {0.0};
  float  vis_tot_tile_lat1[NTILE_LAT ] = {0.0};
  
  float  vis_dir_tile_main[NTILE_MAIN] = {0.0};
  float  vis_dir_tile_lat0[NTILE_LAT ] = {0.0};
  float  vis_dir_tile_lat1[NTILE_LAT ] = {0.0};
  
  float  vis_wls_tile_main[NTILE_MAIN] = {0.0};
  float  vis_wls_tile_lat0[NTILE_LAT ] = {0.0};
  float  vis_wls_tile_lat1[NTILE_LAT ] = {0.0};
  
  const Int_t NSIPM_MAIN = 9600; 
  const Int_t NSIPM_LAT  =  600; 
  float  vis_sipm_main[NSIPM_MAIN] = {0.0}; 
  float  vis_sipm_lat0[NSIPM_LAT ] = {0.0};
  float  vis_sipm_lat1[NSIPM_LAT ] = {0.0};
  
  plib->Branch("x", &coords[0]);
  plib->Branch("y", &coords[1]);
  plib->Branch("z", &coords[2]); 

  plib->Branch("vis_tot", &vis_tot); 
  plib->Branch("vis_dir", &vis_dir); 
  plib->Branch("vis_wls", &vis_wls);
  
  plib->Branch("vis_tot_tile_main" , &vis_tot_tile_main, Form("vis_tot_tile_main[%i]/F" , NTILE_MAIN));
  plib->Branch("vis_tot_tile_edge0", &vis_tot_tile_lat0, Form("vis_tot_tile_edge0[%i]/F", NTILE_LAT));
  plib->Branch("vis_tot_tile_edge1", &vis_tot_tile_lat1, Form("vis_tot_tile_edge1[%i]/F", NTILE_LAT));
  
  plib->Branch("vis_dir_tile_main" , &vis_dir_tile_main, Form("vis_dir_tile_main[%i]/F" , NTILE_MAIN));
  plib->Branch("vis_dir_tile_edge0", &vis_dir_tile_lat0, Form("vis_dir_tile_edge0[%i]/F", NTILE_LAT ));
  plib->Branch("vis_dir_tile_edge1", &vis_dir_tile_lat1, Form("vis_dir_tile_edge1[%i]/F", NTILE_LAT ));
  
  plib->Branch("vis_wls_tile_main" , &vis_wls_tile_main, Form("vis_wls_tile_main[%i]/F" , NTILE_MAIN));
  plib->Branch("vis_wls_tile_edge0", &vis_wls_tile_lat0, Form("vis_wls_tile_edge0[%i]/F", NTILE_LAT ));
  plib->Branch("vis_wls_tile_edge1", &vis_wls_tile_lat1, Form("vis_wls_tile_edge1[%i]/F", NTILE_LAT ));
  
  plib->Branch("vis_sipm_main"  , &vis_sipm_main, Form("vis_sipm_main[%i]/F" , NSIPM_MAIN));
  plib->Branch("vis_sipm_edge00", &vis_sipm_lat1, Form("vis_sipm_edge0[%i]/F", NSIPM_LAT ));
  plib->Branch("vis_sipm_edge11", &vis_sipm_lat1, Form("vis_sipm_edge1[%i]/F", NSIPM_LAT ));

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

    if (genStatus.at(0) != coords[0] || 
        genStatus.at(1) != coords[1] ||
        genStatus.at(2) != coords[2]
       )
    {
      // Point is new. 
      // check if reader is at the first TTree entry 
      if (reader.GetCurrentEntry() != 0) {
        printf("[%lld] Reset variables and fill tree...\n", reader.GetCurrentEntry()); 
        printf("       %u events per point\n", n_events_per_point); 
        // apply proper visibility scaling 
        const float scaling = (n_events_per_point * num_photons);
        vis_tot /= scaling; 
        vis_dir /= scaling; 
        vis_wls /= scaling; 

        for (size_t itile=0; itile < NTILE_LAT; itile++) {
          vis_tot_tile_lat0[itile] /= scaling; 
          vis_dir_tile_lat0[itile] /= scaling;
          vis_wls_tile_lat0[itile] /= scaling; 
        }

        for (size_t itile=0; itile < NTILE_LAT; itile++) {
          vis_tot_tile_lat1[itile] /= scaling; 
          vis_dir_tile_lat1[itile] /= scaling;
          vis_wls_tile_lat1[itile] /= scaling; 
        }

        for (size_t itile=0; itile < NTILE_MAIN; itile++) {
          vis_tot_tile_main[itile] /= scaling; 
          vis_dir_tile_main[itile] /= scaling;
          vis_wls_tile_main[itile] /= scaling; 
        }

        for (size_t isipm=0; isipm < NSIPM_MAIN; isipm++) {
          vis_sipm_main[isipm] /= scaling;
        }

        for (size_t isipm=0; isipm < NSIPM_LAT; isipm++) {
          vis_sipm_lat0[isipm] /= scaling;
        }

        for (size_t isipm=0; isipm < NSIPM_LAT; isipm++) {
          vis_sipm_lat1[isipm] /= scaling;
        }

        // Fill the output tree
        plib->Fill();
      }

      // set the new coordinates
      coords[0] = genStatus.at(0);
      coords[1] = genStatus.at(1); 
      coords[2] = genStatus.at(2); 

      // Reset variables
      n_events_per_point = 0;
      vis_tot = 0.0; 
      vis_dir = 0.0; 
      vis_wls = 0.0; 
      std::memset(vis_sipm_main, 0.0, NSIPM_MAIN);
      std::memset(vis_sipm_lat0, 0.0, NSIPM_LAT );
      std::memset(vis_sipm_lat1, 0.0, NSIPM_LAT );

      std::memset(vis_tot_tile_main, 0.0, NTILE_MAIN);
      std::memset(vis_tot_tile_lat0, 0.0, NTILE_LAT );
      std::memset(vis_tot_tile_lat1, 0.0, NTILE_LAT );

      std::memset(vis_dir_tile_main, 0.0, NTILE_MAIN);
      std::memset(vis_dir_tile_lat0, 0.0, NTILE_LAT );
      std::memset(vis_dir_tile_lat1, 0.0, NTILE_LAT );

      std::memset(vis_wls_tile_main, 0.0, NTILE_MAIN);
      std::memset(vis_wls_tile_lat0, 0.0, NTILE_LAT );
      std::memset(vis_wls_tile_lat1, 0.0, NTILE_LAT );
    }

    n_events_per_point++;

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
            vis_sipm[anode_idx][sipm_idx] += nHitsPerProc[0];
          }
        }
      }
    }
  }

  // close the last point
  const float scaling = (n_events_per_point * num_photons);
  vis_tot /= scaling; 
  vis_dir /= scaling; 
  vis_wls /= scaling; 

  for (UInt_t itile=0; itile < NTILE_LAT; itile++) {
    vis_tot_tile_lat0[itile] /= scaling; 
    vis_dir_tile_lat0[itile] /= scaling;
    vis_wls_tile_lat0[itile] /= scaling; 
  }

  for (UInt_t itile=0; itile < NTILE_LAT; itile++) {
    vis_tot_tile_lat1[itile] /= scaling; 
    vis_dir_tile_lat1[itile] /= scaling;
    vis_wls_tile_lat1[itile] /= scaling; 
  }

  for (UInt_t itile=0; itile < NTILE_MAIN; itile++) {
    vis_tot_tile_main[itile] /= scaling; 
    vis_dir_tile_main[itile] /= scaling;
    vis_wls_tile_main[itile] /= scaling; 
  }

  for (UInt_t isipm=0; isipm < NSIPM_MAIN; isipm++) {
    vis_sipm_main[isipm] /= scaling;
  }

  for (UInt_t isipm=0; isipm < NSIPM_LAT; isipm++) {
    vis_sipm_lat0[isipm] /= scaling;
  }

  for (UInt_t isipm=0; isipm < NSIPM_LAT; isipm++) {
    vis_sipm_lat1[isipm] /= scaling;
  }

  // Fill the output tree
  plib->Fill();


  output_file->cd();
  plib->Write(); 
  output_file->Close();

  return 0;
}

void print_usage() {
  printf("make_vis_tree usage:\n"); 
  printf("\t-i | --input\tinput_file_path\n"); 
  printf("\t-o | --output\toutput_file_path (optional)\n"); 

  return;
}

int main (int argc, char *argv[]) {
  const char* short_opts = "i:o:h";
  static struct option long_opts[4] = 
  {
    {"input", required_argument, 0, 'i'}, 
    {"output", required_argument, 0, 'o'}, 
    {"help", no_argument, 0, 'h'}, 
    {nullptr, no_argument, nullptr, 0}
  };

  int c, option_index; 

  TString input_file_path = ""; 
  TString output_file_path = ""; 

  while ( (c = getopt_long(argc, argv, short_opts, long_opts, &option_index)) != -1) {
    switch(c) {
      case 'i' :
        input_file_path = optarg;
        break;
      case 'o' :
        output_file_path = optarg;
        break;
      case 'h' : 
        print_usage(); 
        exit( EXIT_SUCCESS ); 
        break;
      case '?' : 
        printf("make_vis_tree error: unknown flag %c\n", optopt);
        print_usage(); 
        exit( EXIT_FAILURE ); 
        break;
    }
  }
  printf("Monte Carlo input file: %s\n", input_file_path.Data());
  printf("vis tree output file: %s\n", output_file_path.Data());

  make_vis_tree(input_file_path, output_file_path);
 
  return 0;
}
