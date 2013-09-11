/*
 * HistogramScalingUtil.cc
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "HistogramScalingUtil.h"

#include <Rtypes.h>
#include <TH1.h>
#include <cassert>
#include <list>
#include <stdlib.h>
#include <cmath>

#include "../models/HistogramData.h"
#include "../models/WeightedHistogramData.h"

namespace render {
	// TODO: Using TH1::Integral() does not give the mathematical integral of the histogram as it
	//		 does not consider the bin width ({@link http://root.cern.ch/root/html/TH1.html#TH1:Integral}.
	//		 This means that the scaling code below only works if all histograms to be scaled have
	//		 exactly the same bin widths. Consider modifying (perhaps using TH1::Integral("width")
	//		 to allow this to work regardless of the bin widths
	const Double_t HistogramScalingUtil::REQUIRED_ACCURACY = 0.0001;

	void HistogramScalingUtil::scaleHistogramToArea(TH1 *histogram, Double_t targetArea) {
		Double_t integral = histogram->Integral();

		if(integral > 0) {
			Double_t inverseIntegral = targetArea / integral;
			HistogramScalingUtil::scaleHistogram(histogram, inverseIntegral);

			// Note: We cannot assume that integrating the histogram will give exactly the
			//		 <code>targetArea</code> as we don't know the internals of <code>
			//		 TH1::Scale(Double_t)</code> and <code>TH1::Integral()</code>.
			//		 Therefore we'll just assume a tolerance of <code>REQUIRED_ACCURACY</code> is
			//		 acceptable.
			assert(std::abs(histogram->Integral() - targetArea) <= (targetArea * REQUIRED_ACCURACY));
		}
		else {
			// Histogram does not contain any samples - no scaling required
		}
	}

	void HistogramScalingUtil::scaleHistogram(TH1 *histogram, Double_t scalingFactor) {
		#ifdef DNDEBUG
		Double_t originalArea = histogram->Integral();
		#endif

		histogram->Scale(scalingFactor);

		#ifdef DNDEBUG
		Double_t postOperationArea = histogram->Integral();
		Double_t targetArea = originalArea * scalingFactor;
		assert(std::abs(postOperationArea - targetArea) <= (targetArea * REQUIRED_ACCURACY));
		#endif
	}

	void HistogramScalingUtil::scaleWeightedHistogram(
			WeightedHistogramData weightedHistogramData,
			Double_t combinedTargetArea) {
		TH1 *histogram = weightedHistogramData.getHistogram();
		Double_t weight = weightedHistogramData.getWeight();

		Double_t targetHistogramArea = weight * combinedTargetArea;

		HistogramScalingUtil::scaleHistogramToArea(histogram, targetHistogramArea);
	}
}
