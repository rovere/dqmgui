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
#include "AbstractHistogramStackData.h"

class TH1;

namespace render {
	class HistogramStackData : public AbstractHistogramStackData<HistogramData> {
		public:
			/// Default constructor.
			HistogramStackData();
	};
}

#endif
