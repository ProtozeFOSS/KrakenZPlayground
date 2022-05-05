#include "krakenz_interface.h"
#include "krakenz_driver.h"
#include "krakenz_software.h"

KrakenZDriverSelect* KRAKEN_DRIVER_SELECT = nullptr;
static KrakenZInterface*  HARDWARE_DRIVER = nullptr;
static KrakenZInterface*  SOFTWARE_DRIVER = nullptr;

KrakenZDriverSelect::KrakenZDriverSelect(QApplication* app, KrakenZDriverSelect::DriverType type)
    : QObject{app}
{
    selectDriver(type);
}

KrakenZInterface* KrakenZDriverSelect::currentDriver()
{
    return mDriverType == SOFTWARE ? SOFTWARE_DRIVER:HARDWARE_DRIVER;
}
void KrakenZDriverSelect::initializeDriverSelect(QApplication* app, KrakenZDriverSelect::DriverType type)
{
    if(!KRAKEN_DRIVER_SELECT){
        KRAKEN_DRIVER_SELECT = new KrakenZDriverSelect(app, type);
    }
}

KrakenZDriverSelect* KrakenZDriverSelect::GetInstance()
{
    return KRAKEN_DRIVER_SELECT;
}

void KrakenZDriverSelect::releaseSoftwareDriver()
{
    delete SOFTWARE_DRIVER;
    SOFTWARE_DRIVER = nullptr;
}

void KrakenZDriverSelect::releaseHardwareDriver()
{
    delete HARDWARE_DRIVER;
    HARDWARE_DRIVER = nullptr;
}

void KrakenZDriverSelect::selectDriver(KrakenZDriverSelect::DriverType type)
{
    if(type == DriverType::SOFTWARE){
        if(!SOFTWARE_DRIVER){
            SOFTWARE_DRIVER = new KrakenZSoftware{parent()};
        }
        releaseHardwareDriver();
    } else {
        if(!HARDWARE_DRIVER){
            HARDWARE_DRIVER = new KrakenZDriver{parent()};
        }
        releaseSoftwareDriver();
    }
    mDriverType = type;
}

KrakenZDriverSelect::~KrakenZDriverSelect()
{
    releaseDrivers();
    KRAKEN_DRIVER_SELECT = nullptr;
}
