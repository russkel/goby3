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

#include <set>
#include <map>
#include <string>
#include "MOOSLIB/MOOSMsg.h"
#include "MOOSUtilityLib/MOOSGeodesy.h"

#include "goby/moos/moos_protobuf_helpers.h"
#include "goby/moos/protobuf/translator.pb.h"
#include "goby/util/dynamic_protobuf_manager.h"
#include "goby/moos/transitional/message_algorithms.h"
#include "goby/moos/modem_id_convert.h"


namespace goby
{
    namespace moos
    {

        void alg_power_to_dB(transitional::DCCLMessageVal& val_to_mod);
        void alg_dB_to_power(transitional::DCCLMessageVal& val_to_mod);

// applied to "T" (temperature), references are "S" (salinity), then "D" (depth)
        void alg_TSD_to_soundspeed(transitional::DCCLMessageVal& val,
                                   const std::vector<transitional::DCCLMessageVal>& ref_vals);

        void alg_angle_0_360(transitional::DCCLMessageVal& angle);
        void alg_angle_n180_180(transitional::DCCLMessageVal& angle);

        void alg_to_upper(transitional::DCCLMessageVal& val_to_mod);
        void alg_to_lower(transitional::DCCLMessageVal& val_to_mod);
        
        class MOOSTranslator
        {
          public:
            MOOSTranslator(const goby::moos::protobuf::TranslatorEntry& entry = goby::moos::protobuf::TranslatorEntry(),
                           double lat_origin = std::numeric_limits<double>::quiet_NaN(),
                           double lon_origin = std::numeric_limits<double>::quiet_NaN(),
                           const std::string& modem_id_lookup_path="")
            {
                initialize(lat_origin, lon_origin, modem_id_lookup_path);
                if(entry.IsInitialized())
                    add_entry(entry);
            }

            MOOSTranslator(const google::protobuf::RepeatedPtrField<goby::moos::protobuf::TranslatorEntry>& entries,
                           double lat_origin = std::numeric_limits<double>::quiet_NaN(),
                           double lon_origin = std::numeric_limits<double>::quiet_NaN(),
                           const std::string& modem_id_lookup_path="")
            {
                initialize(lat_origin, lon_origin, modem_id_lookup_path); 
                add_entry(entries);
            }

            
            
            void add_entry(const goby::moos::protobuf::TranslatorEntry& entry)
            { dictionary_[entry.protobuf_name()] =  entry; }
            
            void add_entry(const std::set<goby::moos::protobuf::TranslatorEntry>& entries)
            {
                for(std::set<goby::moos::protobuf::TranslatorEntry>::const_iterator it = entries.begin(),
                        n = entries.end();
                    it != n;
                    ++it)
                { add_entry(*it); }
            }            

            void add_entry(const google::protobuf::RepeatedPtrField<goby::moos::protobuf::TranslatorEntry>& entries)
            {
                for(google::protobuf::RepeatedPtrField<goby::moos::protobuf::TranslatorEntry>::const_iterator it = entries.begin(), n = entries.end();
                    it != n;
                    ++it)
                { add_entry(*it); }
            }
            
            // ownership of returned pointer goes to caller (must use smart pointer or call delete)
            template<typename GoogleProtobufMessagePointer, class StringCMOOSMsgMap >
                GoogleProtobufMessagePointer moos_to_protobuf(const StringCMOOSMsgMap& moos_variables, const std::string& protobuf_name);
            
            std::multimap<std::string, CMOOSMsg> protobuf_to_moos(const google::protobuf::Message& protobuf_msg);

            const std::map<std::string, goby::moos::protobuf::TranslatorEntry>&  dictionary() const { return dictionary_; }
            
          private:
            void initialize(double lat_origin = std::numeric_limits<double>::quiet_NaN(),
                            double lon_origin = std::numeric_limits<double>::quiet_NaN(),
                            const std::string& modem_id_lookup_path="");
            

