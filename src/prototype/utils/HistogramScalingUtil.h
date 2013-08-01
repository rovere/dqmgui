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

namespace prototype { class WeightedHistogramData; }

class TH1D;

namespace prototype {
	class HistogramScalingUtil {
		private:
			/// Static class: the constructor should not called.
			HistogramScalingUtil();

		public:
			/// TODO: Comment
			void scaleHistogram(TH1D histogram, Double_t targetArea);

			/// TODO: Comment
			void scaleWeightedHistograms(std::list<WeightedHistogramData> weightedHistogramData);
	};
}
#endif
