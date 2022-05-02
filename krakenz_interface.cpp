#include "krakenz_interface.h"
#include "krakenz_driver.h"
#include "krakenz_software.h"

KrakenZDriverSelect::KrakenZDriverSelect(QApplication* app, KrakenZDriverSelect::DriverType type)
    : QObject{app}, mApp{app}
{
    selectDriver(type);
}

void KrakenZDriverSelect::selectDriver(KrakenZDriverSelect::DriverType type)
{
    if(type == DriverType::SOFTWARE){
        if(!SOFTWARE_DRIVER){
            SOFTWARE_DRIVER = new KrakenZSoftware{mApp};
        }
        releaseHardwareDriver();
        setCurrentDriver(SOFTWARE_DRIVER);
    } else {
        if(!HARDWARE_DRIVER){
            HARDWARE_DRIVER = new KrakenZDriver{mApp};
        }
        releaseSoftwareDriver();
        setCurrentDriver(HARDWARE_DRIVER);
    }
    mDriverType = type;
}
