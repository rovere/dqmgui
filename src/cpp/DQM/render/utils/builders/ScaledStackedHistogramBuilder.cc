/*
 * ScaledStackedHistogramBuilder.cc
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "StackedHistogramBuilder.h"
#include "ScaledStackedHistogramBuilder.h"

namespace render { class HistogramData; }

namespace render {
	ScaledStackedHistogramBuilder::ScaledStackedHistogramBuilder(Double_t scalingFactor)
			: StackedHistogramBuilder<HistogramData>(&stackData),
			  scalingFactor(scalingFactor) {
		// Nothing to do here
	}
}
