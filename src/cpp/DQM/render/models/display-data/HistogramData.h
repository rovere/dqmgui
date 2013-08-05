/*
 * HistogramData.h
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMDATA_H_
#define HISTOGRAMDATA_H_

class TH1D;

namespace prototype {
	/// TODO: Class documentation
	class HistogramData {
		private:
			/// This histogram represented by this data.
			TH1D *histogram;

		public:
			/// Default constructor.
			HistogramData();

			/// Default constructor.
			HistogramData(TH1D *histogram);

			/// Gets the histogram.
			/// @return a pointer to this pair's histogram
			TH1D* getHistogram();

			/// Sets the histogram.
			/// @param histogram a pointer to this  data's histogram
			void setHistogram(TH1D *histogram);
	};
}
#endif
