/*
 * StackedHistogramCreator.h
 *
 *  Created on: 10 Jul 2013
 *      Author: Colin - CERN
 */
#include <TH1.h>
#include <list>
#include <string.h>

class TApplication;
class TCanvas;
class THStack;


#ifndef STACKEDHISTOGRAMCREATOR_H_
#define STACKEDHISTOGRAMCREATOR_H_
namespace example {
	/// TODO
	///
	///
	class StackedHistogramCreator {
	private:
		/// Default label for the histogram stack
		static const std::string DEFAULT_STACK_LABEL;
		/// Default name for the histogram stack
		static const std::string DEFAULT_STACK_NAME;

		/// TODO: Document
		static const Int_t DEFAULT_COLOURS[];
		/// TODO: Document
		static const Int_t COLOUR_BLACK;

		THStack *histogramStack;
		TH1D dataHistogram;
		std::list<TH1D> monteCarloHistogramList;
		Int_t colourIndex;

	public:
		/// Default constructor.
		/// @param dataHistogram the histogram that describes the real data
		/// @param monteCarloHistogramList a list of Monte Carlo histograms that
		///							  	   each describe sources of noise and signal
		StackedHistogramCreator(
				TH1D dataHistogram,
				std::list<TH1D> monteCarloHistogramList);

		/// Draws all of the histograms - both the MC and data histogram(s).
		void drawAllHistograms();

	private:
		/// Normalise a given list of histograms such that, when stacked, they will
		/// have a unit area.
		/// @param histograms a pointer to the list of histograms to be normalised
		static void normaliseHistograms(std::list<TH1D> *histograms);

		/// Normalises a histogram that is to be included in a stack (or on its own
		/// if {@code totalHistogramsInStack == 1}) such that all histograms in the
		///	stack with have a sum area of 1.
		/// @param histogram a pointer to the histogram to normalise
		/// @param totalHistogramsInStack the number of histograms in the same stack
		///								  as the histogram that's been normalised
		static void normaliseHistogram(TH1D *histogram, Int_t totalHistogramsInStack);

		/// Gets the next colour to be used as a histogram's background colour.
		/// @return integer representing the colour
		Int_t getNextColour();

		/// Adds a given histogram to the histograms stack.
		/// @param histogram copy of the histogram to put on the histogram stack
		void addToHistogramStack(TH1D &histogram);

		/// Adds all of the histograms given to the histogram stack.
		/// @param histograms a pointer to a list of histograms to add to the stack
		void addAllToHistogramStack(std::list<TH1D> *histograms);
	};

}
#endif
