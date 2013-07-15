

//#include <ctype.h>

#include <Rtypes.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TObject.h>
#include <TPaveLabel.h>
#include <cassert>
//#include <cassert>
//#include <iostream>
#include <iostream>
//#include <list>
#include <list>
#include <new>
#include <string>
#include <xstring>

#include "StackedHistogramCreator.h"


namespace example {
	static const Int_t DEFAULT_HISTOGRAM_OBSERVABLES = 20000;

	/// Generates a histogram with a Gaussian frequency distribution.
	/// @param id the identification number of the histogram (used by ROOT)
	/// @param totalObservations the number of observations to include in the histogram
	/// @return the generated histogram
	TH1D* generateGausHistogram(Int_t id, Int_t totalObservations) {
		Int_t nbins = 100;
		Double_t xlow = -5;
		Double_t xup = 5;

		std::string histogramId = "histogram_";
		histogramId.append(std::to_string(id));

		TH1D *histogram = new TH1D(histogramId.c_str(), "Amazing Histogram", nbins, xlow, xup);
		histogram->FillRandom("gaus", totalObservations);

		return(histogram);
	}

	/// Generates a histogram with a Gaussian frequency distribution
	/// and {@code DEFAULT_HISTOGRAM_OBSERVABLES} observables.
	/// @see generateGausHistogram(Int_t, Int_t)
	TH1D* generateGausHistogram(Int_t id) {
		return(generateGausHistogram(id, DEFAULT_HISTOGRAM_OBSERVABLES));
	}

	/// Creates a list of pseudo Monte Carlo (MC) simulated histograms.
	/// @param numberOfHistograms the number of MC histograms to generate
	/// @return a list of generated histograms
	std::list<TH1D> createMCHistogramList(Int_t numberOfHistograms) {
		std::list<TH1D> histograms;

		for(Int_t i = 0 ; i < numberOfHistograms; i++) {
			TH1D *histogram = generateGausHistogram(i);
			histograms.push_back(*histogram);
		}

		assert(histograms.size() == numberOfHistograms);
		return(histograms);
	}

	/// Runs an example of the histogram display program.
	void runExample(Int_t numberOfSignalHistograms, Int_t numberOfNoiseHistograms) {
		TApplication *application = new TApplication("App", 0, 0);
		TPaveLabel *hello = new TPaveLabel(0.2, 0.4, 0.8, 0.6, "Hello World");

		TCanvas *canvas = new TCanvas("c", "Test Application", 400, 400);
		TH1D dataHistogram = *generateGausHistogram(-1);
		std::list<TH1D> histograms = createMCHistogramList(numberOfSignalHistograms + numberOfNoiseHistograms);

		StackedHistogramCreator *creator = new example::StackedHistogramCreator(
				dataHistogram, histograms);

		creator->drawAllHistograms();

		canvas->Update();
		application->Run();
	}
}


#ifndef __CINT__
int main(int argc, const char* argv[]) {

	example::runExample(1, 4);

//	Int_t option;
//	while((option = getopt(argc, argv, "abc:")) != -1) {
//
//	}



	std::cout << "Complete";
	return(0);
}
# endif
