#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_regex.h>

static int static_total_tests = 0;
static int static_failed_tests = 0;

#define FAIL(str, line)                                                        \
    do {                                                                       \
        printf("Fail on line %d: [%s]\n", line, str);                          \
        static_failed_tests++;                                                 \
    } while (0)

#define ASSERT(expr)                                                           \
    do {                                                                       \
        static_total_tests++;                                                  \
        if (!(expr))                                                           \
            FAIL(#expr, __LINE__);                                             \
    } while (0)

/* Regex must have exactly one bracket pair */
static char* XPR_RegexReplace(const char* regex, const char* buf,
                              const char* sub)
{
    char* s = NULL;
    int n, n1, n2, n3, s_len, len = strlen(buf);
    XPR_RegexCap cap = {NULL, 0};

    do {
        s_len = s == NULL ? 0 : strlen(s);
        if ((n = XPR_RegexMatch(regex, buf, len, &cap, 1, 0)) > 0) {
            n1 = cap.ptr - buf, n2 = strlen(sub),
            n3 = &buf[n] - &cap.ptr[cap.len];
        }
        else {
            n1 = len, n2 = 0, n3 = 0;
        }
        s = (char*)realloc(s, s_len + n1 + n2 + n3 + 1);
        memcpy(s + s_len, buf, n1);
        memcpy(s + s_len + n1, sub, n2);
        memcpy(s + s_len + n1 + n2, cap.ptr + cap.len, n3);
        s[s_len + n1 + n2 + n3] = '\0';

        buf += n > 0 ? n : len;
        len -= n > 0 ? n : len;
    } while (len > 0);

    return s;
}

int main(void)
{
    XPR_RegexCap caps[10];

    /* Metacharacters */
    ASSERT(XPR_RegexMatch("$", "abcd", 4, NULL, 0, 0) == 4);
    ASSERT(XPR_RegexMatch("^", "abcd", 4, NULL, 0, 0) == 0);
    ASSERT(XPR_RegexMatch("x|^", "abcd", 4, NULL, 0, 0) == 0);
    ASSERT(XPR_RegexMatch("x|$", "abcd", 4, NULL, 0, 0) == 4);
    ASSERT(XPR_RegexMatch("x", "abcd", 4, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch(".", "abcd", 4, NULL, 0, 0) == 1);
    ASSERT(XPR_RegexMatch("^.*\\\\.*$", "c:\\Tools", 8, NULL, 0,
                          XPR_REGEX_FLAG_IGNORE_CASE) == 8);
    ASSERT(XPR_RegexMatch("\\", "a", 1, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_INVALID_METACHARACTER));
    ASSERT(XPR_RegexMatch("\\x", "a", 1, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_INVALID_METACHARACTER));
    ASSERT(XPR_RegexMatch("\\x1", "a", 1, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_INVALID_METACHARACTER));
    ASSERT(XPR_RegexMatch("\\x20", " ", 1, NULL, 0, 0) == 1);

    ASSERT(XPR_RegexMatch("^.+$", "", 0, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("^(.+)$", "", 0, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("^([\\+-]?)([\\d]+)$", "+", 1, caps, 10,
                          XPR_REGEX_FLAG_IGNORE_CASE) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("^([\\+-]?)([\\d]+)$", "+27", 3, caps, 10,
                          XPR_REGEX_FLAG_IGNORE_CASE) == 3);
    ASSERT(caps[0].len == 1);
    ASSERT(caps[0].ptr[0] == '+');
    ASSERT(caps[1].len == 2);
    ASSERT(memcmp(caps[1].ptr, "27", 2) == 0);

    ASSERT(XPR_RegexMatch("tel:\\+(\\d+[\\d-]+\\d)", "tel:+1-201-555-0123;a=b",
                          23, caps, 10, 0) == 19);
    ASSERT(caps[0].len == 14);
    ASSERT(memcmp(caps[0].ptr, "1-201-555-0123", 14) == 0);

    /* Character sets */
    ASSERT(XPR_RegexMatch("[abc]", "1c2", 3, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("[abc]", "1C2", 3, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("[abc]", "1C2", 3, NULL, 0,
                          XPR_REGEX_FLAG_IGNORE_CASE) == 2);
    ASSERT(XPR_RegexMatch("[.2]", "1C2", 3, NULL, 0, 0) == 1);
    ASSERT(XPR_RegexMatch("[\\S]+", "ab cd", 5, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("[\\S]+\\s+[tyc]*", "ab cd", 5, NULL, 0, 0) == 4);
    ASSERT(XPR_RegexMatch("[\\d]", "ab cd", 5, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("[^\\d]", "ab cd", 5, NULL, 0, 0) == 1);
    ASSERT(XPR_RegexMatch("[^\\d]+", "abc123", 6, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch("[1-5]+", "123456789", 9, NULL, 0, 0) == 5);
    ASSERT(XPR_RegexMatch("[1-5a-c]+", "123abcdef", 9, NULL, 0, 0) == 6);
    ASSERT(XPR_RegexMatch("[1-5a-]+", "123abcdef", 9, NULL, 0, 0) == 4);
    ASSERT(XPR_RegexMatch("[1-5a-]+", "123a--2oo", 9, NULL, 0, 0) == 7);
    ASSERT(XPR_RegexMatch("[htps]+://", "https://", 8, NULL, 0, 0) == 8);
    ASSERT(XPR_RegexMatch("[^\\s]+", "abc def", 7, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch("[^fc]+", "abc def", 7, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("[^d\\sf]+", "abc def", 7, NULL, 0, 0) == 3);

    /* Flags - case sensitivity */
    ASSERT(XPR_RegexMatch("FO", "foo", 3, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("FO", "foo", 3, NULL, 0,
                          XPR_REGEX_FLAG_IGNORE_CASE) == 2);
    ASSERT(XPR_RegexMatch("(?m)FO", "foo", 3, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_UNEXPECTED_QUANTIFIER));
    ASSERT(XPR_RegexMatch("(?m)x", "foo", 3, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_UNEXPECTED_QUANTIFIER));

    ASSERT(XPR_RegexMatch("fo", "foo", 3, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch(".+", "foo", 3, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch(".+k", "fooklmn", 7, NULL, 0, 0) == 4);
    ASSERT(XPR_RegexMatch(".+k.", "fooklmn", 7, NULL, 0, 0) == 5);
    ASSERT(XPR_RegexMatch("p+", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("ok", "fooklmn", 7, NULL, 0, 0) == 4);
    ASSERT(XPR_RegexMatch("lmno", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("mn.", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("o", "fooklmn", 7, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("^o", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("^", "fooklmn", 7, NULL, 0, 0) == 0);
    ASSERT(XPR_RegexMatch("n$", "fooklmn", 7, NULL, 0, 0) == 7);
    ASSERT(XPR_RegexMatch("n$k", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("l$", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch(".$", "fooklmn", 7, NULL, 0, 0) == 7);
    ASSERT(XPR_RegexMatch("a?", "fooklmn", 7, NULL, 0, 0) == 0);
    ASSERT(XPR_RegexMatch("^a*CONTROL", "CONTROL", 7, NULL, 0, 0) == 7);
    ASSERT(XPR_RegexMatch("^[a]*CONTROL", "CONTROL", 7, NULL, 0, 0) == 7);
    ASSERT(XPR_RegexMatch("^(a*)CONTROL", "CONTROL", 7, NULL, 0, 0) == 7);
    ASSERT(XPR_RegexMatch("^(a*)?CONTROL", "CONTROL", 7, NULL, 0, 0) == 7);

    ASSERT(XPR_RegexMatch("\\_", "abc", 3, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_INVALID_METACHARACTER));
    ASSERT(XPR_RegexMatch("+", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_UNEXPECTED_QUANTIFIER));
    ASSERT(XPR_RegexMatch("()+", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("\\x", "12", 2, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_INVALID_METACHARACTER));
    ASSERT(XPR_RegexMatch("\\xhi", "12", 2, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_INVALID_METACHARACTER));
    ASSERT(XPR_RegexMatch("\\x20", "_ J", 3, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("\\x4A", "_ J", 3, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch("\\d+", "abc123def", 9, NULL, 0, 0) == 6);

    /* Balancing brackets */
    ASSERT(XPR_RegexMatch("(x))", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_UNBALANCED_BRACKETS));
    ASSERT(XPR_RegexMatch("(", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_UNBALANCED_BRACKETS));

    ASSERT(XPR_RegexMatch("klz?mn", "fooklmn", 7, NULL, 0, 0) == 7);
    ASSERT(XPR_RegexMatch("fa?b", "fooklmn", 7, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));

    /* Brackets & capturing */
    ASSERT(XPR_RegexMatch("^(te)", "tenacity subdues all", 20, caps, 10, 0) ==
           2);
    ASSERT(XPR_RegexMatch("(bc)", "abcdef", 6, caps, 10, 0) == 3);
    ASSERT(XPR_RegexMatch(".(d.)", "abcdef", 6, caps, 10, 0) == 5);
    ASSERT(XPR_RegexMatch(".(d.)\\)?", "abcdef", 6, caps, 10, 0) == 5);
    ASSERT(caps[0].len == 2);
    ASSERT(memcmp(caps[0].ptr, "de", 2) == 0);
    ASSERT(XPR_RegexMatch("(.+)", "123", 3, caps, 10, 0) == 3);
    ASSERT(XPR_RegexMatch("(2.+)", "123", 3, caps, 10, 0) == 3);
    ASSERT(caps[0].len == 2);
    ASSERT(memcmp(caps[0].ptr, "23", 2) == 0);
    ASSERT(XPR_RegexMatch("(.+2)", "123", 3, caps, 10, 0) == 2);
    ASSERT(caps[0].len == 2);
    ASSERT(memcmp(caps[0].ptr, "12", 2) == 0);
    ASSERT(XPR_RegexMatch("(.*(2.))", "123", 3, caps, 10, 0) == 3);
    ASSERT(XPR_RegexMatch("(.)(.)", "123", 3, caps, 10, 0) == 2);
    ASSERT(XPR_RegexMatch("(\\d+)\\s+(\\S+)", "12 hi", 5, caps, 10, 0) == 5);
    ASSERT(XPR_RegexMatch("ab(cd)+ef", "abcdcdef", 8, NULL, 0, 0) == 8);
    ASSERT(XPR_RegexMatch("ab(cd)*ef", "abcdcdef", 8, NULL, 0, 0) == 8);
    ASSERT(XPR_RegexMatch("ab(cd)+?ef", "abcdcdef", 8, NULL, 0, 0) == 8);
    ASSERT(XPR_RegexMatch("ab(cd)+?.", "abcdcdef", 8, NULL, 0, 0) == 5);
    ASSERT(XPR_RegexMatch("ab(cd)?", "abcdcdef", 8, NULL, 0, 0) == 4);
    ASSERT(XPR_RegexMatch("a(b)(cd)", "abcdcdef", 8, caps, 1, 0) ==
           XPR_ERR_USER(XPR_REGEX_CAPS_ARRAY_TOO_SMALL));
    ASSERT(XPR_RegexMatch("(.+/\\d+\\.\\d+)\\.jpg$", "/foo/bar/12.34.jpg", 18,
                          caps, 1, 0) == 18);
    ASSERT(XPR_RegexMatch("(ab|cd).*\\.(xx|yy)", "ab.yy", 5, NULL, 0, 0) == 5);
    ASSERT(XPR_RegexMatch(".*a", "abcdef", 6, NULL, 0, 0) == 1);
    ASSERT(XPR_RegexMatch("(.+)c", "abcdef", 6, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch("\\n", "abc\ndef", 7, NULL, 0, 0) == 4);
    ASSERT(XPR_RegexMatch("b.\\s*\\n", "aa\r\nbb\r\ncc\r\n\r\n", 14, caps, 10,
                          0) == 8);

    /* Greedy vs non-greedy */
    ASSERT(XPR_RegexMatch(".+c", "abcabc", 6, NULL, 0, 0) == 6);
    ASSERT(XPR_RegexMatch(".+?c", "abcabc", 6, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch(".*?c", "abcabc", 6, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch(".*c", "abcabc", 6, NULL, 0, 0) == 6);
    ASSERT(XPR_RegexMatch("bc.d?k?b+", "abcabc", 6, NULL, 0, 0) == 5);

    /* Branching */
    ASSERT(XPR_RegexMatch("|", "abc", 3, NULL, 0, 0) == 0);
    ASSERT(XPR_RegexMatch("|.", "abc", 3, NULL, 0, 0) == 1);
    ASSERT(XPR_RegexMatch("x|y|b", "abc", 3, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("k(xx|yy)|ca", "abcabc", 6, NULL, 0, 0) == 4);
    ASSERT(XPR_RegexMatch("k(xx|yy)|ca|bc", "abcabc", 6, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch("(|.c)", "abc", 3, caps, 10, 0) == 3);
    ASSERT(caps[0].len == 2);
    ASSERT(memcmp(caps[0].ptr, "bc", 2) == 0);
    ASSERT(XPR_RegexMatch("a|b|c", "a", 1, NULL, 0, 0) == 1);
    ASSERT(XPR_RegexMatch("a|b|c", "b", 1, NULL, 0, 0) == 1);
    ASSERT(XPR_RegexMatch("a|b|c", "c", 1, NULL, 0, 0) == 1);
    ASSERT(XPR_RegexMatch("a|b|c", "d", 1, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));

    /* Optional match at the end of the string */
    ASSERT(XPR_RegexMatch("^.*c.?$", "abc", 3, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch("^.*C.?$", "abc", 3, NULL, 0,
                          XPR_REGEX_FLAG_IGNORE_CASE) == 3);
    ASSERT(XPR_RegexMatch("bk?", "ab", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("b(k?)", "ab", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("b[k-z]*", "ab", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("ab(k|z|y)*", "ab", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("[b-z].*", "ab", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("(b|z|u).*", "ab", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("ab(k|z|y)?", "ab", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch(".*", "ab", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch(".*$", "ab", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("a+$", "aa", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("a*$", "aa", 2, NULL, 0, 0) == 2);
    ASSERT(XPR_RegexMatch("a+$", "Xaa", 3, NULL, 0, 0) == 3);
    ASSERT(XPR_RegexMatch("a*$", "Xaa", 3, NULL, 0, 0) == 3);

    /* Ignore case flag */
    ASSERT(XPR_RegexMatch("[a-h]+", "abcdefghxxx", 11, NULL, 0, 0) == 8);
    ASSERT(XPR_RegexMatch("[A-H]+", "ABCDEFGHyyy", 11, NULL, 0, 0) == 8);
    ASSERT(XPR_RegexMatch("[a-h]+", "ABCDEFGHyyy", 11, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("[A-H]+", "abcdefghyyy", 11, NULL, 0, 0) ==
           XPR_ERR_USER(XPR_REGEX_NO_MATCH));
    ASSERT(XPR_RegexMatch("[a-h]+", "ABCDEFGHyyy", 11, NULL, 0,
                          XPR_REGEX_FLAG_IGNORE_CASE) == 8);
    ASSERT(XPR_RegexMatch("[A-H]+", "abcdefghyyy", 11, NULL, 0,
                          XPR_REGEX_FLAG_IGNORE_CASE) == 8);

    {
        /* Example: HTTP request */
        const char* request = " GET /index.html HTTP/1.0\r\n\r\n";
        XPR_RegexCap caps[4];

        if (XPR_RegexMatch("^\\s*(\\S+)\\s+(\\S+)\\s+HTTP/(\\d)\\.(\\d)",
                           request, strlen(request), caps, 4, 0) > 0) {
            printf("Method: [%.*s], URI: [%.*s]\n", caps[0].len, caps[0].ptr,
                   caps[1].len, caps[1].ptr);
        }
        else {
            printf("Error parsing [%s]\n", request);
        }

        ASSERT(caps[1].len == 11);
        ASSERT(memcmp(caps[1].ptr, "/index.html", caps[1].len) == 0);
    }

    {
        /* Example: string replacement */
        char* s = XPR_RegexReplace(
            "({{.+?}})", "Good morning, {{foo}}. How are you, {{bar}}?", "Bob");
        printf("%s\n", s);
        ASSERT(strcmp(s, "Good morning, Bob. How are you, Bob?") == 0);
        free(s);
    }

    {
        /* Example: find all URLs in a string */
        static const char* str =
            "<img src=\"HTTPS://FOO.COM/x?b#c=tab1\"/> "
            "  <a href=\"http://cesanta.com\">some link</a>";

        static const char* regex = "((https?://)[^\\s/'\"<>]+/?[^\\s'\"<>]*)";
        XPR_RegexCap caps[2];
        int i, j = 0, str_len = (int)strlen(str);

        while (j < str_len &&
               (i = XPR_RegexMatch(regex, str + j, str_len - j, caps, 2,
                                   XPR_REGEX_FLAG_IGNORE_CASE)) > 0) {
            printf("Found URL: [%.*s]\n", caps[0].len, caps[0].ptr);
            j += i;
        }
    }

    {
        /* Example more complex regular expression */
        static const char* str = "aa 1234 xy\nxyz";
        static const char* regex = "aa ([0-9]*) *([x-z]*)\\s+xy([yz])";
        XPR_RegexCap caps[3];
        ASSERT(XPR_RegexMatch(regex, str, strlen(str), caps, 3, 0) > 0);
        ASSERT(caps[0].len == 4);
        ASSERT(caps[1].len == 2);
        ASSERT(caps[2].len == 1);
        ASSERT(caps[2].ptr[0] == 'z');
    }

    printf("Unit test %s (total test: %d, failed tests: %d)\n",
           static_failed_tests > 0 ? "FAILED" : "PASSED", static_total_tests,
           static_failed_tests);

    return static_failed_tests == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
