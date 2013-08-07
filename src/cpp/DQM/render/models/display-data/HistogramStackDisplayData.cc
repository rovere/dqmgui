/*
 * HistogramDisplayDataSet.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "HistogramStackDisplayData.h"

#include <Rtypes.h>
#include <cassert>
#include <list>
#include <stdexcept>

#include "HistogramDisplayData.h"

namespace render {
	HistogramStackDisplayData::HistogramStackDisplayData() {
		;
	}

	void HistogramStackDisplayData::add(HistogramDisplayData histogramDisplayData) {
		Double_t currentTotalWeight = this->getHistogramsTotalWeight();
		Double_t postTotalWeight = currentTotalWeight + histogramDisplayData.getWeight();

		if(postTotalWeight > 1.0) {
			throw std::invalid_argument(
					"Adding this histogram exceeds the total allowed weight of all histograms (1.0)");
		}

		this->allHistogramDisplayData.push_back(histogramDisplayData);
		assert(this->getHistogramsTotalWeight() == postTotalWeight);	// XXX: Is this okey considering the machine's epsilon?
		assert(this->getHistogramsTotalWeight() <= 1.0);
		assert(this->getHistogramsTotalWeight() >= 0.0);
	}

	void HistogramStackDisplayData::add(HistogramStackDisplayData histogramStackDisplayData) {
		#ifdef DNDEBUG
		Int_t originalDataSize = this->getAllHistogramDisplayData().size();
		#endif

		std::list<HistogramDisplayData> allHistogramDisplayData = histogramStackDisplayData
				.getAllHistogramDisplayData();
		std::list<HistogramDisplayData>::iterator it = allHistogramDisplayData.begin();

		while(it != allHistogramDisplayData.end()) {
			HistogramDisplayData histogramDisplayData = (*it);
			this->add(histogramDisplayData);
			it++;
		}

		assert(this->getAllHistogramDisplayData().size() == (originalDataSize + allHistogramDisplayData.size()));
		assert(this->getHistogramsTotalWeight() <= 1.0);
		assert(this->getHistogramsTotalWeight() >= 0.0);
	}

	Double_t HistogramStackDisplayData::getHistogramsTotalWeight() {
		// XXX: It would be nice if the list could be casted to WeightedHistogramData as
		//		we're only interested in seeing the data like this
		std::list<HistogramDisplayData>::iterator it = this->allHistogramDisplayData.begin();
		Double_t totalWeight = 0;

		while(it != this->allHistogramDisplayData.end()) {
			HistogramDisplayData histogramDisplayData = (*it);
			totalWeight += histogramDisplayData.getWeight();
			it++;
		}

		assert(totalWeight <= 1.0);
		assert(totalWeight >= 0.0);
		return(totalWeight);
	}

	std::list<HistogramDisplayData> HistogramStackDisplayData::getAllHistogramDisplayData() {
		return(this->allHistogramDisplayData);
	}
}
