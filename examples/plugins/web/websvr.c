// This file is part of the Mongoose project, http://code.google.com/p/mongoose
// It implements an online chat server. For more details,
// see the documentation on the project web site.
// To test the application,
// 1. type "make" in the directory where this file lives
// 2. point your browser to http://127.0.0.1:8081

/*#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <pthread.h>
#include <expat.h>
//#include <dbconf/dbconf.h>
#include "mongoose.h"
*/
#include "Config.h"



#define MAX_USER_LEN  20
#define MAX_MESSAGE_LEN  100
#define MAX_MESSAGES 5
#define MAX_SESSIONS 2
#define SESSION_TTL 120

//dbco_t    dbc = 0;
static const char* authorize_url = "/authorize";
static const char* login_url = "/doc/page/login.asp";
static const char* ajax_reply_start =
    "HTTP/1.1 200 OK\r\n"
    "Cache: no-cache\r\n"
    "Content-Type: application/x-javascript\r\n"
    "\r\n";

// Describes single message sent to a chat. If user is empty (0 length),
// the message is then originated from the server itself.
struct message {
    long id;                     // Message ID
    char user[MAX_USER_LEN];     // User that have sent the message
    char text[MAX_MESSAGE_LEN];  // Message text
    time_t timestamp;            // Message timestamp, UTC
};

// Describes web session.
struct session {
    char session_id[33];      // Session ID, must be unique
    char random[20];          // Random data used for extra user validation
    char user[MAX_USER_LEN];  // Authenticated user
    time_t expire;            // Expiration timestamp, UTC
};

static struct message messages[MAX_MESSAGES];  // Ringbuffer for messages
static struct session sessions[MAX_SESSIONS];  // Current sessions
static long last_message_id;

// Protects messages, sessions, last_message_id
static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

// Get session object for the connection. Caller must hold the lock.
static struct session* get_session(const struct mg_connection* conn) {
    int i;
    const char* cookie = mg_get_header(conn, "Cookie");
    char session_id[33];
    time_t now = time(NULL);
    mg_get_cookie(cookie, "session", session_id, sizeof(session_id));
    for (i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].expire != 0 &&
            sessions[i].expire > now &&
            strcmp(sessions[i].session_id, session_id) == 0) {
            break;
        }
    }
    return i == MAX_SESSIONS ? NULL :& sessions[i];
}

static void get_qsvar(const struct mg_request_info* request_info,
                      const char* name, char* dst, size_t dst_len)
{
    const char* qs = request_info->query_string;
    mg_get_var(qs, strlen(qs == NULL ? "" : qs), name, dst, dst_len);
}

// Get a get of messages with IDs greater than last_id and transform them
// into a JSON string. Return that string to the caller. The string is
// dynamically allocated, caller must free it. If there are no messages,
// NULL is returned.
static char* messages_to_json(long last_id)
{
    const struct message* message;
    int max_msgs, len;
    char buf[sizeof(messages)];  // Large enough to hold all messages
    // Read-lock the ringbuffer. Loop over all messages, making a JSON string.
    pthread_rwlock_rdlock(&rwlock);
    len = 0;
    max_msgs = sizeof(messages) / sizeof(messages[0]);
    // If client is too far behind, return all messages.
    if (last_message_id - last_id > max_msgs) {
        last_id = last_message_id - max_msgs;
    }
    for (; last_id < last_message_id; last_id++) {
        message = &messages[last_id % max_msgs];
        if (message->timestamp == 0) {
            break;
        }
        // buf is allocated on stack and hopefully is large enough to hold all
        // messages (it may be too small if the ringbuffer is full and all
        // messages are large. in this case asserts will trigger).
        len += snprintf(buf + len, sizeof(buf) - len,
                        "{user: '%s', text: '%s', timestamp: %lu, id: %lu},",
                        message->user, message->text, message->timestamp, message->id);
        assert(len > 0);
        assert((size_t) len < sizeof(buf));
    }
    pthread_rwlock_unlock(&rwlock);
    return len == 0 ? NULL : strdup(buf);
}

