/*
 * MessageRenderer.h
 *
 *  Created on: 6 Sep 2013
 *      Author: Colin - CERN
 */
#ifndef MESSAGERENDERER_H_
#define MESSAGERENDERER_H_

#include <string>

#include "Renderer.h"

namespace render {
	/// Class that renders messages onto ROOT plots.
	class MessageRenderer : public Renderer {
		public:
			/// Default constructor.
			MessageRenderer();

			/// Shows error message on ROOT plot.
			/// @param histogramTitle the title of the error message
			/// @param errorMessage the text of the error message
			void showErrorMessage(std::string histogramTitleText, std::string errorMessageText);
	};
}
#endif
