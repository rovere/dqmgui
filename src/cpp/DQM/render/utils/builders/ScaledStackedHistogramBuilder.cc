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
			: StackedHistogramBuilder<HistogramData>(&stackData) {
		this->setScalingFactor(scalingFactor);
	}

	Double_t ScaledStackedHistogramBuilder::getScalingFactor() {
		return(this->scalingFactor);
	}

	void ScaledStackedHistogramBuilder::setScalingFactor(Double_t scalingFactor) {
		this->scalingFactor = scalingFactor;
	}

	void ScaledStackedHistogramBuilder::addHistogramData(HistogramData histogramData) {
		TH1 *histogram = histogramData.getHistogram();
		Double_t scalingFactor = this->getScalingFactor();
		HistogramScalingUtil::scaleHistogram(histogram, scalingFactor);
		return(StackedHistogramBuilder<HistogramData>::addHistogramData(histogramData));
	}
}
