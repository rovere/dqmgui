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
	class StackedHistogramSettings {
		private:
			Bool_t drawStackedHistogram;

		public:
			/// Default constructor.
			StackedHistogramSettings();

			/// Gets whether a stacked histogram should be drawn.
			/// @return whether a stacked histogram should be drawn
			Bool_t shouldDrawStackedHistogram();

			/// TODO: Comment
			void setDrawStackedHistogram(Bool_t shouldDraw);
	};
}
#endif
