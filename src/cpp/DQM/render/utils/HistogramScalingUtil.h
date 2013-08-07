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

namespace render { class WeightedHistogramData; }

class TH1D;

namespace render {
	class HistogramScalingUtil {
		private:
			/// Static class: the constructor should not called.
			HistogramScalingUtil();

		public:
			/// TODO: Comment
			static void scaleHistogram(TH1D *histogram, Double_t targetArea);

			/// TODO: Comment
			static void scaleWeightedHistograms(
					std::list<WeightedHistogramData> weightedHistogramData,
					Double_t targetCombinedArea);

			/// TODO: Comment
			static void scaleWeightedHistogram(
					WeightedHistogramData weightedHistogramData,
					Double_t combinedTargetArea);
	};
}
#endif
