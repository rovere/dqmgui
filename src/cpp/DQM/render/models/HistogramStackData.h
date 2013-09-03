/*
 * HistogramStackData.h
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMSTACKDATA_H_
#define HISTOGRAMSTACKDATA_H_

#include <vector>

#include "HistogramData.h"
#include "IHistogramStackData.h"

class TH1;

namespace render {
	class HistogramStackData : public IHistogramStackData<HistogramData> {
		private:
			/// List of histogram data.
			std::vector<HistogramData> allHistogramData;

		public:
			/// Default constructor.
			HistogramStackData();

			/// @see AbstractHistogramStackData::getAllHistogramData()
			std::vector<HistogramData> getAllHistogramData();

			/// @see AbstractHistogramStackData::getAllHistograms()
			std::vector<TH1*> getAllHistograms();

			/// @see AbstractHistogramStackData::add(HistogramData)
			void add(HistogramData histogramData);
	};
}

#endif
