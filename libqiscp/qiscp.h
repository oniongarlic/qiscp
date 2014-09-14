#ifndef QISCP_H
#define QISCP_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>
#include <QTime>
#include <QtXml/QXmlSimpleReader>

#include "iscpmsg.h"
#include "deviceinforparser.h"

#define ISCP_PORT 60128

class qiscp : public QObject
{
    Q_OBJECT
    Q_ENUMS(Commands)
public:
    explicit qiscp(QObject *parent = 0);

    Q_PROPERTY (bool power READ power WRITE setPower NOTIFY powerChanged)
    Q_PROPERTY (bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY (int masterVolume READ masterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY (int maxDirectVolume READ maxDirectVolume WRITE setMaxDirectVolume NOTIFY maxDirectVolumeChanged)
    Q_PROPERTY (bool masterMuted READ masterMuted WRITE setMasterMuted NOTIFY masterMutedChanged)

    Q_PROPERTY (int masterInput READ masterInput NOTIFY masterInputChanged)
    Q_PROPERTY (int masterTunerFreq READ masterTunerFreq NOTIFY masterTunerFreqChanged)

    Q_PROPERTY (QString currentArtist READ currentArtist NOTIFY currentArtistChanged)
    Q_PROPERTY (QString currentAlbum READ currentAlbum NOTIFY currentAlbumChanged)
    Q_PROPERTY (QString currentTitle READ currentTitle NOTIFY currentTitleChanged)

    Q_PROPERTY (QTime currentTrackPosition READ currentTrackPosition NOTIFY currentTrackPositionChanged)
    Q_PROPERTY (QTime currentTrackLength READ currentTrackLength NOTIFY currentTrackLengthChanged)

    Q_PROPERTY (int currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY (int currentTracks READ currentTracks NOTIFY currentTracksChanged)

    Q_INVOKABLE void discoverHosts();
    Q_INVOKABLE QVariantList getDevices() const;
    Q_INVOKABLE QVariantList getInputs() const;

    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

    Q_INVOKABLE void connectToHost();
    Q_INVOKABLE bool close();

    Q_INVOKABLE bool writeCommand(QString cmd, QString param);
    Q_INVOKABLE void queueCommand(QString cmd, QString param);

    Q_INVOKABLE void volumeUp();
    Q_INVOKABLE void volumeDown();
    Q_INVOKABLE void setMasterMuted(bool m);

    Q_INVOKABLE void setSleepTimer(int t);

    Q_INVOKABLE void setMasterInput(int t);

    Q_INVOKABLE void setPower(bool p);

    Q_INVOKABLE void bluetoothPairing();
    Q_INVOKABLE void bluetoothClearPairing();

    Q_INVOKABLE void setHost(QString host) { m_host=host; emit hostChanged(); }
    Q_INVOKABLE void setPort(int port) { m_port=port; emit portChanged(); }

    Q_INVOKABLE void tune(int t);
    Q_INVOKABLE void tunePreset(int t);
    Q_INVOKABLE void tuneUp();
    Q_INVOKABLE void tuneDown();

    Q_INVOKABLE QVariantList getPresets() const;
    Q_INVOKABLE void presetUp();
    Q_INVOKABLE void presetDown();

    bool connected() const { return m_connected; }
    int port() const { return m_port; }
    QString host() const { return m_host; }    

    bool power() const { return m_power; }

    bool masterMuted() const { return m_masterMuted; }
    int masterVolume() const { return m_masterVolume; }
    void setMasterVolume(quint8 maxvol);
    int masterInput() const { return m_masterInput; }
    int masterTunerFreq() const { return m_masterTunerFreq; }

    int maxDirectVolume() const { return m_maxvolume; }
    void setMaxDirectVolume(quint8 maxvol);

    QString currentArtist() const { return m_artist; }
    QString currentAlbum() const { return m_album; }
    QString currentTitle() const { return m_title; }

    QTime currentTrackPosition() const { return m_position; }
    QTime currentTrackLength() const { return m_length; }
    int currentTrack() const { return m_track; }
    int currentTracks() const { return m_tracks; }

    enum Commands {
            Play=1,
            Stop,
            Pause,
            TrackUp,
            TrackDown,
            FastForward,
            FastReverse,
            ToggleRepeat,
            ToggleRandom,
            Display,
            Right,
            Left,
            Up,
            Down,
            Select,
            Key0,
            Key1,
            Key2,
            Key3,
            Key4,
            Key5,
            Key6,
            Key7,
            Key8,
            Key9,
            Delete,
            Caps,
            Menu,
            Top
    };

    Q_INVOKABLE void networkCommand(Commands cmd);
    Q_INVOKABLE void tvCommand(Commands cmd);
    Q_INVOKABLE void dvdCommand(Commands cmd);
    Q_INVOKABLE void bdCommand(Commands cmd);
    Q_INVOKABLE void command(Commands cmd);


signals:
    void portChanged();
    void hostChanged();
    void devicesDiscovered();

    void connectedToHost();
    void disconnectedFromHost();

    void connectedChanged();
    void powerChanged();

    void deviceInfo();
    void presetsList();
    void networkList();
    void zoneList();
    void controlList();
    void selectorList();

    void masterVolumeChanged();
    void masterMutedChanged();
    void masterInputChanged();
    void masterTunerFreqChanged();

    void maxDirectVolumeChanged();

    void currentArtistChanged();
    void currentAlbumChanged();
    void currentTitleChanged();

    void currentTrackPositionChanged();
    void currentTrackLengthChanged();
    void currentTrackChanged();
    void currentTracksChanged();

public slots:

private slots:    
    void tcpConnected();
    void tcpDisconnected();
    void tcpError(QAbstractSocket::SocketError se);
    void readISCP();
    void readBroadcastDatagram();
    void discoveryTimeout();
    void handleCommandQueue();

private:
    // Commands
    struct ISCPCommands {
        enum {
            // Main
            MasterPower=0,
            MasterVolume,
            MasterMute,
            MasterInput,
            MasterTuner,
            SleepTimer,
            // Info commands
            InfoAudio,
            InfoVideo,
            // Zone 2
            Zone2Power,
            Zone2Input,
            // Zone 3
            // Zone 4
            // USB/Network
            CurrentArtist,
            CurrentAlbum,
            CurrentTitle,
            ElapsedTime,
            CurrentTrack,
            PlayStatus,
            ListInfo,
            DeviceInformation,
            // Airplay, these are untested but should work as the data format is same as other network
            AirplayCurrentArtist,
            AirplayCurrentAlbum,
            AirplayCurrentTitle,
            AirplayElapsedTime,
            //
            EndCommands // Marker
        };
    };

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

    // Inputs
    struct Inputs {
        enum {
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
            EndCommands=0xFF // Marker
        };
    };


    QMap<int, QString> m_inputs; // Input ID to text map
    QMap<QString, int> m_commands; // CMD to Command ID map

    QMap<int, QString> m_netservices;
    QMap<int, QString> m_zones;
    QMap<int, QString> m_presets;

    DeviceInforParser *m_deviceinfoparser;

    QTimer m_cmdtimer;
    QList<ISCPMsg *> m_cmdqueue;

    QTcpSocket *m_socket;
    QUdpSocket *m_broadcast;
    QString m_host;
    quint16 m_port;
    int m_discover_timeout;
    bool m_connected;

    bool m_power;        
    bool m_masterMuted;
    quint8 m_masterVolume;
    quint8 m_masterInput;
    int m_masterTunerFreq;

    quint8 m_maxvolume;

    // Album/Artist/Title (if known)
    QString m_artist;
    QString m_album;
    QString m_title;
    QTime m_position;
    QTime m_length;

    quint16 m_track;
    quint16 m_tracks;

    // NRI data
    QVariantList m_tunerpresets;
    QVariantList m_networkservices;

    // Device discovery
    QTimer m_timer;
    QVariantList m_devices;

    QByteArray m_buffer;

    void requestInitialState();
    void requestNetworkPlayState();

    bool writeCommand(ISCPMsg *message);
    void parseMessage(ISCPMsg *message);
};

#endif // QISCP_H
