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

#include "settings_bag.hpp"
#include "duckhandy/lexical_cast.hpp"
#include <ciso646>
#include <cassert>
#include <cstdint>
#include <sstream>
#include <spdlog/spdlog.h>

namespace tawashi {
	namespace {
		const IniFile::KeyValueMapType* get_tawashi_node (const IniFile& parIni) {
			auto it_found = parIni.parsed().find("tawashi");
			if (parIni.parsed().end() != it_found) {
				return &it_found->second;
			}
			else {
				spdlog::get("statuslog")->warn("Couldn't find section [tawashi] in the settings file");
				static const IniFile::KeyValueMapType empty_key_values;
				return &empty_key_values;
			}
		}
	} //unnamed namespace

	SettingsBag::SettingsBag (const Kakoune::SafePtr<IniFile>& parIni) :
		m_ini(parIni),
		m_values(get_tawashi_node(*parIni))
	{
		assert(m_values);
	}

	SettingsBag::~SettingsBag() noexcept = default;

	const boost::string_ref& SettingsBag::operator[] (boost::string_ref parIndex) const {
		auto it_found = m_values->find(parIndex);
		if (m_values->end() != it_found)
			return it_found->second;
		else
			return m_defaults.at(parIndex);
	}

	void SettingsBag::add_default (boost::string_ref parKey, boost::string_ref parValue) {
		assert(m_defaults.find(parKey) == m_defaults.end());
		m_defaults[parKey] = parValue;
	}

	template <>
	std::string SettingsBag::as (boost::string_ref parIndex) const {
		auto& setting = this->at(parIndex);
		return std::string(setting.data(), setting.size());
	}

	template <>
	bool SettingsBag::as (boost::string_ref parIndex) const {
		auto& setting = this->at(parIndex);
		if (setting == "true" or setting == "yes" or setting == "1" or setting == "on") {
			return true;
		}
		else if (setting == "false" or setting == "no" or setting == "0" or setting == "off") {
			return false;
		}
		else {
			std::ostringstream oss;
			oss << "Bad conversion: can't convert \"" << setting << "\" to bool";
			throw std::runtime_error(oss.str());
		}
	}

	template <>
	uint16_t SettingsBag::as (boost::string_ref parIndex) const {
		return dhandy::lexical_cast<uint16_t>(this->at(parIndex));
	}

	template <>
	uint32_t SettingsBag::as (boost::string_ref parIndex) const {
		return dhandy::lexical_cast<uint32_t>(this->at(parIndex));
	}

	template std::string SettingsBag::as<std::string> (boost::string_ref parIndex) const;
	template bool SettingsBag::as<bool> (boost::string_ref parIndex) const;
	template uint16_t SettingsBag::as<uint16_t> (boost::string_ref parIndex) const;
	template uint32_t SettingsBag::as<uint32_t> (boost::string_ref parIndex) const;
} //namespace tawashi
