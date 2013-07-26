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
	/// TODO: Document class.
	class HistogramStackDisplayData {
		private:
			/// TODO: Comment.
			std::list<HistogramDisplayData> histogramDisplayData;

		public:
			/// Default constructor.
			HistogramStackDisplayData();

			/// TODO: Comment. Include information about invalid adds.
			/// @param histogramDisplayData data instructing how a histogram is to be displayed
			void add(HistogramDisplayData histogramDisplayData);

			/// TODO: Comment.
			/// @param displayData TODO
			void add(HistogramStackDisplayData histogramStackDisplayData);

			/// TODO: Comment.
			/// @return TODO
			Double_t getHistogramsTotalWeight();

			/// TODO: Comment
			/// @return TODO
			std::list<HistogramDisplayData> getAllHistogramDisplayData();
		};
	}
#endif
