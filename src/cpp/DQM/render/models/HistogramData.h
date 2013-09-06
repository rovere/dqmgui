/*
 * HistogramData.h
 *
 *  Created on: 1 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMDATA_H_
#define HISTOGRAMDATA_H_

#include <string>

class TH1;

namespace render {
	/// Model of data required to display a histogram.
	class HistogramData {
		private:
			/// The histogram represented by this data.
			TH1 *histogram;

			/// The draw options for the histogram represented by this data.
			std::string drawOptions;

		public:
			/// Default constructor.
			HistogramData(TH1 *histogram);

			/// Constructor that allows both the histogram and draw options to
			/// be given on instantiation.
			HistogramData(TH1 *histogram, std::string drawOptions);

			/// Gets the histogram.
			/// @return a pointer to this pair's histogram
			TH1* getHistogram();

			/// Gets the histogram's draw options.
			/// @return the draw options associated to the histogram
			std::string getDrawOptions();

			/// Sets the histogram.
			/// @param histogram a pointer to this  data's histogram
			void setHistogram(TH1 *histogram);

			/// Sets the histogram's draw options.
			/// @param the draw options associated to the histogram
			void setDrawOptions(std::string drawOptions);
	};
}
#endif
