/*
 * HistogramNormalisationUtil.cc
 *
 *  Created on: 16 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "HistogramNormalisationUtil.h"

#include <Rtypes.h>
#include <TH1.h>
#include <list>

#include "../models/display-data/WeightedHistogramData.h"
#include "HistogramScalingUtil.h"

namespace render {
	const Double_t HistogramNormalisationUtil::UNIT_AREA = 1.0;

	void HistogramNormalisationUtil::normaliseHistogram(TH1D *histogram) {
		HistogramScalingUtil::scaleHistogram(histogram, UNIT_AREA);
	}

	void HistogramNormalisationUtil::normaliseWeightedHistograms(
			std::list<WeightedHistogramData> weightedHistogramData) {
		HistogramScalingUtil::scaleWeightedHistograms(weightedHistogramData, UNIT_AREA);
	}

	void HistogramNormalisationUtil::normaliseWeightedHistogram(WeightedHistogramData weightedHistogramData) {
		HistogramScalingUtil::scaleWeightedHistogram(weightedHistogramData, UNIT_AREA);
	}
}
