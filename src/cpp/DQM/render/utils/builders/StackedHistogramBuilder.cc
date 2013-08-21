/*
 * StackedHistogramBuilder.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "StackedHistogramBuilder.h"

#include <Rtypes.h>
#include <TAttFill.h>
#include <TAttLine.h>
#include <TH1.h>
#include <THStack.h>
#include <TObject.h>
#include <cassert>
#include <vector>
#include <stdexcept>

#include "../../controllers/ColourController.h"
#include "../../models/display-data/HistogramStackData.h"
#include "../../models/display-data/WeightedHistogramData.h"
#include "../../utils/HistogramScalingUtil.h"

namespace render {
	StackedHistogramBuilder::StackedHistogramBuilder(Double_t targetHistogramArea) {
		this->setTargetHistogramArea(targetHistogramArea);
		this->histogramStackData = *(new HistogramStackData());
	}

	THStack* StackedHistogramBuilder::build() {
		if(this->histogramStackData.getHistogramsTotalWeight() != 1.0) {
			throw std::invalid_argument(
					"The sum weight of all histograms in the stack to be built must equal 1");
		}

		THStack *histogramStack = new THStack();
		std::vector<TH1*> histograms = this->getAllHistograms();
		this->addAllToHistogramStack(histograms, histogramStack);

		return(histogramStack);
	}

	void StackedHistogramBuilder::addWeightedHistogramData(WeightedHistogramData data) {
		// TODO: Consider using cloning here
		HistogramScalingUtil::scaleWeightedHistogram(data, this->getTargetHistogramArea());
		this->histogramStackData.add(data);
	}

	void StackedHistogramBuilder::addHistogramStackData(HistogramStackData data) {
		std::vector<WeightedHistogramData> allWeightedHistogramsData = data.getAllHistogramsData();
		std::vector<WeightedHistogramData>::iterator it = allWeightedHistogramsData.begin();

		while(it != allWeightedHistogramsData.end()) {
			WeightedHistogramData weightedHistogramData = *it;
			this->addWeightedHistogramData(weightedHistogramData);
			it++;
		}
	}

	void StackedHistogramBuilder::setTargetHistogramArea(Double_t targetHistogramArea) {
		assert(targetHistogramArea >= 0);
		this->targetHistogramArea = targetHistogramArea;
	}

	std::vector<TH1*> StackedHistogramBuilder::getAllHistograms() {
		std::vector<WeightedHistogramData> allHistogramsData = this->histogramStackData.getAllHistogramsData();
		std::vector<WeightedHistogramData>::iterator it = allHistogramsData.begin();
		std::vector<TH1*> histograms;

		while(it != allHistogramsData.end()) {
			WeightedHistogramData *weightedHistogramData = &(*it);
			TH1 *histogram = weightedHistogramData->getHistogram();
			histograms.push_back(histogram);
			it++;
		}

		assert(histograms.size() == allHistogramsData.size());
		return(histograms);
	}

	Double_t StackedHistogramBuilder::getTargetHistogramArea() {
		return(this->targetHistogramArea);
	}

	// XXX: The way in which the THStack object is passed in, looks like this method
	//		should be static...
	void StackedHistogramBuilder::addToHistogramStack(TH1 &histogram, THStack *histogramStack) {
		#ifdef DNDEBUG
		TObjArray *histogramsInStack = histogramStack->GetStack();
		Int_t stackSize = (histogramsInStack == nullptr)
								? 0
								: histogramsInStack->GetSize();
		#endif

		Int_t colour = this->colourController.getNextColour();

		histogram.SetFillColor(colour);
		histogram.SetLineColor(ColourController::COLOUR_BLACK);
		histogramStack->Add(&histogram);

		#ifdef DNDEBUG
		Int_t modifiedStackSize = histogramStack->GetStack()->GetSize();
		assert(modifiedStackSize == (stackSize + 1));
		#endif
	}

	// XXX: The way in which the THStack object is passed in, looks like this method
	//		should be static...
	void StackedHistogramBuilder::addAllToHistogramStack(
			std::vector<TH1*> histograms, THStack *histogramStack) {
		#ifdef DNDEBUG
		TObjArray *histogramsInStack = histogramStack->GetStack();
		Int_t stackSize = (histogramsInStack == nullptr)
								? 0
								: histogramsInStack->GetSize();
		#endif

		std::vector<TH1*>::iterator it = histograms.begin();

		while(it != histograms.end()) {
			TH1 *histogram = (*it);
			this->addToHistogramStack(*histogram, histogramStack);
			it++;
		}

		#ifdef DNDEBUG
		Int_t modifiedStackSize = histogramStack->GetStack()->GetSize();
		assert(modifiedStackSize == (stackSize + histograms.size()));
		#endif
	}
}
