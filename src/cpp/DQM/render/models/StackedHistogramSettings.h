/*
 * StackedHistogramSettings.h
 *
 *  Created on: 6 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef STACKEDHISTOGRAMSETTINGS_H_
#define STACKEDHISTOGRAMSETTINGS_H_

#include <Rtypes.h>
#include <list>
#include <string>

namespace render {
	/// Settings for drawing a stacked histogram.
	class StackedHistogramSettings {
		private:
			Bool_t drawStackedHistogram;

		public:
			/// Default constructor.
			StackedHistogramSettings();

			/// Gets whether a stacked histogram should be drawn.
			/// @return whether a stacked histogram should be drawn
			Bool_t shouldDrawStackedHistogram();

			/// Sets whether a stacked histogram should be drawn.
			/// @param shouldDraw whether a stacked histogram list should be draw
			void setDrawStackedHistogram(Bool_t shouldDraw);
	};
}
#endif
