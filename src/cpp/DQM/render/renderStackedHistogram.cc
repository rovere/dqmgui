/*
 * renderStackedHistogram.cc
 *
 *  Created on: 12 Aug 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include <string>
#include <vector>
#include "Rtypes.h"
#include "TH1.h"

namespace render {
	// XXX: This should be made more OO, which would also solve the excessive
	// 		parameters antipattern.
	void renderStackedHistogram(
			TObject dataHistogram,
			std::vector<TObject> monteCarloHistograms,
			std::vector<Double_t> monteCarloHistogramWeights,
			std::string drawOptions) {
		// TODO
	}
}
