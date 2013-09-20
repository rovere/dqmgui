/*
 * StackedHistogramRenderer.h
 *
 *  Created on: 16 Aug 2013
 *      Author: Colin Nolan
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
	/// Renderes stacked histogram and data (MC vs. data) onto the ROOT canvas.
	class StackedHistogramRenderer : public Renderer {
		private:
			/// Data about the histogram that represents the observed data.
			HistogramData observedData;

			/// Data about the MC histograms that are to be stacked.
			HistogramStackData histogramStackData;

		public:
			/// Default constructor.
			/// @param observedData data about the histogram that represents the observed data
			/// @param histogramStackData data about the MC histograms that are to be stacked
			StackedHistogramRenderer(
					HistogramData observedData, HistogramStackData histogramStackData);

			/// Renders the stacked MC histograms vs. the observed data.
			void render();

		private:
			/// Calculates the scaling factor that should be applied to the MC histograms.
			/// @param dataHistogram pointer to the data histogram
			/// @param histogramsToStack pointers to the MC histograms
			/// @return the calculated scaling factor
			static Double_t calculateScalingFactor(
					TH1 *dataHistogram, std::vector<TH1*> histogramsToStack);
	};
}
#endif
