/*
 * StackedHistogramBuilder.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "WeightedStackedHistogramBuilder.h"

#include <Rtypes.h>
#include <sstream>
#include <stdexcept>

#include <sstream>

namespace render { class WeightedHistogramData; }

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

//	void WeightedStackedHistogramBuilder::addHistogramData(WeightedHistogramData data) {
//		// TODO: Consider using cloning here
//		HistogramScalingUtil::scaleWeightedHistogram(data, this->getTargetHistogramArea());
//		this->histogramStackData.add(data);
//	}
//
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

	void WeightedStackedHistogramBuilder::setTargetHistogramArea(Double_t targetHistogramArea) {
		if(targetHistogramArea < 0) {
			std::ostringstream message;
			message << "Target histogram area (" << targetHistogramArea << ") cannot be less than 0";
			throw std::invalid_argument(message.str());
		}
		this->targetHistogramArea = targetHistogramArea;
	}

	Double_t WeightedStackedHistogramBuilder::getTargetHistogramArea() {
		return(this->targetHistogramArea);
	}
}
