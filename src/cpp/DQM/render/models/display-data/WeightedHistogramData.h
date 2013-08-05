/*
 * HistogramScalingData.h
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMSCALINGDATA_H_
#define HISTOGRAMSCALINGDATA_H_

#include <Rtypes.h>

#include "HistogramData.h"

class TH1D;

namespace prototype {
	class WeightedHistogramData : public prototype::HistogramData {
		private:
			/// The weight of this histogram in a histogram stack.
			Double_t weight;

		public:
			/// Default constructor.
			/// @param histogram a pointer to this data's histogram
			/// @param weight the histogram's weighting in the stack
			WeightedHistogramData(TH1D *histogram, Double_t weight);

			/// Gets the histogram's weight, considering the entire stack
			/// of all histograms.
			/// @return the histogram's weighting in the stack
			Double_t getWeight();

			/// Sets the histogram's weight, considering the entire stack.
			/// @param weight the histogram's weighting in the stack
			void setWeight(Double_t weight);
	};
}
#endif
