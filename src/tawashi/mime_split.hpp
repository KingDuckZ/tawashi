/* Copyright 2017, Michele Santullo
 * This file is part of "tawashi".
 *
 * "tawashi" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "tawashi" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "tawashi".  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <boost/utility/string_view.hpp>
#include <boost/container/flat_map.hpp>
#include <string>

namespace tawashi {
	typedef boost::container::flat_map<boost::string_view, boost::string_view> MimeParametersMapType;

	struct SplitMime {
		boost::string_view type;
		boost::string_view subtype;
		MimeParametersMapType parameters;
	};

	SplitMime string_to_mime (const std::string* parMime, bool& parParseOk, int& parParsedCharCount);
	std::string mime_to_string (const SplitMime& parMime, bool& parWriteOk);
} //namespace tawashi
