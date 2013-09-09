/*
 * StackedHistogramRenderer.h
 *
 *  Created on: 16 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef STACKEDHISTOGRAMRENDERER_H_
#define STACKEDHISTOGRAMRENDERER_H_

#include <Rtypes.h>
#include <vector>

#include "models/HistogramData.h"
#include "models/HistogramStackData.h"
#include "Renderer.h"

class TH1;

namespace render {
	/// TODO
	class StackedHistogramRenderer : private Renderer {
		private:
			/// TODO: Comment
			HistogramData observedData;

			/// TODO: Comment
			HistogramStackData histogramStackData;

		public:
			/// Default constructor
			/// @param TODO
			/// @param TODO
			StackedHistogramRenderer(
					HistogramData observedData, HistogramStackData histogramStackData);

			/// Destructor.
			~StackedHistogramRenderer();

			/// TODO: Comment.
			void render();

		private:
			/// TODO: Comment.
			/// @param histogramsToStack TODO
			/// @param dataHistogram TODO
			/// @return TODO
			static Double_t calculateScalingFactor(
					TH1 *dataHistogram, std::vector<TH1*> histogramsToStack);
	};
}
#endif
