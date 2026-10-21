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
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <cstdlib>
#include <unistd.h>

std::string usage();
int getOptions(int argc, char ** argv, std::set<int> & showers);
std::string baseName(const std::string & path);
void drawAndSave(TH1D * hGround, TCanvas * canvas, int idx, const std::string & fname);
int processFile(const std::string & fname, const std::set<int> & showers,
                TH1D * hGround, TCanvas * canvas);

int main(int argc, char ** argv) {
    std::set<int> showers;
    const int nOptions = getOptions(argc, argv, showers);
    if (nOptions < 0 || argc - nOptions < 1) {
        std::cout << usage() << std::endl;
        return 1;
    }
    if (showers.empty())
        showers.insert(1);   // default: prima shower

    Double_t hs = 1.0;
    Int_t nBins = 1000;

    TH1D * hGround = new TH1D("hGround", "hGround", nBins, 0.0, hs);
    hGround->SetFillColor(kBlue);
    hGround->SetFillStyle(3005);

    TCanvas * cGround = new TCanvas("cGround", "cGround", 1600, 900);
    cGround->SetLogy();

    int status = 0;
    for (int f = nOptions; f < argc; ++f) {
        if (processFile(argv[f], showers, hGround, cGround) != 0)
            status = -1;
    }
    return status;
}

int processFile(const std::string & fname, const std::set<int> & showers,
                TH1D * hGround, TCanvas * canvas) {
    std::cout << "=============================" << std::endl;
    std::cout << "file: " << fname << std::endl;

    crsRead::MCorsikaReader cr(fname, 1);
    crs::MRunHeader run;
    int i = 0;

    while (cr.GetRun(run)) {
        crs::MEventHeader shower;
        while (cr.GetShower(shower)) {
            i++;
            if (!showers.count(i))
                continue;

            hGround->Reset();

            std::cout << "-----------------------------" << std::endl;
            std::cout << "plot of the shower nr: " << i << std::endl;
            Double_t obsLev = shower.GetObservationHeight(shower.GetNObservationLevels() - 1) / 100.0;
            std::cout << "shower obs level: " << obsLev << " m" << std::endl;

            crs::TSubBlock sub_block;
            while (cr.GetData(sub_block)) {
                crs::MParticleBlock data(sub_block);

                if (data.GetBlockType() == crs::TSubBlock::ePARTDATA) {
                    for (crs::MParticleBlock::ParticleListConstIterator p_it = data.FirstParticle();
                         p_it != data.LastParticle(); ++p_it) {
                        crs::MParticle part(*p_it);

                        double px = part.GetPx();
                        double py = part.GetPy();
                        double pz = part.GetPz();

                        hGround->Fill(std::sqrt(px * px + py * py + pz * pz));
                    }
                }
            }

            // Shower finita di leggere: disegno e salvo subito
            drawAndSave(hGround, canvas, i, fname);
        }
    }

    // Controllo: quali shower richieste non esistono nel file?
    int status = 0;
    for (std::set<int>::const_iterator it = showers.begin(); it != showers.end(); ++it) {
        if (*it > i) {
            std::cout << "ERROR: shower " << *it << " not found (file contains "
                      << i << " showers)" << std::endl;
            status = -1;
        }
    }
    return status;
}

void drawAndSave(TH1D * hGround, TCanvas * canvas, int idx, const std::string & fname) {
    std::ostringstream title;
    title << "ground particle momentum for shower: " << idx;
    hGround->SetTitle(title.str().c_str());
    hGround->GetXaxis()->SetTitle("P [GeV/c]");

    canvas->cd();
    hGround->Draw();

    std::ostringstream outFname;
    outFname << "./plots/groundMomenta_" << baseName(fname) << "_" << idx << ".png";
    canvas->SaveAs(outFname.str().c_str());
}

std::string baseName(const std::string & path) {
    size_t pos = path.find_last_of('/');
    return (pos == std::string::npos) ? path : path.substr(pos + 1);
}

std::string usage() {
    return "usage: ./groundMomenta [-n <id>[,<id>...]]... <corsika file> [<corsika file> ...]\n"
           "  -n can be repeated and/or take a comma-separated list (default: shower 1)\n"
           "  e.g. ./groundMomenta -n 3,5,7 file1.dat file2.dat";
}

int getOptions(int argc, char ** argv, std::set<int> & showers) {
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