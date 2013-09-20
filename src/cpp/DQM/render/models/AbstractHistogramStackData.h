/*
* AbstractHistogramStackData.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin Nolan
 */
#ifndef ABSTRACTHISTOGRAMSTACKDATA_H_
#define ABSTRACTHISTOGRAMSTACKDATA_H_

#include <vector>
#include <cassert>

#include "HistogramData.h"

class TH1;

namespace render {
	/// Model of data required to draw an (abstract) histogram stack.
	template <class T> class AbstractHistogramStackData {
		private:
			/// List of data about all histogram in the stack.
			std::vector<T> allHistogramData;

		public:
			/// Destructor.
			virtual ~AbstractHistogramStackData() {};

			/// Gets data about all histograms to be in the histogram stack.
			/// @return the data for all histograms that made up the histogram stack
			std::vector<T> getAllHistogramData() {
				return(this->allHistogramData);
			}

			/// Gets all of the ROOT histograms objects to be made into the histogram stack.
			/// @return all ROOT histograms objects to be included in the histogarm stack
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

			/// Adds data about a histogram that is to be part of the histogram stack.
			/// @param histogramData data instructing how a histogram is to be displayed in the
			///						 histogram stack that is to be drawn
			virtual void add(T histogramData) {
				this->allHistogramData.push_back(histogramData);
			}
	};
}
#endif
