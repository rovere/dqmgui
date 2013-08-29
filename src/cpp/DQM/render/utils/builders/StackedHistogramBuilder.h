/*
 * StackedHistogramBuilder.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef STACKEDHISTOGRAMBUILDER_H_
#define STACKEDHISTOGRAMBUILDER_H_

#include <Rtypes.h>
#include <vector>

#include "../../controllers/ColourController.h"
#include "../../models/display-data/HistogramStackData.h"
#include "Builder.h"

class TH1;
class THStack;
namespace render { class WeightedHistogramData; }

namespace render {
	/// Builder for stacked histograms.
	class StackedHistogramBuilder : public Builder<THStack*> {
		private:
			/// The data associated to the histogram stack that this instance will build
			HistogramStackData histogramStackData;

			/// The object responsible for controlling the colours this builder uses
			/// to create a stacked histogram.
			ColourController colourController;

			/// The area that the built histogram should have.
			Double_t targetHistogramArea;

		public:
			/// Default constructor.
			/// @param targetHistogramArea the area that the built histogram should have
			StackedHistogramBuilder(Double_t targetHistogramArea);

			/// @see HistogramBuilder::build()
			/// Note: A pointer to the <code>THStack</code> object is been returned as in the
			///		  actual system, the calling code using <code>THStack.Draw</code> was ineffective
			///		  even though returning a reference and using <code>THStack->Draw</code> in the
			/// 	  standalone test project was fine!?
			///		  <p>
			///		  This developer does not know exactly why the same code performed differently
			/// 	  in the actual system and can only speculate that it didn't work because of the
			///		  environment in which code executes is different in the actual system.
			THStack* build();

			/// Adds data about a histogram that shall be displayed in the histogram
			/// stack that this builder shall build.
			/// <p>
			/// If by adding the histogram, the total weight of all histograms in the
			/// stack that is to be built exceeds 1.0, a <code>std::invalid_argument</code>
			/// exception will be thrown.
			/// @param data the histogram data to add to the histogram stack that is to be
			///				built
			void addWeightedHistogramData(WeightedHistogramData data);

			/// Adds data about many histograms that are to be used to build the histogram
			/// stack.
			/// <p>
			/// If by adding the histograms, the total weight of all histograms in the
			/// stack that is to be built exceeds 1.0, a <code>std::invalid_argument</code>
			/// exception will be thrown.
			/// @param data the container of the histogram data to be added to the histogram
			///				stack when it is built
			void addHistogramStackData(HistogramStackData data);

			/// Sets the area that the stacked histogram should have.
			/// @param targetHistogramArea the area that the stacked histogram should have
			void setTargetHistogramArea(Double_t targetHistogramArea);

		private:
			/// Gets all of the histograms, regardless of weight.
			/// @return all histograms to be drawn in the stack
			std::vector<TH1*> getAllHistograms();

			/// Gets the area that the built histogram should have.
			/// @return the area that the built histogram should have
			Double_t getTargetHistogramArea();

			/// Adds a given histogram to the given histograms stack.
			/// @param histogram pointer to the histogram to put on the histogram stack
			/// @param histogramStack the histogram stack that the histogram is to be added to
			void addToHistogramStack(TH1 *histogram, THStack *histogramStack);

			/// Adds all of the histograms given to the given histogram stack.
			/// @param histograms the list of histograms to add to the stack
			/// @param histogramStack the histogram stack that the histogram is to be added to
			void addAllToHistogramStack(std::vector<TH1*> histograms, THStack *histogramStack);
	};
}
#endif
