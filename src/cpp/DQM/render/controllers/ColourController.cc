/*
 * ColourController.cc
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "ColourController.h"

#include <Rtypes.h>
#include <cassert>

namespace render {
	const Int_t ColourController::COLOUR_BLACK = kBlack;

	const Int_t ColourController::DEFAULT_COLOURS[] = {
			kRed, kGreen, kBlue, kYellow, kTeal, kGray, kOrange};

	ColourController::ColourController() {
		this->colourIndex = 0;
	}

	Int_t ColourController::getNextColour() {
		const Int_t numberOfColours = sizeof(DEFAULT_COLOURS) / sizeof(Int_t);

		assert(this->colourIndex < numberOfColours);
		assert(this->colourIndex >= 0);

		Int_t colour = this->DEFAULT_COLOURS[this->colourIndex];

		this->colourIndex++;
		if(this->colourIndex == numberOfColours) {
			this->colourIndex = 0;
		}

		return(colour);
	}
}
