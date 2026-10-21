#include <crsRead/MCorsikaReader.h>
#include <crsRead/MLongfileReader.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <iostream>
#include <string>
#include <sstream>
#include "TAxis.h"
#include "TLegend.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TPad.h"

std::string usage();
int getOptions(int argc, char ** argv, int & n);

int main(int argc, char ** argv) {
    int shIndex = 1;
    const int nOptions = getOptions(argc, argv, shIndex);
    if (nOptions < 0 || argc - nOptions < 1) {
        std::cout << usage() << std::endl;
        return 1;
    }

    std::string fname(argv[nOptions]);
    crsRead::MLongfileReader lr(fname, 3);

    if (lr.ReadShower(shIndex)) {
        const int nBins = lr.GetNBinsEdep() - 1;

        TGraph gTotal(nBins);
        TGraph gMuons(nBins);
        TGraph gEM(nBins);

        TCanvas cLong("cLong", "Longitudinal Development", 1600, 900);

        for (int iDep = 0; iDep < nBins; iDep++) {
            const crsRead::MLongfileReader::ParticleEntry & nEntry = lr.GetEntry(iDep);
            //const crsRead::MLongfileReader::EdepEntry& dEntry = lr.GetEntryEdep(iDep);

            int nEM = nEntry.Nelectron + nEntry.Npositron + nEntry.Ngamma;
            int nMu = nEntry.Nmu + nEntry.Nmu_bar;

            gTotal.SetPoint(iDep, nEntry.X, nEM + nMu);
            gMuons.SetPoint(iDep, nEntry.X, nMu);
            gEM.SetPoint(iDep, nEntry.X, nEM);
        }

        gTotal.SetTitle("Longitudinal Particle Development");
        gTotal.GetXaxis() -> SetTitle("X [g/cm^{2}]");
        gTotal.GetYaxis() -> SetTitle("Number of Particles");

        gTotal.SetLineColor(kBlack);
        gTotal.SetLineWidth(4);

        gMuons.SetLineColor(kRed);
        gMuons.SetLineStyle(2);
        gMuons.SetLineWidth(3);

        gEM.SetLineColor(kBlue);
        gEM.SetLineStyle(3);
        gEM.SetLineWidth(3);

        gTotal.Draw("AL");
        gMuons.Draw("L SAME");
        gEM.Draw("L SAME");
        gPad -> SetLogy();

        TLegend legend(0.7, 0.75, 0.9, 0.9);
        legend.AddEntry( & gTotal, "Total Particles", "l");
        legend.AddEntry( & gMuons, "Muons", "l");
        legend.AddEntry( & gEM, "EM Particles", "l");
        legend.Draw();

        std::ostringstream outFname;
        outFname << "./plots/long_particles_" << fname << "_" << shIndex << ".png";
        cLong.SaveAs(outFname.str().c_str());
    }
}

std::string usage() {
    return "usage: ./longReader -n <#shower> <corsika file name>";
}

int getOptions(int argc, char ** argv, int & n) {
    int c;
    while ((c = getopt(argc, argv, "n:h")) != -1) {
        switch (c) {
        case 'n':
            n = std::atoi(optarg);
            break;
        case 'h':
            return -2;
        default:
            return -2;
        }
    }
    return optind;
}