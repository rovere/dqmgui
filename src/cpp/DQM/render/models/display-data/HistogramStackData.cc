/*
 * HistogramStackData.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "HistogramStackData.h"

#include <Rtypes.h>
#include <cassert>
#include <stdexcept>
#include <vector>

#include "WeightedHistogramData.h"

namespace render {
	HistogramStackData::HistogramStackData() {
		// Nothing to do here
	}

	HistogramStackData::~HistogramStackData() {
		// TODO: Check that because allWeightedHistogramsData is kept on the stack,
		//		 when this class' destructor is called, all of the WeightedHistogramData
		// 		 instances (stored through a reference) are deleted and memory is not
		//		 leaked.
	}

	Double_t HistogramStackData::getHistogramsTotalWeight() {
		// XXX: It would be nice if the list could be casted to WeightedHistogramData as
		//		we're only interested in seeing the data like this
		std::vector<WeightedHistogramData>::iterator it = this->allWeightedHistogramsData.begin();
		Double_t totalWeight = 0;

		while(it != this->allWeightedHistogramsData.end()) {
			WeightedHistogramData weightedHistogramData = *it;
			totalWeight += weightedHistogramData.getWeight();
			it++;
		}

		assert(totalWeight <= 1.0);
		assert(totalWeight >= 0.0);
		return(totalWeight);
	}

	std::vector<WeightedHistogramData> HistogramStackData::getAllHistogramsData() {
		return(this->allWeightedHistogramsData);
	}

	void HistogramStackData::add(WeightedHistogramData weightedHistogramData) {
		Double_t currentTotalWeight = this->getHistogramsTotalWeight();
		Double_t postTotalWeight = currentTotalWeight + weightedHistogramData.getWeight();

		if(postTotalWeight > 1.0) {
			throw std::invalid_argument(
					"Adding this histogram exceeds the total allowed weight of all histograms (1.0)");
		}

		this->allWeightedHistogramsData.push_back(weightedHistogramData);
		assert(this->getHistogramsTotalWeight() == postTotalWeight);	// XXX: Is this okey considering the machine's epsilon?
		assert(this->getHistogramsTotalWeight() <= 1.0);
		assert(this->getHistogramsTotalWeight() >= 0.0);
	}

	void HistogramStackData::add(HistogramStackData histogramStackData) {
		#ifdef DNDEBUG
		Int_t originalDataSize = this->getAllHistogramsData().size();
		#endif

		std::vector<WeightedHistogramData> allHistogramData = histogramStackData.getAllHistogramsData();
		std::vector<WeightedHistogramData>::iterator it = allHistogramData.begin();

		while(it != allHistogramData.end()) {
			WeightedHistogramData weightedHistogramData = *it;
			this->add(weightedHistogramData);
			it++;
		}

		assert(this->getAllHistogramsData().size() == (originalDataSize + allHistogramData.size()));
		assert(this->getHistogramsTotalWeight() <= 1.0);
		assert(this->getHistogramsTotalWeight() >= 0.0);
	}
}
