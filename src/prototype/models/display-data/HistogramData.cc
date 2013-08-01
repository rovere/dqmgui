/*
 * HistogramData.cc
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */

#include "HistogramData.h"

namespace prototype {
	HistogramData::HistogramData() {
		this->histogram = nullptr;
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
