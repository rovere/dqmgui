/*
 * StackedHistogramSettings.cpp
 *
 *  Created on: 6 Aug 2013
 *      Author: Colin - CERN
 */
#include "StackedHistogramSettings.h"

namespace prototype {
	StackedHistogramSettings::StackedHistogramSettings() {
		this->setDrawStackedHistogram(false);
	}

	Bool_t StackedHistogramSettings::shouldDrawStackedHistogram() {
		return(this->drawStackedHistogram);
	}

	void StackedHistogramSettings::setDrawStackedHistogram(Bool_t shouldDraw) {
		this->drawStackedHistogram = shouldDraw;
	}
}
