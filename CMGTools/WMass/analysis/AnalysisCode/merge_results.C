#include "../test_numbers_DATA/common.h"

void merge_results(){

  gStyle->SetOptStat(0);
  
  TFile *fout = new TFile(Form("likelihood_results.root"),"RECREATE");
  TGraph *result;
  TCanvas*c_chi2;
  TF1*ffit[3];
  // TGraph *result_NonScaled[WMass::PDF_members][3];
  TGraph *result_NonScaled[WMass::PDF_members][3];
  double deltaM[WMass::PDF_members][3];
  double deltaMmin[3]=0,deltaMmax[3]=0;
  TH1D*h_deltaM_PDF[3];
  TGraphErrors*g_deltaM_PDF[3];

  if(WMass::PDF_members>1){
    for(int k=0;k<3;k++){
      TString eta_str = Form("%.1f",WMass::etaMaxMuons[0]); eta_str.ReplaceAll(".","p");

      g_deltaM_PDF[k] = new TGraphErrors();
      g_deltaM_PDF[k]->SetName(Form("g_deltaM_PDF_%sNonScaled_eta%s",WMass::FitVar_str[k].Data(),eta_str.Data()));
      g_deltaM_PDF[k]->SetTitle(Form("g_deltaM_PDF_%sNonScaled_eta%s",WMass::FitVar_str[k].Data(),eta_str.Data()));
      g_deltaM_PDF[k]->SetMarkerStyle(20);
      h_deltaM_PDF[k] = new TH1D(Form("h_deltaM_PDF_%sNonScaled_eta%s",WMass::FitVar_str[k].Data(),eta_str.Data()),Form("h_deltaM_PDF_%sNonScaled_eta%s",WMass::FitVar_str[k].Data(),eta_str.Data()),200,-50,50);

    }
  }
  
  for(int h=0; h<WMass::PDF_members; h++){
    // for(int i=0; i<WMass::etaMuonNSteps; i++){
    cout << "using pdf " << (WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets) << "-"<<h<< endl;
    
    for(int i=0; i<1; i++){
      TString eta_str = Form("%.1f",WMass::etaMaxMuons[i]); eta_str.ReplaceAll(".","p");
      cout << "merging pdf eta bin= " << i << endl;
      
      for(int k=0;k<3;k++){

        cout << "variable= " << k << endl;
      
        int npoint=0;

        // result = new TGraph();
        // result->SetName(Form("likelihood_results_%s_eta%s",WMass::FitVar_str[k].Data(),eta_str.Data()));
        // result->SetTitle(Form("likelihood_results_%s_eta%s",WMass::FitVar_str[k].Data(),eta_str.Data()));

        result_NonScaled[h][k] = new TGraph();
        result_NonScaled[h][k]->SetName(Form("likelihood_result_%sNonScaled_pdf%d-%d_eta%s",WMass::FitVar_str[k].Data(),WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data()));
        result_NonScaled[h][k]->SetTitle(Form("likelihood_result_%sNonScaled_pdf%d-%d_eta%s",WMass::FitVar_str[k].Data(),WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data()));

        std::vector<double> l_res;
        double lmin=0, lmax=0;

        for(int j=0; j<2*WMass::WMassNSteps+1; j++){
          int jWmass = WMass::WMassCentral_MeV-(WMass::WMassNSteps-j)*WMass::WMassStep_MeV;
      
          // std::ifstream fileNames(Form("dummy_datacard_Wmass_MuPos_eta%s_%d.log",eta_str.Data(),jWmass));
          string StringFromFile;
          TString TStringFromFile;
          double likelihood_val;
          int ncol;
          // while (!fileNames.eof()){
            // getline (fileNames,StringFromFile);
            // TStringFromFile = StringFromFile.c_str();
            // if(TStringFromFile.Contains("nll S+B ->")){
              // break;
            // }
          // }
          // TObjArray* LineColumns = TStringFromFile.Tokenize(" ");
          // ncol = LineColumns->GetEntries();
          // if(ncol<3){
            // cout << Form("problem while analyzing fit result in dummy_datacard_Wmass_MuPos_eta%s_%d.log",eta_str.Data(),jWmass) << endl;
            // return;
          // }
          // TString str_icol = ((TObjString *)LineColumns->At(3))->GetString();
          // likelihood_val = (double) (str_icol.Atof());
          // if(likelihood_val<0) result->SetPoint(npoint,jWmass,likelihood_val);

          
          std::ifstream fileNames_NonScaled(Form("dummy_datacard_Wmass_MuPos_pdf%d-%d_eta%s_%d_%sNonScaled.log",WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data(),jWmass,WMass::FitVar_str[k].Data()));
          // string StringFromFile;
          // TString TStringFromFile;
          while (!fileNames_NonScaled.eof()){
            getline (fileNames_NonScaled,StringFromFile);
            TStringFromFile = StringFromFile.c_str();
            // if(TStringFromFile.Contains("nll S+B ->")){
            if(TStringFromFile.Contains("-2 ln Q_{TEV}")){
              break;
            }
          }
          TObjArray* LineColumns = TStringFromFile.Tokenize(" ");
          ncol = LineColumns->GetEntries();
          // str_icol = ((TObjString *)LineColumns->At(3))->GetString();
          str_icol = ((TObjString *)LineColumns->At(4))->GetString();
          likelihood_val = (double) (str_icol.Atof());
          // cout << jWmass << " LIKELIHOOD VALUE= "<<likelihood_val << " lmin= " << lmin << " lmax=" << lmax << endl;
          cout << " " << jWmass << " LIKELIHOOD VALUE= "<<likelihood_val; fflush(stdout);
          l_res.push_back(likelihood_val);
          if(likelihood_val<lmin) lmin=likelihood_val;
          if(likelihood_val>lmax) lmax=likelihood_val;

          npoint++;
      
        }
        cout << endl;

        double l_offset;
        if(lmax>0)
          l_offset=-lmax;
        else
          l_offset=-lmin;
        // cout << "lmin= " << lmin << " lmax= " << lmax << endl;  
        for(int j=0; j<2*WMass::WMassNSteps+1; j++){
          int jWmass = WMass::WMassCentral_MeV-(WMass::WMassNSteps-j)*WMass::WMassStep_MeV;
          // cout << "result_NonScaled[h][k]->SetPoint("<<j<<","<<jWmass<<","<<(lmax>0 ? -l_res.at(j) -l_offset : l_res.at(j) -l_offset) <<");"<<endl;
          result_NonScaled[h][k]->SetPoint(j,jWmass,lmax>0 ? -l_res.at(j) -l_offset : l_res.at(j) -l_offset );
        }
        
        
        // result->SetMarkerStyle(20);
        // result->SetMarkerSize(1);
        // result->Write();
        result_NonScaled[h][k]->SetMarkerStyle(20);
        result_NonScaled[h][k]->SetMarkerSize(1);
        result_NonScaled[h][k]->Write();
        
        c_chi2=new TCanvas(Form("c_chi2_%s_pdf%d-%d_eta%s",WMass::FitVar_str[k].Data(),WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data()),Form("c_chi2_%s_pdf%d-%d_eta%s",WMass::FitVar_str[k].Data(),WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data()));
        ffit[k]=new TF1(Form("ffit_%s_pdf%d-%d_eta%s",WMass::FitVar_str[k].Data(),WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data()),Form("[0]+TMath::Power((x-[1])/[2],2)"),70,100);
        ffit[k]->SetParameter(0,result_NonScaled[h][k]->GetMinimum());
        ffit[k]->SetParameter(1,80410);
        ffit[k]->SetParameter(2,10); // IF FIT DOES NOT CONVERGE, CHANGE THIS PARAMETER BY LOOKING AT THE CHI2 VS MASS DISTRIBUTION (~value for which Delta_chi2 = 1)
        // ffit[k]->SetParameter(2,1e4); // IF FIT DOES NOT CONVERGE, CHANGE THIS PARAMETER BY LOOKING AT THE CHI2 VS MASS DISTRIBUTION (~value for which Delta_chi2 = 1)
        ffit[k]->SetLineColor(2);
        ffit[k]->SetLineWidth(1);
        result_NonScaled[h][k]->Fit(Form("ffit_%s_pdf%d-%d_eta%s",WMass::FitVar_str[k].Data(),WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data()),"WEM");
        result_NonScaled[h][k]->Draw("ap");
        TLatex *text,*text2;
        text = new TLatex(0.25,0.7,Form("Best M_{W} with %s= %.0f #pm %.0f MeV", WMass::FitVar_str[k].Data(), eta_str.Data(),ffit[k]->GetParameter(1),ffit[k]->GetParameter(2)));
        text->SetNDC();
        text->Draw();
        text = new TLatex(0.25,0.6,Form("#DeltaM_{W} = %.0f MeV", (ffit[k]->GetParameter(1) - WMass::WMassCentral_MeV )));
        text->SetNDC();
        text->Draw();
        // text2 = new TLatex(0.25,0.6,Form("Best #chi^{2} ratio = %.1f", ffit[k]->GetParameter(0) ));
        // text2->SetNDC();
        // text2->Draw();
        cout << Form("Best M_W value with %s = %.0f +/- %.0f MeV, DeltaM_W = %.0f",WMass::FitVar_str[k].Data(), ffit[k]->GetParameter(1), ffit[k]->GetParameter(2), (ffit[k]->GetParameter(1) - WMass::WMassCentral_MeV)) << endl;
        // cout << "Best chi2 ratio value = " << ffit[k]->GetParameter(0) << endl;
        // cout << "Measured mass points chi2 min = " << chi2min << " max = " << chi2max << endl;
        
        c_chi2->Write();
        
        // result_NonScaled[h][k]->Delete();
        // result->Delete();
        
      }
   
      TCanvas *c_summary=new TCanvas(Form("c_summary_pdf%d-%d_eta%s",WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data()),Form("c_summary_pdf%d-%d_eta%s",WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data())); 
      c_summary->SetGridy();
      c_summary->SetGridx();
      TH2D*frame=new TH2D("frame",Form("pdf %d-%d eta %s;M_{W} (MeV); -2ln(L/L_{ref})",WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h),2*WMass::WMassStep_MeV+1,WMass::WMassCentral_MeV-(WMass::WMassNSteps)*WMass::WMassStep_MeV,WMass::WMassCentral_MeV+(WMass::WMassNSteps)*WMass::WMassStep_MeV,10,0,5);
      frame->Draw();
      TLine *l=new TLine(WMass::WMassCentral_MeV,0,WMass::WMassCentral_MeV,5);
      l->SetLineStyle(6);
      l->SetLineColor(4);
      l->SetLineWidth(2);
      l->Draw();

      TLegend *leg1 = new TLegend(0.51,0.7,0.89,0.89);
      leg1->SetFillColor(10);  leg1->SetBorderSize(1);
      leg1->SetTextSize(0.035);
      leg1->AddEntry(l, Form("gen M_{W} (%d MeV)",WMass::WMassCentral_MeV), "l");

      TF1*f[3];
      for(int k=0;k<3;k++){
        // cout << result_NonScaled[h][k]->GetParameter(1) << " " << ffit[k]->GetParameter(2) << endl;
        // ffit[k]->Draw("");
        f[k] = (TF1*) result_NonScaled[h][k]->GetFunction(Form("ffit_%s_pdf%d-%d_eta%s",WMass::FitVar_str[k].Data(),WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets,h,eta_str.Data()));
        f[k]->SetLineColor(k+1);
        f[k]->SetLineWidth(2);
        f[k]->Draw("same");
        leg1->AddEntry(f[k], Form("%s Fit (#DeltaM_{W} = %.0f #pm %0.f MeV)",WMass::FitVar_str[k].Data(), (f[k]->GetParameter(1) - WMass::WMassCentral_MeV),f[k]->GetParameter(2)), "l");
        // result_NonScaled[h][k]->Draw("l same");
        // ffit[k]->SetLineColor(4);
        // ffit[k]->SetLineWidth(4);
        // ffit[k]->Draw(k>0?"same":"");
        // result_NonScaled[h][k]->SaveAs("test_func");
        deltaM[k][h]=f[k]->GetParameter(1) - WMass::WMassCentral_MeV;
        if(deltaM[k][h]<deltaMmin[k]) deltaMmin[k]=deltaM[k][h];
        if(deltaM[k][h]>deltaMmax[k]) deltaMmax[k]=deltaM[k][h];
        
        if(WMass::PDF_members>1){
          g_deltaM_PDF[k]->SetPoint(h,h,deltaM[k][h]);
          g_deltaM_PDF[k]->SetPointError(h,0,f[k]->GetParameter(2));
          h_deltaM_PDF[k]->Fill(deltaM[k][h]);
        }
      }
      leg1->Draw("same");
      c_summary->Write();

    }
  }
  
  if(WMass::PDF_members>1){
    for(int k=0;k<3;k++){
      TCanvas*c= new TCanvas(Form("c_deltaM_PDF_%s",WMass::FitVar_str[k].Data()),Form("c_deltaM_PDF_%s",WMass::FitVar_str[k].Data()));
      g_deltaM_PDF[k]->Draw("ape");
      TLatex *text;
      text = new TLatex(0.2,0.85,Form("PDF %d with %s: #DeltaM_{W, min}= %.0f MeV, #DeltaM_{W,max}= %.0f MeV", WMass::PDF_sets<0?generated_PDF_set:WMass::PDF_sets, WMass::FitVar_str[k].Data(), deltaMmin[k], deltaMmax[k]));
      text->SetTextSize(0.035);
      text->SetNDC();
      text->Draw();

      g_deltaM_PDF[k]->Write();
      h_deltaM_PDF[k]->Write();
      c->Write();
      
    }
  }
    
  fout->Write();
  
  fout->Delete();

}