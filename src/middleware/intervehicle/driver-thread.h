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

#ifndef DriverThread20190619H
#define DriverThread20190619H

#include "goby/acomms/amac.h"
#include "goby/acomms/buffer/dynamic_buffer.h"

#include "goby/middleware/group.h"
#include "goby/middleware/protobuf/intervehicle_config.pb.h"
#include "goby/middleware/protobuf/intervehicle_subscription.pb.h"
#include "goby/middleware/thread.h"
#include "goby/middleware/transport-interthread.h"

namespace goby
{
namespace acomms
{
class ModemDriverBase;
} // namespace acomms

namespace middleware
{
namespace intervehicle
{
namespace groups
{
constexpr Group modem_data_out{"goby::middleware::intervehicle::modem_data_out"};
constexpr Group modem_data_in{"goby::middleware::intervehicle::modem_data_in"};
constexpr Group modem_subscription_forward_tx{
    "goby::middleware::intervehicle::modem_subscription_forward_tx"};
constexpr Group modem_subscription_forward_rx{
    "goby::middleware::intervehicle::modem_subscription_forward_rx"};
constexpr Group modem_driver_ready{"goby::middleware::intervehicle::modem_driver_ready"};
} // namespace groups

class ModemDriverThread
    : public goby::middleware::Thread<protobuf::InterVehiclePortalConfig::LinkConfig,
                                      InterThreadTransporter>
{
  public:
    using modem_id_type = goby::acomms::DynamicBuffer<std::string>::modem_id_type;
    using subbuffer_id_type = goby::acomms::DynamicBuffer<std::string>::subbuffer_id_type;

    ModemDriverThread(const protobuf::InterVehiclePortalConfig::LinkConfig& cfg);
    void loop() override;
    int tx_queue_size() { return sending_.size(); }

  private:
    void _data_request(goby::acomms::protobuf::ModemTransmission* msg);
    void _buffer_message(std::shared_ptr<const protobuf::SerializerTransporterMessage> msg);
    void _receive(const goby::acomms::protobuf::ModemTransmission& rx_msg);
    void _forward_subscription(protobuf::InterVehicleSubscription subscription);
    void _accept_subscription(const protobuf::InterVehicleSubscription& subscription);

    subbuffer_id_type _create_buffer_id(unsigned dccl_id, unsigned group);

    subbuffer_id_type _create_buffer_id(const protobuf::SerializerTransporterKey& key)
    {
        return _create_buffer_id(DCCLSerializerParserHelperBase::id(key.type()),
                                 key.group_numeric());
    }

    subbuffer_id_type _create_buffer_id(const protobuf::InterVehicleSubscription& subscription)
    {
        return _create_buffer_id(subscription.dccl_id(), subscription.group());
    }

    void _create_buffer(modem_id_type dest_id, subbuffer_id_type buffer_id,
                        const std::vector<goby::acomms::protobuf::DynamicBufferConfig>& cfgs);

  private:
    std::unique_ptr<InterThreadTransporter> interthread_;

    std::map<subbuffer_id_type, protobuf::SerializerTransporterKey> publisher_buffer_cfg_;

    std::map<modem_id_type, std::map<subbuffer_id_type, protobuf::InterVehicleSubscription> >
        subscriber_buffer_cfg_;

    std::map<subbuffer_id_type, std::set<modem_id_type> > subbuffers_created_;

    protobuf::SerializerTransporterKey subscription_key_;
    std::set<modem_id_type> subscription_subbuffers_;

    goby::acomms::DynamicBuffer<std::string> buffer_;

    using frame_type = int;
    std::map<frame_type, std::vector<goby::acomms::DynamicBuffer<std::string>::full_value_type> >
        pending_ack_;

    std::deque<std::string> sending_;

    std::unique_ptr<goby::acomms::ModemDriverBase> driver_;
    goby::acomms::MACManager mac_;
};

} // namespace intervehicle
} // namespace middleware
} // namespace goby

#endif
