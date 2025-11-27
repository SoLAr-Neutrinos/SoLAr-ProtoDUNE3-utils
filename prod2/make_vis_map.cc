/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : make_vis_map
 * @created     : Thursday Nov 27, 2025 10:38:31 CET
 */

#include <iostream>
#include <cstdio>
#include <list>
#include <getopt.h>
#include "TFile.h"
#include "TTree.h"
#include "TSystem.h"

#include "rapidjson/document.h"
#include "rapidjson/filereadstream.h"


class LRUFileCache {
  private:
    size_t maxSize;
    std::list<std::string> lruList;
    std::map<std::string, std::list<std::string>::iterator> cacheMap;
    std::map<std::string, TFile*> fileMap;
    std::map<std::string, TTree*> treeMap;
    std::string treeName;

  public:
    LRUFileCache(size_t max, const std::string& tree) : maxSize(max), treeName(tree) {}

    ~LRUFileCache() {
      // Close all remaining files
      for (auto& pair : fileMap) {
        if (pair.second) {
          pair.second->Close();
          delete pair.second;
        }
      }
    }

    TTree* getTree(const std::string& filename) {
      // Check if file is in cache
      auto it = cacheMap.find(filename);

      if (it != cacheMap.end()) {
        // Move to front (most recently used)
        lruList.erase(it->second);
        lruList.push_front(filename);
        cacheMap[filename] = lruList.begin();
        return treeMap[filename];
      }

      // File not in cache - need to open it
      // First check if cache is full
      if (lruList.size() >= maxSize) {
        // Remove least recently used file
        std::string lruFile = lruList.back();
        lruList.pop_back();
        cacheMap.erase(lruFile);

        // Close and delete the file
        if (fileMap[lruFile]) {
          fileMap[lruFile]->Close();
          delete fileMap[lruFile];
        }
        fileMap.erase(lruFile);
        treeMap.erase(lruFile);

        std::cout << "\rCache: Closed " << lruFile << std::string(20, ' ') << std::flush;
      }

      // Open new file
      TFile* f = TFile::Open(filename.c_str(), "READ");
      if (!f || f->IsZombie()) {
        std::cerr << "\nWarning: Cannot open file: " << filename << std::endl;
        return nullptr;
      }

      TTree* t = (TTree*)f->Get(treeName.c_str());
      if (!t) {
        std::cerr << "\nWarning: Cannot find tree '" << treeName << "' in file: " << filename << std::endl;
        f->Close();
        delete f;
        return nullptr;
      }

      // Add to cache
      lruList.push_front(filename);
      cacheMap[filename] = lruList.begin();
      fileMap[filename] = f;
      treeMap[filename] = t;

      return t;
    }

    size_t getCurrentSize() const {
      return lruList.size();
    }
};

void print_usage() {
  printf("make_vis_map usage:\n");
  printf("  --json-filemap <file>   JSON file containing the file map\n");
  printf("  --output <file>         Output ROOT file name (default: vis_map.root)\n");
  return;
}

int make_vis_map(const TString &json_filemap, const TString &output_file_path) {
  
  FILE* json_fp = fopen(json_filemap.Data(), "r");
  if (json_fp == nullptr) {
    std::cerr << "Error: Unable to open JSON file " << json_filemap.Data() << std::endl;
    return 1;
  }

  char readBuffer[65536];
  rapidjson::FileReadStream is(json_fp, readBuffer, sizeof(readBuffer));

  rapidjson::Document d;
  d.ParseStream<rapidjson::kParseCommentsFlag>(is);
  assert(d.IsArray());

  size_t maxCacheSize = 100;
  const TString treeName = "photonLib";

  LRUFileCache cache(maxCacheSize, treeName.Data());
  const auto& first_entry = d[0];

  // Open the first file to get the tree structure
  TTree* firstTree = cache.getTree(first_entry["filepath"].GetString());
  if (!firstTree) {
    std::cerr << "Error: Cannot open first file or tree" << std::endl;
    return 1;
  }

  // Create output file and clone tree structure
  TFile* outFile = TFile::Open(output_file_path, "RECREATE");
  if (!outFile || outFile->IsZombie()) {
    std::cerr << "Error: Cannot create output file: " << output_file_path << std::endl;
    return 1;
  }

  TTree* outTree = firstTree->CloneTree(0);
  outTree->SetDirectory(outFile);

  Long64_t num_entries = 0;
  for (const auto& jval : d.GetArray()) {
    assert(jval.IsObject());

    Long64_t entry_nr = jval["entry"].GetInt();
    TString dirname = gSystem->DirName(jval["filepath"].GetString());
    TString filename = gSystem->BaseName(jval["filepath"].GetString());
    filename.Insert( filename.Index(".root"), "_vtree");

    printf("[%lld] %s - entry %lld\n", num_entries, filename.Data(), entry_nr);
  
    TTree* sourceTree = cache.getTree( (dirname + "/" + filename).Data() );
    if (!sourceTree) {
      fprintf(stderr, "  Skipping entry %lld in %s due to missing tree.\n", 
              entry_nr, (dirname + "/" + filename).Data());
      continue;
    }

    sourceTree->GetEntry(entry_nr);
    outTree->Fill();

    num_entries++;
  }

  fclose(json_fp);

  outFile->cd();
  outTree->Write();
  outFile->Close();
  delete outFile;

  std::cout << "Output written to: " << outFile << std::endl;

  return 0; 
}

int main (int argc, char *argv[]) {
  TString json_filemap = "";
  TString output_file = "vis_map.root";

  static struct option long_options[] = {
    {"json-filemap", required_argument, 0, 'j'},
    {"output", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
  };

  int opt;
  int long_index =0;
  while ((opt = getopt_long(argc, argv,"j:o:h", long_options, &long_index )) != -1) {
    switch (opt) {
      case 'j' : json_filemap = TString(optarg);
        break;
      case 'o' : output_file = TString(optarg);
        break;
      case 'h' : 
        print_usage();
        return 0;
      default:
        print_usage(); 
        return 1;
    }
  }

  if (json_filemap == "") {
    std::cerr << "Error: --json-filemap is required" << std::endl;
    print_usage();
    return 1;
  }

  int status = make_vis_map(json_filemap, output_file);

  return status;
}

