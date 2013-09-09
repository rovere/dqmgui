/*
 * WeightedHistogramStackData.h
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */

#ifndef WEIGHTEDHISTOGRAMSTACKDATA_H_
#define WEIGHTEDHISTOGRAMSTACKDATA_H_

#include <Rtypes.h>

#include "AbstractHistogramStackData.h"
#include "HistogramStackData.h"
#include "WeightedHistogramData.h"

namespace render {
	/// Model of data required to draw an histogram stack using <code>WeightedHistogramData</code>.
	class WeightedHistogramStackData : public AbstractHistogramStackData<WeightedHistogramData> {
		public:
			/// Default constructor.
			WeightedHistogramStackData();

			/// Gets the total weight of all the histograms in this container.
			/// @return the total weight of all histograms in this container.
			///			The weight, w, shall be in the range: 1.0 <= w <= 0.0
			Double_t getHistogramsTotalWeight();

			/// @see AbstractHistogramStackData<WeightedHistogramData>::add(WeightedHistogramData)
			void add(WeightedHistogramData histogramData);
	};
}
#endif
