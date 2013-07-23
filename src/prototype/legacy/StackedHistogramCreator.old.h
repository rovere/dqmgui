///*
// * StackedHistogramCreator.h
// *
// *  Created on: 10 Jul 2013
// *      Author: Colin - CERN
// */
//#ifndef STACKEDHISTOGRAMCREATOR_H_
//#define STACKEDHISTOGRAMCREATOR_H_
//
//#include <Rtypes.h>
//#include <TH1.h>
//#include <list>
//#include <string>
//
//#include "../models/HistogramDisplayData.h"
//
//class TApplication;
//class TCanvas;
//class THStack;
//
//namespace prototype {
//	/// TODO: Document class.
//	///
//	///
//	class OldStackedHistogramCreator {
//	private:
//		/// Default label for the histogram stack.
//		static const std::string DEFAULT_STACK_LABEL;
//		/// Default name for the histogram stack.
//		static const std::string DEFAULT_STACK_NAME;
//
//
//
//		/// The data histogram.
//		TH1D dataHistogram;
//
//
//	public:
//		/// Default constructor.
//		/// @param dataHistogram the histogram that describes the real data
//		/// @param histogramDisplayData a list of pairs of MC histograms and their
//		///								associated weights in the histogram stack
//		///								that is to be created for comparison against
//		///								the <code>dataHistogram</code>.
//		OldStackedHistogramCreator(
//				TH1D dataHistogram,
//				std::list<HistogramDisplayData> histogramDisplayData);
//
//		/// Draws all of the histograms - both the MC and data histogram(s).
//		void drawAllHistograms();
//
//	private:
//
//
//
//	};
//}
//#endif
