#include <cassert>

#include "HistogramWeightPair.h"

#include <cassert>

#include <TH1.h>
#include <cassert>
#include <iostream>
#include <list>

#include <Rtypes.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TH1F.h>
#include <TObject.h>
#include <TPaveLabel.h>
#include <cassert>
#include <iostream>
#include <list>
#include <new>
#include <string>
#include <xstring>

#include "StackedHistogramCreator.h"

namespace prototype {
	static const Int_t DEFAULT_HISTOGRAM_ENTRIES = 50000;

	Int_t nextHistogramId = 0;

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

		TH1D *histogram = new TH1D(histogramId.c_str(), "Gaussian Histogram", nbins, xlow, xup);
		histogram->FillRandom("gaus", totalObservations);

		return(histogram);
	}

	/// Generates a histogram with a Gaussian frequency distribution
	/// and <code>DEFAULT_HISTOGRAM_ENTRIES</code> entries.
	/// @see generateGausHistogram(Int_t, Int_t)
	TH1D* generateGausHistogram(Int_t id) {
		return(generateGausHistogram(id, DEFAULT_HISTOGRAM_ENTRIES));
	}

	/// Creates a list of pseudo Monte Carlo (MC) simulated histograms.
	/// @param numberOfHistograms the number of MC histograms to generate
	/// @return a list of generated histograms
	std::list<TH1D> createMCHistogramList(Int_t numberOfHistograms) {
		std::list<TH1D> histograms;

		for(Int_t i = 0 ; i < numberOfHistograms; i++) {
			TH1D *histogram = generateGausHistogram(++nextHistogramId);
			histograms.push_back(*histogram);
		}

		assert(histograms.size() == numberOfHistograms);
		return(histograms);
	}

	/// TODO: Comment
	TH1D* createMCHistogram() {
		return(generateGausHistogram(++nextHistogramId));
	}

	/// Runs an example of the histogram display program.
	void runExample(Int_t numberOfMCHistograms, Double_t weights[]) {
//		assert((sizeof(weights) / sizeof(Double_t)) == numberOfMCHistograms);

		TApplication *application = new TApplication("App", 0, 0);
//		TPaveLabel *tPaveLabel = new TPaveLabel(0.2, 0.4, 0.8, 0.6, "TPaveLabel not set");

		TCanvas *canvas = new TCanvas("c", "Test Application", 400, 400);
		TH1D dataHistogram = *generateGausHistogram(-1);
		std::list<HistogramWeightPair> histogramWeightPairs;

		for(Int_t i = 0; i < numberOfMCHistograms; i++) {
			TH1D *histogram = createMCHistogram();
			Double_t weight = weights[i];

			HistogramWeightPair *histogramWeightPair = new HistogramWeightPair(histogram, weight);
			histogramWeightPairs.push_back(*histogramWeightPair);
		}

		StackedHistogramCreator *creator = new prototype::StackedHistogramCreator(
				dataHistogram, histogramWeightPairs);
		creator->drawAllHistograms();

		canvas->Update();
		application->Run();
	}
}


#ifndef __CINT__
int main(int argc, const char* argv[]) {
	Double_t weights[] = {0.6, 0.1, 0.3};

	Int_t numberOfWeights = sizeof(weights) / sizeof(Double_t);
	prototype::runExample(numberOfWeights, weights);

	std::cout << "Complete";
	return(0);
}
# endif
