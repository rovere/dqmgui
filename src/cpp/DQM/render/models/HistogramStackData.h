/*
 * HistogramStackData.h
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMSTACKDATA_H_
#define HISTOGRAMSTACKDATA_H_

#include "AbstractHistogramStackData.h"

namespace render {
	/// Model of data required to draw an histogram stack using <code>HistogramData</code>.
	class HistogramStackData : public AbstractHistogramStackData<HistogramData> {
		public:
			/// Default constructor.
			HistogramStackData();
	};
}
#endif
