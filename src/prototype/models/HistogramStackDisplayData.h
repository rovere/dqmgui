/*
 * HistogramDisplayDataSet.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMDISPLAYDATA_H_
#define HISTOGRAMDISPLAYDATA_H_

#include <Rtypes.h>
#include <list>

#include "HistogramDisplayData.h"

namespace prototype {
	/// Data container for a histogram stack.
	class HistogramStackDisplayData {
		private:
			/// List of histogram data relating to the histograms that are to be included
			/// in the stack.
			std::list<HistogramDisplayData> histogramDisplayData;

		public:
			/// Default constructor.
			HistogramStackDisplayData();

			/// Adds data about a histogram that is to be displayed in this histogram stack's data.
			/// <p>
			/// If by adding the histogram, the total weight of all histograms in the stack exceeds
			/// 1.0, a <code>std::invalid_argument</code> exception will be thrown.
			/// @param histogramDisplayData data instructing how a histogram is to be displayed
			void add(HistogramDisplayData histogramDisplayData);

			/// Adds all the <code>HistogramDisplayData</code> data contained in a given
			/// <code>HistogramStackDisplayData</code> to this histogram stack's data container.
			/// If by adding the histogram, the total weight of all histograms in the stack exceeds
			/// 1.0, a <code>std::invalid_argument</code> exception will be thrown.
			/// @param displayData the stack's data to add to this container
			void add(HistogramStackDisplayData histogramStackDisplayData);

			/// Gets the total weight of all the histograms in this container.
			/// @return the total weight of all histograms in this container.
			///			The weight, w, shall be in the range: 1.0 <= w <= 0.0
			Double_t getHistogramsTotalWeight();

			/// Gets the data for all the histograms' contained in this histogram stack data.
			/// @return the data for all histograms that made up this stack
			std::list<HistogramDisplayData> getAllHistogramDisplayData();
		};
	}
#endif
