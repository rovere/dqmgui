/*
 * HistogramNormalisationUtil.cpp
 *
 *  Created on: 16 Jul 2013
 *      Author: Colin - CERN
 */

#include "HistogramNormalisationUtil.h"

#include <Rtypes.h>
#include <TH1.h>
#include <list>
#include <new>

#include "HistogramWeightPair.h"

namespace example {
	///
	///
	///
	void HistogramNormalisationUtil::normaliseHistogram(TH1D *histogram) {
		HistogramWeightPair *histogramWeightPair = new HistogramWeightPair(histogram, 1);
		HistogramNormalisationUtil::normaliseHistogram(*histogramWeightPair);
	}

	///
	///
	///
	void HistogramNormalisationUtil::normaliseHistograms(
			std::list<HistogramWeightPair> *histogramWeightPairs) {
		std::list<HistogramWeightPair>::iterator it = histogramWeightPairs->begin();

		while(it != histogramWeightPairs->end()) {
			HistogramWeightPair histogramWeightPair = *it;
			HistogramNormalisationUtil::normaliseHistogram(histogramWeightPair);
			it++;
		}
	}

	///
	///
	///
	void HistogramNormalisationUtil::normaliseHistogram(HistogramWeightPair histogramWeightPair) {
		TH1D *histogram = histogramWeightPair.getHistogram();
		Double_t weight = histogramWeightPair.getWeight();

		Double_t integral = histogram->Integral();

		if(integral > 0) {
			Double_t inverseIntegral = (1 / integral) * weight;
			histogram->Scale(inverseIntegral);
		}
		else {
			// Histogram does not contain any samples - no normalisation required
		}
	}
}
