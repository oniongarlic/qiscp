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
    Q_ENUMS(Zones)
public:
    explicit qiscp(QObject *parent = 0);

    enum Zones {
        Zone1=0x1,
        Zone2=0x2,
        Zone3=0x4,
        Zone4=0x8
    };

    Q_PROPERTY (bool connected READ connected NOTIFY connectedChanged)

    Q_PROPERTY (bool power READ power WRITE setPower NOTIFY powerChanged)
    Q_PROPERTY (int masterVolume READ masterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY (int maxDirectVolume READ maxDirectVolume WRITE setMaxDirectVolume NOTIFY maxDirectVolumeChanged)
    Q_PROPERTY (bool masterMuted READ masterMuted WRITE setMasterMuted NOTIFY masterMutedChanged)

    Q_PROPERTY (int masterInput READ masterInput WRITE setMasterInput NOTIFY masterInputChanged)
    Q_PROPERTY (int masterTunerFreq READ masterTunerFreq NOTIFY masterTunerFreqChanged)

    Q_PROPERTY (int bassLevel READ bassLevel WRITE setBassLevel NOTIFY bassLevelChanged)
    Q_PROPERTY (int trebleLevel READ trebleLevel WRITE setTrebleLevel NOTIFY trebleLevelChanged)

    Q_PROPERTY (int centerLevel READ centerLevel WRITE setCenterLevel NOTIFY centerLevelChanged)
    Q_PROPERTY (int subwooferLevel READ subwooferLevel WRITE setSubwooferLevel NOTIFY subwooferLevelChanged)

    Q_PROPERTY (bool zone2power READ zone2Power WRITE setZone2Power NOTIFY zone2PowerChanged)
    Q_PROPERTY (int zone2Volume READ zone2Volume NOTIFY zone2VolumeChanged)
    Q_PROPERTY (int zone2Input READ zone2Input WRITE setZone2Input NOTIFY zone2InputChanged)

    Q_PROPERTY (bool zone3power READ zone3Power WRITE setZone3Power NOTIFY zone3PowerChanged)
    Q_PROPERTY (int zone3Volume READ zone3Volume NOTIFY zone3VolumeChanged)
    Q_PROPERTY (int zone3Input READ zone3Input WRITE setZone3Input NOTIFY zone3InputChanged)

    Q_PROPERTY (bool zone4power READ zone4Power WRITE setZone4Power NOTIFY zone4PowerChanged)
    Q_PROPERTY (int zone4Volume READ zone4Volume NOTIFY zone4VolumeChanged)
    Q_PROPERTY (int zone4Input READ zone4Input WRITE setZone4Input NOTIFY zone4InputChanged)

    Q_PROPERTY (QString currentArtist READ currentArtist NOTIFY currentArtistChanged)
    Q_PROPERTY (QString currentAlbum READ currentAlbum NOTIFY currentAlbumChanged)
    Q_PROPERTY (QString currentTitle READ currentTitle NOTIFY currentTitleChanged)

    Q_PROPERTY (QTime currentTrackPosition READ currentTrackPosition NOTIFY currentTrackPositionChanged)
    Q_PROPERTY (QTime currentTrackLength READ currentTrackLength NOTIFY currentTrackLengthChanged)

    Q_PROPERTY (int currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY (int currentTracks READ currentTracks NOTIFY currentTracksChanged)

    Q_PROPERTY (bool hdmiAudio READ hdmiAudio NOTIFY hdmiAudioChanged)
    Q_PROPERTY (bool cec READ cec NOTIFY cecChanged)
    Q_PROPERTY (bool musicOptimizer READ musicOptimizer NOTIFY musicOptimizerChanged)

    Q_PROPERTY (bool discovering READ discovering NOTIFY discoveringChanged)

    Q_INVOKABLE void discoverHosts();
    Q_INVOKABLE QVariantList getDevices() const;
    Q_INVOKABLE QVariantList getStaticInputs() const;
    Q_INVOKABLE QVariantList getInputs() const;    
    Q_INVOKABLE QVariantList getZones() const;    
    Q_INVOKABLE QVariantList getControls() const;

    Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
    Q_PROPERTY(int port READ port WRITE setPort NOTIFY portChanged)

    Q_INVOKABLE void connectToHost();
    Q_INVOKABLE void setHost(QString host) { m_host=host; emit hostChanged(); }
    Q_INVOKABLE void setPort(int port) { m_port=port; emit portChanged(); }
    Q_INVOKABLE bool close();

    Q_INVOKABLE bool writeCommand(QString cmd, QString param);
    Q_INVOKABLE bool writeCommand(QString cmd, bool param);
    Q_INVOKABLE void queueCommand(QString cmd, QString param);

    Q_INVOKABLE void volumeUp(Zones zone=Zone1);
    Q_INVOKABLE void volumeDown(Zones zone=Zone1);
    Q_INVOKABLE void setMasterMuted(bool m);

    Q_INVOKABLE void bluetoothPairing();
    Q_INVOKABLE void bluetoothClearPairing();

    Q_INVOKABLE void setSleepTimer(int t);

    Q_INVOKABLE void tune(int t, Zones zone=Zone1);
    Q_INVOKABLE void tunePreset(int t, Zones zone=Zone1);
    Q_INVOKABLE void tuneUp(Zones zone=Zone1);
    Q_INVOKABLE void tuneDown(Zones zone=Zone1);

    Q_INVOKABLE QVariantList getPresets() const;
    Q_INVOKABLE void presetUp(Zones zone=Zone1);
    Q_INVOKABLE void presetDown(Zones zone=Zone1);

    Q_INVOKABLE void bassLevelUp(Zones zone=Zone1);
    Q_INVOKABLE void bassLevelDown(Zones zone=Zone1);

    Q_INVOKABLE void trebleLevelUp(Zones zone=Zone1);
    Q_INVOKABLE void trebleLevelDown(Zones zone=Zone1);

    Q_INVOKABLE void subwooferLevelUp();
    Q_INVOKABLE void subwooferLevelDown();

    Q_INVOKABLE void centerLevelUp();
    Q_INVOKABLE void centerLevelDown();

    Q_INVOKABLE void setCEC(bool m);
    Q_INVOKABLE void setHDMIAudio(bool m);
    Q_INVOKABLE void setMusicOptimizer(bool m);
    Q_INVOKABLE void setListeningMode(int m); // XXX Use enum for this!
    Q_INVOKABLE void setLateNightMode(int m);

    Q_INVOKABLE void setZone2Muted(bool m);
    Q_INVOKABLE void setZone3Muted(bool m);
    Q_INVOKABLE void setZone4Muted(bool m);   

    Q_INVOKABLE void setZoneInput(Zones zone, int t);
    Q_INVOKABLE void setMasterInput(int t);
    Q_INVOKABLE void setZone2Input(int t);
    Q_INVOKABLE void setZone3Input(int t);
    Q_INVOKABLE void setZone4Input(int t);

    Q_INVOKABLE void setPower(bool p);
    Q_INVOKABLE void setZone2Power(bool p);
    Q_INVOKABLE void setZone3Power(bool p);
    Q_INVOKABLE void setZone4Power(bool p);

    bool connected() const { return m_connected; }
    bool discovering() const { return m_discovering; }
    int port() const { return m_port; }
    QString host() const { return m_host; }    

    bool power() const { return m_power; }

    int maxDirectVolume() const { return m_maxvolume; }
    void setMaxDirectVolume(quint8 maxvol);

    bool masterMuted() const { return m_masterMuted; }
    int masterVolume() const { return m_masterVolume; }
    void setMasterVolume(quint8 maxvol);
    int masterInput() const { return m_masterInput; }
    int masterTunerFreq() const { return m_masterTunerFreq; }

    int centerLevel() const { return m_centerLevel; }
    int subwooferLevel() const { return m_subwooferLevel; }

    void setCenterLevel(qint8 level);
    void setSubwooferLevel(qint8 level);

    int bassLevel() const { return m_bassLevel; }
    int trebleLevel() const { return m_trebleLevel; }

    void setBassLevel(qint8 level, Zones zone=Zone1);
    void setTrebleLevel(qint8 level, Zones zone=Zone1);

    bool zone2Power() const { return m_z2Power; }
    bool zone2Muted() const { return m_z2Muted; }
    int zone2Volume() const { return m_z2Volume; }
    void setZone2Volume(quint8 maxvol);
    int zone2Input() const { return m_z2Input; }

    bool zone3Power() const { return m_z3Power; }
    bool zone3Muted() const { return m_z3Muted; }
    int zone3Volume() const { return m_z3Volume; }
    void setZone3Volume(quint8 maxvol);
    int zone3Input() const { return m_z3Input; }

    bool zone4Power() const { return m_z4Power; }
    bool zone4Muted() const { return m_z4Muted; }
    int zone4Volume() const { return m_z4Volume; }
    void setZone4Volume(quint8 maxvol);
    int zone4Input() const { return m_z4Input; }

    QString currentArtist() const { return m_artist; }
    QString currentAlbum() const { return m_album; }
    QString currentTitle() const { return m_title; }

    QTime currentTrackPosition() const { return m_position; }
    QTime currentTrackLength() const { return m_length; }
    int currentTrack() const { return m_track; }
    int currentTracks() const { return m_tracks; }

    bool hdmiAudio() const { return m_hdmiAudio; }
    bool cec() const { return m_cec; }
    bool musicOptimizer() const { return m_musicOptimizer; }

    enum Commands {
        Play=1,
        Stop,
        Pause,
        TrackUp,
        TrackDown,
        FastForward,
        FastReverse,
        SkipForward,
        SkipReverse,
        ToggleRepeat,
        ToggleRandom,
        Display,
        Clear,
        Right,
        Left,
        Up,
        Down,
        Enter,
        Return,
        Previous,
        Repeat,
        Random,
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
        KeyA,
        KeyB,
        KeyC,
        KeyD,
        Delete,
        Caps,
        Menu,        
        Guide,
        Angle,
        Top,
        Setup,
        Input,
        OpenClose,
        Power,
        PowerOn,
        PowerOff,
        ChannelUp,
        ChannelDown,
        VolumeUp,
        VolumeDown,
        Mute
    };

    Q_INVOKABLE bool networkCommand(Commands cmd);
    Q_INVOKABLE bool tvCommand(Commands cmd);
    Q_INVOKABLE bool dvdCommand(Commands cmd);
    Q_INVOKABLE bool bdCommand(Commands cmd);
    Q_INVOKABLE bool command(Commands cmd, Zones zone=Zone1);

signals:
    void portChanged();
    void hostChanged();
    void discoveringChanged();
    void devicesDiscovered();

    void connectedToHost();
    void disconnectedFromHost();

    void connectedChanged();
    void powerChanged();

    void hdmiAudioChanged();
    void cecChanged();
    void musicOptimizerChanged();
    void listeningModeChanged();
    void lateNightModeChanged();

    void deviceInfo();
    void presetsList();
    void networkList();
    void zonesList();
    void inputsList();
    void controlList();
    void selectorList();

    void masterVolumeChanged();
    void masterMutedChanged();
    void masterInputChanged();
    void masterTunerFreqChanged();

    void bassLevelChanged();
    void trebleLevelChanged();

    void centerLevelChanged();
    void subwooferLevelChanged();

    void maxDirectVolumeChanged();

    void zone2PowerChanged();
    void zone3PowerChanged();
    void zone4PowerChanged();

    void zone2VolumeChanged();
    void zone3VolumeChanged();
    void zone4VolumeChanged();

    void zone2MutedChanged();
    void zone3MutedChanged();
    void zone4MutedChanged();

    void zone2InputChanged();
    void zone3InputChanged();
    void zone4InputChanged();

    void zone2BassLevelChanged();
    void zone2TrebleLevelChanged();

    void zone3BassLevelChanged();
    void zone3TrebleLevelChanged();

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
            MasterTone,
            MasterBalance,
            CenterLevel,
            SubwooferLevel,
            SleepTimer,
            // Info commands
            InfoAudio,
            InfoVideo,
            // Zone 2
            Zone2Power,
            Zone2Input,
            Zone2Mute,
            Zone2Volume,
            Zone2Tone,
            Zone2Balance,            
            // Zone 3
            Zone3Power,
            Zone3Input,
            Zone3Mute,
            Zone3Volume,
            Zone3Tone,
            Zone3Balance,            
            // Zone 4
            Zone4Power,
            Zone4Input,
            Zone4Mute,
            Zone4Volume,            
            // USB/Network
            CurrentArtist,
            CurrentAlbum,
            CurrentTitle,
            ElapsedTime,
            CurrentTrack,
            PlayStatus,
            ListInfo,
            DeviceInformation,
            CEC,
            HDMIAudio,
            MusicOptimizer,
            ListeningMode,
            LateNightMode,
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

    // The static input list codes
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
            USBToggle=0x2C,
            AirPlay=0x2D,
            Bluetooth=0x2E,
            Multi=0x30,
            Xm1=0x31,
            Sirius=0x32,
            DAB=0x33,
            UniversalPort=0x40,
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
    bool m_discovering;
    bool m_connected;

    bool m_power;
    bool m_masterMuted;
    quint8 m_masterVolume;
    quint8 m_masterInput;
    int m_masterTunerFreq;

    qint8 m_masterTreble;
    qint8 m_masterBass;

    quint8 m_maxvolume;

    qint8 m_bassLevel;
    qint8 m_trebleLevel;

    qint8 m_centerLevel;
    qint8 m_subwooferLevel;

    quint8 m_zonesAvailable;

    // Zone 2    
    bool m_z2Power;
    quint8 m_z2Input;
    bool m_z2Muted;
    quint8 m_z2Volume;
    qint8 m_z2Treble;
    qint8 m_z2Bass;
    qint8 m_z2Balance;

    // Zone 3    
    bool m_z3Power;
    quint8 m_z3Input;
    bool m_z3Muted;
    quint8 m_z3Volume;
    qint8 m_z3Treble;
    qint8 m_z3Bass;
    qint8 m_z3Balance;

    // Zone 4    
    bool m_z4Power;
    quint8 m_z4Input;
    bool m_z4Muted;
    quint8 m_z4Volume;

    // Album/Artist/Title (if known)
    QString m_artist;
    QString m_album;
    QString m_title;
    QTime m_position;
    QTime m_length;

    quint16 m_track;
    quint16 m_tracks;

    bool m_cec;
    bool m_hdmiAudio;
    bool m_musicOptimizer;

    quint8 m_listeningmode;
    quint8 m_latenight;

    // NRI data
    QVariantList m_tunerpresets;
    QVariantList m_networkservices;
    QVariantList m_zonesdata;
    QVariantList m_inputsdata;    
    QVariantList m_controls;

    // Device discovery
    QTimer m_timer;
    QVariantList m_devices;

    QByteArray m_buffer;

    void requestInitialState();
    void requestNetworkPlayState();
    void requestZone2State();
    void requestZone3State();
    void requestZone4State();

    bool writeCommand(ISCPMsg *message);    
    bool writeCommand(QString cmd, const char *param);
    void parseMessage(ISCPMsg *message);

    bool keyCommand(QString c, Commands cmd);
    bool baseCommand(QString c, Commands cmd);
};

#endif // QISCP_H
