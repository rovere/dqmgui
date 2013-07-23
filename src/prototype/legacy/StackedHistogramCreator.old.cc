///*
// * StackedHistogramCreator.cc
// *
// *  Created on: 10 Jul 2013
// *      Author: Colin - CERN
// */
//#define DNDEBUG
//
//#include "StackedHistogramCreator.old.h"
//
//#include <Rtypes.h>
//#include <TAttFill.h>
//#include <TAttLine.h>
//#include <TH1.h>
//#include <THStack.h>
//#include <TObject.h>
//#include <cassert>
//#include <list>
//#include <string>
//#include <iostream>
//
//#include "../HistogramNormalisationUtil.h"
//
//namespace prototype {
//
//
//	OldStackedHistogramCreator::OldStackedHistogramCreator(
//			TH1D dataHistogram,
//			std::list<HistogramDisplayData> histogramWeightPairs)
//				: dataHistogram(dataHistogram),
//				  histogramDisplayData(histogramWeightPairs),
//				  colourIndex(0) {
//
//
//		HistogramNormalisationUtil::normaliseHistograms(&this->histogramDisplayData);
//	}
//
//	void OldStackedHistogramCreator::drawAllHistograms() {
//		std::list<TH1D*> histograms = this->getAllHistograms();
//
//		// XXX: It is assumed that all of the histograms have been normalised at this point...
//		this->addAllToHistogramStack(histograms);
//		this->histogramStack->Draw();
//
//		HistogramNormalisationUtil::normaliseHistogram(&this->dataHistogram);
//		this->dataHistogram.SetLineColor(COLOUR_BLACK);
//		this->dataHistogram.SetLineStyle(2);
//		this->dataHistogram.SetLineWidth(5);
//		this->dataHistogram.Draw("SAME");
//	}
//
//
//
//
//
//}
