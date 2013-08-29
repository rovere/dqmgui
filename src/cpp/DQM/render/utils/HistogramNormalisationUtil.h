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

class TH1;
namespace render { class WeightedHistogramData; }

namespace render {
	/// Utility class for normalising histograms.
	class HistogramNormalisationUtil {
		private:
			/// Defines the size of a unit area (the area of a normalised histogram)
			static const Double_t UNIT_AREA;

		private:
			/// Static class: the constructor should not called.
			HistogramNormalisationUtil();

		public:
			/// Normalises a histogram such that it has an area of <code>UNIT_AREA</code>.
			/// @param histogram pointer to the histogram to normalise.
			///					 This histogram object is modified
			static void normaliseHistogram(TH1 *histogram);

			/// Normalise a given list of histograms such that, when stacked, they will
			/// have an area of <code>UNIT_AREA</code>. The weight of each histogram in the stack
			///	(i.e. histograms each influence the resulting stack to different degrees)
			/// is defined within the <code>HistogramWeightPair</code> instance.
			/// @param weightedHistogramData a list of the <code>WeightedHistogramData</code> instances
			///								containing histograms that are to be normalised considering their weights.
			///								The sum of all the histogram's weights must equal <code>UNIT_AREA</code>
			///								else an <code>std::invalid_argument</code> exception shall
			///								be thrown. The histograms within <code>weightedHistogramData</code>
			///								are modified.
			static void normaliseWeightedHistograms(std::list<WeightedHistogramData> weightedHistogramData);

			/// Normalises a histogram that is to be included in a stack such that all
			/// histograms in the stack with have an area that sums to <code>UNIT_AREA</code>.
			/// <p>
			/// Therefore the area of this histogram after normalisation shall be:
			/// <code>1 / histogramWeightPair.getWeight()</code>.
			/// @param weightedHistogramData a <code>WeightedHistogramData</code> instance
			///							    containing a histogram that is to be normalised (with respect
			///								to a stack of histograms) and the weight of the normalisation
			static void normaliseWeightedHistogram(WeightedHistogramData weightedHistogramData);
	};
}
#endif
