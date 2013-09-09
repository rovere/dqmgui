/*
 * WeightedHistogramData.h
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef WEIGHTEDHISTOGRAMDATA_H_
#define WEIGHTEDHISTOGRAMDATA_H_

#include <Rtypes.h>

#include "HistogramData.h"

class TH1;

namespace render {
	/// Model of data about a histogram that is to be scaled according to a given weighted before
	/// being used to construct a histogram stack.
	class WeightedHistogramData : public HistogramData {
		private:
			/// The weighting of the histogram in a histogram stack.
			Double_t weight;

		public:
			/// Default constructor.
			/// @param histogram a pointer to this data's histogram
			/// @param weight see {@link WeightedHistogramData::setWeight(Double_t)}
			WeightedHistogramData(TH1 *histogram, Double_t weight);

			/// Destructor.
			~WeightedHistogramData();

			/// Gets the histogram's weighting in the stack of histograms that is to be built.
			/// @return the histogram's weighting in the histogram stack
			Double_t getWeight();

			/// Sets the histogram's weight, considering the entire histogram stack.
			/// @param weight the histogram's weighting in the histogram stack.
			///				  The weighting, w, must be such that 0.0 <= w <= 1.0, else a
			///				  std::exception will be thrown
			void setWeight(Double_t weight);
	};
}
#endif
