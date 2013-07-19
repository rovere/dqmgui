/*
 * HistogramNormalisationUtil.h
 *
 *  Created on: 16 Jul 2013
 *      Author: Colin - CERN
 */

#ifndef HISTOGRAMNORMALISATIONUTIL_H_
#define HISTOGRAMNORMALISATIONUTIL_H_

//#include <Rtypes.h>
#include <list>
//#include <list>

namespace example { class HistogramWeightPair; }

class TH1D;

namespace example {
	class HistogramNormalisationUtil {
		public:
			/// TODO: Comment
			static void normaliseHistogram(TH1D *histogram);

			/// Normalise a given list of histograms such that, when stacked, they will
			/// have a unit area.
			/// @param histogramWeightPairs TODO
			static void normaliseHistograms(std::list<HistogramWeightPair> *histogramWeightPairs);

		private:
			/// Normalises a histogram that is to be included in a stack such that all
			/// histograms in the stack with have a sum area of 1. TODO: Update
			/// @param histogramWeightPair TODO
			static void normaliseHistogram(HistogramWeightPair histogramWeightPair);
	};
}
#endif
