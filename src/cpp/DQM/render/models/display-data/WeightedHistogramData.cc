/*
 * WeightedHistogramData.cc
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "WeightedHistogramData.h"

#include <Rtypes.h>
#include <stdexcept>

#include "HistogramData.h"

class TH1;

namespace render {
	WeightedHistogramData::WeightedHistogramData(TH1 *histogram, Double_t weight) {
		this->setHistogram(histogram);
		this->setWeight(weight);
	}

	Double_t WeightedHistogramData::getWeight() {
		return(this->weight);
	}

	void WeightedHistogramData::setWeight(Double_t weight) {
		if(weight > 1.0) {
			throw std::invalid_argument("Histogram weighting cannot be more than 1");
		}
		else if(weight < 0.0) {
			throw std::invalid_argument("Histogram weighting cannot be less than 0");
		}

		this->weight = weight;
	}
}
