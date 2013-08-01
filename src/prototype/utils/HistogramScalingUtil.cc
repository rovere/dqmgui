/*
 * HistogramScalingUtil.cpp
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "HistogramScalingUtil.h"

#include <math.h>
#include <Rtypes.h>
#include <TH1.h>
#include <cassert>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <list>

#include "../models/display-data/HistogramData.h"
#include "../models/display-data/WeightedHistogramData.h"

namespace prototype {
	void HistogramScalingUtil::scaleHistogram(TH1D *histogram, Double_t targetArea) {
		Double_t integral = histogram->Integral();

		if(integral > 0) {
			Double_t inverseIntegral = (targetArea / integral);
			histogram->Scale(inverseIntegral);

			// Note: This assertion is not as simple as using the equality operator.
			//		 As double is a continuous variable and the equality operator is
			//		 not smart, the machine's epsilon must be considered (for two doubles).
			assert(std::abs(histogram->Integral() - targetArea) < (2 * std::numeric_limits<Double_t>::epsilon()));
		}
		else {
			// Histogram does not contain any samples - no scaling required
		}
	}

	void HistogramScalingUtil::scaleWeightedHistograms(
			std::list<WeightedHistogramData> weightedHistogramData,
			Double_t targetCombinedArea) {
		#ifdef DNDEBUG
		Double_t combinedArea = 0;
		#endif

		std::list<WeightedHistogramData>::iterator it = weightedHistogramData.begin();

		while(it != weightedHistogramData.end()) {
			WeightedHistogramData weightedHistogramData = *it;

			// TODO: This is likely a reasonably computational expensive function call.
			//		 For efficiency gains, consider executing the below/loop in a new thread
			//		 (if thread safe) and then waiting for all threads to complete.
			HistogramScalingUtil::scaleWeightedHistogram(weightedHistogramData, targetCombinedArea);

			#ifdef DNDEBUG
			combinedArea += *weightedHistogramData.getHistogram()->GetIntegral();
			#endif
			it++;
		}

		assert(std::abs(combinedArea - targetCombinedArea) < (2 * std::numeric_limits<Double_t>::epsilon()));
	}

	void HistogramScalingUtil::scaleWeightedHistogram(
			WeightedHistogramData weightedHistogramData,
			Double_t combinedTargetArea) {
		TH1D *histogram = weightedHistogramData.getHistogram();		// TODO: Change to reference
		Double_t weight = weightedHistogramData.getWeight();

		Double_t targetHistogramArea = weight * combinedTargetArea;

		HistogramScalingUtil::scaleHistogram(histogram, targetHistogramArea);
	}
}
