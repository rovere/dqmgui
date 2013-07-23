/*
 * StackedHistogramCreator.cpp
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include <Rtypes.h>
#include <TAttFill.h>
#include <TAttLine.h>
#include <TH1.h>
#include <cassert>
#include <list>

#include <TObject.h>
#include <xstring>

#include <TCollection.h>
#include <THStack.h>
#include <TObjArray.h>
#include <cassert>
#include <list>

#include "models/HistogramDisplayData.h"
#include "models/HistogramStackDisplayData.h"
#include "StackedHistogramBuilder.h"
#include "controllers/ColourController.h"

namespace prototype {
	const std::string DEFAULT_STACK_LABEL = "MC vs Data";	// FIXME: Localisation required?
	const std::string DEFAULT_STACK_NAME = "MC vs Data";	// XXX: Does this have to be globally unique - what exactly is a label?

	StackedHistogramBuilder::StackedHistogramBuilder(HistogramStackDisplayData displayData)
			: displayData(displayData) {
		this->histogramStack = new THStack(
				DEFAULT_STACK_LABEL.c_str(),
				DEFAULT_STACK_NAME.c_str());
	}

	THStack StackedHistogramBuilder::build() {
		return(*this->histogramStack);
	}

	void StackedHistogramBuilder::addToHistogramStack(TH1D &histogram) {
		#ifdef DNDEBUG
		TObjArray *histogramStack = this->histogramStack->GetStack();
		Int_t stackSize = (histogramStack == nullptr)
								? 0
								: histogramStack->GetSize();
		#endif

		Int_t colour = this->colourController.getNextColour();

		histogram.SetFillColor(colour);
		histogram.SetLineColor(ColourController::COLOUR_BLACK);
		this->histogramStack->Add(&histogram);

		#ifdef DNDEBUG
		Int_t modifiedStackSize = this->histogramStack->GetStack()->GetSize();
		assert(modifiedStackSize == (stackSize + 1));
		#endif
	}

	void StackedHistogramBuilder::addAllToHistogramStack(std::list<TH1D*> histograms) {
		#ifdef DNDEBUG
		TObjArray *histogramStack = this->histogramStack->GetStack();
		Int_t stackSize = (histogramStack == nullptr)
								? 0
								: histogramStack->GetSize();
		#endif

		std::list<TH1D*>::iterator it = histograms.begin();

		while(it != histograms.end()) {
			TH1D *histogram = (*it);
			this->addToHistogramStack(*histogram);
			it++;
		}

		#ifdef DNDEBUG
		Int_t modifiedStackSize = this->histogramStack->GetStack()->GetSize();
		assert(modifiedStackSize == (stackSize + histograms.size()));
		#endif
	}

	std::list<TH1D*> StackedHistogramBuilder::getAllHistograms() {
		std::list<HistogramDisplayData> histogramDisplayData = this->displayData.getAllHistogramDisplayData();
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
