/*
 * renderStackedHistogram.cc
 *
 *  Created on: 12 Aug 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include <string>
#include <vector>
#include <stdexcept>
#include "Rtypes.h"
#include "TH1.h"
#include "THStack.h"

#include "builders/StackedHistogramBuilder.h";
#include "models/display-data/WeightedHistogramData.h";
#include "models/display-data/HistogramStackData.h"

namespace render {
	// XXX: This should be made more OO, which would also solve the excessive
	// 		parameters anti-pattern.
	void renderStackedHistogram(
			TH1 dataHistogram,
			std::vector<TH1D> monteCarloHistograms,
			std::vector<Double_t> monteCarloHistogramWeights,
			std::string drawOptions) {
		if(monteCarloHistograms.size() != monteCarloHistogramWeights.size()) {
			throw std::invalid_argument("A single weight must be associated to all histograms");
		}

		Double_t histogramArea = dataHistogram.Integral();
		StackedHistogramBuilder *stackedHistogramBuilder = new StackedHistogramBuilder(histogramArea);

		for(Int_t i = 0; i < monteCarloHistograms.size(); i++) {
			TH1D histogram = monteCarloHistograms.at(i);
			Double_t histogramWeight = monteCarloHistogramWeights.at(i);

			WeightedHistogramData *weightedHistogramData = new WeightedHistogramData(
					&histogram, histogramWeight);
			stackedHistogramBuilder->addWeightedHistogramData(*weightedHistogramData);
		}

		THStack histogramStack = stackedHistogramBuilder->build();

		// TODO: Check that these get drawn on the same canvas
		histogramStack.Draw();		// Does this require draw options that are dependent on certain settings?
		dataHistogram.Draw(drawOptions.c_str());
	}
}
