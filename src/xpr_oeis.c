#include <xpr/xpr_oeis.h>
#include <xpr/xpr_url.h>

typedef struct XPR_OEIS_CHUNK_DATA {
    unsigned int length;
    unsigned int offset;
    char* data;
    struct XPR_OEIS_CHUNK_DATA* next;
} XPR_OEIS_CHUNK_DATA;

typedef struct XPR_OEIS_PARAMS {
    char akey[128];
    int chunked;
    char password[128];
    char uid[128];
    char username[128]; 
} XPR_OEIS_PARAMS;

struct XPR_OEIS {
    int socket;
    int status;
    int headerExists;
    XPR_OEIS_CHUNK_DATA* chunks;
    XPR_OEIS_PARAMS params;
    XPR_Url* url_parser;
};

static void CloseOnExec(int socket)
{
    fcntl(socket, F_SETFD, FD_CLOEXEC);
}

static void Connect(int socket, const char* host, int port)
{
    struct sockaddr_in in;
    socklen_t sl = sizeof(in);
    memset(&in, 0, sizeof(in));
    in.sin_family = AF_INET;
    in.sin_addr.s_addr = inet_addr(host);
    in.sin_port = htons(port);
    connect(socket, (const struct sockaddr*)&in, sl);
}

static void EnableBlocking(int socket)
{
    int flags = fcntl(socket, F_GETFL);
    flags &= ~O_NONBLOCK;
    (void)fcntl(socket, F_SETFL, flags);
}

static void DisableBlocking(int socket)
{
    int flags = fcntl(socket, F_GETFL);
    flags |= O_NONBLOCK;
    (void)fcntl(socket, F_SETFL, flags);
}

static int MakeHeader(XPR_OEIS* oeis)
{
}

static int BufferData(XPR_OEIS* oeis, const char* data, int length)
{
}

static int RecvAll(XPR_OEIS* oeis)
{
}

static int SendBufferred(XPR_OEIS* oeis)
{
}

static int Perform(XPR_OEIS* oeis)
{
}

XPR_OEIS* XPR_OEIS_New(const char* url)
{
    XPR_OEIS* oeis = (XPR_OEIS*)calloc(sizeof(*oeis), 1);
    if (oeis) {
        oeis->socket = socket(AF_INET, SOCK_STREAM, IPROTO_TCP);
        oeis->url_parser = XPR_UrlParse(url);
        if (oeis->socket > 0 && oeis->url_parser) {
            CloseOnExec(oeis->socket);
            EnableBlocking(oeis->socket);
            Connect(oeis->socket, XPR_UrlGetHost(oeis->url_parser),
                    XPR_UrlGetPort(oeis->url_parser));
        }
    }
    return oeis;
}

int XPR_OEIS_Destroy(XPR_OEIS* oeis)
{
    if (oeis) {
        if (oeis->socket) {
            close(oeis->socket);
            oeis->socket = -1;
        }
        if (oeis->url_parser) {
            XPR_UrlDestroy(oeis->url_parser);
            oeis->url_parser = 0;
        }
        free(oeis);
    }
}

int XPR_OEIS_SetParam(XPR_OEIS* oeis, XPR_OEIS_PARAM param, const void* data, int length)
{
    switch (param) {
    XPR_OEIS_PARAM_AKEY:
        strcpy(oeis->params.akey, (char*)data);
        break;
    XPR_OEIS_PARAM_CHUNKED:
        oeis->params.chunked = length > 0 ? *(int*)data : (int)data;
        break;
    }
    return 0;
}

int XPR_OEIS_GetParam(XPR_OEIS* oeis, XPR_OEIS_PARAM param, void* buffer, int* size)
{
    return 0;
}

int XPR_OEIS_Post(XPR_OEIS* oeis, const char* data, int length)
{
    int result = 0;
    result = BufferData(oeis, data, length);
    if (!result)
        return Perform(oeis);
    return result;
}

int XPR_OEIS_Tick(XPR_OEIS* oeis)
{
    return Perform(oeis);
}
