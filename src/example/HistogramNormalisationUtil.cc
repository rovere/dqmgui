/*
 * HistogramNormalisationUtil.cpp
 *
 *  Created on: 16 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include <Rtypes.h>
#include <TH1.h>
#include <cassert>
#include <iostream>
#include <limits>
#include <list>
#include <new>
#include <stdexcept>

#include "HistogramNormalisationUtil.h"
#include "HistogramWeightPair.h"

namespace prototype {
	const Double_t HistogramNormalisationUtil::UNIT_AREA = 1.0;

	void HistogramNormalisationUtil::normaliseHistogram(TH1D *histogram) {
		HistogramWeightPair *histogramWeightPair = new HistogramWeightPair(
				histogram, UNIT_AREA);
		HistogramNormalisationUtil::normaliseHistogram(*histogramWeightPair);
	}

	void HistogramNormalisationUtil::normaliseHistograms(
			std::list<HistogramWeightPair> *histogramWeightPairs) {
		std::list<HistogramWeightPair>::iterator it = histogramWeightPairs->begin();
		Double_t weightSum = 0;

		while(it != histogramWeightPairs->end()) {
			HistogramWeightPair histogramWeightPair = *it;
			// TODO: This is likely a reasonably computational expensive function call.
			//		 For efficiency gains, consider executing the below in a new thread
			//		 and then waiting for all threads to complete.
			HistogramNormalisationUtil::normaliseHistogram(histogramWeightPair);
			weightSum += histogramWeightPair.getWeight();
			it++;
		}

		if(histogramWeightPairs->size() != 0) {
			if(weightSum != UNIT_AREA) {
				throw std::invalid_argument(
						"The sum weight of all histograms to be normalised must equal 1");
			}
		}
	}

	void HistogramNormalisationUtil::normaliseHistogram(HistogramWeightPair histogramWeightPair) {
		TH1D *histogram = histogramWeightPair.getHistogram();
		Double_t weight = histogramWeightPair.getWeight();
		Double_t integral = histogram->Integral();

		if(integral > 0) {
			Double_t inverseIntegral = (1 / integral) * weight;
			histogram->Scale(inverseIntegral);

			// Note: This assertion is not as simple as using the equality operator.
			//		 As double is a continuous variable and the equality operator is
			//		 not smart, the machine's epsilon must be considered (for two doubles).
			assert((histogram->Integral() - (UNIT_AREA * weight)) <= (2 * std::numeric_limits<Double_t>::epsilon()));
		}
		else {
			// Histogram does not contain any samples - no normalisation required
		}
	}
}
