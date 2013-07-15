#include <iostream>
#include <string>

#include <TApplication.h>
#include <TCanvas.h>
#include <TPaveLabel.h>
#include <TF1.h>
#include <TH1D.h>
#include <THStack.h>
#include <TRandom.h>


void plotSimpleGraph() {
	TF1 *plot = new TF1("tmp", "sin(x)", 0, 6.3);
	plot->Draw();
}

///
TH1D* generateGausHistogram(Int_t id) {
	Int_t nbins = 100;
	Double_t xlow = -5;
	Double_t xup = 5;

	std::string histogramId = "histogram_";
	histogramId.append(std::to_string(id));

	TH1D *histogram = new TH1D(histogramId.c_str(), "Amazing Histogram", nbins, xlow, xup);
	histogram->FillRandom("gaus", 20000);

	return(histogram);
}

///
void drawHistogramStack() {
	THStack *histogramStack = new THStack("histogramStack", "Stacked Histograms");

	const Int_t colours[] = {kRed, kGreen, kBlue, kYellow};
	const Int_t numberOfColours = sizeof(colours) / sizeof(Int_t);

	for(Int_t i = 0 ; i < numberOfColours; i++) {
		TH1D *histogram = generateGausHistogram(i);
		histogram->SetFillColor(colours[i]);
		histogramStack->Add(histogram);
		histogram->Draw();
	}

	histogramStack->Draw();

	TH1D *histogram2 = new TH1D("h2", "2", 1000, -5, 5);
	histogram2->FillRandom("gaus", 200000);
	histogram2->Draw("same");
}

/*
#ifndef __CINT__
int main() {
	TApplication application("App", 0, 0);
	TCanvas *canvas = new TCanvas("c", "Test Application", 400, 400);
	TPaveLabel *hello = new TPaveLabel(0.2,0.4,0.8,0.6,"Hello World");

	drawHistogramStack();
	canvas->Update();

	application.Run();

	return(0);
}
# endif
*/
