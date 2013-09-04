/*
 * ScaledStackedHistogramBuilder.cc
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "ScaledStackedHistogramBuilder.h"

#include <Rtypes.h>

#include "../../models/HistogramData.h"
#include "../HistogramScalingUtil.h"
#include "StackedHistogramBuilder.h"

class TH1;

namespace render {
	ScaledStackedHistogramBuilder::ScaledStackedHistogramBuilder(Double_t scalingFactor)
			: StackedHistogramBuilder<HistogramData>(&stackData),
			  scalingFactor(scalingFactor) {
		// Nothing to do here
	}

	// TODO: When adding a histogram, it needs to be scaled by the scaling factor!
	void ScaledStackedHistogramBuilder::addHistogramData(HistogramData histogramData) {
		HistogramScalingUtil::scaleHistogram(histogramData.getHistogram(), this->scalingFactor);
		return(StackedHistogramBuilder<HistogramData>::addHistogramData(histogramData));
	}
}
