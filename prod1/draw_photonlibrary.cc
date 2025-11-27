/**
 * @author      : Daniele Guffanti (daniele.guffanti@mib.infn.it)
 * @file        : draw_photonlibrary.cc
 * @created     : Wednesday Sep 03, 2025 13:34:12 CEST
 */

#include <iostream>
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderArray.h"
#include "TH3F.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TStyle.h"

int draw_photonlibrary(const TString& photonlib_path)
{
  TFile* photonlib_file = TFile::Open(photonlib_path);
  TTree* plib = photonlib_file->Get<TTree>("photonLib"); 

  TTreeReader reader(plib);
  TTreeReaderValue<float> x(reader, "x");
  TTreeReaderValue<float> y(reader, "y");
  TTreeReaderValue<float> z(reader, "z");
  TTreeReaderValue<float> vtot(reader, "vis_tot");
  TTreeReaderValue<float> vdir(reader, "vis_dir");
  TTreeReaderValue<float> vwls(reader, "vis_wls");
  TTreeReaderArray<float> tmain_vtot(reader, "vis_tot_tile_main");
  TTreeReaderArray<float> tmain_vdir(reader, "vis_dir_tile_main");
  TTreeReaderArray<float> tmain_vwls(reader, "vis_wls_tile_main");
  TTreeReaderArray<float> tmain_sipm(reader, "vis_sipm_main");

  float xref[3] = {0.0, -175.05, 0.0};

  auto get_bin_vec = [](const float& x0, const float& x1, const float& step) {
    std::vector<float> v;
    float tmp = x0; 
    while (tmp < x1) {
      v.push_back(tmp);
      tmp+=step;
    }
    return v;
  };

  std::vector<float> binx = get_bin_vec(-150, 150, 10);
  std::vector<float> biny = get_bin_vec(-175, 175, 10);
  std::vector<float> binz = get_bin_vec(-150, 150, 10);

  TH3F* h3vis_tot = new TH3F("h3vis_tot", "SoLAr anode visibility;x [cm];z [cm];y [cm]", 
      binx.size()-1, binx.data(), binz.size()-1, binz.data(), biny.size()-1, biny.data() ); 
  TH3F* h3vis_dir = new TH3F("h3vis_dir", "SoLAr anode direct visibility;x [cm];z [cm];y [cm]", 
      binx.size()-1, binx.data(), binz.size()-1, binz.data(), biny.size()-1, biny.data() ); 
  TH3F* h3vis_wls = new TH3F("h3vis_wls", "SoLAr anode WLS visibility;x [cm];z [cm];y [cm]", 
      binx.size()-1, binx.data(), binz.size()-1, binz.data(), biny.size()-1, biny.data() ); 

  while (reader.Next()) {
    float xx = (*x)*0.1;
    float yy = (*y)*0.1 + 175.05;
    float zz = (*z)*0.1;

    int ibin = h3vis_tot->FindBin(xx, zz, yy);
    h3vis_tot->SetBinContent(ibin, (*vtot));
    h3vis_dir->SetBinContent(ibin, (*vdir));
    h3vis_wls->SetBinContent(ibin, (*vwls));
  }

  gStyle->SetCanvasPreferGL();
  gStyle->SetOptStat(0);
  gStyle->SetPalette(kSunset);
  TCanvas* cvis3D = new TCanvas("cvis3D", "cvis3D", 0, 0, 2000, 800);
  cvis3D->Divide(3,1);
  cvis3D->cd(1); h3vis_tot->Draw("glbox");
  cvis3D->cd(2); h3vis_dir->Draw("glbox");  
  cvis3D->cd(3); h3vis_wls->Draw("glbox");

  TH2F* h2vis_tot = (TH2F*)h3vis_tot->Project3D("zy");
  h2vis_tot->SetTitle("SoLAr anode visibility (x:[-150, 150]);z [cm];y [cm]");
  h2vis_tot->Scale(1.0/(binx.size()-1));
  TH2F* h2vis_dir = (TH2F*)h3vis_dir->Project3D("zy");
  h2vis_dir->SetTitle("SoLAr anode direct visibility (x:[-150, 150]);z [cm];y [cm]");
  h2vis_dir->Scale(1.0/(binx.size()-1));
  TH2F* h2vis_wls = (TH2F*)h3vis_wls->Project3D("zy");
  h2vis_wls->SetTitle("SoLAr anode WLS visibility (x:[-150, 150]);z [cm];y [cm]");
  h2vis_wls->Scale(1.0/(binx.size()-1));

  TH2F* h2vis[3] = {h2vis_tot, h2vis_dir, h2vis_wls};

  double maxv = h2vis_tot->GetMaximum();
  for (int i=0; i<3; i++) {
    h2vis[i]->GetZaxis()->SetRangeUser(0, maxv);
    h2vis[i]->GetYaxis()->SetTitleOffset(1.4);
    h2vis[i]->SetEntries( h3vis_tot->GetNbinsX() * h3vis_tot->GetNbinsY() * h3vis_tot->GetNbinsZ());
  }
  h2vis_dir->GetZaxis()->SetRangeUser(0, maxv);
  h2vis_wls->GetZaxis()->SetRangeUser(0, maxv);  

  TCanvas* cvis2D = new TCanvas("cvis2D", "cvis2D", 0, 0, 1600, 800);
  cvis2D->Divide(2,1);
  //cvis2D->cd(1); gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.15); h2vis_tot->Draw("colz");
  cvis2D->cd(1); gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.15); h2vis_dir->Draw("colz");
  cvis2D->cd(2); gPad->SetLeftMargin(0.12); gPad->SetRightMargin(0.15); h2vis_wls->Draw("colz");

  return 0;
}

