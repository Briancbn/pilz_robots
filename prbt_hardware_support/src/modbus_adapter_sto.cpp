/*
 * Copyright (c) 2019 Pilz GmbH & Co. KG
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <prbt_hardware_support/modbus_adapter_sto.h>

#include <functional>
#include <sstream>

#include <prbt_hardware_support/modbus_msg_sto_wrapper.h>

namespace prbt_hardware_support
{

using std::placeholders::_1;

ModbusAdapterSto::ModbusAdapterSto(ros::NodeHandle& nh,
                                   const ModbusApiSpec& api_spec)
  : AdapterSto(nh)
  , api_spec_(api_spec)
  , filter_pipeline_(new FilterPipeline(nh, std::bind(&ModbusAdapterSto::modbusMsgCallback, this, _1 )) )
{

}

void ModbusAdapterSto::modbusMsgCallback(const ModbusMsgInStampedConstPtr& msg_raw)
{
  ModbusMsgStoWrapper msg(msg_raw, api_spec_);

  if(msg.isDisconnect())
  {
    ROS_ERROR("A disconnect from the modbus server happend.");
    performStop();
    return;
  }

  try
  {
    msg.checkStructuralIntegrity();
  }
  catch(const ModbusMsgWrapperException &e)
  {
    ROS_ERROR_STREAM(e.what());
    performStop();
    return;
  }

  if(msg.getVersion() != MODBUS_API_VERSION_REQUIRED)
  {
    std::ostringstream os;
    os << "Received Modbus message of unsupported API Version: "
       << msg.getVersion()
       << ", required Version: " << MODBUS_API_VERSION_REQUIRED;
    ROS_ERROR_STREAM(os.str());
    performStop();
    return;
  }

  updateSto(msg.getSTO());
}


}
