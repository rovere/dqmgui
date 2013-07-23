/*
 * StackedHistogramCreator.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef STACKEDHISTOGRAMCREATOR_H_
#define STACKEDHISTOGRAMCREATOR_H_

#include <list>

#include "controllers/ColourController.h"
#include "models/HistogramStackDisplayData.h"

class TH1D;
class THStack;

namespace prototype {
	/// TODO: Document class.
	class StackedHistogramBuilder {
		private:
			/// TODO: Comment.
			THStack *histogramStack;
			/// TODO: Comment.
			HistogramStackDisplayData displayData;
			/// TODO: Comment.
			ColourController colourController;

		public:
			/// Default constructor.
			/// @param TODO
			StackedHistogramBuilder(HistogramStackDisplayData displayData);

			/// TODO: Comment.
			THStack build();

		private:
			/// Adds a given histogram to the histograms stack.
			/// @param histogram a copy of the histogram to put on the histogram stack
			void addToHistogramStack(TH1D &histogram);

			/// Adds all of the histograms given to the histogram stack.
			/// @param histograms the list of histograms to add to the stack
			void addAllToHistogramStack(std::list<TH1D*> histograms);

			/// Gets all of the histograms, regardless of weight.
			/// @return all histograms to be drawn in the stack
			std::list<TH1D*> getAllHistograms();
	};
}
#endif
