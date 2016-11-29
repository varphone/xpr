#ifndef XPR_URL_H
#define XPR_URL_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_URL_TYPE_DEFINED
#define XPR_URL_TYPE_DEFINED
struct XPR_Url;
typedef struct XPR_Url XPR_Url;
#endif // XPR_URL_TYPE_DEFINED

/// @brief Create Parser and Parse url
XPR_API XPR_Url* XPR_UrlParse(const char* url, int length);

/// @brief Destroy a parser
XPR_API void XPR_UrlDestroy(XPR_Url* url);

/// @brief Get Protocol
XPR_API const char* XPR_UrlGetProtocol(const XPR_Url* url);

/// @brief Get Protocol major name
XPR_API const char* XPR_UrlGetProtocolMajor(const XPR_Url* url);

/// @brief Get Protocol minor name
XPR_API const char* XPR_UrlGetProtocolMinor(const XPR_Url* url);

/// @brief Get Username
XPR_API const char* XPR_UrlGetUsername(const XPR_Url* url);

/// @brief Get Password
XPR_API const char* XPR_UrlGetPassword(const XPR_Url* url);

/// @brief Get Host(optional)
XPR_API const char* XPR_UrlGetHost(const XPR_Url* url);

/// @brief Get Port(optional)
XPR_API int XPR_UrlGetPort(const XPR_Url* url);

/// @brief Get Path
XPR_API const char* XPR_UrlGetPath(const XPR_Url* url);

/// @brief Get Relative Path
XPR_API const char* XPR_UrlGetRelPath(const XPR_Url* url);

/// @brief Get Query string
XPR_API const char* XPR_UrlGetQuery(const XPR_Url* url);

/// @brief Check if have protocol field
XPR_API int XPR_UrlHaveProtocol(const XPR_Url* url);

/// @brief Check if have protocol major field
XPR_API int XPR_UrlHaveProtocolMajor(const XPR_Url* url);

/// @brief Check if have protocol minor field
XPR_API int XPR_UrlHaveProtocolMinor(const XPR_Url* url);

/// @brief Check if have username field
XPR_API int XPR_UrlHaveUsername(const XPR_Url* url);

/// @brief Check if have password field
XPR_API int XPR_UrlHavePassword(const XPR_Url* url);

/// @brief Check if have host field
XPR_API int XPR_UrlHaveHost(const XPR_Url* url);

/// @brief Check if have port field
XPR_API int XPR_UrlHavePort(const XPR_Url* url);

/// @brief Check if have path field
XPR_API int XPR_UrlHavePath(const XPR_Url* url);

/// @brief Check if have query field
XPR_API int XPR_UrlHaveQuery(const XPR_Url* url);

/// @brief Replace the procotol field
XPR_API void XPR_UrlReplaceProtocol(XPR_Url* url, const char* value);

/// @brief Replace the username field
XPR_API void XPR_UrlReplaceUsername(XPR_Url* url, const char* value);

/// @brief Replace the password field
XPR_API void XPR_UrlReplacePassword(XPR_Url* url, const char* value);

/// @brief Replace the host field
XPR_API void XPR_UrlReplaceHost(XPR_Url* url, const char* value);

/// @brief Replace the port field
XPR_API void XPR_UrlReplacePort(XPR_Url* url, int value);

/// @brief Replace the path field
XPR_API void XPR_UrlReplacePath(XPR_Url* url, const char* value);

/// @brief Replace the query field
XPR_API void XPR_UrlReplaceQuery(XPR_Url* url, const char* value);

/// @brief Get Full string
XPR_API const char* XPR_UrlGetFullString(XPR_Url* url);

#ifdef __cplusplus
}
#endif

#endif // XPR_URL_H

