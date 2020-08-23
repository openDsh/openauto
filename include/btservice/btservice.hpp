#pragma once

#include <QBluetoothAddress>
#include "OpenautoLog.hpp"
#include "btservice/AndroidBluetoothService.hpp"
#include "btservice/AndroidBluetoothServer.hpp"
namespace openauto{

namespace btservice{

class btservice{
    public:
        btservice();
    private:
        const uint16_t servicePortNumber = 22;
        openauto::btservice::AndroidBluetoothServer androidBluetoothServer;
        openauto::btservice::AndroidBluetoothService androidBluetoothService;

};

}
}