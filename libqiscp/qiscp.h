#ifndef QISCP_H
#define QISCP_H

#include <QObject>
#include <QDebug>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>
#include <QTime>
#include <QFile>

#include "iscpmsg.h"
#include "deviceinforparser.h"
#include "artworkparser.h"
#include "qiscpinputs.h"

#define ISCP_PORT 60128


class qiscp : public QObject
{
    Q_OBJECT
    Q_FLAGS(Zone)
    Q_ENUMS(Zones)
    Q_ENUMS(Commands)    
    Q_ENUMS(LateNightModes)
    Q_ENUMS(ListeningModesQuick)
    Q_ENUMS(NetworkService)
    Q_ENUMS(PlayModes)
    Q_ENUMS(EcoMode)
    Q_ENUMS(RepeatModes)
    Q_ENUMS(ShuffleModes)
    Q_ENUMS(Audyssey2EQ)
    Q_ENUMS(AudysseyDynamicVolume)
    Q_ENUMS(AudysseyDynamicEQ)

public:
    explicit qiscp(QObject *parent = 0);
    ~qiscp();

    enum Zones {
        Zone1=0x01,
        Zone2=0x02,
        Zone3=0x04,
        Zone4=0x08
    };
    Q_DECLARE_FLAGS(Zone, Zones)

    enum LateNightModes {
        Off=0,
        Low=1,
        High=2,
        Auto=3,
    };

    enum EcoMode {
        Eco1dB=1,
        Eco3dB=3,
        Eco6dB=6,
    };

    enum ListeningModes {
        Stereo=0,
        Direct,
        Surround,
        Film,
        THX,
        Action,
    };

    enum ListeningModesQuick {
        ToggleMovie,
        ToggleMusic,
        ToggleGame
    };

    enum NetworkService {
        NSDLNA=0x0,
        NSFavorite=0x1,
        vTuner=0x2,
        Sirius=0x3,
        Pandora=0x4,
        Rhapsody=0x5,
        LastFM=0x6,
        Napster=0x7,
        Slacker=0x8,
        Mediafly=0x9,
        Spotify=0xA,
        AUPEO=0xB,
        Radiko=0xC,
        eOnkyo=0xD,
        TuneInRadio=0xE,
        mp3tunes=0xF,
        Simfy=0x10,
        HomeMedia=0x11
    };

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

    enum PlayModes {
        Stopped,
        Playing,
        Paused,
        FastForwarding,
        FastReversing,
        EndOfFile
    };

    enum RepeatModes {
        RepeatOff,
        RepeatAll,
        RepeatFolder,
        RepeatOne,
        RepeatDisabled
    };

    enum AudysseyDynamicVolume {
        AudysseyDynamicVolumeOff=0,
        AudysseyDynamicVolumeLight=1,
        AudysseyDynamicVolumeMedium=2,
        AudysseyDynamicVolumeHard=3
    };

    enum Audyssey2EQ {
        Audyssey2EQOff=0,
        Audyssey2EQMovie=1,
        Audyssey2EQMusic=2
    };

    enum AudysseyDynamicEQ {
        AudysseyDynamicEQOff=0,
        AudysseyDynamicEQOn=1
    };

    enum ShuffleModes {
        ShuffleOff,
        ShuffleAll,
        ShuffleAlbum,
        ShuffleFolder,
        ShuffleDisabled
    };

