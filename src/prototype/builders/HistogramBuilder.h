/*
 * HistogramBuilder.h
 *
 *  Created on: 26 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMBUILDER_H_
#define HISTOGRAMBUILDER_H_

namespace prototype {
	template <class T> class HistogramBuilder {
		public:
			/// TODO: Comment.
			virtual T build() = 0;
	};
}
#endif
