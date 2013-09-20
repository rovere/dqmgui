/*
 * Renderer.h
 *
 *  Created on: 6 Sep 2013
 *      Author: Colin Nolan
 */
#ifndef RENDERER_H_
#define RENDERER_H_

#include <deque>

class TObject;

namespace render {
	/// Abstract superclass for classes that render ROOT plots.
	class Renderer {
		private:
			/// List of ROOT TObjects that can only be deleted once the rendered ROOT
			/// object is no longer required.
			std::deque<TObject*> rootObjectPointers;

		public:
			/// Gets the pointers stored for all ROOT objects created by the render.
			std::deque<TObject*> getRootObjectPointers() {
				return(this->rootObjectPointers);
			}

		protected:
			/// Default constructor (protected to make class abstract).
			Renderer() {};

			/// Stores a pointer to a ROOT <code>TObject</code> object.
			/// @param obj pointer to ROOT <code>TObject</code> to be stored
			void storeRootObjectPointer(TObject *obj) {
				this->rootObjectPointers.push_back(obj);
			}

			/// Deletes all of the stored ROOT objects.
			void deleteStoredRootObjects() {
				while(this->rootObjectPointers.size() != 0) {
					TObject *pointer = this->rootObjectPointers.front();
					delete pointer;
					this->rootObjectPointers.pop_front();
				}
			}
	};
}
#endif
