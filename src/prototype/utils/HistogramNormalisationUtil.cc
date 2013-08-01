/*
 * HistogramNormalisationUtil.cpp
 *
 *  Created on: 16 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "HistogramNormalisationUtil.h"

#include <Rtypes.h>
#include <TH1.h>
#include <cassert>
#include <limits>
#include <list>
#include <stdexcept>

#include "../models/display-data/HistogramData.h"
#include "../models/display-data/HistogramDisplayData.h"
#include "../models/display-data/WeightedHistogramData.h"

namespace prototype {
	const Double_t HistogramNormalisationUtil::UNIT_AREA = 1.0;

	void HistogramNormalisationUtil::normaliseHistogram(TH1D *histogram) {
		WeightedHistogramData *histogramScalingData = new WeightedHistogramData(
				histogram, UNIT_AREA);
		HistogramNormalisationUtil::normaliseWeightedHistogram(*histogramScalingData);
		assert(std::abs(histogram->Integral() - UNIT_AREA) < (2 * std::numeric_limits<Double_t>::epsilon()));
	}

	void HistogramNormalisationUtil::normaliseWeightedHistograms(
			std::list<WeightedHistogramData> weightedHistogramData) {
		std::list<WeightedHistogramData>::iterator it = weightedHistogramData.begin();
		Double_t weightSum = 0;

		while(it != weightedHistogramData.end()) {
			WeightedHistogramData weightedHistogramData = *it;
			// TODO: This is likely a reasonably computational expensive function call.
			//		 For efficiency gains, consider executing the below in a new thread
			//		 (if thread safe) and then waiting for all threads to complete.
			HistogramNormalisationUtil::normaliseWeightedHistogram(weightedHistogramData);
			weightSum += weightedHistogramData.getWeight();
			it++;
		}

		if(weightedHistogramData.size() != 0) {
			if(weightSum != UNIT_AREA) {
				throw std::invalid_argument(
						"The sum weight of all histograms to be normalised must equal 1");
				/// FIXME: This histograms will have been incorrectly normalised at this point!
			}
		}
	}

	void HistogramNormalisationUtil::normaliseWeightedHistogram(WeightedHistogramData weightedHistogramData) {
		TH1D *histogram = weightedHistogramData.getHistogram();
		Double_t weight = weightedHistogramData.getWeight();
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
