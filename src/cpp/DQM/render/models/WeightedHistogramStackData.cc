/*
 * WeightedHistogramStackData.cc
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "WeightedHistogramStackData.h"

#include <Rtypes.h>
#include <cassert>
#include <stdexcept>
#include <vector>

#include "AbstractHistogramStackData.h"
#include "HistogramData.h"
#include "HistogramStackData.h"
#include "WeightedHistogramData.h"

class TH1;

namespace render {
	WeightedHistogramStackData::WeightedHistogramStackData() {
		// Nothing to do here
	}

	Double_t WeightedHistogramStackData::getHistogramsTotalWeight() {
		std::vector<WeightedHistogramData>::iterator it = this->getAllHistogramData().begin();
		Double_t totalWeight = 0;

		while(it != this->getAllHistogramData().end()) {
			WeightedHistogramData weightedHistogramData = *it;
			totalWeight += weightedHistogramData.getWeight();
			it++;
		}

//		assert(totalWeight <= 1.0);
		assert(totalWeight >= 0.0);
		return(totalWeight);
	}

	/// XXX: Should we enforce that the total weight of all constituent histograms should not >1.0?
	void WeightedHistogramStackData::add(WeightedHistogramData histogramData) {
		Double_t currentTotalWeight = this->getHistogramsTotalWeight();
		Double_t postTotalWeight = currentTotalWeight + histogramData.getWeight();

//		if(postTotalWeight > 1.0) {
//			throw std::invalid_argument(
//					"Adding this histogram exceeds the total allowed weight of all histograms (1.0)");
//		}

		AbstractHistogramStackData<WeightedHistogramData>::add(histogramData);
		assert(this->getHistogramsTotalWeight() == postTotalWeight);	// XXX: Is this okey considering the machine's epsilon?
//		assert(this->getHistogramsTotalWeight() <= 1.0);
		assert(this->getHistogramsTotalWeight() >= 0.0);
	}
}