// If "callback" param is present in query string, this is JSONP call.
// Return 1 in this case, or 0 if "callback" is not specified.
// Wrap an output in Javascript function call.
static int handle_jsonp(struct mg_connection* conn,
                        const struct mg_request_info* request_info)
{
    char cb[64];
    get_qsvar(request_info, "callback", cb, sizeof(cb));
    if (cb[0] != '\0') {
        mg_printf(conn, "%s(", cb);
    }
    return cb[0] == '\0' ? 0 : 1;
}

// A handler for the /ajax/get_messages endpoint.
// Return a list of messages with ID greater than requested.
static void ajax_get_messages(struct mg_connection* conn,
                              const struct mg_request_info* request_info)
{
    char last_id[32], *json;
    int is_jsonp;
    mg_printf(conn, "%s", ajax_reply_start);
    is_jsonp = handle_jsonp(conn, request_info);
    get_qsvar(request_info, "last_id", last_id, sizeof(last_id));
    if ((json = messages_to_json(strtoul(last_id, NULL, 10))) != NULL) {
        mg_printf(conn, "[%s]", json);
        free(json);
    }
    if (is_jsonp) {
        mg_printf(conn, "%s", ")");
    }
}

// Allocate new message. Caller must hold the lock.
static struct message* new_message(void) {
    static int size = sizeof(messages) / sizeof(messages[0]);
    struct message* message = &messages[last_message_id % size];
    message->id = last_message_id++;
    message->timestamp = time(0);
    return message;
}

static void my_strlcpy(char* dst, const char* src, size_t len)
{
    strncpy(dst, src, len);
    dst[len - 1] = '\0';
}

// A handler for the /ajax/send_message endpoint.
static void ajax_send_message(struct mg_connection* conn,
                              const struct mg_request_info* request_info)
{
    struct message* message;
    struct session* session;
    char text[sizeof(message->text) - 1];
    int is_jsonp;
    mg_printf(conn, "%s", ajax_reply_start);
    is_jsonp = handle_jsonp(conn, request_info);
    get_qsvar(request_info, "text", text, sizeof(text));
    if (text[0] != '\0') {
        // We have a message to store. Write-lock the ringbuffer,
        // grab the next message and copy data into it.
        pthread_rwlock_wrlock(&rwlock);
        message = new_message();
        // TODO(lsm): JSON-encode all text strings
        session = get_session(conn);
        assert(session != NULL);
        my_strlcpy(message->text, text, sizeof(text));
        my_strlcpy(message->user, session->user, sizeof(message->user));
        pthread_rwlock_unlock(&rwlock);
    }
    mg_printf(conn, "%s", text[0] == '\0' ? "false" : "true");
    if (is_jsonp) {
        mg_printf(conn, "%s", ")");
    }
}

// Redirect user to the login form. In the cookie, store the original URL
// we came from, so that after the authorization we could redirect back.
static void redirect_to_login(struct mg_connection* conn,
                              const struct mg_request_info* request_info)
{
    mg_printf(conn, "HTTP/1.1 302 Found\r\n"
              "Set-Cookie: original_url=%s\r\n"
              "Location: %s\r\n\r\n",
              request_info->uri, login_url);
}

// Return 1 if username/password is allowed, 0 otherwise.
static int check_password(const char* user, const char* password)
{
    // In production environment we should ask an authentication system
    // to authenticate the user.
    // Here however we do trivial check that user and password are not empty
    return (user[0] && password[0]);
}

// Allocate new session object
static struct session* new_session(void) {
    int i;
    time_t now = time(NULL);
    pthread_rwlock_wrlock(&rwlock);
    for (i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].expire == 0 || sessions[i].expire < now) {
            sessions[i].expire = time(0) + SESSION_TTL;
            break;
        }
    }
    pthread_rwlock_unlock(&rwlock);
    return i == MAX_SESSIONS ? NULL :& sessions[i];
}

