/*
 * HistogramBuilder.h
 *
 *  Created on: 23 Jul 2013
 *      Author: Colin - CERN
 */
#ifndef HISTOGRAMBUILDER_H_
#define HISTOGRAMBUILDER_H_

class TH1D;

namespace prototype {
	/// TODO: Document class.
	/// XXX: This class promotes a very strange pattern...
	class DataHistogramBuilder {
		private:
			/// TODO: Comment.
			TH1D *histogram;

		public:
			/// Default constructor.
			/// @param histogram TODO
			DataHistogramBuilder(TH1D *histogram);

			/// TODO: Comment.
			TH1D build();
		};
	}
#endif
