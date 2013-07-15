/*
 * StackedHistogramCreator.cc
 *
 *  Created on: 10 Jul 2013
 *      Author: Colin - CERN
 */

#include "StackedHistogramCreator.h"

#include <Rtypes.h>
#include <TAttFill.h>
#include <TAttLine.h>
#include <TH1.h>
#include <THStack.h>
#include <TObject.h>
#include <cassert>
#include <iostream>
#include <list>
#include <string>


namespace example {
	const std::string StackedHistogramCreator::DEFAULT_STACK_LABEL = "MC vs Data";			// FIXME: Localisation required?
	const std::string StackedHistogramCreator::DEFAULT_STACK_NAME = "MonteCarloVsData";		// XXX: Does this have to be globally unique - what exactly is a label?

	const Int_t StackedHistogramCreator::DEFAULT_COLOURS[] = {kRed, kGreen, kBlue, kYellow, kTeal, kGray, kOrange};
	const Int_t StackedHistogramCreator::COLOUR_BLACK = kBlack;


	StackedHistogramCreator::StackedHistogramCreator(
			TH1D dataHistogram,
			std::list<TH1D> monteCarloHistogramList)
				: dataHistogram(dataHistogram),
				  monteCarloHistogramList(monteCarloHistogramList),
				  colourIndex(0) {

		this->histogramStack = new THStack(
				DEFAULT_STACK_LABEL.c_str(),
				DEFAULT_STACK_NAME.c_str());

		StackedHistogramCreator::normaliseHistograms(&this->monteCarloHistogramList);
	}


	void StackedHistogramCreator::drawAllHistograms() {
		this->addAllToHistogramStack(this->monteCarloHistogramList);

//		TH1D *normalisedDataHistogram = this->normaliseHistogram(this->dataHistogram);
//		normalisedDataHistogram->Draw("same");
//		this->histogramStack->Draw();
	}


	///
	void StackedHistogramCreator::normaliseHistograms(std::list<TH1D> *histograms) {
		std::list<TH1D>::iterator it = histograms->begin();

		while(it != histograms->end()) {
			TH1D *histogram = &(*it);
			StackedHistogramCreator::normaliseHistogram(histogram);
			it++;
		}
	}

	// Calls to this could probably be
	void StackedHistogramCreator::normaliseHistogram(TH1D *histogram) {
		Double_t integral = histogram->Integral();

		if(integral > 0) {
			Double_t inverseIntegral = 1 / integral;
			histogram->Scale(inverseIntegral);
		}
		else {
			// Histogram does not contain any samples - no normalisation required
		}
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
		std::cout << &(histogram) << '\n';

		Int_t colour = getNextColour();

		histogram.SetFillColor(colour);
		histogram.SetLineColor(COLOUR_BLACK);
		this->histogramStack->Add(&histogram);
	}

	void StackedHistogramCreator::addAllToHistogramStack(std::list<TH1D> &histograms) {
		std::list<TH1D>::iterator it = histograms.begin();

		while(it != histograms.end()) {
			TH1D *histogram = &(*it);

			std::cout << histogram << "|\n";

			this->addToHistogramStack(*histogram);
			it++;
		}
	}
}
