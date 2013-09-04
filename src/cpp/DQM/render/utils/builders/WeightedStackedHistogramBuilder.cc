/*
 * StackedHistogramBuilder.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
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

//	THStack* WeightedStackedHistogramBuilder::build() {
//		if(this->histogramStackData.getHistogramsTotalWeight() != 1.0) {
//			throw std::invalid_argument(
//					"The sum weight of all histograms in the stack to be built must equal 1");
//		}
//
//		// TODO: Call super
//	}

	void WeightedStackedHistogramBuilder::addHistogramData(WeightedHistogramData data) {
		if((this->stackData.getHistogramsTotalWeight() + data.getWeight()) > 1.0) {
			throw std::invalid_argument(
					"The sum weight of all histograms in the stack cannot be greater than 1");
		}

		// TODO: Consider using cloning here
		HistogramScalingUtil::scaleWeightedHistogram(data, this->getTargetHistogramArea());
		StackedHistogramBuilder<WeightedHistogramData>::addHistogramData(data);
	}

//	void WeightedStackedHistogramBuilder::addHistogramStackData(HistogramStackData data) {
//		std::vector<WeightedHistogramData> allWeightedHistogramsData = data.getAllHistogramData();
//		std::vector<WeightedHistogramData>::iterator it = allWeightedHistogramsData.begin();
//
//		while(it != allWeightedHistogramsData.end()) {
//			WeightedHistogramData weightedHistogramData = *it;
//			this->addWeightedHistogramData(weightedHistogramData);
//			it++;
//		}
//	}

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
