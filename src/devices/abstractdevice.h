


class AbstractDevice : public QObject
{
    Q_OBJECT
    Q_ENUMS(Feature)
    
    public:
    enum Feature{
        FEATURE_HRM = 1,
        FEATURE_WEATHER,
        FEATURE_ACTIVITY,
        FEATURE_STEPS,
        FEATURE_ALARMS
    };
    
    explicit AbstracDevice(QObject *parent = 0);
    
    bool supportsFeature(Feature f) = 0;
    QString deviceType() = 0;
    QString deviceName() = 0;
        
    bool operationRunning();
    QString prepareFirmwareDownload(const QString &path);
    void startDownload();
    void downloadSportsData();
    void sendWeather(CurrentWeather *weather);
    
    //signals    
    Q_SIGNAL void message(const QString &text);
    Q_SIGNAL void downloadProgress(int percent);
    Q_SIGNAL void operationRunningChanged();
    Q_SIGNAL void buttonPressed(int presses);

}