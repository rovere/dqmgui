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
		/// @param dataHistogram pointer to the histogram that describes the real data,
		///						 including noise and signal
		/// @param monteCarloHistogramList pointer a list of Monte Carlo histograms that
		///							  	  each describe sources of noise and signal
		StackedHistogramCreator(
				TH1D dataHistogram,
				std::list<TH1D> monteCarloHistogramList);

		/// TODO: Comment
		void drawAllHistograms();

	private:
		/// TODO:Document
		static void normaliseHistograms(std::list<TH1D> *histograms);

		/// TODO: Document
		/// @param histogram the histogram to normalise
		static void normaliseHistogram(TH1D *histogram);

		/// TODO: Document
		Int_t getNextColour();

		/// TODO: Document
		void addToHistogramStack(TH1D &histogram);
		void addAllToHistogramStack(std::list<TH1D> &histograms);
	};

}
#endif
