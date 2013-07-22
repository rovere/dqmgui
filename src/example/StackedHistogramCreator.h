/*
 * StackedHistogramCreator.h
 *
 *  Created on: 10 Jul 2013
 *      Author: Colin - CERN
 */

#include <Rtypes.h>
#include <TH1.h>
#include <list>
//#include <list>
//#include <string>
#include <xstring>

#include "HistogramWeightPair.h"

class TApplication;
class TCanvas;
class THStack;

#ifndef STACKEDHISTOGRAMCREATOR_H_
#define STACKEDHISTOGRAMCREATOR_H_
namespace prototype {
	/// TODO: Document
	///
	///
	class StackedHistogramCreator {
	private:
		/// Default label for the histogram stack.
		static const std::string DEFAULT_STACK_LABEL;
		/// Default name for the histogram stack.
		static const std::string DEFAULT_STACK_NAME;

		/// List of default colours.
		static const Int_t DEFAULT_COLOURS[];
		/// The black colour.
		static const Int_t COLOUR_BLACK;

		/// The histogram stack to be created.
		THStack *histogramStack;
		/// The pairs of histograms to create the histogram stack from.
		std::list<HistogramWeightPair> histogramWeightPairs;
		/// The data histogram.
		TH1D dataHistogram;
		/// The index for {@code DEFAULT_COLOURS} of the colour to be used next.
		Int_t colourIndex;

	public:
		/// Default constructor.
		/// @param dataHistogram the histogram that describes the real data
		/// @param histogramWeightPairs a list of pairs of MC histograms and their
		///								associated weights in the histogram stack
		///								that is to be created for comparison against
		///								the {@code dataHistogram}.
		StackedHistogramCreator(
				TH1D dataHistogram,
				std::list<HistogramWeightPair> histogramWeightPairs);

		/// Draws all of the histograms - both the MC and data histogram(s).
		void drawAllHistograms();

	private:
		/// Gets the next colour to be used as a histogram's background colour.
		/// @return integer representing the colour
		Int_t getNextColour();

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
