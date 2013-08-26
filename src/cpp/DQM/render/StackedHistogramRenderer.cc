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
#include "THStack.h"
#include "TObject.h"
#include "TText.h"
#include "TColor.h"

#include <sstream>

#include "StackedHistogramRenderer.h"
#include "utils/builders/StackedHistogramBuilder.h"

namespace render {
	void StackedHistogramRenderer::render(
			TH1 *dataHistogram,
			std::vector<TH1*> monteCarloHistograms,
			std::vector<Double_t> monteCarloHistogramWeights,
			std::string drawOptions) {
		if(dataHistogram == nullptr) {
			throw std::invalid_argument("dataHistogram cannot be null");
		}
		if(monteCarloHistograms.size() != monteCarloHistogramWeights.size()) {
			throw std::invalid_argument("A (single) weight must be associated to all histograms");
		}

		Double_t histogramArea = dataHistogram->Integral();
		StackedHistogramBuilder *stackedHistogramBuilder = new StackedHistogramBuilder(histogramArea);

		for(Int_t i = 0; i < monteCarloHistograms.size(); i++) {
			TH1 *histogram = monteCarloHistograms.at(i);
			Double_t histogramWeight = monteCarloHistogramWeights.at(i);

			WeightedHistogramData *weightedHistogramData = new WeightedHistogramData(
					histogram, histogramWeight);

			stackedHistogramBuilder->addWeightedHistogramData(*weightedHistogramData);
		}

		THStack *histogramStack = stackedHistogramBuilder->build();
		delete stackedHistogramBuilder;

		// TODO: Check that these get drawn on the same canvas
		dataHistogram->Draw(drawOptions.c_str());
		histogramStack->Draw("SAME");

//		TList *histograms = histogramStack->GetHists();
//		TH1 *h1 = (TH1*) histograms->At(0);
//		TH1 *h2 = (TH1*) histograms->At(1);
//		std::ostringstream stream;
//		stream << *(dataHistogram->GetIntegral());
//		throw std::invalid_argument(stream.str());

//		delete histogramStack;			// This cannot be deleted else the histogramStack is not drawn,
										// indicating deletion it is this code's responsibility
	}

	// XXX: There are a lot of magic numbers floating around in this method...
	void StackedHistogramRenderer::showErrorMessage(
			std::string histogramTitleText, std::string errorMessageText) {
		Color_t TEXT_COLOR = TColor::GetColor(178, 32, 32);		// TODO: Considering making this a static class constant

		TText *histogramTitle = new TText(.5, .58, histogramTitleText.c_str());
		TText *errorMessage = new TText(.5, .42, errorMessageText.c_str());

		TText* textElements[] = {histogramTitle, errorMessage};
		Int_t numberOfTextElements = sizeof(textElements) / sizeof(TText*);

		for(Int_t i = 0; i < numberOfTextElements; i++) {
			TText *textElement = textElements[i];
			textElement->SetNDC(true);
			textElement->SetTextSize(0.10);
			textElement->SetTextAlign(22);
			textElement->SetTextColor(TEXT_COLOR);
			textElement->Draw();
		}

//		delete [] textElements;			// This cannot be deleted else the histogramStack is not drawn,
										// indicating deletion it is this code's responsibility
	}
}
