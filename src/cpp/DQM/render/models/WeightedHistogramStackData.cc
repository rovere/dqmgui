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

	std::vector<WeightedHistogramData> WeightedHistogramStackData::getAllHistogramData() {
		std::vector<HistogramData> histogramData = this->histogramStackData.getAllHistogramData();
		std::vector<WeightedHistogramData> weightedHistogramData;

		// This is the cost of making this class using composition with HistogramData
		std::vector<WeightedHistogramData>::iterator it = histogramData.begin();
		while(it != histogramData.end()) {
			HistogramData histogramData = *it;
			weightedHistogramData.push_back((WeightedHistogramData) histogramData);
			it++;
		}

		return(weightedHistogramData);
	}

	std::vector<TH1*> WeightedHistogramStackData::getAllHistograms() {
		return(this->histogramStackData.getAllHistograms());
	}

	Double_t WeightedHistogramStackData::getHistogramsTotalWeight() {
		// XXX: It would be nice if the list could be casted to WeightedHistogramData as
		//		we're only interested in seeing the data like this
		std::vector<WeightedHistogramData>::iterator it = this->getAllHistogramData().begin();
		Double_t totalWeight = 0;

		while(it != this->getAllHistogramData().end()) {
			WeightedHistogramData weightedHistogramData = *it;
			totalWeight += weightedHistogramData.getWeight();
			it++;
		}

		assert(totalWeight <= 1.0);
		assert(totalWeight >= 0.0);
		return(totalWeight);
	}

	void WeightedHistogramStackData::add(WeightedHistogramData histogramData) {
		Double_t currentTotalWeight = this->getHistogramsTotalWeight();
		Double_t postTotalWeight = currentTotalWeight + histogramData.getWeight();

		if(postTotalWeight > 1.0) {
			throw std::invalid_argument(
					"Adding this histogram exceeds the total allowed weight of all histograms (1.0)");
		}

		this->histogramStackData.add(histogramData);
		assert(this->getHistogramsTotalWeight() == postTotalWeight);	// XXX: Is this okey considering the machine's epsilon?
		assert(this->getHistogramsTotalWeight() <= 1.0);
		assert(this->getHistogramsTotalWeight() >= 0.0);
	}

//	void WeightedHistogramStackData::add(WeightedHistogramStackData histogramStackData) {
//		#ifdef DNDEBUG
//		Int_t originalDataSize = this->getAllHistogramData().size();
//		#endif
//
//		std::vector<WeightedHistogramData> allHistogramData = histogramStackData.getAllHistogramData();
//		std::vector<WeightedHistogramData>::iterator it = allHistogramData.begin();
//
//		while(it != allHistogramData.end()) {
//			WeightedHistogramData weightedHistogramData = *it;
//			this->add(weightedHistogramData);
//			it++;
//		}
//
//		#ifdef DNDEBUG
//			assert(this->getAllHistogramData().size() == (originalDataSize + allHistogramData.size()));
//			assert(this->getHistogramsTotalWeight() <= 1.0);
//			assert(this->getHistogramsTotalWeight() >= 0.0);
//		#endif
//	}
}
