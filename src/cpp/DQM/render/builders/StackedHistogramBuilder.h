/*
 * StackedHistogramCreator.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef STACKEDHISTOGRAMBUILDER_H_
#define STACKEDHISTOGRAMBUILDER_H_

#include <list>
#include <string>

#include "../controllers/ColourController.h"
#include "../models/display-data/HistogramStackDisplayData.h"

class TH1D;
class THStack;
namespace prototype { class WeightedHistogramData; }

namespace prototype {
	/// Builder for stacked histograms.
	class StackedHistogramBuilder {
		private:
			/// The data associated to the stacked histogram this builder shall build.
			HistogramStackDisplayData histogramStackDisplayData;

			/// The object responsible for controlling the colours this builder uses
			/// to create a stacked histogram.
			ColourController colourController;

			/// TODO: Comment
			Double_t targetHistogramArea;


		public:
			/// Default constructor.
			/// TODO: Comment param
			StackedHistogramBuilder(Double_t targetHistogramArea);

			/// @see StackedHistogramBuilder::build()
			THStack build();

			/// Adds data about a histogram that shall be displayed in the histogram
			/// stack that this builder shall build.
			/// <p>
			/// If by adding the histogram, the total weight of all histograms in the
			/// stack that is to be built exceeds 1.0, a <code>std::invalid_argument</code>
			/// exception will be thrown.
			/// @param data the histogram data to add to the histogram stack that is to be
			///				built
			void addHistogramDisplayData(HistogramDisplayData data);

			/// Adds data about many histograms that are to be used to build the histogram
			/// stack.
			/// <p>
			/// If by adding the histograms, the total weight of all histograms in the
			/// stack that is to be built exceeds 1.0, a <code>std::invalid_argument</code>
			/// exception will be thrown.
			/// @param data the container of the histogram data to be added to the histogram
			///				stack when it is built
			void addHistogramStackDisplayData(HistogramStackDisplayData data);

			/// TODO: Comment
			void setTargetHistogramArea(Double_t targetHistogramArea);

		private:
			/// Gets all of the histograms, regardless of weight.
			/// @return all histograms to be drawn in the stack
			std::list<TH1D*> getAllHistograms();

			/// TODO: Comment
			Double_t getTargetHistogramArea();

			/// Adds a given histogram to the given histograms stack.
			/// @param histogram the histogram to put on the histogram stack
			/// @param histogramStack TODO
			void addToHistogramStack(TH1D &histogram, THStack *histogramStack);

			/// Adds all of the histograms given to the given histogram stack.
			/// @param histograms the list of histograms to add to the stack
			/// @param histogramStack TODO
			void addAllToHistogramStack(std::list<TH1D*> histograms, THStack *histogramStack);
	};
}
#endif
