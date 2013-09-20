/*
 *  Builder.h
 *
 *  Created on: 26 Jul 2013
 *      Author: Colin Nolan
 */
#ifndef BUILDER_H_
#define BUILDER_H_

namespace render {
	/// Abstract superclass for all builder classes.
	/// @param <T> the type of the object that is built
	template <class T> class Builder {
		public:
			/// Destructor.
			virtual ~Builder() {};

			/// Builds an object of type <code>T</code>, considering
			/// all settings that may have been set beforehand.
			virtual T build() = 0;
	};
}
#endif
