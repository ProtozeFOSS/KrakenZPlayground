#ifndef KRAKENZ_SOFTWARE_H
#define KRAKENZ_SOFTWARE_H

#include "krakenz_interface.h"

class KrakenZSoftware: public KrakenZInterface {
    Q_OBJECT
public:
    KrakenZSoftware(QObject* parent = nullptr) : KrakenZInterface(parent){};
    Q_INVOKABLE bool initialize(bool&) override;
    Q_INVOKABLE bool initialized() override;

public slots:
    void setBrightness(quint8 brightness) override;

};


#endif // KRAKENZ_SOFTWARE_H
