#include "rtsp_connectionmanager.hpp"
#include "rtsp_connection.hpp"
#include "rtsp_worker.hpp"
#include <algorithm>
#include <map>
#include <xpr/xpr_rtsp.h>
#include <xpr/xpr_url.h>

namespace xpr
{

namespace rtsp
{

// ConnectionManager
//============================================================================
ConnectionManager::ConnectionManager(int id, Port* parent)
    : Port(id, parent)
    , mMaxConnections(32)
    , mMaxWorkers(2)
    , mConnections()
    , mWorkers()
{
    DBG(DBG_L5, "XPR_RTSP: ConnectionManager::ConnectionManager(%d,%p) = %p",
        id, parent, this);
    memset(mConnections, 0, sizeof(mConnections));
    memset(mWorkers, 0, sizeof(mWorkers));
}

ConnectionManager::~ConnectionManager(void)
{
    DBG(DBG_L5, "XPR_RTSP: ConnectionManager::~ConnectionManager(id,%p) = %p",
        id(), parent(), this);
}

int ConnectionManager::isPortValid(int port)
{
    uint32_t major = XPR_RTSP_PORT_MAJOR(port);
    uint32_t streamId = XPR_RTSP_PORT_STREAM(port);
    uint32_t trackId = XPR_RTSP_PORT_TRACK(port);
    if (streamId == XPR_RTSP_PORT_TRACK_ALL ||
        streamId == XPR_RTSP_PORT_TRACK_ANY ||
        streamId == XPR_RTSP_PORT_TRACK_NUL)
        return XPR_TRUE;
    if (trackId == XPR_RTSP_PORT_TRACK_ALL ||
        trackId == XPR_RTSP_PORT_TRACK_ANY ||
        trackId == XPR_RTSP_PORT_TRACK_NUL)
        return XPR_TRUE;
    if (streamId <= mMaxConnections)
        return XPR_TRUE;
    return XPR_FALSE;
}

int ConnectionManager::open(int port, const char* url)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): open(%08X,%s)", this, port,
        url);
    if (isPortValid(port) == XPR_FALSE || url == NULL) {
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    }
    // Open ConnectionManager if XPR_RTSP_PORT_MINOR_NUL
    if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
        return openConnectionManager(url);
    // FIXME: XPR_RTSP_PORT_MINOR_ANY
    return openConnection(port, url);
}

int ConnectionManager::close(int port)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): close(%08X)", this, port);
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    // Close ConnectionManager if XPR_RTSP_PORT_MINOR_NUL
    if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
        return closeConnectionManager();
    return closeConnection(port);
}

int ConnectionManager::start(int port)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): start(%08X)", this, port);
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    // Start ConnectionManager if XPR_RTSP_PORT_MINOR_NUL
    if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
        return startConnectionManager();
    return startConnection(port);
}

int ConnectionManager::stop(int port)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): stop(%08X)", this, port);
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    // Stop ConnectionManager if XPR_RTSP_PORT_MINOR_NUL
    if (XPR_RTSP_PORT_MINOR(port) == XPR_RTSP_PORT_MINOR_NUL)
        return stopConnectionManager();
    return stopConnection(port);
}

int ConnectionManager::pushData(int port, XPR_StreamBlock* stb)
{
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    int minor = XPR_RTSP_PORT_MINOR(port);
    // Not support pushData if minor ==
    //   XPR_RTSP_PORT_MINOR_NUL ||
    //   XPR_RTSP_PORT_MINOR_ALL ||
    //   XPR_RTSP_PORT_MINOR_ANY
    if (minor == XPR_RTSP_PORT_MINOR_NUL || minor == XPR_RTSP_PORT_MINOR_ALL ||
        minor == XPR_RTSP_PORT_MINOR_ANY)
        return XPR_ERR_GEN_NOT_SUPPORT;
    return mConnections[minor]->pushData(port, stb);
}

int ConnectionManager::runTask(int port, TaskId task)
{
    if (isPortValid(port) == XPR_FALSE)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    int minor = XPR_RTSP_PORT_MINOR(port);
    // Not support runTask if minor ==
    //   XPR_RTSP_PORT_MINOR_NUL ||
    //   XPR_RTSP_PORT_MINOR_ALL ||
    //   XPR_RTSP_PORT_MINOR_ANY
    if (minor == XPR_RTSP_PORT_MINOR_NUL || minor == XPR_RTSP_PORT_MINOR_ALL ||
        minor == XPR_RTSP_PORT_MINOR_ANY)
        return XPR_ERR_GEN_NOT_SUPPORT;
    return mConnections[minor]->runTask(port, task);
}

