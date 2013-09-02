/*
 * WeightedStackedHistogramBuilder.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef WEIGHTEDSTACKEDHISTOGRAMBUILDER_H_
#define WEIGHTEDSTACKEDHISTOGRAMBUILDER_H_

#include <Rtypes.h>
#include "StackedHistogramBuilder.h"

namespace render { class HistogramStackData; }

class TH1;
class THStack;
namespace render { class WeightedHistogramData; }

namespace render {
	/// Builder for stacked histograms.
	class WeightedStackedHistogramBuilder : public StackedHistogramBuilder {
		private:
			/// The area that the built histogram should have.
			Double_t targetHistogramArea;

		public:
			/// Default constructor.
			/// @param targetHistogramArea the area that the built histogram should have
			WeightedStackedHistogramBuilder(Double_t targetHistogramArea);

//			/// @see HistogramBuilder::build()
//			/// Note: A pointer to the <code>THStack</code> object is been returned as in the
//			///		  actual system, the calling code using <code>THStack.Draw</code> was ineffective
//			///		  even though returning a reference and using <code>THStack->Draw</code> in the
//			/// 	  standalone test project was fine!?
//			///		  <p>
//			///		  This developer does not know exactly why the same code performed differently
//			/// 	  in the actual system and can only speculate that it didn't work because of the
//			///		  environment in which code executes is different in the actual system.
//			THStack* build();

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


		private:
			/// Gets the area that the built histogram should have.
			/// @return the area that the built histogram should have
			Double_t getTargetHistogramArea();

			/// Sets the area that the stacked histogram should have.
			/// @param targetHistogramArea the area that the stacked histogram should have
			void setTargetHistogramArea(Double_t targetHistogramArea);
	};
}
#endif
