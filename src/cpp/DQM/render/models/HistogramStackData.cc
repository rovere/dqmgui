/*
 * HistogramStackData.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "HistogramStackData.h"

#include <vector>
#include <cassert>

#include "HistogramData.h"

class TH1;

namespace render {
	HistogramStackData::HistogramStackData() {
		// Nothing to do here
	}

	std::vector<HistogramData> HistogramStackData::getAllHistogramData() {
		return(this->allHistogramData);
	}

	std::vector<TH1*> HistogramStackData::getAllHistograms() {
		std::vector<HistogramData> allHistogramsData = this->getAllHistogramData();
		std::vector<HistogramData>::iterator it = allHistogramsData.begin();
		std::vector<TH1*> histograms;

		while(it != allHistogramsData.end()) {
			HistogramData *histogramData = &(*it);
			TH1 *histogram = histogramData->getHistogram();
			histograms.push_back(histogram);
			it++;
		}

		assert(histograms.size() == allHistogramsData.size());
		return(histograms);
	}

	void HistogramStackData::add(HistogramData histogramData) {
		this->allHistogramData.push_back(histogramData);
	}
}
