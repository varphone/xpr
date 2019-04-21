#ifndef XPR_RTSP_DRIVER_LIVE_RTSP_CONNECTIONMANAGER_HPP
#define XPR_RTSP_DRIVER_LIVE_RTSP_CONNECTIONMANAGER_HPP

#include "rtsp.hpp"
#include <stdint.h>
#include <string>

namespace xpr
{

namespace rtsp
{

class Connection;
class Worker;

// RTSP 连接管理器
class ConnectionManager : public Port
{
public:
    ConnectionManager(int id, Port* parent);
    virtual ~ConnectionManager(void);

    // Port interfaces
    virtual int isPortValid(int port);
    virtual int open(int port, const char* url);
    virtual int close(int port);
    virtual int start(int port);
    virtual int stop(int port);
    virtual int pushData(int port, XPR_StreamBlock* stb);
    virtual int runTask(int port, TaskId task);
    virtual Port* getPort(int port);
    virtual Port* getMajorPort(int port);

    // Properties
    Connection** connections(void);
    Worker* worker(int index) const;
    Worker** workers(void);

    // Methods
    size_t getMaxConnections(void) const;
    void setMaxConnections(size_t maxConnections);

    size_t getMaxWorkers(void) const;
    void setMaxWorkers(size_t maxWorkers);

    Worker* getPreferWorker(int streamId);

    bool isValidStreamId(int streamId);
    bool isValidStreamTrackId(int streamTrackId);

private:
    int openConnectionManager(const char* url);
    int closeConnectionManager(void);

    int startConnectionManager(void);
    int stopConnectionManager(void);

    int openConnection(int port, const char* url);
    int closeConnection(int port);

    int startConnection(int port);
    int stopConnection(int port);

    int setupConnectionManager(const char* url);
    int clearConnectionManager(void);
    void configConnectionManager(const char* query);
    void configConnectionManager(const char* key, const char* value);

    void setupConnections(void);
    void clearConnections(void);
    void configConnection(int port, const char* query);
    void configConnection(int port, const char* key, const char* value);

    void setupWorkers(void);
    void clearWorkers(void);

    static void handleConnectionManagerConfig(void* opaque, char* seg);

private:
    size_t              mMaxConnections;
    size_t              mMaxWorkers;
    Connection*         mConnections[XPR_RTSP_PORT_MINOR_MAX + 2];
    Worker*             mWorkers[XPR_RTSP_MAX_WORKERS];
};

} // namespace xpr::rtsp
} // namespace xpr

#endif // XPR_RTSP_DRIVER_LIVE_RTSP_CONNECTIONMANAGER_HPP