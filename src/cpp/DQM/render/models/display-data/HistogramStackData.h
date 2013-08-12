/*
 * HistogramStackData.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMDISPLAYDATA_H_
#define HISTOGRAMDISPLAYDATA_H_

#include <Rtypes.h>
#include <list>
#include <vector>

#include "WeightedHistogramData.h"


namespace render {
	/// Data container for a histogram stack.
	class HistogramStackData {
		private:
			/// List of histogram data relating to the histograms that are to be included
			/// in the stack.
			std::vector<WeightedHistogramData> allWeightedHistogramsData;

		public:
			/// Default constructor.
			HistogramStackData();

			/// Adds data about a histogram that is to be displayed in this histogram stack's data.
			/// <p>
			/// If by adding the histogram, the total weight of all histograms in the stack exceeds
			/// 1.0, a <code>std::invalid_argument</code> exception will be thrown.
			/// @param weightedHistogramData data instructing how a histogram is to be displayed
			void add(WeightedHistogramData weightedHistogramData);

			/// Adds all the <code>WeightedHistogramData</code> data contained in a given
			/// <code>HistogramStackDisplayData</code> to this histogram stack's data container.
			/// If by adding the histogram, the total weight of all histograms in the stack exceeds
			/// 1.0, a <code>std::invalid_argument</code> exception will be thrown.
			/// @param histogramStackData the stack's data to add to this container
			void add(HistogramStackData histogramStackData);

			/// Gets the total weight of all the histograms in this container.
			/// @return the total weight of all histograms in this container.
			///			The weight, w, shall be in the range: 1.0 <= w <= 0.0
			Double_t getHistogramsTotalWeight();

			/// Gets the data for all the histograms' contained in this histogram stack data.
			/// @return the data for all histograms that made up this stack
			std::vector<WeightedHistogramData> getAllHistogramsData();
		};
	}
#endif