// Generate session ID. buf must be 33 bytes in size.
// Note that it is easy to steal session cookies by sniffing traffic.
// This is why all communication must be SSL-ed.
static void generate_session_id(char* buf, const char* random,
                                const char* user)
{
    mg_md5(buf, random, user, NULL);
}

static void send_server_message(const char* fmt, ...)
{
    va_list ap;
    struct message* message;
    pthread_rwlock_wrlock(&rwlock);
    message = new_message();
    message->user[0] = '\0';  // Empty user indicates server message
    va_start(ap, fmt);
    vsnprintf(message->text, sizeof(message->text), fmt, ap);
    va_end(ap);
    pthread_rwlock_unlock(&rwlock);
}

// A handler for the /authorize endpoint.
// Login page form sends user name and password to this endpoint.
static void authorize(struct mg_connection* conn,
                      const struct mg_request_info* request_info)
{
    /* char user[MAX_USER_LEN], password[MAX_USER_LEN];
     struct session *session;

     // Fetch user name and password.
     get_qsvar(request_info, "user", user, sizeof(user));
     get_qsvar(request_info, "password", password, sizeof(password));

     if (check_password(user, password) && (session = new_session()) != NULL) {
       // Authentication success:
       //   1. create new session
       //   2. set session ID token in the cookie
       //   3. remove original_url from the cookie - not needed anymore
       //   4. redirect client back to the original URL
       //
       // The most secure way is to stay HTTPS all the time. However, just to
       // show the technique, we redirect to HTTP after the successful
       // authentication. The danger of doing this is that session cookie can
       // be stolen and an attacker may impersonate the user.
       // Secure application must use HTTPS all the time.
       my_strlcpy(session->user, user, sizeof(session->user));
       snprintf(session->random, sizeof(session->random), "%d", rand());
       generate_session_id(session->session_id, session->random, session->user);
       send_server_message("<%s> joined", session->user);
       mg_printf(conn, "HTTP/1.1 302 Found\r\n"
           "Set-Cookie: session=%s; max-age=3600; http-only\r\n"  // Session ID
           "Set-Cookie: user=%s\r\n"  // Set user, needed by Javascript code
           "Set-Cookie: original_url=/; max-age=0\r\n"  // Delete original_url
           "Location: /\r\n\r\n",
           session->session_id, session->user);
     } else {
       // Authentication failure, redirect to login.
       redirect_to_login(conn, request_info);
     }*/
}

// Return 1 if request is authorized, 0 otherwise.
static int is_authorized(const struct mg_connection* conn,
                         const struct mg_request_info* request_info)
{
    struct session* session;
    char valid_id[33];
    int authorized = 0;
    // Always authorize accesses to login page and to authorize URI
    if (!strcmp(request_info->uri, login_url) ||
        !strcmp(request_info->uri, authorize_url)) {
        return 1;
    }
    pthread_rwlock_rdlock(&rwlock);
    if ((session = get_session(conn)) != NULL) {
        generate_session_id(valid_id, session->random, session->user);
        if (strcmp(valid_id, session->session_id) == 0) {
            session->expire = time(0) + SESSION_TTL;
            authorized = 1;
        }
    }
    pthread_rwlock_unlock(&rwlock);
    return 0;//authorized;
}

static void redirect_to_ssl(struct mg_connection* conn,
                            const struct mg_request_info* request_info)
{
    const char* p, *host = mg_get_header(conn, "Host");
    if (host != NULL && (p = strchr(host, ':')) != NULL) {
        mg_printf(conn, "HTTP/1.1 302 Found\r\n"
                  "Location: https://%.*s:8082/%s:8082\r\n\r\n",
                  (int)(p - host), host, request_info->uri);
    } else {
        mg_printf(conn, "%s", "HTTP/1.1 500 Error\r\n\r\nHost: header is not set");
    }
}

extern int handle_psia_request(struct mg_connection* conn,
                               const struct mg_request_info* request_info, XML_Parser p);
