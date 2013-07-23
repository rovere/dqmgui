/*
 * StackedHistogramCreator.cc
 *
 *  Created on: 10 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "StackedHistogramCreator.h"

#include <Rtypes.h>
#include <TAttFill.h>
#include <TAttLine.h>
#include <TH1.h>
#include <THStack.h>
#include <TObject.h>
#include <cassert>
#include <list>
#include <string>
#include <iostream>

#include "HistogramNormalisationUtil.h"

namespace prototype {
	const std::string StackedHistogramCreator::DEFAULT_STACK_LABEL = "MC vs Data";	// FIXME: Localisation required?
	const std::string StackedHistogramCreator::DEFAULT_STACK_NAME = "MC vs Data";	// XXX: Does this have to be globally unique - what exactly is a label?

	const Int_t StackedHistogramCreator::DEFAULT_COLOURS[] = {
			kRed, kGreen, kBlue, kYellow, kTeal, kGray, kOrange};
	const Int_t StackedHistogramCreator::COLOUR_BLACK = kBlack;


	StackedHistogramCreator::StackedHistogramCreator(
			TH1D dataHistogram,
			std::list<HistogramDisplayData> histogramWeightPairs)
				: dataHistogram(dataHistogram),
				  histogramDisplayData(histogramWeightPairs),
				  colourIndex(0) {

		this->histogramStack = new THStack(
				DEFAULT_STACK_LABEL.c_str(),
				DEFAULT_STACK_NAME.c_str());

		HistogramNormalisationUtil::normaliseHistograms(&this->histogramDisplayData);
	}

	void StackedHistogramCreator::drawAllHistograms() {
		std::list<TH1D*> histograms = this->getAllHistograms();

		// XXX: It is assumed that all of the histograms have been normalised at this point...
		this->addAllToHistogramStack(histograms);
		this->histogramStack->Draw();

		HistogramNormalisationUtil::normaliseHistogram(&this->dataHistogram);
		this->dataHistogram.SetLineColor(COLOUR_BLACK);
		this->dataHistogram.SetLineStyle(2);
		this->dataHistogram.SetLineWidth(5);
		this->dataHistogram.Draw("SAME");
	}

	Int_t StackedHistogramCreator::getNextColour() {
		const Int_t numberOfColours = sizeof(DEFAULT_COLOURS) / sizeof(Int_t);

		assert(this->colourIndex < numberOfColours);
		assert(this->colourIndex >= 0);

		Int_t index = colourIndex;
		Int_t colour = DEFAULT_COLOURS[colourIndex];

		colourIndex++;
		if(colourIndex == numberOfColours) {
			colourIndex = 0;
		}

		return(colour);
	}

	void StackedHistogramCreator::addToHistogramStack(TH1D &histogram) {
		#ifdef DNDEBUG
		TObjArray *histogramStack = this->histogramStack->GetStack();
		Int_t stackSize = (histogramStack == nullptr)
								? 0
								: histogramStack->GetSize();
		#endif

		Int_t colour = getNextColour();

		histogram.SetFillColor(colour);
		histogram.SetLineColor(COLOUR_BLACK);
		this->histogramStack->Add(&histogram);

		#ifdef DNDEBUG
		Int_t modifiedStackSize = this->histogramStack->GetStack()->GetSize();
		assert(modifiedStackSize == (stackSize + 1));
		#endif
	}

	void StackedHistogramCreator::addAllToHistogramStack(std::list<TH1D*> histograms) {
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

	std::list<TH1D*> StackedHistogramCreator::getAllHistograms() {
		std::list<HistogramDisplayData>::iterator it = this->histogramDisplayData.begin();
		std::list<TH1D*> histograms;

		while(it != this->histogramDisplayData.end()) {
			HistogramDisplayData *histogramWeightPair = &(*it);
			TH1D *histogram = histogramWeightPair->getHistogram();
			histograms.push_back(histogram);
			it++;
		}

		assert(histograms.size() == this->histogramDisplayData.size());
		return(histograms);
	}
}
