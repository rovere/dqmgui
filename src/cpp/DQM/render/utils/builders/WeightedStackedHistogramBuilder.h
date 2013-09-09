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
	/// Builder for ROOT <code>THStack</code> histograms using <code>WeightedHistogramData</code>.
	/// The builder is designed to scale the histograms by a weight before they are added to the
	/// stack.
	class WeightedStackedHistogramBuilder : public StackedHistogramBuilder<WeightedHistogramData> {
		private:
			/// The data required to build the histogram stack.
			WeightedHistogramStackData stackData;

			/// The area that the built histogram stack should have.
			Double_t targetHistogramArea;

		public:
			/// Default constructor.
			/// @param targetHistogramArea the area that the built histogram stack should have
			WeightedStackedHistogramBuilder(Double_t targetHistogramArea);

			/// @see StackedHistogramBuilder::addHistogramData(HistogramData)
			void addHistogramData(WeightedHistogramData histogramData);

		private:
			/// Gets the area that the built histogram stack should have.
			/// @return the area that the built histogram stack should have
			Double_t getTargetHistogramArea();

			/// Sets the area that the built stacked histogram should have.
			/// @param targetHistogramArea the area that the stacked histogram should have
			void setTargetHistogramArea(Double_t targetHistogramArea);
	};
}
#endif