#if defined(BOARD_MINOR_RY1208)
extern int handle_custom_request(struct mg_connection* conn,
                               const struct mg_request_info* request_info, XML_Parser p);
#endif
static int event_handler(struct mg_event* event)
{
    struct mg_request_info* request_info = event->request_info;
    struct mg_connection* conn = event->conn;
    int result = 0;
    XML_Parser p = XML_ParserCreate(NULL);
	printf("result--: %d, request_info->uri: %s\n", result, request_info->uri);
    if (event->type != MG_REQUEST_BEGIN) return 0;
    if (strncmp(request_info->uri, "/Custom/", 8) == 0)
        printf("------------\n");
    //if (!request_info->is_ssl) {
    //  redirect_to_ssl(conn, request_info);
    //} else if (!is_authorized(conn, request_info)) {
    //  redirect_to_login(conn, request_info);
    //} else if (strcmp(request_info->uri, authorize_url) == 0) {
    //  authorize(conn, request_info);
    if (strncmp(request_info->uri, "/PSIA/", 6) == 0) {
        result = handle_psia_request(conn, request_info, p);
#if defined(BOARD_MINOR_RY1208)
    } else if (strncmp(request_info->uri, "/Custom/", 8) == 0) {
        printf("**********\n");
        printf("query_string %s\n",request_info->query_string);
        result = handle_custom_request(conn, request_info, p);
#endif
    } else {
        // No suitable handler found, mark as not processed. Mongoose will
        // try to serve the request.
        result = 0;
    }
    if (event->type == MG_REQUEST_BEGIN) {
        printf("mg_requset!!!\n");
        if (!strncmp(request_info->uri, "/upload", 7)) {
            char path[200];
            // Make sure the directory exists
            system("mkdir -p /tmp/fw");
            FILE* fp = mg_upload(conn, "/tmp/fw/", path, sizeof(path));
            if (fp != NULL) {
                fclose(fp);
               // system("tar -czf /tmp/fw.tar.gz /tmp/A5S.*");
                        mg_printf(conn, "HTTP/1.0 200 OK\r\n\r\nSaved: [%s]", path);
                //system("/usr/scripts/fw.upgrade /tmp/fw.tar.gz");
                mg_printf(conn, "HTTP/1.0 200 OK\r\n\r\n");
            } else {
                mg_printf(conn, "%s", "HTTP/1.0 200 OK\r\n\r\nNo files sent");
            }
            result = 1;
        } /*else {
      // Show HTML form. Make sure it has enctype="multipart/form-data" attr.
      static const char *html_form =
        "<html><body>Upload example."
        "<form method=\"POST\" action=\"/upload\" "
        "  enctype=\"multipart/form-data\">"
        "<input type=\"file\" name=\"file\" /> <br/>"
        "<input type=\"submit\" value=\"Upload\" />"
        "</form></body></html>";

      mg_printf(event->conn, "HTTP/1.0 200 OK\r\n"
          "Content-Length: %d\r\n"
          "Content-Type: text/html\r\n\r\n%s",
          (int) strlen(html_form), html_form);
    }*/
    }
    printf("result: %d, request_info->uri: %s\n", result, request_info->uri);
    XML_ParserFree(p);
    return result;
}

static const char* options[] = {
    //"document_root", ".",
    "document_root", "/webSvr/",
    "listening_ports", "80",
    "num_threads", "1",
    "error_log_file", "error.log",
    "global_auth_file", "/webSvr/.htpasswd",
    "authentication_domain", "realm",
    "extra_mime_types", ".asp=text/html",
    NULL
};

static void wait_key(int ch)
{
    while (getchar() != ch);
}

