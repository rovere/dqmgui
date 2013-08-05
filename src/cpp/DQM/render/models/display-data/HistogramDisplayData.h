/*
 * HistogramDisplayData.h
 *
 *  Created on: 19 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMSTACKDISPLAYDATA_H_
#define HISTOGRAMSTACKDISPLAYDATA_H_

#include <Rtypes.h>

#include "WeightedHistogramData.h"

class TH1D;

namespace prototype {
	/// A model containing the data required to display a histogram,
	/// including the histogram to be displayed and its associated weight
	/// in the context of a list of histograms that a histogram
	/// stack is to be created from.
	class HistogramDisplayData : public prototype::WeightedHistogramData  {
		public:
			/// Default constructor.
			/// @param histogram {@link WeightedHistogramData}
			/// @param weight {@link WeightedHistogramData}
			HistogramDisplayData(TH1D *histogram, Double_t weight) : WeightedHistogramData(histogram, weight) {};
	};
}
#endif
