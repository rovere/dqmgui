/*
 * StackedHistogramBuilder.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin Nolan
 */
#define DNDEBUG

#include "WeightedStackedHistogramBuilder.h"

#include <Rtypes.h>
#include <cassert>
#include <sstream>
#include <stdexcept>

#include "../../models/WeightedHistogramData.h"
#include "../HistogramScalingUtil.h"
#include "StackedHistogramBuilder.h"

namespace render {
	WeightedStackedHistogramBuilder::WeightedStackedHistogramBuilder(
			Double_t targetHistogramArea)
			: StackedHistogramBuilder<WeightedHistogramData>(&stackData) {
		this->setTargetHistogramArea(targetHistogramArea);
	}

	void WeightedStackedHistogramBuilder::addHistogramData(WeightedHistogramData data) {
		if((this->stackData.getHistogramsTotalWeight() + data.getWeight()) > 1.0) {
			throw std::invalid_argument(
					"The sum weight of all histograms in the stack cannot be greater than 1");
		}

		HistogramScalingUtil::scaleWeightedHistogram(data, this->getTargetHistogramArea());
		StackedHistogramBuilder<WeightedHistogramData>::addHistogramData(data);
	}

	Double_t WeightedStackedHistogramBuilder::getTargetHistogramArea() {
		assert(this->targetHistogramArea >= 0);
		return(this->targetHistogramArea);
	}

	void WeightedStackedHistogramBuilder::setTargetHistogramArea(Double_t targetHistogramArea) {
		if(targetHistogramArea < 0) {
			std::ostringstream message;
			message << "Target histogram area (" << targetHistogramArea << ") cannot be less than 0";
			throw std::invalid_argument(message.str());
		}
		this->targetHistogramArea = targetHistogramArea;
	}
}
