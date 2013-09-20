/*
 * MessageRenderer.cc
 *
 *  Created on: 6 Sep 2013
 *      Author: Colin Nolan
 */
#define DNDEBUG

#include <Rtypes.h>
#include <TAttText.h>
#include <TColor.h>
#include <TObject.h>
#include <TText.h>
#include <vector>
#include <string>

#include "MessageRenderer.h"

namespace render {
	MessageRenderer::MessageRenderer() {
		// Nothing to do here.
	}

	// This code is based on the error message code in render.cc
	// XXX: There are a lot of magic numbers floating around in this method...
	void MessageRenderer::showErrorMessage(
			std::string histogramTitleText, std::string errorMessageText) {
		Color_t TEXT_COLOR = TColor::GetColor(178, 32, 32);		// TODO: Considering making this a static class constant

		TText *histogramTitle = new TText(.5, .58, histogramTitleText.c_str());
		this->storeRootObjectPointer(histogramTitle);
		TText *errorMessage = new TText(.5, .42, errorMessageText.c_str());
		this->storeRootObjectPointer(errorMessage);

		TText* textElements[] = {histogramTitle, errorMessage};
		Int_t numberOfTextElements = sizeof(textElements) / sizeof(TText*);

		for(Int_t i = 0; i < numberOfTextElements; i++) {
			TText *textElement = textElements[i];
			textElement->SetNDC(true);
			textElement->SetTextSize(0.10);
			textElement->SetTextAlign(22);
			textElement->SetTextColor(TEXT_COLOR);
			textElement->Draw();
			this->storeRootObjectPointer(textElement);
		}

		// Note: Due to the way ROOT works, the TObjects cannot be deleted
		// 		 until they have been converted to an image. Therefore, it
		//		 is the developer's responsibility to delete these objects
		//		 and ensure memory is not leaked.
	}
}
