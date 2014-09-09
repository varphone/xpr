#ifndef XPR_URL_H
#define XPR_URL_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_URL_TYPE_DEFINED
#define XPR_URL_TYPE_DEFINED
struct XPR_Url;
typedef struct XPR_Url XPR_Url;
#endif // XPR_URL_TYPE_DEFINED

/// @brief Create Parser and Parse url
XPR_Url* XPR_UrlParse(const char* url, int length);

/// @brief Destroy a parser
void XPR_UrlDestroy(XPR_Url* url);

/// @brief Get Protocol
const char* XPR_UrlGetProtocol(const XPR_Url* url);

/// @brief Get Protocol major name
const char* XPR_UrlGetProtocolMajor(const XPR_Url* url);

/// @brief Get Protocol minor name
const char* XPR_UrlGetProtocolMinor(const XPR_Url* url);

/// @brief Get Username
const char* XPR_UrlGetUsername(const XPR_Url* url);

/// @brief Get Password
const char* XPR_UrlGetPassword(const XPR_Url* url);

/// @brief Get Host(optional)
const char* XPR_UrlGetHost(const XPR_Url* url);

/// @brief Get Port(optional)
int XPR_UrlGetPort(const XPR_Url* url);

/// @brief Get Path
const char* XPR_UrlGetPath(const XPR_Url* url);

/// @brief Get Relative Path
const char* XPR_UrlGetRelPath(const XPR_Url* url);

/// @brief Get Query string
const char* XPR_UrlGetQuery(const XPR_Url* url);

/// @brief Check if have protocol field
int XPR_UrlHaveProtocol(const XPR_Url* url);

/// @brief Check if have protocol major field
int XPR_UrlHaveProtocolMajor(const XPR_Url* url);

/// @brief Check if have protocol minor field
int XPR_UrlHaveProtocolMinor(const XPR_Url* url);

/// @brief Check if have username field
int XPR_UrlHaveUsername(const XPR_Url* url);

/// @brief Check if have password field
int XPR_UrlHavePassword(const XPR_Url* url);

/// @brief Check if have host field
int XPR_UrlHaveHost(const XPR_Url* url);

/// @brief Check if have port field
int XPR_UrlHavePort(const XPR_Url* url);

/// @brief Check if have path field
int XPR_UrlHavePath(const XPR_Url* url);

/// @brief Check if have query field
int XPR_UrlHaveQuery(const XPR_Url* url);

/// @brief Replace the procotol field
void XPR_UrlReplaceProtocol(XPR_Url* url, const char* value);

/// @brief Replace the username field
void XPR_UrlReplaceUsername(XPR_Url* url, const char* value);

/// @brief Replace the password field
void XPR_UrlReplacePassword(XPR_Url* url, const char* value);

/// @brief Replace the host field
void XPR_UrlReplaceHost(XPR_Url* url, const char* value);

/// @brief Replace the port field
void XPR_UrlReplacePort(XPR_Url* url, int value);

/// @brief Replace the path field
void XPR_UrlReplacePath(XPR_Url* url, const char* value);

/// @brief Replace the query field
void XPR_UrlReplaceQuery(XPR_Url* url, const char* value);

/// @brief Get Full string
const char* XPR_UrlGetFullString(XPR_Url* url);

#ifdef __cplusplus
}
#endif

#endif // XPR_URL_H

