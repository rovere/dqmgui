/*
 * RandomHistogramGenerator.h
 *
 *  Created on: 24 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef RANDOMHISTOGRAMGENERATOR_H_
#define RANDOMHISTOGRAMGENERATOR_H_

#include <Rtypes.h>
#include <list>

class TH1D;

namespace prototype {
	class RandomHistogramGenerator {
		public:
			/// The default number of histogram entries put into a randomly generated histogram.
			static const Int_t DEFAULT_HISTOGRAM_ENTRIES = 50000;
			/// The ID to give to the next histogram.
			static Int_t nextHistogramId;

		private:
			/// Static class: the constructor should not called.
			RandomHistogramGenerator();

		public:
			/// Creates a histogram with a Gaussian frequency distribution
			/// and <code>DEFAULT_HISTOGRAM_ENTRIES</code> entries.
			/// @see generateGausHistogram(Int_t, Int_t)
			static TH1D* createGausHistogram(Int_t id);

			/// Creates a histogram with a Gaussian frequency distribution.
			/// @param id the identification number of the histogram (used by ROOT)
			/// @param totalObservations the number of observations to include in the histogram
			/// @return the generated histogram
			static TH1D* createGausHistogram(Int_t id, Int_t totalObservations);

			/// Creates a single histogram to mimic one created by a Monte Carlo simulation.
			/// @return the generated histogram
			static TH1D* createMCHistogram();

			/// Creates a list of pseudo Monte Carlo (MC) simulated histograms.
			/// @param numberOfHistograms the number of MC histograms to generate
			/// @return a list of generated histograms
			static std::list<TH1D> createMCHistogramList(Int_t numberOfHistograms);
	};
}
#endif
