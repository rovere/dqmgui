/*
 * StackedHistogramRenderer.cc
 *
 *  Created on: 16 Aug 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include <string>
#include <vector>
#include <stdexcept>
#include "Rtypes.h"
#include "TH1F.h"
#include "THStack.h"
#include "TObject.h"

#include "StackedHistogramRenderer.h";
#include "utils/builders/StackedHistogramBuilder.h";

namespace render {
	std::string StackedHistogramRenderer::render(
			TH1F *dataHistogram,
			std::vector<TH1D> monteCarloHistograms,
			std::vector<Double_t> monteCarloHistogramWeights,
			std::string drawOptions) {
		if(dataHistogram == nullptr) {
			throw std::invalid_argument("dataHistogram cannot be null");
		}
		if(monteCarloHistograms.size() != monteCarloHistogramWeights.size()) {
			throw std::invalid_argument("A single weight must be associated to all histograms");
		}

		Double_t histogramArea = dataHistogram->Integral();
		StackedHistogramBuilder *stackedHistogramBuilder = new StackedHistogramBuilder(histogramArea);

		for(Int_t i = 0; i < monteCarloHistograms.size(); i++) {
			TH1D histogram = monteCarloHistograms.at(i);
			Double_t histogramWeight = monteCarloHistogramWeights.at(i);

			WeightedHistogramData *weightedHistogramData = new WeightedHistogramData(
					&histogram, histogramWeight);
			stackedHistogramBuilder->addWeightedHistogramData(*weightedHistogramData);
		}

//		THStack histogramStack = stackedHistogramBuilder->build();

		// TODO: Check that these get drawn on the same canvas
//		histogramStack.Draw();		// Does this require draw options that are dependent on certain settings?
		dataHistogram->Draw(drawOptions.c_str());

		return("Complete!");
	}
}
