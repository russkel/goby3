// copyright 2009 t. schneider tes@mit.edu
// ocean engineering graudate student - mit / whoi joint program
// massachusetts institute of technology (mit)
// laboratory for autonomous marine sensing systems (lamss)    
// 
// this file is part of goby-util, a collection of utility libraries
//
// this is an almagamation of various utilities
// i have written for miscellaneous tasks
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this software.  If not, see <http://www.gnu.org/licenses/>.

#ifndef TES_UTILS20091211H
#define TES_UTILS20091211H

#include <cmath>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>
#include <iomanip>
#include <iostream>

#include <boost/dynamic_bitset.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>


namespace tes_util
{
    //
    // SCIENCE
    //
    
    // round 'r' to 'dec' number of decimal places
// we want no upward bias so
// round 5 up if odd next to it, down if even
    inline double sci_round(double r, double dec)
    {
        double ex = pow(10, dec);
        double final = floor(r * ex);
        double s = (r * ex) - final;

        // remainder less than 0.5 or even number next to it
        if (s < 0.5 || (s==0.5 && !(static_cast<unsigned long>(final)&1)))
            return final / ex;
        else 
            return (final+1) / ex;
    }

// K.V. Mackenzie, Nine-term equation for the sound speed in the oceans (1981) J. Acoust. Soc. Am. 70(3), pp 807-812
// http://scitation.aip.org/getabs/servlet/GetabsServlet?prog=normal&id=JASMAN000070000003000807000001&idtype=cvips&gifs=yes
    inline double mackenzie_soundspeed(double T, double S, double D)
    {
        return
            1448.96 + 4.591*T - 5.304e-2*T*T + 2.374e-4*T*T*T +
            1.340*(S-35) + 1.630e-2*D+1.675e-7*D*D -
            1.025e-2*T*(S-35)-7.139e-13*T*D*D*D;
    }

    
    //
    // BINARY ENCODING
    //

    
    // converts a char (byte) array into a hex string where
// c = pointer to array of char
// s = reference to string to put char into as hex
// n = length of c
// first two hex chars in s are the 0 index in c
    inline bool char_array2hex_string(const unsigned char * c, std::string & s, const unsigned int n)
    {
        std::stringstream ss;
        for (unsigned int i=0; i<n; i++)
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned int>(c[i]);
        
        s = ss.str();
        return true;
    }

// turns a string of hex chars ABCDEF into a character array reading
// each byte 0xAB,0xCD, 0xEF, etc.
    inline bool hex_string2char_array(unsigned char * c, const std::string s, const unsigned int n)
    {
        for (unsigned int i = 0; i<n; i++)
        {
            std::stringstream ss;
            unsigned int in;
            ss <<  s.substr(2*i, 2);
            ss >> std::hex >> in;
            c[i] = static_cast<unsigned char>(in); 
        }
        return true;
    }

// return a string represented the binary value of 'l' for 'bits' number of bits
// reads MSB -> LSB
    inline std::string long2binary_string(unsigned long l, unsigned short bits)
    {
        char s [bits+1];
        for (unsigned int i=0; i<bits; i++)
        {
            s[bits-i-1] = (l&1) ? '1' : '0';
            l >>= 1;
        }
        s[bits] = '\0';    
        return (std::string)s;
    }
    
    inline std::string binary_string2hex_string(const std::string & bs)
    {
        std::string hs;
        unsigned int bytes = (unsigned int)(ceil(bs.length()/8.0));
        unsigned char c[bytes];
    
        for(size_t i = 0; i < bytes; ++i)
        {
            std::bitset<8> b(bs.substr(i*8,8));    
            c[i] = (char)b.to_ulong();
        }

        char_array2hex_string(c, hs, bytes);

        return hs;
    }

    inline std::string dyn_bitset2hex_string(const boost::dynamic_bitset<unsigned char>& bits, unsigned trim_to_bytes_size = 0)
    {
        std::stringstream binary;
        binary << bits;
        std::string out = tes_util::binary_string2hex_string(binary.str());

        if(trim_to_bytes_size)
            return out.substr(out.length() - trim_to_bytes_size*2);
        else
            return out;
    }
    
    // only works on whole byte string
    inline std::string hex_string2binary_string(const std::string & bs)
    {
        int bytes = bs.length()/2;
        unsigned char c[bytes];
    
        hex_string2char_array(c, bs, bytes);

        std::string hs;
    
        for (size_t i = 0; i < (size_t)bytes; i++)
            hs += long2binary_string((unsigned long)c[i], 8);

        return hs;
    }


    inline boost::dynamic_bitset<unsigned char> hex_string2dyn_bitset(const std::string & hs, unsigned bits_size = 0)
    {
        boost::dynamic_bitset<unsigned char> bits;
        std::stringstream bin_str;
        bin_str << hex_string2binary_string(hs);       
        bin_str >> bits;

        if(bits_size) bits.resize(bits_size);        
        return bits;
    }

    
    template <typename T> bool hex_string2number(const std::string & s, T & t)
    {
        std::stringstream ss;
        ss << s;
        ss >> std::hex >> t;
        return !ss.fail();        
    }


    
    template <typename T> bool number2hex_string(std::string & s, const T & t, unsigned int width = 2)
    {
        std::stringstream ss;
        ss << std::hex << std::setw(width) << std::setfill('0') << static_cast<unsigned int>(t);
        s  = ss.str();
        return !ss.fail();        
    }

