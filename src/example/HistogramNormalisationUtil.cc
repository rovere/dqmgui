/*
 * HistogramNormalisationUtil.cpp
 *
 *  Created on: 16 Jul 2013
 *      Author: Colin - CERN
 */

#include "HistogramNormalisationUtil.h"

#include <Rtypes.h>
#include <TH1.h>
#include <cassert>
#include <iostream>

namespace example {
	///
	///
	///
	void HistogramNormalisationUtil::normaliseHistogram(TH1D *histogram) {
		HistogramNormalisationUtil::normaliseHistogram(histogram, 1);
	}

	///
	///
	///
	void HistogramNormalisationUtil::normaliseHistograms(std::list<TH1D> *histograms) {
		Int_t totalEntries = HistogramNormalisationUtil::getEntriesInHistograms(histograms);

		std::list<TH1D>::iterator it = histograms->begin();

		while(it != histograms->end()) {
			TH1D *histogram = &(*it);
			Double_t weight = histogram->GetEntries() / totalEntries;
			HistogramNormalisationUtil::normaliseHistogram(histogram, weight);
			it++;
		}
	}

	///
	///
	///
	void HistogramNormalisationUtil::normaliseHistogram(TH1D *histogram, Double_t histogramWeight) {
		assert(histogramWeight <= 1.0);
		Double_t integral = histogram->Integral();

		if(integral > 0) {
			Double_t inverseIntegral = (1 / integral) * histogramWeight;
			histogram->Scale(inverseIntegral);
		}
		else {
			// Histogram does not contain any samples - no normalisation required
		}
	}

	///
	///
	///
	Int_t HistogramNormalisationUtil::getEntriesInHistograms(std::list<TH1D> *histograms) {
		std::list<TH1D>::iterator it = histograms->begin();
		Int_t totalEntries = 0;

		while(it != histograms->end()) {
			TH1D *histogram = &(*it);
			totalEntries += histogram->GetEntries();
			it++;
		}

		return totalEntries;
	}
}
