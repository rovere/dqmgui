/*
 * ScaledStackedHistogramBuilder.h
 *
 *  Created on: 2 Sep 2013
 *      Author: Colin - CERN
 */
#ifndef SCALEDSTACKEDHISTOGRAMBUILDER_H_
#define SCALEDSTACKEDHISTOGRAMBUILDER_H_

#include <vector>

#include "AbstractStackedHistogramBuilder.h"

class TH1;
class THStack;
namespace render { class HistogramData; }

namespace render {
	class ScaledStackedHistogramBuilder : StackedHistogramBuilder {
		public:
			/// @see AbstractStackedHistogramBuilder::build()
			THStack* build();

			/// @see AbstractStackedHistogramBuilder::
			virtual HistogramData addHistogramData(HistogramData histogramData) = 0;

		private:
			/// Adds a given histogram to the given histograms stack.
			/// @param histogram pointer to the histogram to put on the histogram stack
			/// @param histogramStack the histogram stack that the histogram is to be added to
			static void addToTHStack(TH1 *histogram, THStack *histogramStack);

			/// Adds all of the histograms given to the given histogram stack.
			/// @param histograms the list of histograms to add to the stack
			/// @param histogramStack the histogram stack that the histogram is to be added to
			static void addAllToTHStack(std::vector<TH1*> histograms, THStack *histogramStack);
	};
}
#endif
