/*
 * StackedHistogramRenderer.h
 *
 *  Created on: 16 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef STACKEDHISTOGRAMRENDERER_H_
#define STACKEDHISTOGRAMRENDERER_H_

#include <Rtypes.h>
#include <string>
#include <vector>

class TH1;
class TH1D;

namespace render {
	/// TODO
	class StackedHistogramRenderer {
		public:
			/// TODO
			static void render(
					TH1 dataHistogram,
					std::vector<TH1D> monteCarloHistograms,
					std::vector<Double_t> monteCarloHistogramWeights,
					std::string drawOptions);
		};
	}
#endif
