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

#include <boost/foreach.hpp>

#include "util/tes_utils.h"

#include "message_var.h"
#include "message_val.h"
#include "dccl_constants.h"
#include "message_algorithms.h"

dccl::MessageVar::MessageVar()
    : array_length_(1),
      is_key_frame_(true),
      source_set_(false),
      ap_(AlgorithmPerformer::getInstance())
{ }

void dccl::MessageVar::initialize(const std::string& trigger_var)
{
    // add trigger_var_ as source_var for any message_vars without a source
    if(!source_set_)
        source_var_ = trigger_var;

    initialize_specific();

}

void dccl::MessageVar::set_defaults(std::map<std::string,std::vector<MessageVal> >& vals, unsigned modem_id, unsigned id)
{
    vals[name_].resize(array_length_);    

    std::vector<MessageVal>& vm = vals[name_];

    for(std::vector<MessageVal>::size_type i = 0, n = vm.size(); i < n; ++i)
        set_defaults_specific(vm[i], modem_id, id);

}

    
void dccl::MessageVar::var_encode(std::map<std::string,std::vector<MessageVal> >& vals, boost::dynamic_bitset<unsigned char>& bits)
{    
    // ensure that every MessageVar has the full number of (maybe blank) MessageVals
    vals[name_].resize(array_length_);

    // copy so algorithms can modify directly and not affect other algorithms' use of original values
    std::vector<MessageVal> vm = vals[name_];
    
    // write all the delta values first
    is_key_frame_ = false;
    
    for(std::vector<MessageVal>::size_type i = 0, n = vm.size(); i < n; ++i)
    {
        for(std::vector<std::string>::size_type j = 0, m = algorithms_.size(); j < m; ++j)
            ap_->algorithm(vm[i], i, algorithms_[j], vals);

        // read the first value as the key
        if(i == 0) key_val_ = vm[i];
        // otherwise add the bits to the stream
        else encode_value(vm[i], bits);
    }

    is_key_frame_ = true;
    
    // insert the key at the end of the bitstream
    encode_value(key_val_, bits);
}

void dccl::MessageVar::encode_value(const MessageVal& val, boost::dynamic_bitset<unsigned char>& bits)
{
    bits <<= calc_size();
    
    boost::dynamic_bitset<unsigned char> add_bits = encode_specific(val);
    add_bits.resize(bits.size());
    
    bits |= add_bits;
}


void dccl::MessageVar::var_decode(std::map<std::string,std::vector<MessageVal> >& vals, boost::dynamic_bitset<unsigned char>& bits)
{
    vals[name_].resize(array_length_);
    
    // count down from one-past-the-end to 1, because we'll put the key at the beginning (array position 0)
    for(unsigned i = array_length_, n = 0; i > n; --i)
    {
        is_key_frame_ = (i == array_length_) ? true : false;
        
        boost::dynamic_bitset<unsigned char> remove_bits = bits;
        remove_bits.resize(calc_size());

        MessageVal val = decode_specific(remove_bits);
        
        bits >>= calc_size();

        // read the key first on the reverse bitstream
        if(is_key_frame_) key_val_ = val;
        else vals[name_][i] = val;
    }

    // insert the key at the beginning of the return vector
    vals[name_][0] = key_val_;    
}


void dccl::MessageVar::read_pubsub_vars(std::map<std::string,std::vector<MessageVal> >& vals,
                                        const std::map<std::string,std::vector<MessageVal> >& in)
{
    const std::map<std::string, std::vector<dccl::MessageVal> >::const_iterator it =
        in.find(source_var_);
    
    if(it != in.end())
    {
        const std::vector<MessageVal>& vm = it->second;

        BOOST_FOREACH(MessageVal val, vm)
        {
            switch(val.type())
            {
                case cpp_string:
                    val = parse_string_val(val);
                    break;
                    
                default:
                    break;
            }

            // if we're expecting a vector,
            // split up vector quantities and add to vector
            if(array_length_ > 1)
                tes_util::explode(val, vals[name_], ',', false);
            else // otherwise just use the value as is
                vals[name_] = val;
        }        
    }
}


// deal with cases where key=value exists within the string
std::string dccl::MessageVar::parse_string_val(const std::string& sval)
{
    std::string pieceval;

    // is the parameter part of the std::string (as opposed to being the std::string)
    // that is, in_str is true if "key=value" is part of the string, rather
    // than the std::string simply being "value"
    bool in_str = false;
        
    // see if the parameter is *in* the string, if so put it in pieceval
    // use source_key if specified, otherwise try the name
    std::string subkey = (source_key_ == "") ? name_ : source_key_;
        
    in_str = tes_util::val_from_string(pieceval, sval, subkey);        
    //pick the substring from the string
    if(in_str)
        return pieceval;
    else
        return sval;
}

std::string dccl::MessageVar::get_display() const
{
    std::stringstream ss;    
    ss << "\t" << name_ << " (" << type_to_string(type()) << "):" << std::endl;    
    
    for(std::vector<std::string>::size_type j = 0, m = algorithms_.size(); j < m; ++j)
    {
        if(!j)
            ss << "\t\talgorithm(s): ";
        else
            ss << ", ";
        ss << algorithms_[j];
        if (j==(m-1))
            ss << std::endl;
    }    

    if(source_var_ != "")
    {
        ss << "\t\t" << "source: {";
        ss << source_var_;
        ss  << "}";
        if(source_key_ != "")
            ss << " key: " << source_key_;
        ss << std::endl;
    }

    if(array_length_ > 1)
        ss << "\t\tarray length: " << array_length_ << std::endl;
    
    get_display_specific(ss);

    
    if(array_length_ > 1)
        ss << "\t\telement size [bits]: [" << calc_size() << "]" << std::endl;
    

    ss << "\t\ttotal size [bits]: [" << calc_total_size() << "]" << std::endl;

    return ss.str();
}


std::ostream& dccl::operator<< (std::ostream& out, const MessageVar& mv)
{
    out << mv.get_display();
    return out;
}
