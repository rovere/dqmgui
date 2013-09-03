/*
 * ScaledStackedHistogramBuilder.h
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */
#ifndef SCALEDSTACKEDHISTOGRAMBUILDER_H_
#define SCALEDSTACKEDHISTOGRAMBUILDER_H_

#include "StackedHistogramBuilder.h"
#include "../../models/HistogramData.h"
#include "../../models/HistogramStackData.h"

class THStack;

namespace render {
	/// TODO: Comment
	class ScaledStackedHistogramBuilder : public StackedHistogramBuilder<HistogramData> {
		private:
			/// TODO: Comment
			HistogramStackData stackData;

		public:
			/// Default constructor.
			ScaledStackedHistogramBuilder();

			/// @see StackedHistogramBuilder::build()
//			THStack* build();

			/// @see StackedHistogramBuilder::addHistogramData(HistogramData)
//			void addHistogramData(HistogramData histogramData);
	};
}
#endif