Port* ConnectionManager::getPort(int port)
{
    if (isPortValid(port) == XPR_FALSE)
        return NULL;
    int minor = XPR_RTSP_PORT_MINOR(port);
    return mConnections[minor];
}

Port* ConnectionManager::getMajorPort(int port)
{
    return this;
}

Connection** ConnectionManager::connections(void)
{
    return mConnections;
}

Worker* ConnectionManager::worker(int index) const
{
    return mWorkers[index];
}

Worker** ConnectionManager::workers(void)
{
    return mWorkers;
}

size_t ConnectionManager::getMaxConnections(void) const
{
    return mMaxConnections;
}

void ConnectionManager::setMaxConnections(size_t maxConnections)
{
    mMaxConnections = MIN(maxConnections, XPR_RTSP_PORT_STREAM_MAX);
}

size_t ConnectionManager::getMaxWorkers(void) const
{
    return mMaxWorkers;
}

void ConnectionManager::setMaxWorkers(size_t maxWorkers)
{
    mMaxWorkers = MIN(maxWorkers, XPR_RTSP_MAX_WORKERS);
}

Worker* ConnectionManager::getPreferWorker(int streamId)
{
    return mWorkers[streamId % mMaxWorkers];
}

bool ConnectionManager::isValidStreamId(int streamId)
{
    return (streamId > 0) && (streamId <= mMaxConnections);
}

bool ConnectionManager::isValidStreamTrackId(int streamTrackId)
{
    return false;
}

int ConnectionManager::openConnectionManager(const char* url)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): openConnectionManager(%s)",
        this, url);
    if (activeFlags() != PortFlags::PORT_FLAG_NULL ||
        activeFlags() & PortFlags::PORT_FLAG_CLOSE)
        return XPR_ERR_GEN_BUSY;
    if (activeFlags() & PortFlags::PORT_FLAG_OPEN)
        return XPR_ERR_OK;
    int ret = setupConnectionManager(url);
    if (ret != XPR_ERR_OK)
        return ret;
    // Connection must be setup first
    setupConnections();
    // Worker must be setup after conn
    setupWorkers();
    //
    activeFlags(PortFlags::PORT_FLAG_OPEN);
    return XPR_ERR_OK;
}

int ConnectionManager::closeConnectionManager(void)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): closeConnectionManager()",
        this);
    if (activeFlags() & PortFlags::PORT_FLAG_CLOSE)
        return XPR_ERR_OK;
    // Must be stop first if started
    if ((activeFlags() & PortFlags::PORT_FLAG_START) &&
        (!(activeFlags() & PortFlags::PORT_FLAG_STOP)))
        return XPR_ERR_GEN_BUSY;
    // Connection must be clear first
    clearConnections();
    // Worker must be clear after conn
    clearWorkers();
    // ConnectionManager must be clear last
    clearConnectionManager();
    activeFlags(PortFlags::PORT_FLAG_CLOSE, 0);
    return XPR_ERR_OK;
}

int ConnectionManager::startConnectionManager(void)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): startConnectionManager()",
        this);
    if (!(activeFlags() & PortFlags::PORT_FLAG_OPEN))
        return XPR_ERR_GEN_SYS_NOTREADY;
    if (activeFlags() & PortFlags::PORT_FLAG_START)
        return XPR_ERR_OK;
    for (size_t i = 0; i < mMaxWorkers; i++) {
        if (mWorkers[i]) {
            mWorkers[i]->start();
        }
    }
    activeFlags(PortFlags::PORT_FLAG_START, 0);
    return 0;
}

int ConnectionManager::stopConnectionManager(void)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): stopConnectionManager()",
        this);
    if (!(activeFlags() & PortFlags::PORT_FLAG_START))
        return XPR_ERR_GEN_SYS_NOTREADY;
    if (activeFlags() & PortFlags::PORT_FLAG_STOP)
        return XPR_ERR_OK;
    for (size_t i = 0; i < mMaxWorkers; i++) {
        if (mWorkers[i]) {
            mWorkers[i]->stop();
        }
    }
    activeFlags(PortFlags::PORT_FLAG_STOP, 0);
    return 0;
}

