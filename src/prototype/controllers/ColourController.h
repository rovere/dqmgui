/*
 * ColourController.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */

#ifndef COLOURCONTROLLER_H_
#define COLOURCONTROLLER_H_

#include <Rtypes.h>

namespace prototype {
	/// TODO: Document class.
	class ColourController {
		public:
			/// The colour: black.
			static const Int_t COLOUR_BLACK;

		private:
			/// List of default colours.
			static const Int_t DEFAULT_COLOURS[];

			/// The index for <code>DEFAULT_COLOUR</code> of the colour to be used next.
			Int_t colourIndex;

		public:
			/// Default constructor.
			ColourController();

			/// Gets the next colour to be used as a histogram's background colour.
			/// @return integer representing the colour
			Int_t getNextColour();
		};
	}
#endif
