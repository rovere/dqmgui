/*
 * ScaledStackedHistogramBuilder.cc
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */

#include "ScaledStackedHistogramBuilder.h"

#include <TH1.h>
#include <THStack.h>
#include <TObject.h>
#include <cassert>
#include <vector>

#include "Builder.h"

class TH1;

namespace render {
	THStack* ScaledStackedHistogramBuilder::build() {
		THStack *histogramStack = new THStack();
		std::vector<TH1*> histograms = this->histogramStackData.getAllHistograms();
		ScaledStackedHistogramBuilder::addAllToTHStack(histograms, histogramStack);

		return(histogramStack);
	}

	void ScaledStackedHistogramBuilder::addToTHStack(TH1 *histogram, THStack *histogramStack) {
		#ifdef DNDEBUG
		TObjArray *histogramsInStack = histogramStack->GetStack();
		Int_t stackSize = (histogramsInStack == nullptr)
								? 0
								: histogramsInStack->GetSize();
		#endif

		// TODO: Sort this out...
//		Int_t colour = this->colourController.getNextColour();
//
//		histogram->SetFillColor(colour);
//		histogram->SetLineColor(ColourController::COLOUR_BLACK);
		histogramStack->Add(histogram);

		#ifdef DNDEBUG
		Int_t modifiedStackSize = histogramStack->GetStack()->GetSize();
		assert(modifiedStackSize == (stackSize + 1));
		#endif
	}

	void ScaledStackedHistogramBuilder::addAllToTHStack(
			std::vector<TH1*> histograms, THStack *histogramStack) {
		#ifdef DNDEBUG
		TObjArray *histogramsInStack = histogramStack->GetStack();
		Int_t stackSize = (histogramsInStack == nullptr)
								? 0
								: histogramsInStack->GetSize();
		#endif

		std::vector<TH1*>::iterator it = histograms.begin();

		while(it != histograms.end()) {
			TH1 *histogram = *it;
			ScaledStackedHistogramBuilder::addToTHStack(histogram, histogramStack);
			it++;
		}

		#ifdef DNDEBUG
		Int_t modifiedStackSize = histogramStack->GetStack()->GetSize();
		assert(modifiedStackSize == (stackSize + histograms.size()));
		#endif
	}
}