            void alg_lat2utm_y(transitional::DCCLMessageVal& mv,
                               const std::vector<transitional::DCCLMessageVal>& ref_vals);

            void alg_lon2utm_x(transitional::DCCLMessageVal& mv,
                               const std::vector<transitional::DCCLMessageVal>& ref_vals);

            void alg_utm_x2lon(transitional::DCCLMessageVal& mv,
                               const std::vector<transitional::DCCLMessageVal>& ref_vals);

            void alg_utm_y2lat(transitional::DCCLMessageVal& mv,
                               const std::vector<transitional::DCCLMessageVal>& ref_vals);
            
            void alg_modem_id2name(transitional::DCCLMessageVal& in);
            void alg_modem_id2type(transitional::DCCLMessageVal& in);
            void alg_name2modem_id(transitional::DCCLMessageVal& in);

            
          private:
            std::map<std::string, goby::moos::protobuf::TranslatorEntry> dictionary_;
            CMOOSGeodesy geodesy_;
            tes::ModemIdConvert modem_lookup_;
        };

        inline std::ostream& operator<<(std::ostream& os, const MOOSTranslator& tl)
        {
            os << "= Begin MOOSTranslator =\n";

            int i = 0;
            for(std::map<std::string, goby::moos::protobuf::TranslatorEntry>::const_iterator it = tl.dictionary().begin(),
                    n = tl.dictionary().end();
                it != n;
                ++it)
            {
                os << "== Begin Entry " << i << " ==\n"
                   << it->second.DebugString()
                   << "== End Entry " << i << " ==\n";
                
                ++i;
            }
            
            os << "= End MOOSTranslator =";
            return os;
        }

        namespace protobuf
        {
            inline bool operator<(const protobuf::TranslatorEntry& a, const protobuf::TranslatorEntry& b)
            {
                return a.protobuf_name() < b.protobuf_name();
            }
        }
        

    }
}



inline std::multimap<std::string, CMOOSMsg> goby::moos::MOOSTranslator::protobuf_to_moos(const google::protobuf::Message& protobuf_msg)
{
    std::map<std::string, goby::moos::protobuf::TranslatorEntry>::const_iterator it =
        dictionary_.find(protobuf_msg.GetDescriptor()->full_name());

    if(it == dictionary_.end())
        throw(std::runtime_error("No TranslatorEntry for Protobuf type: " + protobuf_msg.GetDescriptor()->full_name()));

    const goby::moos::protobuf::TranslatorEntry& entry = it->second;
    
    std::multimap<std::string, CMOOSMsg> moos_msgs;

    for(int i = 0, n = entry.publish_size();
        i < n; ++ i)
    {
        std::string return_string;
        std::string moos_var = entry.publish(i).moos_var();
        
        switch(entry.publish(i).technique())
        {
            case protobuf::TranslatorEntry::TECHNIQUE_PROTOBUF_TEXT_FORMAT:
                goby::moos::MOOSTranslation<goby::moos::protobuf::TranslatorEntry::TECHNIQUE_PROTOBUF_TEXT_FORMAT>::serialize(&return_string, protobuf_msg);
                break;
                
            case protobuf::TranslatorEntry::TECHNIQUE_PROTOBUF_NATIVE_ENCODED:
                goby::moos::MOOSTranslation<goby::moos::protobuf::TranslatorEntry::TECHNIQUE_PROTOBUF_NATIVE_ENCODED>::serialize(&return_string, protobuf_msg);
                break;

            case protobuf::TranslatorEntry::TECHNIQUE_COMMA_SEPARATED_KEY_EQUALS_VALUE_PAIRS:
                goby::moos::MOOSTranslation<protobuf::TranslatorEntry::TECHNIQUE_COMMA_SEPARATED_KEY_EQUALS_VALUE_PAIRS>::serialize(&return_string, protobuf_msg, entry.publish(i).algorithm(), entry.use_short_enum());
                break;
                
            case protobuf::TranslatorEntry::TECHNIQUE_FORMAT:
                // process moos_variable too (can be a format string itself!)
                goby::moos::MOOSTranslation<protobuf::TranslatorEntry::TECHNIQUE_FORMAT>::serialize(&moos_var, protobuf_msg, entry.publish(i).algorithm(), entry.publish(i).moos_var(), entry.publish(i).repeated_delimiter(), entry.use_short_enum());
                // now do the format values
                goby::moos::MOOSTranslation<protobuf::TranslatorEntry::TECHNIQUE_FORMAT>::serialize(&return_string, protobuf_msg, entry.publish(i).algorithm(), entry.publish(i).format(), entry.publish(i).repeated_delimiter(), entry.use_short_enum());
                break;
        }

        
        try
        {
            double return_double = boost::lexical_cast<double>(return_string);
            moos_msgs.insert(std::make_pair(moos_var,
                                            CMOOSMsg(MOOS_NOTIFY, moos_var, return_double)));
        }
        catch(boost::bad_lexical_cast&)
        {
            moos_msgs.insert(std::make_pair(moos_var,
                                            CMOOSMsg(MOOS_NOTIFY, moos_var, return_string)));
        }        
    }
    
    return moos_msgs;
}

