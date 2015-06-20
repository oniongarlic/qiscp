#ifndef QISCPINPUTS_H
#define QISCPINPUTS_H

#include <QObject>

class qiscpInputs : public QObject {
    Q_OBJECT
    Q_ENUMS(Input)

public:

    /*
    "00"	sets VIDEO1    VCR/DVR     STB/DVR
    "01"	sets VIDEO2    CBL/SAT
    "02"	sets VIDEO3    GAME/TV    GAME     GAME1
    "03"	sets VIDEO4    AUX1(AUX)
    "04"	sets VIDEO5    AUX2                          GAME2
    "05"	sets VIDEO6    PC
    "06"	sets VIDEO7
    "07"	Hidden1     EXTRA1
    "08"	Hidden2     EXTRA2
    "09"	Hidden3     EXTRA3
    "10"	sets DVD          BD/DVD
    "20"	sets TAPE(1)    TV/TAPE
    "21"	sets TAPE2
    "22"	sets PHONO
    "23"	sets CD    TV/CD
    "24"	sets FM
    "25"	sets AM
    “26”	sets TUNER
    "27"	sets MUSIC SERVER    P4S   DLNA*2
    "28"	sets INTERNET RADIO           iRadio Favorite*3
    "29"	sets USB/USB(Front)
    "2A"	sets USB(Rear)
    "2B"	sets NETWORK                      NET
    "2C"	sets USB(toggle)
    "2D"	sets Aiplay
    "40"	sets Universal PORT
    "30"	sets MULTI CH
    "31"	sets XM*1
    "32"	sets SIRIUS*1
    "33"	sets DAB *5
    */

    enum Input {
        Video1=0x00,
        Video2=0x01,
        Video3=0x02,
        Video4=0x03,
        Video5=0x04,
        Video6=0x05,
        Video7=0x06,
        Extra1=0x07,
        Extra2=0x08,
        Extra3=0x09,
        DVD=0x10,
        Tape1=0x20,
        Tape2=0x21,
        Phono=0x22,
        CD=0x23,
        FM=0x24,
        AM=0x25,
        Tuner=0x26,
        MusicServer=0x27,
        InternetRadio=0x28,
        USBFront=0x29,
        USBBack=0x2A,
        Network=0x2B,
        USBToggle=0x2C,
        AirPlay=0x2D,
        Bluetooth=0x2E,
        Multi=0x30,
        Xm1=0x31,
        Sirius=0x32,
        DAB=0x33,
        UniversalPort=0x40,
        Source=0x80,
        EndCommands=0xFF // Marker
    };

    Q_INVOKABLE static bool isTuner(Input i) {
        return (i==FM || i==AM || i==Tuner);
    }
    Q_INVOKABLE static bool isNetworkOrUSB(Input i) {
        return (i==MusicServer || i==Network || i==USBBack || i==USBFront);
    }
    Q_INVOKABLE static bool isNetwork(Input i) {
        return (i==MusicServer || i==Network || i==InternetRadio);
    }
    Q_INVOKABLE static bool isUSB(Input i) {
        return (i==USBBack || i==USBFront);
    }
    Q_INVOKABLE static bool isControllable(Input i) {
        return (isNetworkOrUSB(i) || i==DVD);
    }
};

#endif // QISCPINPUTS_H
