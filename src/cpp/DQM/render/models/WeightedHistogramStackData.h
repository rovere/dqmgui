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
	/// TODO: Document
	class WeightedHistogramStackData : public AbstractHistogramStackData<WeightedHistogramData> {
		private:
			// Making this class using composition with a HistogramStackData instance.
			HistogramStackData histogramStackData;

		public:
			WeightedHistogramStackData();

			/// @see AbstractHistogramStackData::getAllHistogramData()
			std::vector<WeightedHistogramData> getAllHistogramData();

			/// @see AbstractHistogramStackData::getAllHistograms()
			std::vector<TH1*> getAllHistograms();

			/// Gets the total weight of all the histograms in this container.
			/// @return the total weight of all histograms in this container.
			///			The weight, w, shall be in the range: 1.0 <= w <= 0.0
			Double_t getHistogramsTotalWeight();

			/// TODO: Get documentation from history
			void add(WeightedHistogramData histogramData);

			/// TODO: Get documentation from history
//			void add(WeightedHistogramStackData histogramStackData);
	}
}
#endif
