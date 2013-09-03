/*
 * WeightedStackedHistogramBuilder.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef WEIGHTEDSTACKEDHISTOGRAMBUILDER_H_
#define WEIGHTEDSTACKEDHISTOGRAMBUILDER_H_

#include <Rtypes.h>

#include "../../models/WeightedHistogramData.h"
#include "../../models/WeightedHistogramStackData.h"
#include "StackedHistogramBuilder.h"

namespace render {
	/// Builder for stacked histograms.
	class WeightedStackedHistogramBuilder : public StackedHistogramBuilder<WeightedHistogramData> {
		private:
			WeightedHistogramStackData stackData;

			/// The area that the built histogram should have.
			Double_t targetHistogramArea;

		public:
			/// Default constructor.
			/// @param targetHistogramArea the area that the built histogram should have
			WeightedStackedHistogramBuilder(Double_t targetHistogramArea);

		private:
			/// Gets the area that the built histogram should have.
			/// @return the area that the built histogram should have
			Double_t getTargetHistogramArea();

			/// Sets the area that the stacked histogram should have.
			/// @param targetHistogramArea the area that the stacked histogram should have
			void setTargetHistogramArea(Double_t targetHistogramArea);
	};
}
#endif