    template <typename T> std::string number2hex_string(const T & t, unsigned int width = 2)
    {
        std::string s;
        number2hex_string(s,t,width);
        return s;
    }


    //
    // STRING PARSING
    //

    
    inline void stripblanks(std::string& s)
    {
        for(std::string::iterator it=s.end()-1; it!=s.begin()-1; --it)
        {    
            if(isspace(*it))
                s.erase(it);
        }
    }

    
// "explodes" a string on a delimiter into a vector of strings
    template<typename T>
        inline void explode(std::string s, std::vector<T>& rs, char d, bool do_stripblanks)
    {
        std::string::size_type pos;
    
        pos = s.find(d);
        while(pos != std::string::npos)
        {
            std::string p = s.substr(0, pos);
            if(do_stripblanks)
                stripblanks(p);
            
            
            rs.push_back(p);
            
            if (pos+1 < s.length())
                s = s.substr(pos+1);
            else
                return;
            pos = s.find(d);
        }
        
        // last piece
        if(do_stripblanks)
            stripblanks(s);
        
        rs.push_back(s);
    }

    inline std::vector<std::string> explode(const std::string& s, char d, bool do_stripblanks)
    {
        std::vector<std::string> out;
        explode(s, out, d, do_stripblanks);
        return out;
    }

    
    inline bool charicmp(char a, char b) { return(tolower(a) == tolower(b)); }
    inline bool stricmp(const std::string & s1, const std::string & s2)
    {
        return((s1.size() == s2.size()) &&
               equal(s1.begin(), s1.end(), s2.begin(), charicmp));
    }


    // find `key` in `str` and if successful put it in out
    // and return true

    // deal with these basic forms:
    // str = foo=1,bar=2,pig=3
    // str = foo=1,bar={2,3,4,5},pig=3
    inline bool val_from_string(std::string & out, const std::string & str, const std::string & key)
    {
        // str:  foo=1,bar={2,3,4,5},pig=3,cow=yes
        // two cases:
        // key:  bar
        // key:  pig
        
        out.erase();

        // str:  foo=1,bar={2,3,4,5},pig=3,cow=yes
        //  start_pos  ^             ^
        std::string::size_type start_pos = str.find(std::string(key+"="));
        
        // deal with foo=bar,o=bar problem when looking for "o=" since
        // o is contained in foo

        // ok: beginning of string, end of string, comma right before start_pos
        while(!(start_pos == 0 || start_pos == std::string::npos || str[start_pos-1] == ','))
            start_pos = str.find(std::string(key+"="), start_pos+1);
        
        if(start_pos != std::string::npos)
        {
            // chopped:   bar={2,3,4,5},pig=3,cow=yes
            // chopped:   pig=3,cow=yes
            std::string chopped = str.substr(start_pos);

            // chopped:  bar={2,3,4,5},pig=3,cow=yes
            // equal_pos    ^         
            // chopped:  pig=3,cow=yes
            // equal_pos    ^         
            std::string::size_type equal_pos = chopped.find("=");

            // check for array
            bool is_array = (equal_pos+1 < chopped.length() && chopped[equal_pos+1] == '{');
            
            if(equal_pos != std::string::npos)
            {
                // chopped_twice:  ={2,3,4,5},pig=3,cow=yes
                // chopped_twice:  =pig=3,cow=yes              
                std::string chopped_twice = chopped.substr(equal_pos);

                // chopped_twice:  ={2,3,4,5},pig=3,cow=yes  
                // end_pos                  ^     
                // chopped_twice:  =pig=3,cow=yes
                // end_pos               ^         
                std::string::size_type end_pos =
                    (is_array) ? chopped_twice.find("}") : chopped_twice.find(",");

                // out:  2,3,4,5
                // out:  3
                out = (is_array) ? chopped_twice.substr(2, end_pos-2) : chopped_twice.substr(1, end_pos-1);
                return true;
            }
        }

        return false;        
    }
    
    inline bool string2bool(const std::string & in)
    { return (stricmp(in, "true") || stricmp(in, "1")) ? true : false; }
    
    //
    // TIME
    //


    inline double ptime2unix_double(boost::posix_time::ptime given_time)
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;
        
        if (given_time == not_a_date_time)
            return -1;
    
        ptime time_t_epoch(date(1970,1,1));
        
        time_duration diff = given_time - time_t_epoch;
        
        return (double(diff.total_seconds()) + double(diff.fractional_seconds()) / double(time_duration::ticks_per_second()));
    }
    
    inline double time_duration2unix_double(boost::posix_time::time_duration time_of_day)
    {
        return (double(time_of_day.total_seconds()) + double(time_of_day.fractional_seconds()) / double(boost::posix_time::time_duration::ticks_per_second()));
    }
    
    inline double date2unix_double(boost::gregorian::date date)
    {
        return ptime2unix_double(boost::posix_time::ptime(date, boost::posix_time::seconds(0)));
    }
            
    

    // good to the microsecond
    inline boost::posix_time::ptime unix_double2ptime(double given_time)
    {
        using namespace boost::posix_time;
        using namespace boost::gregorian;
    
        if (given_time == -1)
            return boost::posix_time::ptime(not_a_date_time);
    
        ptime time_t_epoch(date(1970,1,1));

        double s = floor(given_time);
        double micro_s = (given_time - s)*1e6;
        return time_t_epoch + seconds(s) + microseconds(micro_s);
    }
    
    inline boost::posix_time::ptime ptime_now()
    { return boost::posix_time::ptime(boost::posix_time::microsec_clock::universal_time()); }
    
}

#endif
