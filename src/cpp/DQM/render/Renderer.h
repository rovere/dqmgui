/*
 * Renderer.h
 *
 *  Created on: 6 Sep 2013
 *      Author: Colin - CERN
 */
#ifndef RENDERER_H_
#define RENDERER_H_

#include <vector>

class TObject;

namespace render {
	/// TODO: Document.
	class Renderer {
		private:
			/// List of ROOT TObjects that can only be deleted once the rendered ROOT
			/// object is no longer required.
			std::vector<TObject*> rootObjectPointers;

		public:
			/// Destructor.
			virtual ~Renderer() {
				while(!this->rootObjectPointers.empty()) {
					delete this->rootObjectPointers.back();
					this->rootObjectPointers.pop_back();
				}
			}

		protected:
			/// Default constructor (protected to make class abstract).
			Renderer() {};

			/// TODO: Comment.
			/// @param pointer TODO
			void storeRootObjectPointer(TObject *pointer) {
				this->rootObjectPointers.push_back(pointer);
			}
	};
}
#endif
