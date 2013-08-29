/*
 * HistogramScalingUtil.h
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMSCALINGUTIL_H_
#define HISTOGRAMSCALINGUTIL_H_

#include <Rtypes.h>
#include <list>

class TH1;
namespace render { class WeightedHistogramData; }

namespace render {
	/// Utility class for scaling histograms.
	class HistogramScalingUtil {
		private:
			/// Static class: the constructor should not called.
			HistogramScalingUtil();

		public:
			/// Scales a given <code>histogram</code> such that it's integral
			/// equals <code>targetArea</code>.
			/// @param histogram a pointer to the histogram that is to be scaled
			/// @param targetArea the area under the histogram required
			static void scaleHistogram(TH1 *histogram, Double_t targetArea);

			/// Scales a list of histograms with respect to their relative weights
			/// such that their combined weight when scaled equals <code>targetCombinedArea</code>.
			/// @param weightedHistogramData list of data corresponding to weighted histograms
			/// @param targetCombinedArea the area that the histograms should have when stacked
			static void scaleWeightedHistograms(
					std::list<WeightedHistogramData> weightedHistogramData,
					Double_t targetCombinedArea);

			/// Scales a weighted histogram such that its area equals the product of the histogram's
			/// weight and the <code>combinedTargetArea</code>. i.e. the histogram's area should be
			/// a proportion of the combined area, with the size of the proportion dependent on the
			/// histogram's weight.
			/// @param weightedHistogramData data corresponding to the weighted histogram to scale
			/// @param combinedTargetArea the area that all the weighted histograms should have when
			///							  stacked
			static void scaleWeightedHistogram(
					WeightedHistogramData weightedHistogramData,
					Double_t combinedTargetArea);
	};
}
#endif
