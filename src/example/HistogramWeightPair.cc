/*
 * HistogramWeightPair.cc
 *
 *  Created on: 19 Jul 2013
 *      Author: Colin - CERN
 */
#include <Rtypes.h>
#include <TH1.h>

#include "HistogramWeightPair.h"

namespace prototype {
	HistogramWeightPair::HistogramWeightPair(TH1D *histogram, Double_t weight) {
		this->setHistogram(histogram);
		this->setWeight(weight);
	}

	TH1D* HistogramWeightPair::getHistogram() {
		return(this->histogram);
	}

	Double_t HistogramWeightPair::getWeight() {
		return(this->weight);
	}

	void HistogramWeightPair::setHistogram(TH1D* histogram) {
		this->histogram = histogram;
	}

	void HistogramWeightPair::setWeight(Double_t weight) {
		if(weight > 1.0) {
			std::invalid_argument("Histogram weight cannot be more than 1");
		}
		else if(weight < 0.0) {
			std::invalid_argument("Histogram weight cannot be less than 0");
		}

		this->weight = weight;
	}
}
