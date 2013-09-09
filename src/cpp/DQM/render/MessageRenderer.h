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
	/// TODO: Document.
	class MessageRenderer : private Renderer {
//	class MessageRenderer {
		public:
			/// Default constructor.
			MessageRenderer();

			/// Destructor.
			~MessageRenderer();

			/// TODO: Comment.
			/// @param histogramTitle TODO
			/// @param errorMessage TODO
			void showErrorMessage(std::string histogramTitleText, std::string errorMessageText);
	};
}
#endif
