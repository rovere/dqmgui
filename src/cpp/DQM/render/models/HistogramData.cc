/*
 * HistogramData.cc
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin Nolan
 */
#define DNDEBUG

#include "HistogramData.h"

#include <string>

class TH1;

namespace render {
	HistogramData::HistogramData(TH1 *histogram) {
		this->setHistogram(histogram);
	}

	HistogramData::HistogramData(TH1 *histogram, std::string drawOptions) {
		this->setDrawOptions(drawOptions);
		// XXX: Should really call the other constructor here to prevent code duplication
		this->setHistogram(histogram);
	}

	TH1* HistogramData::getHistogram() {
		return(this->histogram);
	}

	std::string HistogramData::getDrawOptions() {
		return(this->drawOptions);
	}

	void HistogramData::setHistogram(TH1 *histogram) {
		this->histogram = histogram;
	}

	void HistogramData::setDrawOptions(std::string drawOptions) {
		this->drawOptions = drawOptions;
	}
}
