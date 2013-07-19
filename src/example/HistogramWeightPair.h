/*
 * HistogramWeightPair.h
 *
 *  Created on: 19 Jul 2013
 *      Author: Colin - CERN
 */

#ifndef HISTOGRAMWEIGHTPAIR_H_
#define HISTOGRAMWEIGHTPAIR_H_

#include <Rtypes.h>
#include <TH1.h>
#include <utility>

namespace example {
	/// TODO: Comment class
	class HistogramWeightPair {
		public:
			/// Default constructor.
			/// @param histogram the histogram
			/// @param weight the histogram's weighting in the stack
			HistogramWeightPair(TH1D *histogram, Double_t weight);

			/// Gets the histogram.
			/// @return the histogram
			TH1D* getHistogram();

			/// Gets the histogram's weight, considering the entire stack
			/// of all histograms.
			/// @return the histogram's weighting in the stack
			Double_t getWeight();

			/// Sets the histogram.
			/// @param histogram the histogram
			void setHistogram(TH1D *histogram);

			/// Sets the histogram's weight, considering the entire stack.
			/// @param weight the histogram's weighting in the stack
			void setWeight(Double_t weight);

		private:
			TH1D *histogram;
			Double_t weight;
	};
}
#endif
