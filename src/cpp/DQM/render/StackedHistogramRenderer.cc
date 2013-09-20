/*
 * StackedHistogramRenderer.cc
 *
 *  Created on: 16 Aug 2013
 *      Author: Colin Nolan
 */
#define DNDEBUG

#include "Renderer.h"
#include "StackedHistogramRenderer.h"

#include <Rtypes.h>
#include <TH1.h>
#include <THStack.h>
#include <TObject.h>
#include <TColor.h>
#include <TText.h>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <string>
#include <sstream>

#include "models/AbstractHistogramStackData.h"
#include "models/HistogramData.h"
#include "models/HistogramStackData.h"
#include "utils/builders/ScaledStackedHistogramBuilder.h"
#include "utils/builders/StackedHistogramBuilder.h"
#include "MessageRenderer.h"

namespace render {
	StackedHistogramRenderer::StackedHistogramRenderer(
			HistogramData observedData, HistogramStackData histogramStackData)
			: observedData(observedData),
			  histogramStackData(histogramStackData) {
		if(observedData.getHistogram() == nullptr) {
			throw std::invalid_argument("dataHistogram cannot be null");
		}
		if(histogramStackData.getAllHistograms().size() == 0) {
			throw std::invalid_argument("Data about at least one histogram must be given to create stack");
		}
	}

	void StackedHistogramRenderer::render() {
		TH1* dataHistogram = this->observedData.getHistogram();

		// Calculate what each the histograms that are to be stacked should be scaled by
		// to match the data histogram
		Double_t scalingFactor = StackedHistogramRenderer::calculateScalingFactor(
				dataHistogram, this->histogramStackData.getAllHistograms());

		// Create the histogram stack builder
		ScaledStackedHistogramBuilder *stackedHistogramBuilder = new ScaledStackedHistogramBuilder(
				scalingFactor);

		// Add all of the histograms to the histogram stack builder
		std::vector<HistogramData> allHistogramData = this->histogramStackData.getAllHistogramData();
		for(Int_t i = 0; i < allHistogramData.size(); i++) {
			HistogramData histogramData = allHistogramData.at(i);
			stackedHistogramBuilder->addHistogramData(histogramData);
		}

		// Build the histogram stack object (THStack)
		THStack *histogramStack = stackedHistogramBuilder->build();
//		delete stackedHistogramBuilder;		// FIXME: This causes a double free error to occur - find out why...
		this->storeRootObjectPointer(histogramStack);

		// XXX: This is a very hacky way of ensuring that the y-axis has the correct range
		//		to show both the histogram and the stacked histogram.
		//
		//		The axis is drawn for the first histogram. Therefore, draw the histogram
		//		that needs the largest axis first. If this is the data histogram however,
		// 		it means that the data histogram is drawn twice on the same canvas! This
		//		is bad practice but has no visual impact. This developer was unable to find
		//		another solution to the problem of y-axis sizing at the current time.
		Bool_t useAxisFromDataHistogram = dataHistogram->GetMaximum() > histogramStack->GetMaximum();
		if(useAxisFromDataHistogram) {
			dataHistogram->Draw();
		}

		// Draw the histogram stack and then the histogram on top
		histogramStack->Draw((useAxisFromDataHistogram ? "same" : ""));
		dataHistogram->SetLineColor(kBlack);
		// Note: binding string "SAMES" to first parameter - the extra 'S' is not a typo!
		// It is required to draw the statistics box
		dataHistogram->Draw("SAMES");			// TODO: Use draw options?

		// XXX: Remove debug
//		this->debug();
	}

	void StackedHistogramRenderer::debug() {
		std::ostringstream debugMessage;

		std::vector<TH1*> allHistograms = this->histogramStackData.getAllHistograms();
		for(Int_t i = 0; i < allHistograms.size(); i++) {
			TH1* histogram = allHistograms.at(i);
			debugMessage << "Histogram " << i << ": " << histogram->Integral() << ", ";
		}

		debugMessage << "Data: " << this->observedData.getHistogram()->Integral();

		MessageRenderer *messageRenderer = new MessageRenderer();
		messageRenderer->showErrorMessage("Debug:", debugMessage.str());
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
}
