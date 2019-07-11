/*
 * File: xpr_regex.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 正则表达式
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2019-07-11 08:14:23 Thursday, 11 July
 * Last Modified : 2019-07-11 08:14:32 Thursday, 11 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2004-2013 Sergey Lyubka <valenok@gmail.com>
 * Copyright (C) 2012-2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012-2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-11   varphone    初始版本建立
 */
#ifndef XPR_REGEX_H
#define XPR_REGEX_H

#include <stddef.h>
#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_REGEXCAP_TYPE_DEFINED
#define XPR_REGEXCAP_TYPE_DEFINED
/// 捕捉到的分组内容
struct XPR_RegexCap
{
    const char* ptr; ///< 内容指针
    int len;         ///< 内容字节数
};
typedef struct XPR_RegexCap XPR_RegexCap;
#endif // XPR_REGEXCAP_TYPE_DEFINED

#ifndef XPR_REGEXERRNO_TYPE_DEFINED
#define XPR_REGEXERRNO_TYPE_DEFINED
/// 正则表达式匹配错误码
enum XPR_RegexErrno
{
    XPR_REGEX_NO_MATCH = 1,          ///< 没有匹配
    XPR_REGEX_UNEXPECTED_QUANTIFIER, ///< 意外的计量符
    XPR_REGEX_UNBALANCED_BRACKETS,   ///< 不配套的括号
    XPR_REGEX_INTERNAL_ERROR,        ///< 内部错误
    XPR_REGEX_INVALID_CHARACTER_SET, ///< 非法字符集
    XPR_REGEX_INVALID_METACHARACTER, ///< 非法元字符
    XPR_REGEX_CAPS_ARRAY_TOO_SMALL,  ///< 分组数组太小
    XPR_REGEX_TOO_MANY_BRANCHES,     ///< 太多分支
    XPR_REGEX_TOO_MANY_BRACKETS,     ///< 太多括号
};
typedef enum XPR_RegexErrno XPR_RegexErrno;
#endif // XPR_REGEXERRNO_TYPE_DEFINED

#ifndef XPR_REGEXFLAGS_TYPE_DEFINED
#define XPR_REGEXFLAGS_TYPE_DEFINED
/// 正则表达式匹配标志
enum XPR_RegexFlags
{
    XPR_REGEX_FLAG_NONE = 0,        ///< 无
    XPR_REGEX_FLAG_IGNORE_CASE = 1, ///< 忽略大小写匹配
};
typedef enum XPR_RegexFlags XPR_RegexFlags;
#endif // XPR_REGEXFLAGS_TYPE_DEFINED

/// 根据正则表达式 `regexp` 在字符串 `str` 中进行匹配操作，
/// 如果 `regexp` 有分组信息，则将匹配到的分组内容保存到 `caps` 中。
/// @param [in] regexp      正则表达式
/// @param [in] str         要匹配的字符串
/// @param [in] len         要匹配的字符串字节数
/// @param [in,out] caps    接收匹配到的分组内容
/// @param [in] numCaps     接收匹配到的分组内容数量
/// @param [in] flags       正则表达式匹配标志
/// @retval > 0     匹配成功到的内容长度
/// @retval = 0     匹配成功但没任何内容
/// @retval < 0     匹配失败，代码为 XPR_ERR_USER(XPR_RegexErrno)
XPR_API int XPR_RegexMatch(const char* regexp, const char* str, int len,
                           XPR_RegexCap* caps, int numCaps, int flags);

#ifdef __cplusplus
}
#endif

#endif // XPR_REGEX_H
