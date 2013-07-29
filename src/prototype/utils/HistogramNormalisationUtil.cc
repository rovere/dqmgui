/*
 * HistogramNormalisationUtil.cpp
 *
 *  Created on: 16 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include <cassert>
#include <new>

#include "HistogramNormalisationUtil.h"

#include <Rtypes.h>
#include <TH1.h>
#include <limits>
#include <list>
#include <stdexcept>
#include <iostream>

#include "../models/HistogramDisplayData.h"

namespace prototype {
	const Double_t HistogramNormalisationUtil::UNIT_AREA = 1.0;

	void HistogramNormalisationUtil::normaliseHistogram(TH1D *histogram) {
		HistogramDisplayData *histogramDisplayData = new HistogramDisplayData(
				histogram, UNIT_AREA);
		HistogramNormalisationUtil::normaliseWeightedHistogram(*histogramDisplayData);
		assert(std::abs(histogram->Integral() - UNIT_AREA) < (2 * std::numeric_limits<Double_t>::epsilon()));
	}

	void HistogramNormalisationUtil::normaliseWeightedHistograms(
			std::list<HistogramDisplayData> histogramDisplayData) {
		std::list<HistogramDisplayData>::iterator it = histogramDisplayData.begin();
		Double_t weightSum = 0;

		while(it != histogramDisplayData.end()) {
			HistogramDisplayData histogramDisplayData = *it;
			// TODO: This is likely a reasonably computational expensive function call.
			//		 For efficiency gains, consider executing the below in a new thread
			//		 (if thread safe) and then waiting for all threads to complete.
			HistogramNormalisationUtil::normaliseWeightedHistogram(histogramDisplayData);
			weightSum += histogramDisplayData.getWeight();
			it++;
		}

		if(histogramDisplayData.size() != 0) {
			if(weightSum != UNIT_AREA) {
				throw std::invalid_argument(
						"The sum weight of all histograms to be normalised must equal 1");
				/// FIXME: This histograms will have been incorrectly normalised at this point!
			}
		}
	}

	void HistogramNormalisationUtil::normaliseWeightedHistogram(HistogramDisplayData histogramDisplayData) {
		TH1D *histogram = histogramDisplayData.getHistogram();
		Double_t weight = histogramDisplayData.getWeight();
		Double_t integral = histogram->Integral();

		if(integral > 0) {
			Double_t inverseIntegral = (1 / integral) * weight;
			histogram->Scale(inverseIntegral);

			// Note: This assertion is not as simple as using the equality operator.
			//		 As double is a continuous variable and the equality operator is
			//		 not smart, the machine's epsilon must be considered (for two doubles).
			assert(std::abs(histogram->Integral() - (UNIT_AREA * weight)) < (2 * std::numeric_limits<Double_t>::epsilon()));
		}
		else {
			// Histogram does not contain any samples - no normalisation required
		}
	}
}
