#include <crsRead/MCorsikaReader.h>
#include <crs/TSubBlock.h>
#include <crs/MRunHeader.h>
#include <crs/MEventHeader.h>
#include <crs/MEventEnd.h>
#include <crs/MParticleBlock.h>
#include <crs/MLongitudinalBlock.h>
#include <crs/MParticle.h>
#include <TCanvas.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>
#include <TEllipse.h>
#include <TLine.h>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <cstdlib>
#include <unistd.h>

struct Hists {
    TH2D *tot, *e, *c, *m, *n;
};

std::string usage();
int getOptions(int argc, char** argv, std::set<int>& showers);
std::string baseName(const std::string& path);
void drawAndSave(Hists& h, TCanvas* canvas, Double_t hs, int idx,
                 Double_t energy, const std::string& fname);
int processFile(const std::string& fname, const std::set<int>& showers,
                Hists& h, TCanvas* canvas, Double_t hs);

int main(int argc, char ** argv) {
    std::set<int> showers;
    const int nOptions = getOptions(argc, argv, showers);
    if (nOptions < 0 || argc - nOptions < 1) {
        std::cout << usage() << std::endl;
        return 1;
    }
    if (showers.empty())
        showers.insert(1);   // default: prima shower

    Double_t hs = 2000.0;
    Int_t nBins = hs;

    Hists h;
    h.tot = new TH2D("xy_tot", "xy_tot", nBins, -hs, hs, nBins, -hs, hs);
    h.e   = new TH2D("xy_e",   "xy_e",   nBins, -hs, hs, nBins, -hs, hs);
    h.c   = new TH2D("xy_c",   "xy_c",   nBins, -hs, hs, nBins, -hs, hs);
    h.m   = new TH2D("xy_m",   "xy_m",   nBins, -hs, hs, nBins, -hs, hs);
    h.n   = new TH2D("xy_n",   "xy_n",   nBins, -hs, hs, nBins, -hs, hs);

    h.tot->SetMarkerColor(kBlack);  h.tot->SetMarkerStyle(2);
    h.n->SetMarkerColor(kRed);      h.n->SetMarkerStyle(2);
    h.c->SetMarkerColor(kViolet);   h.c->SetMarkerStyle(2);
    h.m->SetMarkerColor(kGreen);    h.m->SetMarkerStyle(2);
    h.e->SetMarkerColor(kBlue);     h.e->SetMarkerStyle(2);

    TCanvas * cFoot = new TCanvas("cFoot", "cFoot", 1600, 900);

    int status = 0;
    for (int f = nOptions; f < argc; ++f) {
        if (processFile(argv[f], showers, h, cFoot, hs) != 0)
            status = -1;
    }
    return status;
}

int processFile(const std::string& fname, const std::set<int>& showers,
                Hists& h, TCanvas* canvas, Double_t hs) {
    std::cout << "=============================" << std::endl;
    std::cout << "file: " << fname << std::endl;

    crsRead::MCorsikaReader cr(fname, 1);
    crs::MRunHeader run;
    int i = 0;
    const int maxIdx = *showers.rbegin(); 

    while (cr.GetRun(run)) {
        crs::MEventHeader shower;
        while (cr.GetShower(shower)) {
            i++;
            if (!showers.count(i))
                continue;

            h.tot->Reset(); h.e->Reset(); h.c->Reset(); h.m->Reset(); h.n->Reset();

            std::cout << "-----------------------------" << std::endl;
            std::cout << "plot of the shower nr: " << i << std::endl;
            Double_t energy = shower.GetEnergy();
            Double_t obsLev = shower.GetObservationHeight(shower.GetNObservationLevels() - 1) / 100.0;
            std::cout << "shower obs level: " << obsLev << " m" << std::endl;

            crs::TSubBlock sub_block;
            while (cr.GetData(sub_block)) {
                crs::MParticleBlock data(sub_block);

                if (data.GetBlockType() == crs::TSubBlock::ePARTDATA) {
                    for (crs::MParticleBlock::ParticleListConstIterator p_it = data.FirstParticle();
                         p_it != data.LastParticle(); ++p_it) {
                        crs::MParticle part(*p_it);
                        Int_t pID = part.GetParticleID();
                        Double_t x = part.GetX() / 100.0;
                        Double_t y = part.GetY() / 100.0;

                        if (pID < 4)
                            h.e->Fill(x, y);
                        if (pID == 5 || pID == 6)
                            h.m->Fill(x, y);
                        if (part.IsCherenkov())
                            h.c->Fill(x, y);
                        if ((part.IsNucleus() || (pID > 6 && pID < 200)) && !part.IsMuonProductionInfo())
                            h.n->Fill(x, y);
                        if (!part.IsMuonProductionInfo())
                            h.tot->Fill(x, y);
                    }
                }
            }

            drawAndSave(h, canvas, hs, i, energy, fname);
        }
    }

    int status = 0;
    for (std::set<int>::const_iterator it = showers.begin(); it != showers.end(); ++it) {
        if (*it > i || *it < 1) {
            std::cout << "ERROR: shower " << *it << " not found (file contains "
                      << i << " showers)" << std::endl;
            status = -1;
        }
    }
    (void)maxIdx;
    return status;
}

void drawAndSave(Hists& h, TCanvas* canvas, Double_t hs, int idx,
                 Double_t energy, const std::string& fname) {
    std::ostringstream title;
    title << "shower " << idx << " Energy " << energy << " GeV ";
    h.tot->SetTitle(title.str().c_str());
    h.tot->GetXaxis()->SetTitle("X [m]");
    h.tot->GetYaxis()->SetTitle("Y [m]");

    canvas->Clear();
    h.tot->Draw("AXIS");

    TEllipse circle(0, 0, 3);
    circle.SetFillColorAlpha(kOrange - 9, 0.35);
    circle.SetLineColor(kOrange + 7);
    circle.SetLineWidth(2);
    circle.Draw("SAME");

    h.tot->Draw("SAME");
    h.e->Draw("SAME");
    h.m->Draw("SAME");
    h.n->Draw("SAME");
    h.c->Draw("SAME");
    gPad->RedrawAxis();

    std::ostringstream outFname;
    outFname << "./plots/footprint_" << baseName(fname) << "_" << idx << ".png";
    canvas->SaveAs(outFname.str().c_str());
}

std::string baseName(const std::string& path) {
    size_t pos = path.find_last_of('/');
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

std::string usage() {
    return "usage: ./footprint [-n <id>[,<id>...]]... <corsika file> [<corsika file> ...]\n"
           "  -n can be repeated and/or take a comma-separated list (default: shower 1)\n"
           "  e.g. ./footprint -n 3,5,7 file1.dat file2.dat";
}

int getOptions(int argc, char ** argv, std::set<int>& showers) {
    int c;
    while ((c = getopt(argc, argv, "n:h")) != -1) {
        switch (c) {
        case 'n': {
            std::stringstream ss(optarg);
            std::string tok;
            while (std::getline(ss, tok, ',')) {
                if (tok.empty()) continue;
                int v = std::atoi(tok.c_str());
                if (v < 1) {
                    std::cerr << "invalid shower id: " << tok << std::endl;
                    return -2;
                }
                showers.insert(v);
            }
            break;
        }
        case 'h':
            return -2;
        default:
            return -2;
        }
    }
    return optind;
}