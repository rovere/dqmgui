/*
 * RandomHistogramGenerator.cc
 *
 *  Created on: 24 Jul 2013
 *      Author: Colin - CERN
 */

#include "RandomHistogramGenerator.h"

#include <Rtypes.h>
#include <TH1.h>
#include <TObject.h>
#include <cassert>
#include <list>
#include <string>
#include <xstring>

namespace prototype {
	Int_t RandomHistogramGenerator::nextHistogramId = 0;

	TH1D* RandomHistogramGenerator::createGausHistogram(Int_t id) {
		return(createGausHistogram(id, DEFAULT_HISTOGRAM_ENTRIES));
	}

	TH1D* RandomHistogramGenerator::createGausHistogram(Int_t id, Int_t totalObservations) {
		Int_t nbins = 100;
		Double_t xlow = -5;
		Double_t xup = 5;

		std::string histogramId = "histogram_";
		histogramId.append(std::to_string(id));

		TH1D *histogram = new TH1D(histogramId.c_str(), "Gaussian Histogram", nbins, xlow, xup);
		histogram->FillRandom("gaus", totalObservations);

		return(histogram);
	}

	TH1D* RandomHistogramGenerator::createMCHistogram() {
		return(createGausHistogram(++nextHistogramId));
	}

	std::list<TH1D> RandomHistogramGenerator::createMCHistogramList(Int_t numberOfHistograms) {
		std::list<TH1D> histograms;

		for(Int_t i = 0 ; i < numberOfHistograms; i++) {
			TH1D *histogram = createGausHistogram(++nextHistogramId);
			histograms.push_back(*histogram);
		}

		assert(histograms.size() == numberOfHistograms);
		return(histograms);
	}
}
