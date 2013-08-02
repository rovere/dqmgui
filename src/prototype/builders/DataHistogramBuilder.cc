/*
 * HistogramBuilder.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "DataHistogramBuilder.h"

#include <TAttLine.h>
#include <TH1.h>

#include "../controllers/ColourController.h"
#include "../utils/HistogramNormalisationUtil.h"

namespace prototype {
	DataHistogramBuilder::DataHistogramBuilder(TH1D *histogram)
			: histogram(histogram) {
		// TODO: Consider cloning here!
	}

	TH1D DataHistogramBuilder::build() {
		this->histogram->SetLineColor(ColourController::COLOUR_BLACK);
		this->histogram->SetLineStyle(2);
		this->histogram->SetLineWidth(5);

		return(*this->histogram);
	}
}
