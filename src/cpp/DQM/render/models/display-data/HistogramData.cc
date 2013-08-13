class TH1D;

/*
 * HistogramData.cc
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "HistogramData.h"

namespace render {
	HistogramData::HistogramData()
		/*	: histogram(nullptr)	// This is C++11 so cannot be used right now */ {
		;
	}

	HistogramData::HistogramData(TH1D *histogram) {
		this->histogram = histogram;
	}

	TH1D* HistogramData::getHistogram() {
		return(this->histogram);
	}

	void HistogramData::setHistogram(TH1D *histogram) {
		this->histogram = histogram;
	}
}
