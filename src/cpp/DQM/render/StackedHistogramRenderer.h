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

namespace render {
	/// TODO
	class StackedHistogramRenderer {
		public:
			/// TODO: Comment.
			/// @param dataHistogram a pointer to the histogram representing the distribution
			///						 of the data. A pointer must be passed opposed to a reference.
			/// @param monteCarloHistograms TODO
			/// @param monteCarloHistogramWeights TODO
			/// @param drawOptions TODO
			static void render(
					TH1 *dataHistogram,
					std::vector<TH1*> monteCarloHistograms,
					std::vector<Double_t> monteCarloHistogramWeights,
					std::string drawOptions);

			/// TODO: Comment.
			/// @param histogramTitle TODO
			/// @param errorMessage TODO
			static void showErrorMessage(
					std::string histogramTitleText, std::string errorMessageText);
		};
	}
#endif
