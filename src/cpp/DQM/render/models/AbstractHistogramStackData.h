/*
* AbstractHistogramStackData.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef ABSTRACTHISTOGRAMSTACKDATA_H_
#define ABSTRACTHISTOGRAMSTACKDATA_H_

#include <vector>
#include <cassert>

#include "HistogramData.h"

class TH1;

namespace render {
	/// Data container for a histogram stack.
	template <class T> class AbstractHistogramStackData {
		private:
			/// List of histogram data.
			std::vector<T> allHistogramData;

		public:
			/// Destructor.
			virtual ~AbstractHistogramStackData() {};

			/// Gets the data for all the histograms contained in this histogram stack data.
			/// @return the data for all histograms that made up this stack
			std::vector<T> getAllHistogramData() {
				return(this->allHistogramData);
			}

			/// Gets all of the histograms.
			/// @return all histograms to be drawn in the stack
			std::vector<TH1*> getAllHistograms() {
				std::vector<T> allHistogramsData = this->getAllHistogramData();
				typename std::vector<T>::iterator it = allHistogramsData.begin();
				std::vector<TH1*> histograms;

				while(it != allHistogramsData.end()) {
					T *histogramData = &(*it);
					TH1 *histogram = histogramData->getHistogram();
					histograms.push_back(histogram);
					it++;
				}

				assert(histograms.size() == allHistogramsData.size());
				return(histograms);
			}

			/// Adds data about a histogram that is to be displayed in this histogram stack's data.
			/// <p>
			/// If by adding the histogram, the total weight of all histograms in the stack exceeds
			/// 1.0, a <code>std::invalid_argument</code> exception will be thrown.
			/// @param histogramData data instructing how a histogram is to be displayed
			virtual void add(T histogramData)  {
				this->allHistogramData.push_back(histogramData);
			}

			/// Adds all the <code>WeightedHistogramData</code> data contained in a given
			/// <code>HistogramStackDisplayData</code> to this histogram stack's data container.
			/// If by adding the histogram, the total weight of all histograms in the stack exceeds
			/// 1.0, a <code>std::invalid_argument</code> exception will be thrown.
			/// @param histogramStackData the stack's data to add to this container
//			void add(AbstractHistogramStackData<T> histogramStackData);
	};
}
#endif
