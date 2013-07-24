/*
 * HistogramDisplayData.cc
 *
 *  Created on: 19 Jul 2013
 *      Author: Colin - CERN
 */
#include "HistogramDisplayData.h"

#include <Rtypes.h>
#include <stdexcept>

class TH1D;

namespace prototype {
	HistogramDisplayData::HistogramDisplayData(TH1D *histogram, Double_t weight) {
		this->setHistogram(histogram);
		this->setWeight(weight);
	}

	TH1D* HistogramDisplayData::getHistogram() {
		return(this->histogram);
	}

	Double_t HistogramDisplayData::getWeight() {
		return(this->weight);
	}

	void HistogramDisplayData::setHistogram(TH1D *histogram) {
		this->histogram = histogram;
	}

	void HistogramDisplayData::setWeight(Double_t weight) {
		if(weight > 1.0) {
			throw std::invalid_argument("Histogram weighting cannot be more than 1");
		}
		else if(weight < 0.0) {
			throw std::invalid_argument("Histogram weighting cannot be less than 0");
		}

		this->weight = weight;
	}
}
