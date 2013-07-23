/*
 * HistogramDisplayDataSet.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMDISPLAYDATASET_H_
#define HISTOGRAMDISPLAYDATASET_H_

#include <list>

#include "HistogramDisplayData.h"

namespace prototype {
	/// TODO: Document class.
	class HistogramStackDisplayData {
		private:
			/// TODO: Comment.
			std::list<HistogramDisplayData> histogramDisplayData;

		public:
			/// Default constructor.
			HistogramStackDisplayData();

			/// TODO: Comment. Include information about invalid adds.
			/// @param displayData data instructing how a histogram is to be displayed
			void add(HistogramDisplayData displayData);

			/// TODO: Comment.
			/// @return TODO
			Double_t getHistogramsTotalWeight();

			/// TODO: Comment
			/// @return TODO
			std::list<HistogramDisplayData> getAllHistogramDisplayData();
		};
	}
#endif
