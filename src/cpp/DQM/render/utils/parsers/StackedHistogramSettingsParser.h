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
#include <string>

namespace render { class StackedHistogramSettings; }

namespace render {
	/// Parser for stacked histogram settings.
	class StackedHistogramSettingsParser {
		private:
			/// The character that delimits settings.
			static const Char_t SETTINGS_DELIMITER;
			/// The character that delimits the key from the value in a setting.
			static const Char_t KEY_VALUE_DELIMITER;
			/// The name of the key that confirms that a stacked histogram should be drawn.
			static const std::string STACKED_HISTOGRAM_CONFIRMATION_KEY;

			/// Static class: the constructor should not called.
			StackedHistogramSettingsParser();

		public:
			/// Parses the given parameter string.
			/// @param parameterString the parameter string to parse
			/// @return settings for a stacked histogram to be drawn
			static StackedHistogramSettings parse(std::string parameterString);

		private:
			/// Extracts parameters from a string and returns in a map of key/value pairs.
			/// @param parameterString the parameter string to parse
			/// @return map containing parameter names as a key and the parameter value as the value
			static std::map<std::string, std::string> extractParameters(std::string parameterString);

			/// Splits a string at a given <code>delimiter</code>.
			/// @param str the string to be split
			/// @param delimiter the delimiter to split the string at
			/// @return the substrings of the string split into parts at the <code>delimiter</code>
			/// XXX: This shouldn't probably be here: consider making a custom string utils class.
			static std::vector<std::string> split(const std::string &str, Char_t delimiter);
	};
}
#endif
