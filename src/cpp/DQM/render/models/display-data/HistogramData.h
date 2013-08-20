/*
 * HistogramData.h
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMDATA_H_
#define HISTOGRAMDATA_H_

class TH1;

namespace render {
	/// Model of data required to display a histogram.
	class HistogramData {
		private:
			/// This histogram represented by this data.
			TH1 *histogram;

		public:
			/// Default constructor.
			HistogramData();

			/// Default constructor.
			HistogramData(TH1 *histogram);

			/// Gets the histogram.
			/// @return a pointer to this pair's histogram
			TH1* getHistogram();

			/// Sets the histogram.
			/// @param histogram a pointer to this  data's histogram
			void setHistogram(TH1 *histogram);
	};
}
#endif
