/*
 * StackedHistogramCreator.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef STACKEDHISTOGRAMBUILDER_H_
#define STACKEDHISTOGRAMBUILDER_H_

#include <list>

#include "../controllers/ColourController.h"
#include "../models/HistogramStackDisplayData.h"

class TH1D;
class THStack;
namespace prototype { class HistogramDisplayData; }

namespace prototype {
	/// TODO: Document class.
	class StackedHistogramBuilder {
		private:
			/// TODO: Comment.
			HistogramStackDisplayData histogramStackDisplayData;
			/// TODO: Comment.
			ColourController colourController;

			/// The title of the stacked histogram that is to be built.
			std::string title;
			/// The label of the stacked histogram that is to be build.
			std::string label;

		public:
			/// Default constructor.
			StackedHistogramBuilder();

			/// TODO: Comment.
			/// @param data TODO
			void addHistogramDisplayData(HistogramDisplayData data);

			/// TODO: Comment. (Does not overwrite)
			/// @param data TODO
			void addHistogramStackDisplayData(HistogramStackDisplayData data);

			/// TODO: Comment.
			THStack build();

		private:
			/// Adds a given histogram to the given histograms stack.
			/// @param histogram the histogram to put on the histogram stack
			/// @param histogramStack TODO
			void addToHistogramStack(TH1D &histogram, THStack *histogramStack);

			/// Adds all of the histograms given to the given histogram stack.
			/// @param histograms the list of histograms to add to the stack
			/// @param histogramStack TODO
			void addAllToHistogramStack(std::list<TH1D*> histograms, THStack *histogramStack);

			/// Gets all of the histograms, regardless of weight.
			/// @return all histograms to be drawn in the stack
			std::list<TH1D*> getAllHistograms();
	};
}
#endif
