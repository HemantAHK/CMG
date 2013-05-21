#include "../test_numbers_DATA/common.h"

void merge_results(){
  
  TFile *fout = new TFile(Form("likelihood_results.root"),"RECREATE");
  
  // for(int i=0; i<WMass::etaMuonNSteps; i++){
  for(int i=0; i<1; i++){
    TString eta_str = Form("%.1f",WMass::etaMaxMuons[i]); eta_str.ReplaceAll(".","p");
    
    int npoint=0;

    TGraph *result = new TGraph();
    result->SetName(Form("likelihood_results_eta%s",eta_str.Data()));
    result->SetTitle(Form("likelihood_results_eta%s",eta_str.Data()));

    TGraph *result_NonScaled = new TGraph();
    result_NonScaled->SetName(Form("likelihood_result_NonScaled_eta%s",eta_str.Data()));
    result_NonScaled->SetTitle(Form("likelihood_result_NonScaled_eta%s",eta_str.Data()));

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

      
      std::ifstream fileNames_NonScaled(Form("dummy_datacard_Wmass_MuPos_eta%s_%d_NonScaled.log",eta_str.Data(),jWmass));
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
      cout << jWmass << " LIKELIHOOD VALUE= "<<likelihood_val << endl;
      result_NonScaled->SetPoint(npoint,jWmass,result_NonScaled->GetMaximum()>0?likelihood_val:-likelihood_val);

      npoint++;
  
    }

    
    
    result->SetMarkerStyle(20);
    result->SetMarkerSize(1);
    result->Write();
    result_NonScaled->SetMarkerStyle(20);
    result_NonScaled->SetMarkerSize(1);
    result_NonScaled->Write();
    
    TCanvas*c_chi2=new TCanvas(Form("c_chi2_eta%s",eta_str.Data()),Form("c_chi2_eta%s",eta_str.Data()));
    TF1*ffit=new TF1(Form("ffit_eta%s",eta_str.Data()),Form("[0]+TMath::Power((x-[1])/[2],2)"),70,100);
    ffit->SetParameter(0,result_NonScaled->GetMinimum());
    ffit->SetParameter(1,80410);
    ffit->SetParameter(2,10); // IF FIT DOES NOT CONVERGE, CHANGE THIS PARAMETER BY LOOKING AT THE CHI2 VS MASS DISTRIBUTION (~value for which Delta_chi2 = 1)
    // ffit->SetParameter(2,1e4); // IF FIT DOES NOT CONVERGE, CHANGE THIS PARAMETER BY LOOKING AT THE CHI2 VS MASS DISTRIBUTION (~value for which Delta_chi2 = 1)
    ffit->SetLineColor(2);
    ffit->SetLineWidth(1);
    result_NonScaled->Fit(Form("ffit_eta%s",eta_str.Data()),"WEM");
    result_NonScaled->Draw("ap");
    TLatex *text,*text2;
    text = new TLatex(0.25,0.7,Form("Best M_{W} = %.0f +/- %.0f MeV", ffit->GetParameter(1), ffit->GetParameter(2) ));
    text->SetNDC();
    text->Draw();
    // text2 = new TLatex(0.25,0.6,Form("Best #chi^{2} ratio = %.1f", ffit->GetParameter(0) ));
    // text2->SetNDC();
    // text2->Draw();
    cout << "Best M_W value = " << ffit->GetParameter(1) << " +/- " << Form("%d",TMath::Nint(ffit->GetParameter(2))) << " MeV" << endl;
    // cout << "Best chi2 ratio value = " << ffit->GetParameter(0) << endl;
    // cout << "Measured mass points chi2 min = " << chi2min << " max = " << chi2max << endl;
    
    c_chi2->Write();
    
    result_NonScaled->Delete();
    result->Delete();
    
  }
  
  fout->Write();
  
  fout->Delete();

}