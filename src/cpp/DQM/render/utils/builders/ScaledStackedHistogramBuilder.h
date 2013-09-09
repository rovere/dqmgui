/*
 * ScaledStackedHistogramBuilder.h
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */
#ifndef SCALEDSTACKEDHISTOGRAMBUILDER_H_
#define SCALEDSTACKEDHISTOGRAMBUILDER_H_

#include <Rtypes.h>

#include "../../models/HistogramData.h"
#include "../../models/HistogramStackData.h"
#include "StackedHistogramBuilder.h"

class THStack;

namespace render {
	/// Builder for ROOT <code>THStack</code> histograms using <code>HistogramData</code>.
	/// The builder is designed to scale the histograms by a constact factor before they are added
	/// to the stack.
	class ScaledStackedHistogramBuilder : public StackedHistogramBuilder<HistogramData> {
		private:
			/// The data required to build the histogram stack.
			HistogramStackData stackData;

			/// The factor by which all histograms contained in the histogram stack data should be
			/// scaled by before being added to the histogram stack.
			Double_t scalingFactor;

		public:
			/// Default constructor.
			/// @param scalingFactor see {@link ScaledStackedHistogramBuilder::setScalingFactor(Double_t)}
			ScaledStackedHistogramBuilder(Double_t scalingFactor);

			/// Gets the histogram scaling factor.
			/// @return the factor by which histograms should be scaled by before been added to the
			///			histogram stack
			Double_t getScalingFactor();

			/// Sets the histogram scaling factor.
			/// @return scalingFactor the factor by which histograms should be scaled by before
			///			been added to the histogram stack
			void setScalingFactor(Double_t scalingFactor);

			/// @see StackedHistogramBuilder::addHistogramData(HistogramData).
			void addHistogramData(HistogramData histogramData);
	};
}
#endif
