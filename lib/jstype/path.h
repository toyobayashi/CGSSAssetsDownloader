#ifndef __PATH_H__
#define __PATH_H__

#include "JSString.h"

// Alphabet chars.
#define CHAR_UPPERCASE_A 65 /* A */
#define CHAR_LOWERCASE_A 97 /* a */
#define CHAR_UPPERCASE_Z 90 /* Z */
#define CHAR_LOWERCASE_Z 122 /* z */

// Non-alphabetic chars.
#define CHAR_DOT 46 /* . */
#define CHAR_FORWARD_SLASH 47 /* / */
#define CHAR_BACKWARD_SLASH 92 /* \ */
#define CHAR_VERTICAL_LINE 124 /* | */
#define CHAR_COLON 58 /* : */
#define CHAR_QUESTION_MARK 63 /* ? */
#define CHAR_UNDERSCORE 95 /* _ */
#define CHAR_LINE_FEED 10 /* \n */
#define CHAR_CARRIAGE_RETURN 13 /* \r */
#define CHAR_TAB 9 /* \t */
#define CHAR_FORM_FEED 12 /* \f */
#define CHAR_EXCLAMATION_MARK 33 /* ! */
#define CHAR_HASH 35 /* # */
#define CHAR_SPACE 32 /*   */
#define CHAR_NO_BREAK_SPACE 160 /* \u00A0 */
#define CHAR_ZERO_WIDTH_NOBREAK_SPACE 65279 /* \uFEFF */
#define CHAR_LEFT_SQUARE_BRACKET 91 /* [ */
#define CHAR_RIGHT_SQUARE_BRACKET 93 /* ] */
#define CHAR_LEFT_ANGLE_BRACKET 60 /* < */
#define CHAR_RIGHT_ANGLE_BRACKET 62 /* > */
#define CHAR_LEFT_CURLY_BRACKET 123 /* { */
#define CHAR_RIGHT_CURLY_BRACKET 125 /* } */
#define CHAR_HYPHEN_MINUS 45 /* - */
#define CHAR_PLUS 43 /* + */
#define CHAR_DOUBLE_QUOTE 34 /* " */
#define CHAR_SINGLE_QUOTE 39 /* ' */
#define CHAR_PERCENT 37 /* % */
#define CHAR_SEMICOLON 59 /* ; */
#define CHAR_CIRCUMFLEX_ACCENT 94 /* ^ */
#define CHAR_GRAVE_ACCENT 96 /* ` */
#define CHAR_AT 64 /* @ */
#define CHAR_AMPERSAND 38 /* & */
#define CHAR_EQUAL 61 /* = */

// Digits
#define CHAR_0 48 /* 0 */
#define CHAR_9 57 /* 9 */

#ifdef _WIN32
#define EOL "\r\n"
#else
#define EOL "n"
#endif

class path {
public:
  typedef struct FormatInputPathObject {
    String dir;
    String root;
    String base;
    String name;
    String ext;
  } FormatInputPathObject;
  virtual ~path();
  static String delimiter();
  static String sep();
  static String basename(const String&);
  static String basename(const String&, const String&);
  static String dirname(const String&);
  static String extname(const String&);
  static String normalize(const String&);
  static bool isAbsolute(const String&);
  static String relative(String, String);
  static FormatInputPathObject parse(const String&);
  static String format(const FormatInputPathObject&);
  static String toNamespacedPath(const String&);

  static String resolve(const String& arg = "", const String& arg1 = "");

  template <typename... Args>
  static String resolve(const String& arg1, const String& arg2, Args... args) {
    String tmp = resolve(arg1, arg2);
    return resolve(tmp, args...);
  }

  static String join(const String&, const String& arg2 = "");

  template <typename... Args>
  static String join(const String& arg1, const String& arg2, Args... args) {
    String tmp = join(arg1, arg2);
    return join(tmp, args...);
  }

private:
  path();
  static String _sep;
  static String _delimiter;
  static bool _isPathSeparator(int);
  static bool _isPosixPathSeparator(int);
  static bool _isWindowsDeviceRoot(int);
  static String _normalizeString(const String&, bool, const String&, bool (*)(int));
  static String _format(const String&, const FormatInputPathObject&);
};

#endif
