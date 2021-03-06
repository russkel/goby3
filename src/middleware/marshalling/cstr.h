// Copyright 2009-2018 Toby Schneider (http://gobysoft.org/index.wt/people/toby)
//                     GobySoft, LLC (2013-)
//                     Massachusetts Institute of Technology (2007-2014)
//                     Community contributors (see AUTHORS file)
//
//
// This file is part of the Goby Underwater Autonomy Project Libraries
// ("The Goby Libraries").
//
// The Goby Libraries are free software: you can redistribute them and/or modify
// them under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// The Goby Libraries are distributed in the hope that they will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with Goby.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SerializeParseString20190717H
#define SerializeParseString20190717H

#include <vector>

#include "interface.h"

namespace goby
{
namespace middleware
{
template <typename DataType> struct SerializerParserHelper<DataType, MarshallingScheme::CSTR>
{
    static std::vector<char> serialize(const DataType& msg)
    {
        std::vector<char> bytes(std::begin(msg), std::end(msg));
        bytes.push_back('\0');
        return bytes;
    }

    static std::string type_name() { return "CSTR"; }

    static std::string type_name(const DataType& d) { return type_name(); }

    template <typename CharIterator>
    static std::shared_ptr<DataType> parse(CharIterator bytes_begin, CharIterator bytes_end,
                                           CharIterator& actual_end)
    {
        actual_end = bytes_end;
        if (bytes_begin != bytes_end)
        {
            return std::make_shared<DataType>(bytes_begin, bytes_end - 1);
        }
        else
        {
            return std::make_shared<DataType>();
        }
    }
};

template <typename T, typename std::enable_if<std::is_same<T, std::string>::value>::type* = nullptr>
constexpr int scheme()
{
    return goby::middleware::MarshallingScheme::CSTR;
}
} // namespace middleware
} // namespace goby

#endif