/*
#if defined(BOARD_MINOR_RY1208)
extern int IrCutInit();
extern int Network_Init();
#endif
static int webInit()
{
    DeviceInfoInit();
    ImageConfigurationInit();
    StreamsInit();
    NetWorkInit();
    UserInit();
    OsdInit();
    EventInit();
    PTZRS485Init();
    system("cp /tmp/snapshot.jpeg /webSvr/doc/images/config/tt.jpeg");
    initGepgrahic();
#if defined(BOARD_MINOR_RY1208) //此次有可能需要改变
    IrCutInit();
    Network_Init();
#endif
    return 0;
}
*/
struct msg_t {
    int enable;
    int noChangeDir;
    int noCloseFile;
};

static struct msg_t msg_daemon;

static void printUsage()
{
    //TODO:
}

static int init_param(int argc, char** argv)
{
    msg_daemon.enable = 1;
    msg_daemon.noChangeDir = 0;
    msg_daemon.noCloseFile = 0;
    int opt = 0;
    while ((opt = getopt(argc, argv, "dcCV:hv")) != -1) {
        //printf("opt :%s",opt);
        switch (opt) {
        case 'd':
            msg_daemon.enable = 0;
            break;
        case 'c':
            msg_daemon.noChangeDir = 1;
            break;
        case 'C':
            msg_daemon.noCloseFile = 1;
            break;
        case 'V':
            //MF_set_log_level(strtol(optarg, NULL, 0));
            break;
        case 'h':
            printUsage();
            return 1;
        case 'v':
            //printVersion();
            return 1;
        default:
            printUsage();
            return -1;
        }
    }
    return 0;
}
#if 0
int main(int argc, char** argv)
{
    if (init_param(argc, argv) < 0)
        return -1;
    if (msg_daemon.enable) {
        if (daemon(0, 0) != 0) {
            printf("error :%d\n",errno);
            return (-1);
        }
    }
    // Open global configuration database
    /*dbc = dbc_open("/ambarella/onvifs.db");
    if (!dbc) {
        return -1;
    }*/
    struct mg_context* ctx;
    // Initialize random number generator. It will be used later on for
    // the session identifier creation.
    srand((unsigned) time(0));
    //mg_set_auth_handler(server, auth_handler);
    //webInit();
    // Setup and start Mongoose
    if ((ctx = mg_start(options, event_handler, NULL)) == NULL) {
        printf("%s\n", "Cannot start chat server, fatal exit");
        //exit(EXIT_FAILURE);
    }
    // Wait until enter is pressed, then exit
    printf("Chat server started on ports %s, press 'q' to quit.\n",
           mg_get_option(ctx, "listening_ports"));
//    wait_key('q');
    while(1)
        sleep(60);
    mg_stop(ctx);
    printf("%s\n", "Chat server stopped.");
    //uninit();
    //dbc_close(dbc);
    return EXIT_SUCCESS;
}
#endif

static struct mg_context* ctx;
static void websvr_init()
{
    // Initialize random number generator. It will be used later on for
    // the session identifier creation.
    srand((unsigned) time(0));
	int result = 0;

	result = XPR_UPS_Init();
	if(result!=0) {
		printf("XPR_UPS_Init failed err = %d\n", result);
		return 0;
	}
    DeviceInfoInit();
    //mg_set_auth_handler(server, auth_handler);
    //webInit();
    // Setup and start Mongoose
    if ((ctx = mg_start(options, event_handler, NULL)) == NULL) {
        printf("%s\n", "Cannot start chat server, fatal exit");
        exit(EXIT_FAILURE);
    }
    // Wait until enter is pressed, then exit
    printf("Chat server started on ports %s, press 'q' to quit.\n",
           mg_get_option(ctx, "listening_ports"));
//    wait_key('q');
    while(1)
        sleep(60);

}


static void websvr_fini()
{
    mg_stop(ctx);
    XPR_UPS_Fini();
}



static XPR_Plugin plugin = {
    "ex2",
    "websvr",
    0,
    0,
};

XPR_Plugin* XPR_PluginInit(void)
{
    websvr_init();
    return &plugin;
}

int XPR_PluginFini(XPR_Plugin* plugin)
{
    websvr_fini();
    return 0;
}