int ConnectionManager::openConnection(int port, const char* url)
{
    int streamId = XPR_RTSP_PORT_STREAM(port);
    Connection* conn = mConnections[streamId];
    if (conn)
        return conn->open(port, url);
    return XPR_ERR_GEN_UNEXIST;
}

int ConnectionManager::closeConnection(int port)
{
    int streamId = XPR_RTSP_PORT_STREAM(port);
    Connection* conn = mConnections[streamId];
    if (conn)
        return conn->close(port);
    return XPR_ERR_GEN_UNEXIST;
}

int ConnectionManager::startConnection(int port)
{
    int streamId = XPR_RTSP_PORT_STREAM(port);
    Connection* conn = mConnections[streamId];
    if (conn)
        return conn->start(port);
    return XPR_ERR_GEN_UNEXIST;
}

int ConnectionManager::stopConnection(int port)
{
    int streamId = XPR_RTSP_PORT_STREAM(port);
    Connection* conn = mConnections[streamId];
    if (conn)
        return conn->stop(port);
    return XPR_ERR_GEN_UNEXIST;
}

int ConnectionManager::setupConnectionManager(const char* url)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): setupConnectionManager(%s)",
        this, url);
    // 解析地址参数
    XPR_Url* u = XPR_UrlParse(url, -1);
    if (u == NULL)
        return XPR_ERR_GEN_ILLEGAL_PARAM;
    mUrl = url;
    // 使用 Query 参数来配置管理器
    configConnectionManager(XPR_UrlGetQuery(u));
    XPR_UrlDestroy(u);
    return XPR_ERR_OK;
}

int ConnectionManager::clearConnectionManager(void)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): clearConnectionManager()",
        this);
    mUrl.clear();
    return XPR_ERR_OK;
}

void ConnectionManager::configConnectionManager(const char* query)
{
    if (query && query[0]) {
        DBG(DBG_L4,
            "XPR_RTSP: ConnectionManager(%p): configuration query: \"%s\"",
            this, query);
        xpr_foreach_s(query, -1, "&",
                      ConnectionManager::handleConnectionManagerConfig, this);
    }
}

void ConnectionManager::configConnectionManager(const char* key,
                                                const char* value)
{
    DBG(DBG_L4, "XPR_RTSP: ConnectionManager(%p): configuration: \"%s\"=\"%s\"",
        this, key, value);
    if (strcmp(key, "maxConnections") == 0)
        setMaxConnections(strtol(value, NULL, 10));
    else if (strcmp(key, "maxWorkers") == 0)
        setMaxWorkers(strtol(value, NULL, 10));
    else {
        DBG(DBG_L4,
            "XPR_RTSP: ConnectionManager(%p): configuration: \"%s\" "
            "unsupported.",
            this, key);
    }
}

void ConnectionManager::setupConnections(void)
{
    for (size_t i = XPR_RTSP_PORT_STREAM_MIN; i <= mMaxConnections; i++) {
        mConnections[i] = new Connection(i, this);
    }
}

void ConnectionManager::clearConnections(void)
{
    for (size_t i = XPR_RTSP_PORT_STREAM_MIN; i <= mMaxConnections; i++) {
        if (mConnections[i]) {
            delete mConnections[i];
            mConnections[i] = NULL;
        }
    }
}

void ConnectionManager::configConnection(int port, const char* query)
{
    // FIXME:
}

void ConnectionManager::configConnection(int port, const char* key,
                                         const char* value)
{
    // FIXME:
}

void ConnectionManager::setupWorkers(void)
{
    for (size_t i = 0; i < mMaxWorkers; i++) {
        mWorkers[i] = new Worker(i, this);
    }
}

void ConnectionManager::clearWorkers(void)
{
    for (size_t i = 0; i < mMaxWorkers; i++) {
        if (mWorkers[i]) {
            delete mWorkers[i];
            mWorkers[i] = NULL;
        }
    }
}

void ConnectionManager::handleConnectionManagerConfig(void* opaque, char* seg)
{
    if (opaque && seg) {
        char* key = NULL;
        char* value = NULL;
        if (xpr_split_to_kv(seg, &key, &value) == XPR_ERR_OK)
            ((ConnectionManager*)opaque)->configConnectionManager(key, value);
    }
}

} // namespace rtsp
} // namespace xpr
