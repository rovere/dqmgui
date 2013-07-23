/*
 * HistogramDisplayData.h
 *
 *  Created on: 19 Jul 2013
 *      Author: Colin - CERN
 */

#ifndef HISTOGRAMWEIGHTPAIR_H_
#define HISTOGRAMWEIGHTPAIR_H_

#include <Rtypes.h>

class TH1D;

namespace prototype {
	/// A model containing the data required to display a histogram,
	/// including the histogram to be displayed and its associated weight
	/// in the context of a list of histograms that a histogram
	/// stack is to be created from.
	class HistogramDisplayData {
		private:
			/// This pair's histogram.
			TH1D *histogram;
			/// The weight of this histogram in a histogram stack.
			Double_t weight;

		public:
			/// Default constructor.
			/// @param histogram a pointer to this data's histogram
			/// @param weight the histogram's weighting in the stack
			HistogramDisplayData(TH1D *histogram, Double_t weight);

			/// Gets the histogram.
			/// @return a pointer to this pair's histogram
			TH1D* getHistogram();

			/// Gets the histogram's weight, considering the entire stack
			/// of all histograms.
			/// @return the histogram's weighting in the stack
			Double_t getWeight();

			/// Sets the histogram.
			/// @param histogram a pointer to this  data's histogram
			void setHistogram(TH1D *histogram);

			/// Sets the histogram's weight, considering the entire stack.
			/// @param weight the histogram's weighting in the stack
			void setWeight(Double_t weight);
	};
}
#endif
