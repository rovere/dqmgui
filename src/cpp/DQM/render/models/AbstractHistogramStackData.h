/*
 * AbstractHistogramStackData.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef ABSTRACTHISTOGRAMSTACKDATA_H_
#define ABSTRACTHISTOGRAMSTACKDATA_H_

#include <vector>

#include "HistogramData.h"

class TH1;

namespace render {
	/// Data container for a histogram stack.
	template <class T> class AbstractHistogramStackData {
		public:
			/// Gets the data for all the histograms contained in this histogram stack data.
			/// @return the data for all histograms that made up this stack
			virtual std::vector<T> getAllHistogramData() = 0;

			/// Gets all of the histograms.
			/// @return all histograms to be drawn in the stack
			virtual std::vector<TH1*> getAllHistograms() = 0;

			/// Adds data about a histogram that is to be displayed in this histogram stack's data.
			/// <p>
			/// If by adding the histogram, the total weight of all histograms in the stack exceeds
			/// 1.0, a <code>std::invalid_argument</code> exception will be thrown.
			/// @param histogramData data instructing how a histogram is to be displayed
			virtual void add(T histogramData) = 0;

			/// Adds all the <code>WeightedHistogramData</code> data contained in a given
			/// <code>HistogramStackDisplayData</code> to this histogram stack's data container.
			/// If by adding the histogram, the total weight of all histograms in the stack exceeds
			/// 1.0, a <code>std::invalid_argument</code> exception will be thrown.
			/// @param histogramStackData the stack's data to add to this container
//			void add(AbstractHistogramStackData<T> histogramStackData);
		};
	}
#endif
