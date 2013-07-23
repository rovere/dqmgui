#include <limits>
#include <list>

/*
 * HistogramDisplayDataSet.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include <Rtypes.h>
#include <cassert>
#include <list>
#include <limits>

#include "HistogramDisplayData.h"
#include "HistogramStackDisplayData.h"

namespace prototype {
	void HistogramStackDisplayData::add(HistogramDisplayData displayData) {
		Double_t currentTotalWeight = this->getHistogramsTotalWeight();
		Double_t postTotalWeight = currentTotalWeight + displayData.getWeight();

		if(postTotalWeight > 1.0) {
			std::invalid_argument("Adding this histogram exceeds the total allowed weight of all histograms (1.0)");
		}

		this->histogramDisplayData.push_back(displayData);
		assert(this->getHistogramsTotalWeight() == postTotalWeight);	// XXX: Is this okey considering the machine's epsilon?
	}

	Double_t HistogramStackDisplayData::getHistogramsTotalWeight() {
		std::list<HistogramDisplayData>::iterator it = this->histogramDisplayData.begin();
		Double_t totalWeight = 0;

		while(it != this->histogramDisplayData.end()) {
			HistogramDisplayData histogramDisplayData = (*it);
			totalWeight += histogramDisplayData.getWeight();
			it++;
		}

		return(totalWeight);
	}

	std::list<HistogramDisplayData> HistogramStackDisplayData::getAllHistogramDisplayData() {
		return(this->histogramDisplayData);
	}
}
