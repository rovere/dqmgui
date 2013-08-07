#include <cassert>
#include <new>
#include <sstream>

/*
 * StackedHistogramSettingsParser.cpp
 *
 *  Created on: 6 Aug 2013
 *      Author: Colin - CERN
 */
#define DNDEBUG

#include "StackedHistogramSettingsParser.h"

#include <Rtypes.h>
#include <map>
#include <sstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>
#include <xmemory0>
#include <xstddef>
#include <string>
#include <xtree>
#include <cassert>

#include "../../models/StackedHistogramSettings.h"


namespace render {
	const Char_t StackedHistogramSettingsParser::SETTINGS_DELIMITER = ';';
	const Char_t StackedHistogramSettingsParser::KEY_VALUE_DELIMITER = '=';
	const std::string StackedHistogramSettingsParser::STACKED_HISTOGRAM_CONFIRMATION_KEY = "stacked";

	StackedHistogramSettings StackedHistogramSettingsParser::parse(std::string parameterString) {
		StackedHistogramSettings *settings = new StackedHistogramSettings();
		std::map<std::string, std::string> parameters = StackedHistogramSettingsParser::extractParameters(parameterString);

		std::map<std::string, std::string>::iterator it = parameters.begin();
		while(it != parameters.end()) {
			std::pair<std::string, std::string> parameter = *it;
			std::string key = parameter.first;
			std::string value = parameter.second;

			if(key == STACKED_HISTOGRAM_CONFIRMATION_KEY) {
				settings->setDrawStackedHistogram((value == "1") ? true : false);
			}

			it++;
		}

		return(*settings);
	}

	std::map<std::string, std::string> StackedHistogramSettingsParser::extractParameters(std::string parameterString) {
		std::vector<std::string> parameters = StackedHistogramSettingsParser::split(parameterString, SETTINGS_DELIMITER);
		std::map<std::string, std::string> settingsMap;

		std::vector<std::string>::iterator it = parameters.begin();
		while(it != parameters.end()) {
			std::string parameter = *it;
			std::vector<std::string> keyAndValue = StackedHistogramSettingsParser::split(parameter, KEY_VALUE_DELIMITER);

			if(keyAndValue.size() == 2) {
				std::string key = keyAndValue.at(0);
				std::string value = keyAndValue.at(1);
				std::pair<std::string, std::string> keyValuePair(key, value);
				settingsMap.insert(keyValuePair);
			}
			else {
				// Parameter is considered corrupt as it does not have a value
			}

			it++;
		}

		return(settingsMap);
	}

	std::vector<std::string> StackedHistogramSettingsParser::split(const std::string &str, Char_t delimiter) {
		std::vector<std::string> substrings;
		if(str.size() == 0) {
			return(substrings);
		}
		std::stringstream stringStream(str);
		std::string substring;

		while(std::getline(stringStream, substring, delimiter)) {
			substrings.push_back(substring);
		}

		if(str.at(str.length() - 1) == delimiter) {
			// Edge-case
			substrings.push_back("");
		}

		assert(substrings.size() == (std::count(str.begin(), str.end(), delimiter) + 1));
		return(substrings);
	}
}
