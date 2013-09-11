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
			/// The minimum required accuracy to which the histograms are to be scaled to.
			static const Double_t REQUIRED_ACCURACY;

		private:
			/// Static class: the constructor should not called.
			HistogramScalingUtil();

		public:
			/// Scales a given <code>TH1</code> histogram object such that it's integral
			/// equals <code>targetArea</code>.
			/// @param histogram a pointer to the histogram that is to be scaled
			/// @param targetArea the area under the histogram required
			static void scaleHistogramToArea(TH1 *histogram, Double_t targetArea);

			/// Scales a given <code>TH1</code> histogram object such that it's integral
			/// is multiplied by the given <code>scalingFactor</code>.
			/// @param histogram a pointer to the histogram that is to be scaled
			/// @param scalingFactor the factor by which the given histogram's area is to be
			///						 multiplied by
			static void scaleHistogram(TH1 *histogram, Double_t scalingFactor);

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
