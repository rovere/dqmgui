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
			/// The ROOT histogram object that is to be displayed.
			TH1 *histogram;

			/// The draw options associated to the ROOT <code>histogram</code> object.
			std::string drawOptions;

		public:
			/// Default constructor.
			/// @param histogram pointer to the ROOT histogram object to be displayed
			HistogramData(TH1 *histogram);

			/// Constructor that allows both the histogram and draw options to be given on
			/// instantiation.
			/// @param histogram pointer to the ROOT histogram object to be displayed
			/// @param drawOptions draw options for the given ROOT histogram
			HistogramData(TH1 *histogram, std::string drawOptions);

			/// Gets a pointer to the ROOT histogram that is to be displayed.
			/// @return a pointer to the ROOT histogram that is to be displayed
			TH1* getHistogram();

			/// Gets the ROOT histogram's draw options.
			/// @return the draw options associated to the histogram
			std::string getDrawOptions();

			/// Sets the ROOT histogram.
			/// @param histogram pointer to the ROOT histogram to display
			void setHistogram(TH1 *histogram);

			/// Sets the draw options associated to the ROOT histogram.
			/// @param drawOptions the draw options associated to the histogram
			void setDrawOptions(std::string drawOptions);
	};
}
#endif
