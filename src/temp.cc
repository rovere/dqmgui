#include <iostream>
#include <string>

#include <TApplication.h>
#include <TCanvas.h>
#include <TPaveLabel.h>
#include <TF1.h>
#include <TH1D.h>
#include <THStack.h>
#include <TRandom.h>


void test() {
	TF1 *plot = new TF1("tmp", "sin(x)", 0, 6.3);
	plot->Draw();
}

///
TH1D* generateGausHistogram(Int_t id) {
	Int_t nbins = 100;
	Double_t xlow = -5;
	Double_t xup = 5;

	std::string histogramId = "histogram";
	histogramId.append(std::to_string(id));


	std::cout << histogramId.c_str();


	TH1D *histogram = new TH1D(histogramId.c_str(), "Amazing Histogram", nbins, xlow, xup);
	histogram->FillRandom("gaus", 20000);

	return(histogram);
}

///
void drawHistogramStack() {
	THStack *histogramStack = new THStack("histogramStack", "Stacked Histograms");

	Int_t colours[] = {kRed, kGreen, kBlue, kYellow};


	for(Int_t i = 0 ; i < sizeof(colours); i++) {
		TH1D *histogram = generateGausHistogram(i);
		histogram->SetFillColor(colours[i]);
		histogramStack->Add(histogram);
		histogram->Draw();
	}

	histogramStack->Draw();
}


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
