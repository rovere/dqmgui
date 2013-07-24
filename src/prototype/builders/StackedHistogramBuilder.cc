/*
 * StackedHistogramCreator.cpp
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "StackedHistogramBuilder.h"

#include <Rtypes.h>
#include <TAttFill.h>
#include <TAttLine.h>
#include <TCollection.h>
#include <TH1.h>
#include <THStack.h>
#include <TObjArray.h>
#include <TObject.h>
#include <cassert>
#include <list>
#include <stdexcept>
#include <xstring>
#include <iostream>

#include "../controllers/ColourController.h"
#include "../models/HistogramDisplayData.h"
#include "../models/HistogramStackDisplayData.h"
#include "../utils/HistogramNormalisationUtil.h"

namespace prototype {
	StackedHistogramBuilder::StackedHistogramBuilder() {
		;
	}

	void StackedHistogramBuilder::addHistogramDisplayData(HistogramDisplayData data) {
		HistogramNormalisationUtil::normaliseWeightedHistogram(data);
		this->histogramStackDisplayData.add(data);
	}

	void StackedHistogramBuilder::addHistogramStackDisplayData(HistogramStackDisplayData data) {
		HistogramNormalisationUtil::normaliseWeightedHistograms(data.getAllHistogramDisplayData());
		this->histogramStackDisplayData.add(data);
	}

	THStack StackedHistogramBuilder::build() {
		if(this->histogramStackDisplayData.getHistogramsTotalWeight() != 1.0) {
			throw std::invalid_argument(
					"The sum weight of all histograms in the stack to be built must equal 1");
		}

		THStack *histogramStack = new THStack();
		std::list<TH1D*> histograms = this->getAllHistograms();
		this->addAllToHistogramStack(histograms, histogramStack);

		return(*histogramStack);
	}

	void StackedHistogramBuilder::addToHistogramStack(TH1D &histogram, THStack *histogramStack) {
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

	void StackedHistogramBuilder::addAllToHistogramStack(
			std::list<TH1D*> histograms, THStack *histogramStack) {
		#ifdef DNDEBUG
		TObjArray *histogramsInStack = histogramStack->GetStack();
		Int_t stackSize = (histogramsInStack == nullptr)
								? 0
								: histogramsInStack->GetSize();
		#endif

		std::list<TH1D*>::iterator it = histograms.begin();

		while(it != histograms.end()) {
			TH1D *histogram = (*it);
			this->addToHistogramStack(*histogram, histogramStack);
			it++;
		}

		#ifdef DNDEBUG
		Int_t modifiedStackSize = histogramStack->GetStack()->GetSize();
		assert(modifiedStackSize == (stackSize + histograms.size()));
		#endif
	}

	std::list<TH1D*> StackedHistogramBuilder::getAllHistograms() {
		std::list<HistogramDisplayData> histogramDisplayData = this->
				histogramStackDisplayData.getAllHistogramDisplayData();
		std::list<HistogramDisplayData>::iterator it = histogramDisplayData.begin();
		std::list<TH1D*> histograms;

		while(it != histogramDisplayData.end()) {
			HistogramDisplayData *histogramWeightPair = &(*it);
			TH1D *histogram = histogramWeightPair->getHistogram();
			histograms.push_back(histogram);
			it++;
		}

		assert(histograms.size() == histogramDisplayData.size());
		return(histograms);
	}
}