template<typename GoogleProtobufMessagePointer, class StringCMOOSMsgMap >
GoogleProtobufMessagePointer goby::moos::MOOSTranslator::moos_to_protobuf(const StringCMOOSMsgMap& moos_variables, const std::string& protobuf_name)
{
    std::map<std::string, goby::moos::protobuf::TranslatorEntry>::const_iterator it = dictionary_.find(protobuf_name);
    
    if(it == dictionary_.end())
        throw(std::runtime_error("No TranslatorEntry for Protobuf type: " + protobuf_name));

    const goby::moos::protobuf::TranslatorEntry& entry = it->second;

    
    GoogleProtobufMessagePointer msg =
        goby::util::DynamicProtobufManager::new_protobuf_message<GoogleProtobufMessagePointer>(protobuf_name);

    if(&*msg == 0)
        throw(std::runtime_error("Unknown Protobuf type: " + protobuf_name + "; be sure it is compiled in or directly loaded into the goby::util::DynamicProtobufManager."));

    
    for(int i = 0, n = entry.create_size();
        i < n; ++ i)
    {
        std::multimap<std::string, CMOOSMsg>::const_iterator it = moos_variables.find(entry.create(i).moos_var());
        std::string source_string = (it == moos_variables.end())
            ? ""
            : (it->second.IsString() ? it->second.GetString() : goby::util::as<std::string>(it->second.GetDouble()));
            
        switch(entry.create(i).technique())
        {
            case protobuf::TranslatorEntry::TECHNIQUE_PROTOBUF_TEXT_FORMAT:
                goby::moos::MOOSTranslation<protobuf::TranslatorEntry::TECHNIQUE_PROTOBUF_TEXT_FORMAT>::parse(source_string, &*msg);
                break;
                
            case protobuf::TranslatorEntry::TECHNIQUE_PROTOBUF_NATIVE_ENCODED:
                goby::moos::MOOSTranslation<protobuf::TranslatorEntry::TECHNIQUE_PROTOBUF_NATIVE_ENCODED>::parse(source_string, &*msg);
                break;

            case protobuf::TranslatorEntry::TECHNIQUE_COMMA_SEPARATED_KEY_EQUALS_VALUE_PAIRS:
                goby::moos::MOOSTranslation<protobuf::TranslatorEntry::TECHNIQUE_COMMA_SEPARATED_KEY_EQUALS_VALUE_PAIRS>::parse(source_string, &*msg, entry.create(i).algorithm(), entry.use_short_enum());
                break;

            case protobuf::TranslatorEntry::TECHNIQUE_FORMAT:
                goby::moos::MOOSTranslation<protobuf::TranslatorEntry::TECHNIQUE_FORMAT>::parse(source_string, &*msg, entry.create(i).algorithm(), entry.create(i).format(), entry.use_short_enum());
                break;
        }
    }

    return msg;
}

