// copyright 2008, 2009 t. schneider tes@mit.edu
// 
// this file is part of the Dynamic Compact Control Language (DCCL),
// the goby-acomms codec. goby-acomms is a collection of libraries 
// for acoustic underwater networking
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

#ifndef MESSAGE_VAR20091211H
#define MESSAGE_VAR20091211H

#include <bitset>
#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <ostream>
#include <cmath>
#include <limits>

#include <boost/dynamic_bitset.hpp>
#include <boost/lexical_cast.hpp>

#include "dccl_constants.h"
#include "message_val.h"

namespace dccl
{
    class MessageVal;
    class AlgorithmPerformer;
    
// defines a piece of a Message (an <int>, a <bool>, etc)
    class MessageVar
    {
      public:
        MessageVar();
        
        // set
        void set_name(std::string name) {name_ = name;}
        void set_source_var(std::string source_var) {source_var_ = source_var; source_set_=true;}
        void set_source_key(std::string source_key) {source_key_ = source_key;}
        void set_source_set(bool source_set) {source_set_ = source_set;}
        void set_algorithms(const std::vector<std::string>& algorithm) {algorithms_ = algorithm;}

        // should be overloaded by derived types if supported
        virtual void set_max(const std::string& s)
        { bad_overload("set_max()"); }
        virtual void set_min(const std::string& s)
        { bad_overload("set_min()"); }
        virtual void set_precision(const std::string& s)
        { bad_overload("set_precision()"); }
        virtual void set_max_length(const std::string& s)
        { bad_overload("set_max_length()"); }
        virtual void set_num_bytes(const std::string& s)
        { bad_overload("set_num_bytes()"); }
        virtual void set_static_val(const std::string& static_val)
        { bad_overload("set_static_val()"); }
        virtual void add_enum(std::string senum)
        { bad_overload("add_enum()"); }
        virtual void set_expected_delta(const std::string& s)
        { bad_overload("set_expected_delta()"); }
        
        virtual void set_delta_var(bool b) { }

        virtual double max() const
        { bad_overload("max()"); return 0; }
        virtual double min() const
        { bad_overload("min()"); return 0; }
        virtual int precision() const
        { bad_overload("precision()"); return 0;}
        virtual unsigned max_length() const
        { bad_overload("max_length()"); return 0; }
        virtual unsigned num_bytes() const
        { bad_overload("num_bytes()"); return 0; }
        virtual std::string static_val() const
        { bad_overload("static_val()"); return ""; }
        virtual std::vector<std::string>* enums()
        { bad_overload("enums()"); return 0; }  

        // get
        virtual DCCLType type() const = 0;        

        std::string name() const {return name_;}
        std::string source_var() const {return source_var_;}

        // other
        void initialize(const std::string& trigger_var);
        std::string get_display() const;
        void read_pubsub_vars(std::map<std::string,MessageVal>& vals,
                              const std::map<std::string, MessageVal>& in);
        
        std::string parse_string_val(const std::string& sval);
        
        void var_encode(std::map<std::string,MessageVal>& vals,
                        boost::dynamic_bitset<unsigned char>& bits);
        void var_decode(std::map<std::string,MessageVal>& vals,
                        boost::dynamic_bitset<unsigned char>& bits);

        virtual int calc_size() const = 0;
        
      protected:
        virtual void initialize_specific() = 0;
        virtual boost::dynamic_bitset<unsigned char> encode_specific(const MessageVal& v) = 0;
        virtual MessageVal decode_specific(boost::dynamic_bitset<unsigned char>& bits) = 0;
        virtual void get_display_specific(std::stringstream& ss) const = 0;


        
      private:
        void bad_overload(const std::string& s) const
        {
            throw(std::runtime_error(std::string(s + " not supported by this MessageVar: " + name() + " (" + type_to_string(type()) + ")")));
        }        
        
      private:
        bool source_set_;
        std::string name_;
        AlgorithmPerformer * ap_;
        std::string source_var_;
        std::string source_key_;
        std::vector<std::string> algorithms_;

    };


    std::ostream& operator<< (std::ostream& out, const MessageVar& m);
}

#endif