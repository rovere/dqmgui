/*
 * HistogramData.cc
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "HistogramData.h"

class TH1;

namespace render {
	HistogramData::HistogramData()
			: histogram(nullptr) {
		;
	}

	HistogramData::HistogramData(TH1 *histogram) {
		this->histogram = histogram;
	}

	TH1* HistogramData::getHistogram() {
		return(this->histogram);
	}

	void HistogramData::setHistogram(TH1 *histogram) {
		this->histogram = histogram;
	}
}
