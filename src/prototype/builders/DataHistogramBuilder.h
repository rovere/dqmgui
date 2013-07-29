/*
 * HistogramBuilder.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef DATAHISTOGRAMBUILDER_H_
#define DATAHISTOGRAMBUILDER_H_

#include <TH1.h>

#include "HistogramBuilder.h"

namespace prototype {
	/// Builder for the (physical) data histogram.
	class DataHistogramBuilder : public HistogramBuilder<TH1D> {
		private:
			/// The histogram on which the one that is built shall be
			/// based on.
			TH1D *histogram;

		public:
			/// Default constructor.
			/// @param histogram the histogram to base the one that is built on
			/// XXX: Taking such parameter promotes a very strange pattern...
			DataHistogramBuilder(TH1D *histogram);

			/// @see HistogramBuilder<TH1D>.build()
			TH1D build();
		};
	}
#endif