    Q_PROPERTY (bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY (int discoveryTimeout READ discoveryTimeout WRITE setDiscoveryTimeout NOTIFY discoveryTimeoutChanged)
    Q_PROPERTY (int discovered READ discovered NOTIFY discoveredChanged)

    Q_PROPERTY (bool debug READ getDebug WRITE setDebug NOTIFY debugChanged)

    Q_PROPERTY (bool power READ power WRITE setPower NOTIFY powerChanged)
    Q_PROPERTY (int masterVolume READ masterVolume NOTIFY masterVolumeChanged)
    Q_PROPERTY (int maxDirectVolume READ maxDirectVolume WRITE setMaxDirectVolume NOTIFY maxDirectVolumeChanged)
    Q_PROPERTY (bool masterMuted READ masterMuted WRITE setMasterMuted NOTIFY masterMutedChanged)

    Q_PROPERTY (Zone poweredZones READ poweredZones NOTIFY poweredZonesChanged)

    Q_PROPERTY (int masterInput READ masterInput WRITE setMasterInput NOTIFY masterInputChanged)
    Q_PROPERTY (int masterTunerFreq READ masterTunerFreq NOTIFY masterTunerFreqChanged)
    Q_PROPERTY (int masterTunerPreset READ masterTunerPreset NOTIFY masterTunerPresetChanged)

    Q_PROPERTY (NetworkService networkService READ networkService WRITE setNetworkService NOTIFY networkServiceChanged)
    Q_PROPERTY (int networkRadioPreset READ networkRadioPreset NOTIFY networkRadioPresetChanged)

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
    Q_PROPERTY (bool hasArtwork READ hasArtwork NOTIFY hasArtworkChanged)

    // In Seconds
    Q_PROPERTY (int currentTrackPosition READ currentTrackPosition NOTIFY currentTrackPositionChanged)
    Q_PROPERTY (int currentTrackLength READ currentTrackLength NOTIFY currentTrackLengthChanged)

    Q_PROPERTY (int currentTrack READ currentTrack NOTIFY currentTrackChanged)
    Q_PROPERTY (int currentTracks READ currentTracks NOTIFY currentTracksChanged)

    Q_PROPERTY(PlayModes playMode READ playMode WRITE setPlayMode NOTIFY playModeChanged)
    Q_PROPERTY(ShuffleModes shuffleMode READ shuffleMode WRITE setShuffleMode NOTIFY shuffleModeChanged)
    Q_PROPERTY(RepeatModes repeatMode READ repeatMode WRITE setRepeatMode NOTIFY repeatModeChanged)

    Q_PROPERTY (bool hdmiAudio READ hdmiAudio NOTIFY hdmiAudioChanged)
    Q_PROPERTY (bool cec READ cec NOTIFY cecChanged)

    Q_PROPERTY (bool musicOptimizer READ musicOptimizer NOTIFY musicOptimizerChanged)
    Q_PROPERTY (bool phaseMatchingBass READ phaseMatchingBass NOTIFY phaseMatchingBassChanged)
    Q_PROPERTY (Audyssey2EQ audyssey2EQ READ audyssey2EQ WRITE setAudyssey2EQ NOTIFY audyssey2EQChanged)
    Q_PROPERTY (AudysseyDynamicEQ audysseyDynamicEQ READ audysseyDynamicEQ WRITE setAudysseyDynamicEQ NOTIFY audysseyDynamicEQChanged)
    Q_PROPERTY (AudysseyDynamicVolume audysseyDynamicVolume READ audysseyDynamicVolume WRITE setAudysseyDynamicVolume NOTIFY audysseyDynamicVolumeChanged)

    Q_PROPERTY (int sleepTimer READ sleepTimer NOTIFY sleepTimerChanged)

    Q_PROPERTY (bool discovering READ discovering NOTIFY discoveringChanged)

    Q_INVOKABLE void discoverHosts(bool clear=true);
    Q_INVOKABLE void discoverHostsCancel();

    Q_INVOKABLE QVariantList getDevices() const;
    Q_INVOKABLE QVariantList getStaticInputs() const;
    Q_INVOKABLE QVariantList getInputs() const;    
    Q_INVOKABLE QVariantList getZones() const;    
    Q_INVOKABLE QVariantList getControls() const;
    Q_INVOKABLE QVariantList getNetworkSources() const;

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
    Q_INVOKABLE bool tuneStorePreset(int ml);
    Q_INVOKABLE void tunerDisplayRDSToggle();

    Q_INVOKABLE void bassLevelUp(Zones zone=Zone1);
    Q_INVOKABLE void bassLevelDown(Zones zone=Zone1);

    Q_INVOKABLE void trebleLevelUp(Zones zone=Zone1);
    Q_INVOKABLE void trebleLevelDown(Zones zone=Zone1);

    Q_INVOKABLE void subwooferLevelUp();
    Q_INVOKABLE void subwooferLevelDown();

    Q_INVOKABLE void centerLevelUp();
    Q_INVOKABLE void centerLevelDown();

    Q_INVOKABLE void setCEC(bool m);
    Q_INVOKABLE void setECO(EcoMode m);
    Q_INVOKABLE void setHDMIAudio(bool m);
    Q_INVOKABLE void setMusicOptimizer(bool m);
    Q_INVOKABLE void setPhaseMatchingBass(bool m);
    Q_INVOKABLE void setListeningMode(ListeningModes m);
    Q_INVOKABLE void setLateNightMode(LateNightModes m);

    Q_INVOKABLE void toggleAudyssey2EQ();
    Q_INVOKABLE void toggleAudysseyDynamicEQ();
    Q_INVOKABLE void toggleAudysseyDynamicVolume();

    Q_INVOKABLE void toggleReEQ();
    Q_INVOKABLE void toggleCinemaFilter();
    Q_INVOKABLE void toggleLateNightMode();
    Q_INVOKABLE void toggleListeningModeUp();
    Q_INVOKABLE void toggleListeningModeDown();
    Q_INVOKABLE void toggleListeningModeMusic();
    Q_INVOKABLE void toggleListeningModeMovie();
    Q_INVOKABLE void toggleListeningModeGame();

    Q_INVOKABLE void setZoneMuted(Zone zone, bool m);
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

    // Debugging and logging
    Q_INVOKABLE bool debugLog(QString logfile, bool log);

    Q_INVOKABLE bool networkCommand(Commands cmd);
    Q_INVOKABLE bool tvCommand(Commands cmd);
    Q_INVOKABLE bool dvdCommand(Commands cmd);
    Q_INVOKABLE bool bdCommand(Commands cmd);
    Q_INVOKABLE bool command(Commands cmd, Zones zone=Zone1);
    Q_INVOKABLE bool saveArtwork(QString file);

    Q_INVOKABLE void seekTo(int position);

    Q_INVOKABLE void setNetworkService(NetworkService arg);
    Q_INVOKABLE void setDiscoveryTimeout(int arg);
    Q_INVOKABLE void setPlayMode(PlayModes arg);
    Q_INVOKABLE void setShuffleMode(ShuffleModes arg);
    Q_INVOKABLE void setRepeatMode(RepeatModes arg);

    Q_INVOKABLE void setAudyssey2EQ(Audyssey2EQ arg);
    Q_INVOKABLE void setAudysseyDynamicEQ(AudysseyDynamicEQ arg);
    Q_INVOKABLE void setAudysseyDynamicVolume(AudysseyDynamicVolume arg);

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

    int currentTrackPosition() const { return m_timeRef.secsTo(m_position); }
    int currentTrackLength() const { return m_timeRef.secsTo(m_length); }
    int currentTrack() const { return m_track; }
    int currentTracks() const { return m_tracks; }

    bool hdmiAudio() const { return m_hdmiAudio; }
    bool cec() const { return m_cec; }
    bool musicOptimizer() const { return m_musicOptimizer; }

    bool getDebug() const
    {
        return m_debug;
    }

    NetworkService networkService() const
    {
        return m_networkService;
    }

    int discoveryTimeout() const
    {
        return m_discoveryTimeout;
    }


    bool hasArtwork() const
    {
        return m_artworkParser.complete();
    }

    PlayModes playMode() const
    {
        return m_playMode;
    }

    ShuffleModes shuffleMode() const
    {
        return m_shuffleMode;
    }

    RepeatModes repeatMode() const
    {
        return m_repeatMode;
    }

    int sleepTimer() const
    {
        return m_sleepTimer;
    }

    int masterTunerPreset() const
    {
        return m_masterTunerPreset;
    }

    Audyssey2EQ audyssey2EQ() const
    {
        return m_audyssey2EQ;
    }

    AudysseyDynamicEQ audysseyDynamicEQ() const
    {
        return m_audysseyDynamicEQ;
    }

    AudysseyDynamicVolume audysseyDynamicVolume() const
    {
        return m_audysseyDynamicVolume;
    }

    int networkRadioPreset() const
    {
        return m_networkRadioPreset;
    }

    Zone poweredZones() const
    {
        return m_poweredZones;
    }

    bool phaseMatchingBass() const
    {
        return m_phaseMatchingBass;
    }

    int discovered() const
    {
        return m_discovered;
    }

    void cacheDiscoveredHosts();
    void loadCachedHosts();
    const QString getElapsedTimeString(int seconds) const;
signals:
    void portChanged();
    void hostChanged();
    void discoveringChanged();
    void discoveredHost(QString mac, QVariantMap device);
    void devicesDiscovered();

    void connectedToHost();
    void connectionError(QAbstractSocket::SocketError error);
    void disconnectedFromHost();

    void connectedChanged();
    void debugChanged();
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
    void currentTrackChanged(quint16 track);
    void currentTracksChanged(quint16 tracks);
    void currentArtworkChanged();

    void networkServiceChanged(NetworkService arg);

    void discoveryTimeoutChanged(int arg);
    void hasArtworkChanged(bool arg);
    void playModeChanged(PlayModes arg);
    void shuffleModeChanged(ShuffleModes arg);
    void repeatModeChanged(RepeatModes arg);
    void sleepTimerChanged(int arg);
    void masterTunerPresetChanged(int arg);
    void audyssey2EQChanged(Audyssey2EQ arg);
    void audysseyDynamicEQChanged(AudysseyDynamicEQ arg);
    void audysseyDynamicVolumeChanged(AudysseyDynamicVolume arg);
    void networkRadioPresetChanged(int arg);

    void poweredZonesChanged(Zone arg);

    void phaseMatchingBassChanged(bool arg);

    void discoveredChanged(int arg);

public slots:

    void setDebug(bool arg)
    {
        if (m_debug != arg) {
            m_debug = arg;
            emit debugChanged();
        }
    }

private slots:
    void tcpConnected();
    void tcpDisconnected();
    void tcpError(QAbstractSocket::SocketError se);
    void readISCP();
    void readBroadcastDatagram();    
    void handleCommandQueue();  
    void deviceDiscoveryTimeout();

protected:
    enum DebugLogDirection {
        toNetwork,
        fromNetwork
    };
    void debugLogWrite(DebugLogDirection direction, const ISCPMsg *data);

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
            MasterTunerPreset,
            MasterTone,
            MasterBalance,
            CenterLevel,
            SubwooferLevel,
            SleepTimer,
            // Information
            AudioInformation,
            VideoInformation,
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
            TrackInfo,
            ListInfo,
            DeviceInformation,
            Artwork,
            MenuStatus,
            DeviceStatus,
            MenuList,
            MenuListTitle,
            NetworkRadioPreset,
            CEC,
            HDMIAudio,
            MusicOptimizer,
            PhaseMatchingBass,
            ListeningMode,
            LateNightMode,
            // Audyssey
            Audyssey2EQ,
            AudysseyDynamicEQ,
            AudysseyDynamicVolume,
            // Airplay, these are untested but should work as the data format is same as other network
            AirplayCurrentArtist,
            AirplayCurrentAlbum,
            AirplayCurrentTitle,
            AirplayElapsedTime,
            //
            EndCommands // Marker
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

    QTime m_timeRef;
    QTime m_position;
    QTime m_length;

    PlayModes m_playMode;
    ShuffleModes m_shuffleMode;
    RepeatModes m_repeatMode;

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

    // Audio/Video info
    QVariantList m_audio_info;
    QVariantList m_video_info;

    // Device discovery
    QTimer m_timer;
    QVariantMap m_devices;

    QByteArray m_buffer;      

    QFile m_debuglog;

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
    bool m_debug;
    NetworkService m_networkService;

    ArtworkParser m_artworkParser;

    int m_discoveryTimeout;
    bool m_hasArtwork;
    void parseElapsedTime(QString et);
    void clearCurrentTrack();
    void parseDeviceInformation(QString data);
    void parsePlayStatus(QString data);

    void setTracks(quint16 tracks);
    void setTrack(quint16 track);
    void parseTrackInfo(QString data);
    int m_sleepTimer;
    void setArtwork(QByteArray data);
    void clearArtwork();
    int m_masterTunerPreset;
    Audyssey2EQ m_audyssey2EQ;
    AudysseyDynamicEQ m_audysseyDynamicEQ;
    AudysseyDynamicVolume m_audysseyDynamicVolume;
    void clearAllTrackInformation();
    int m_networkRadioPreset;
    void requestInformationState();
    Zone m_poweredZones;
    bool m_phaseMatchingBass;
    int m_discovered;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(qiscp::Zone)

#endif // QISCP_H
