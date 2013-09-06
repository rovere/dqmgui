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
#include <cassert>
#include "Rtypes.h"
#include "THStack.h"
#include "TObject.h"
#include "TText.h"
#include "TColor.h"
#include <numeric>

#include "TList.h"
#include <sstream>

#include "StackedHistogramRenderer.h"
#include "models/HistogramData.h"
#include "utils/builders/ScaledStackedHistogramBuilder.h"

namespace render {
	void StackedHistogramRenderer::render(
			TH1 *dataHistogram,
			std::vector<TH1*> histogramsToStack,
			std::string drawOptions) {
		if(dataHistogram == nullptr) {
			throw std::invalid_argument("dataHistogram cannot be null");
		}
		if(histogramsToStack.size() == 0) {
			throw std::invalid_argument("At least one histogram must be given to stack");
		}

		// Calculate what each the histograms that are to be stacked should be scaled by
		// to match the data histogram
		Double_t scalingFactor = StackedHistogramRenderer::calculateScalingFactor(
				dataHistogram, histogramsToStack);

		// Create the histogram stack builder
		ScaledStackedHistogramBuilder *stackedHistogramBuilder = new ScaledStackedHistogramBuilder(
				scalingFactor);

		// Add all of the histograms to the histogram stack builder
		for(Int_t i = 0; i < histogramsToStack.size(); i++) {
			TH1* histogram = histogramsToStack.at(i);
			HistogramData histogramData(histogram);
			stackedHistogramBuilder->addHistogramData(histogramData);
		}

		// Build the histogram stack object (THStack)
		THStack *histogramStack = stackedHistogramBuilder->build();

		// Draw the histogram stack and then the histogram on top
		histogramStack->Draw(drawOptions.c_str());
		dataHistogram->Draw("SAME" /*drawOptions.c_str()*/);	// FIXME: use drawOptions

		// TODO: Remove this temp debug code
//		TList *histograms = histogramStack->GetHists();
//		TH1 *h1 = (TH1*) histograms->At(0);
//		TH1 *h2 = (TH1*) histograms->At(1);
//		std::ostringstream stream;
//		stream << h1->Integral() << " + " << h2->Integral() << " = " << dataHistogram->Integral();
//		StackedHistogramRenderer::showErrorMessage("Debug:", stream.str());

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

	Double_t StackedHistogramRenderer::calculateScalingFactor(
			TH1 *dataHistogram, std::vector<TH1*> histogramsToStack) {
		Double_t totalStackArea = 0;

		for(Int_t i = 0; i < histogramsToStack.size(); i++) {
			TH1 *histogram = histogramsToStack.at(i);
			Double_t histogramArea= histogram->Integral();
			assert(histogramArea >= 0);
			totalStackArea += histogramArea;
		}

		if(totalStackArea == 0) {
			// Edge case - none of the histograms that are to be stacked have any area
			// so it doesn't matter what the scaling factor is
			return(1.0);
		}

		return(dataHistogram->Integral() / totalStackArea);
	}

//	std::vector<Double_t> StackedHistogramRenderer::calculateWeightingsFromNumberOfEntries(
//			std::vector<TH1*> histogramsToStack) {
//		std::vector<Double_t> stackedHistogramWeights;
//
//		Double_t totalHistogramArea = 0;
//		for(Int_t i = 0; i < histogramsToStack.size(); i++) {
//			TH1 *histogram = histogramsToStack.at(i);
//			assert(histogram->Integral() >= 0);
//			totalHistogramArea += histogram->Integral();
//		}
//
//		if(totalHistogramArea == 0) {
//			// Edge case - none of the histograms to stack have any area - do 0/1 not 0/0
//			totalHistogramArea = 1;
//		}
//
//		for(Int_t i = 0; i < histogramsToStack.size(); i++) {
//			TH1 *histogram = histogramsToStack.at(i);
//			Double_t histogramWeight = histogram->Integral() / totalHistogramArea;
//			assert(histogramWeight >= 0.0 && histogramWeight <= 1.0);
//			stackedHistogramWeights.push_back(histogramWeight);
//		}
//
//		assert(histogramsToStack.size() == stackedHistogramWeights.size());
////		assert(std::accumulate(stackedHistogramWeights.begin(), stackedHistogramWeights.end(), 0) == 1);
//		return(stackedHistogramWeights);
//	}
}
