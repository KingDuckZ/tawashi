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

#include "ini_file.hpp"
#include <utility>
#include <boost/spirit/include/qi_core.hpp>
#include <boost/spirit/include/qi_sequence.hpp>
#include <boost/spirit/include/qi_plus.hpp>
#include <boost/spirit/include/qi_not_predicate.hpp>
#include <boost/spirit/include/qi_raw.hpp>
#include <boost/spirit/include/qi_lit.hpp>
#include <boost/spirit/include/qi_kleene.hpp>
#include <boost/spirit/include/qi_rule.hpp>
#include <boost/spirit/include/qi_eol.hpp>
#include <boost/spirit/include/qi_grammar.hpp>
#include <boost/spirit/include/qi_hold.hpp>
#include <boost/spirit/include/qi_char_class.hpp>
#include <boost/spirit/include/qi_list.hpp>
#include <boost/spirit/include/qi_optional.hpp>
#include <boost/spirit/include/qi_eps.hpp>
#include <boost/spirit/include/phoenix_stl.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/phoenix/object/construct.hpp>
#include <boost/phoenix/bind/bind_member_function.hpp>
#include <boost/phoenix/bind/bind_member_variable.hpp>
#include <boost/fusion/adapted/std_pair.hpp>
#include <type_traits>
#include <iterator>

namespace kamokan {
	namespace {
		typedef boost::string_view string_type;

		template <typename Iterator, typename Skipper>
		struct IniGrammar : boost::spirit::qi::grammar<Iterator, IniFile::IniMapType(), Skipper> {
			explicit IniGrammar (const std::string* parString);
			boost::spirit::qi::rule<Iterator, IniFile::IniMapType(), Skipper> start;
			boost::spirit::qi::rule<Iterator, string_type(), Skipper> section;
			boost::spirit::qi::rule<Iterator, string_type(), Skipper> key;
			boost::spirit::qi::rule<Iterator, IniFile::KeyValueMapType::value_type(), Skipper> key_value;
			boost::spirit::qi::rule<Iterator, IniFile::KeyValueMapType(), Skipper> key_values;
			const std::string* m_master_string;
			Iterator m_begin;
		};

		template <typename Iterator>
		struct IniCommentSkipper : boost::spirit::qi::grammar<Iterator> {
			IniCommentSkipper() :
				IniCommentSkipper::base_type(skipping),
				first_char(true)
			{
				namespace px = boost::phoenix;
				using boost::spirit::qi::blank;
				using boost::spirit::qi::lit;
				using boost::spirit::qi::eol;
				using boost::spirit::qi::char_;
				using boost::spirit::qi::eps;

				skipping = comment | blank;
				comment = (eps(px::cref(first_char) == true) | eol) >>
					*blank >> lit("#")[px::ref(first_char) = false] >>
					*(!eol >> char_);
			}

			boost::spirit::qi::rule<Iterator> skipping;
			boost::spirit::qi::rule<Iterator> comment;
			bool first_char;
		};

		template <typename Iterator, typename Skipper>
		IniGrammar<Iterator, Skipper>::IniGrammar (const std::string* parString) :
			IniGrammar::base_type(start),
			m_master_string(parString),
			m_begin(m_master_string->cbegin())
		{
			assert(m_master_string);
			namespace px = boost::phoenix;
			using boost::spirit::qi::_val;
			using boost::spirit::_1;
			using boost::spirit::qi::eol;
			using boost::spirit::qi::raw;
			using boost::string_view;
			using boost::spirit::qi::hold;
			using boost::spirit::qi::graph;
			using boost::spirit::qi::blank;
			typedef IniFile::KeyValueMapType::value_type refpair;

			section = '[' >> raw[+(graph - ']') >> *(hold[+blank >> +(graph - ']')])]
				[_val = px::bind(
					&string_view::substr,
					px::construct<string_view>(px::ref(*m_master_string)),
					px::begin(_1) - px::cref(m_begin), px::size(_1)
				)] >> ']';
			key = raw[(graph - '[' - '=') >> *(graph - '=') >> *(hold[+blank >> +(graph - '=')])][_val = px::bind(
				&string_view::substr,
				px::construct<string_view>(px::ref(*m_master_string)),
				px::begin(_1) - px::cref(m_begin), px::size(_1)
			)];
			key_value = key[px::bind(&refpair::first, _val) = _1] >> '=' >>
				raw[*(!eol >> graph) % +blank][px::bind(&refpair::second, _val) = px::bind(
					&string_view::substr,
					px::construct<string_view>(px::ref(*m_master_string)),
					px::begin(_1) - px::cref(m_begin), px::size(_1)
			)];
			key_values = -(key_value % +eol);
			start = *eol >> *(section >> +eol >> key_values >> *eol);
		}

		IniFile::IniMapType parse_ini (const std::string* parIni, bool& parParseOk, int& parParsedCharCount) {
			using boost::spirit::qi::blank_type;
			using skipper_type = IniCommentSkipper<std::string::const_iterator>;

			IniGrammar<std::string::const_iterator, skipper_type> gramm(parIni);
			IniFile::IniMapType result;

			parParseOk = false;
			parParsedCharCount = 0;

			std::string::const_iterator start_it = parIni->cbegin();
			//TODO: make a skipper that also skips comments eg: blank | lit("//") >> *(char_ - eol)
			const bool parse_ok = boost::spirit::qi::phrase_parse(
				start_it,
				parIni->cend(),
				gramm,
				skipper_type(),
				result
			);

			parParseOk = parse_ok and (parIni->cend() == start_it);
			parParsedCharCount = std::distance(parIni->cbegin(), start_it);
			assert(parParsedCharCount >= 0);
			return result;
		}
	} //unnamed namespace

	IniFile::IniFile (std::istream_iterator<char> parInputFrom, std::istream_iterator<char> parInputEnd) :
		IniFile(std::string(parInputFrom, parInputEnd))
	{
	}

	IniFile::IniFile (std::string&& parIniData) :
		m_raw_ini(std::move(parIniData)),
		m_map(parse_ini(&m_raw_ini, m_parse_ok, m_parsed_chars))
	{
	}

	IniFile::IniFile (IniFile&& parOther) {
		auto* const old_data_ptr = parOther.m_raw_ini.data();
		m_raw_ini = std::move(parOther.m_raw_ini);
		if (m_raw_ini.data() == old_data_ptr)
			m_map = std::move(parOther.m_map);
		else
			m_map = parse_ini(&m_raw_ini, m_parse_ok, m_parsed_chars);
	}

	IniFile::~IniFile() noexcept = default;
} //namespace kamokan
