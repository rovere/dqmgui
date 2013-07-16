/*
 * HistogramNormalisationUtil.h
 *
 *  Created on: 16 Jul 2013
 *      Author: Colin - CERN
 */

#ifndef HISTOGRAMNORMALISATIONUTIL_H_
#define HISTOGRAMNORMALISATIONUTIL_H_

#include <Rtypes.h>
#include <list>

class TH1D;

namespace example {
	class HistogramNormalisationUtil {
		public:
			/// TODO: Comment
			static void normaliseHistogram(TH1D *histogram);

			/// Normalise a given list of histograms such that, when stacked, they will
			/// have a unit area.
			/// @param histograms a pointer to the list of histograms to be normalised
			static void normaliseHistograms(std::list<TH1D> *histograms);

		private:
			/// Normalises a histogram that is to be included in a stack (or on its own
			/// if {@code totalHistogramsInStack == 1}) such that all histograms in the
			///	stack with have a sum area of 1.
			/// @param histogram a pointer to the histogram to normalise
			/// @param totalHistogramsInStack TODO
			static void normaliseHistogram(TH1D *histogram, Double_t totalHistogramsInStack);

			/// TODO: Comment
			static Int_t getEntriesInHistograms(std::list<TH1D> *histograms);
	};
}
#endif
