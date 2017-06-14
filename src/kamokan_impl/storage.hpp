/* Copyright 2017, Michele Santullo
 * This file is part of "kamokan".
 *
 * "kamokan" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "kamokan" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "kamokan".  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "kakoune/safe_ptr.hh"
#include "error_reasons.hpp"
#include <memory>
#include <cstdint>
#include <boost/utility/string_view.hpp>
#include <boost/optional.hpp>
#include <string>

namespace redis {
	class IncRedis;
} //namespace redis

namespace kamokan {
	class SettingsBag;

	class Storage {
	public:
		struct SubmissionResult {
			std::string token;
			boost::optional<tawashi::ErrorReasons> error;
		};

		explicit Storage (const Kakoune::SafePtr<SettingsBag>& parSettings);
		kamokan_virtual_testing ~Storage();

		kamokan_virtual_testing void connect_async();
		kamokan_virtual_testing bool is_connected() const;
		kamokan_virtual_testing void finalize_connection();
		kamokan_virtual_testing SubmissionResult submit_pastie (
			const boost::string_view& parText,
			uint32_t parExpiry,
			const boost::string_view& parLang,
			bool parSelfDestruct,
			const std::string& parRemoteIP
		) const;

		kamokan_virtual_testing boost::optional<std::string> retrieve_pastie (const boost::string_view& parToken) const;

#if defined(KAMOKAN_WITH_TESTING)
		const SettingsBag& settings() const;
#endif

	private:
		std::unique_ptr<redis::IncRedis> m_redis;
		Kakoune::SafePtr<SettingsBag> m_settings;
	};
} //namespace kamokan
