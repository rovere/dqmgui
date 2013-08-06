/*
 * StackedHistogramSettingsParser.h
 *
 *  Created on: 6 Aug 2013
 *      Author: Colin - CERN
 */
#ifndef PARAMETERPARSER_H_
#define PARAMETERPARSER_H_

#include <Rtypes.h>
#include <map>
#include <vector>
#include <xstring>

namespace prototype { class StackedHistogramSettings; }

namespace prototype {
	/// TODO: Class comment
	class StackedHistogramSettingsParser {
		private:
			/// TODO: Comment
			static const Char_t SETTINGS_DELIMITER;
			/// TODO: Comment
			static const Char_t KEY_VALUE_DELIMITER;
			/// TODO: Comment
			static const std::string STACKED_HISTOGRAM_CONFIRMATION_KEY;

			/// Static class: the constructor should not called.
			StackedHistogramSettingsParser();

		public:
			/// TODO: Comment
			static StackedHistogramSettings parse(std::string parameterString);

		private:
			/// TODO: Comment
			static std::map<std::string, std::string> extractParameters(std::string parameterString);

			/// TODO: Comment
			/// XXX: This shouldn't probably be here: consider making a custom string utils class.
			static std::vector<std::string> split(const std::string &str, Char_t delimiter);
	};
}
#endif
