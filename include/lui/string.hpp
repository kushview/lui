// Copyright 2022 Michael Fisher <mfisher@lvtk.org>
// SPDX-License-Identifier: ISC

#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstring>
#include <format>
#include <string>

namespace lui {

/** A high-performance UTF-8 aware String type.
    
    Can be passed to functions which take const char* or std::string
    as an argument. Supports UTF-8 operations including character counting,
    validation, and proper substring handling.

    @headerfile lui/string.hpp
    @ingroup lvtk
 */
class String final {
public:
    using str_type = std::string;

    /** Create an empty string */
    String() = default;
    /** Create a string from a const char */
    String (const char* str) : _str (str) {}
    /** Create a string from a std::string */
    String (const std::string& o) : _str (o) {}
    /** Copy constructor */
    String (const String& o) : _str (o._str) {}
    /** Move constructor */
    String (std::string&& o) noexcept : _str (std::move (o)) {}
    /** Move constructor from another String */
    String (String&& o) noexcept : _str (std::move (o._str)) {}

    /** Clear this string. */
    void clear() { _str.clear(); }

    /** Return the byte length of the string (not character count). 
        Use charCount() to get the number of UTF-8 code points. */
    size_t length() const noexcept { return _str.length(); }

    /** Return the byte length of the string (not character count). 
        Use charCount() to get the number of UTF-8 code points. */
    size_t size() const noexcept { return _str.size(); }

    /** Return the character count (number of UTF-8 code points). 
        Note: This is different from length() which returns bytes. 
        For multi-byte UTF-8 characters, charCount() will be less than length(). */
    size_t charCount() const noexcept {
        size_t count = 0;
        for (size_t i = 0; i < _str.length(); ++i) {
            unsigned char c = static_cast<unsigned char> (_str[i]);
            // Count bytes that are not continuation bytes (10xxxxxx)
            if ((c & 0xC0) != 0x80) {
                ++count;
            }
        }
        return count;
    }

    /** Check if string is empty. */
    bool empty() const noexcept { return _str.empty(); }

    /** Validate UTF-8 encoding. Returns true if valid. */
    bool valid_utf8() const noexcept {
        for (size_t i = 0; i < _str.length(); ++i) {
            unsigned char c              = static_cast<unsigned char> (_str[i]);
            size_t expected_continuation = 0;

            if ((c & 0x80) == 0) {
                // Single byte character (0xxxxxxx)
                expected_continuation = 0;
            } else if ((c & 0xE0) == 0xC0) {
                // Two byte character (110xxxxx)
                expected_continuation = 1;
            } else if ((c & 0xF0) == 0xE0) {
                // Three byte character (1110xxxx)
                expected_continuation = 2;
            } else if ((c & 0xF8) == 0xF0) {
                // Four byte character (11110xxx)
                expected_continuation = 3;
            } else {
                // Invalid start byte
                return false;
            }

            for (size_t j = 0; j < expected_continuation; ++j) {
                ++i;
                if (i >= _str.length()) {
                    return false;
                }
                unsigned char cont = static_cast<unsigned char> (_str[i]);
                if ((cont & 0xC0) != 0x80) {
                    return false;
                }
            }
        }
        return true;
    }

    // clang-format on

    /** Append another String to this one */
    String& append (const String& o) {
        _str.append (o._str);
        return *this;
    }

    /** Append a std::string to this one */
    String& append (const std::string& o) {
        _str.append (o);
        return *this;
    }

    /** Append a C string to this one */
    String& append (const char* o) {
        _str.append (o);
        return *this;
    }

    /** append a single char. */
    String& append (char c) {
        _str.append (1, c);
        return *this;
    }

    /** Append an int */
    String& append (int i) {
        return append_formatted ("{}", i);
    }

    /** Append an int64 */
    String& append (int64_t i) {
        return append_formatted ("{}", i);
    }

    /** Append a float */
    String& append (float i) {
        return append_formatted ("{}", i);
    }

    /** Append a double */
    String& append (double i) {
        return append_formatted ("{}", i);
    }

    /** Append formatted output using C++20 std::format.
        @param fmt Format string (e.g., "{}, {:.2f}")
        @param args Arguments to format
        @return Reference to this String
        
        Example: str.append_formatted("Value: {}", 42);
    */
    template <typename... Args>
    String& append_formatted (std::format_string<Args...> fmt, Args&&... args) {
        _str.append (std::format (fmt, std::forward<Args> (args)...));
        return *this;
    }

    /** Create a new formatted String using C++20 std::format.
        @param fmt Format string (e.g., "{} items: {:.2f}")
        @param args Arguments to format
        @return A new String with the formatted content
        
        Example: auto msg = String::formatted("{} + {} = {}", 1, 2, 3);
    */
    template <typename... Args>
    static String formatted (std::format_string<Args...> fmt, Args&&... args) {
        return String (std::format (fmt, std::forward<Args> (args)...));
    }

    /** Return substring by byte position and length. 
        Warning: Does not validate UTF-8 boundaries. Use with caution.
        @param start_byte Starting byte position (0-based)
        @param length_bytes Number of bytes to extract
        @return A new String containing the substring, or empty string if out of bounds
    */
    String substring (size_t start_byte, size_t length_bytes) const {
        if (start_byte >= _str.length()) {
            return String();
        }
        return String (_str.substr (start_byte, length_bytes));
    }

    /** Check if this string contains substring. */
    bool contains (const String& substr) const noexcept {
        return _str.find (substr._str) != std::string::npos;
    }

    /** Check if this string contains substring. */
    bool contains (const char* substr) const noexcept {
        return _str.find (substr) != std::string::npos;
    }

    /** Check if string starts with prefix. Case-sensitive. 
        @param prefix The prefix to match
        @return True if this string starts with the given prefix
    */
    bool starts_with (const String& prefix) const noexcept {
        return _str.rfind (prefix._str, 0) == 0;
    }

    /** Check if string starts with prefix. Case-sensitive. 
        @param prefix The prefix to match
        @return True if this string starts with the given prefix
    */
    bool starts_with (const char* prefix) const noexcept {
        return _str.rfind (prefix, 0) == 0;
    }

    /** Check if string ends with suffix. Case-sensitive.
        @param suffix The suffix to match
        @return True if this string ends with the given suffix
    */
    bool ends_with (const String& suffix) const noexcept {
        if (suffix._str.length() > _str.length()) {
            return false;
        }
        return _str.compare (_str.length() - suffix._str.length(), suffix._str.length(), suffix._str) == 0;
    }

    /** Check if string ends with suffix. Case-sensitive.
        @param suffix The suffix to match
        @return True if this string ends with the given suffix
    */
    bool ends_with (const char* suffix) const noexcept {
        size_t suffix_len = std::strlen (suffix);
        if (suffix_len > _str.length()) {
            return false;
        }
        return _str.compare (_str.length() - suffix_len, suffix_len, suffix) == 0;
    }

    /** Return a copy with leading and trailing whitespace removed.
        Removes spaces, tabs, newlines, and other whitespace characters.
        @return A new trimmed String
    */
    String trim() const {
        auto start = _str.begin();
        while (start != _str.end() && std::isspace (static_cast<unsigned char> (*start))) {
            ++start;
        }

        auto end = _str.end();
        do {
            --end;
        } while (std::distance (start, end) > 0 && std::isspace (static_cast<unsigned char> (*end)));

        if (std::distance (start, end) <= 0) {
            return String();
        }
        return String (std::string (start, end + 1));
    }

    /** Replace all occurrences of substring with replacement.
        Modifies this string in-place.
        @param search The substring to find
        @param replacement The string to replace it with
        @return Reference to this String
    */
    String& replace (const String& search, const String& replacement) {
        return replace (search._str, replacement._str);
    }

    /** Replace all occurrences of substring with replacement.
        Modifies this string in-place.
        @param search The substring to find
        @param replacement The string to replace it with
        @return Reference to this String
    */
    String& replace (const std::string& search, const std::string& replacement) {
        size_t pos = 0;
        while ((pos = _str.find (search, pos)) != std::string::npos) {
            _str.replace (pos, search.length(), replacement);
            pos += replacement.length();
        }
        return *this;
    }

    /** Replace all occurrences of substring with replacement.
        Modifies this string in-place.
        @param search The substring to find
        @param replacement The string to replace it with
        @return Reference to this String
    */
    String& replace (const char* search, const char* replacement) {
        return replace (std::string (search), std::string (replacement));
    }

    /** Return an uppercase copy of this string.
        Uses std::toupper for ASCII range. Multi-byte UTF-8 characters 
        outside ASCII range are unchanged.
        @return A new String with uppercase ASCII characters
    */
    String to_upper() const {
        String result (*this);
        std::transform (result._str.begin(), result._str.end(), result._str.begin(), [] (unsigned char c) { return std::toupper (c); });
        return result;
    }

    /** Return a lowercase copy of this string.
        Uses std::tolower for ASCII range. Multi-byte UTF-8 characters 
        outside ASCII range are unchanged.
        @return A new String with lowercase ASCII characters
    */
    String to_lower() const {
        String result (*this);
        std::transform (result._str.begin(), result._str.end(), result._str.begin(), [] (unsigned char c) { return std::tolower (c); });
        return result;
    }

    //=================================================================
    String& operator= (const String& o) {
        if (this != &o) {
            _str = o._str;
        }
        return *this;
    }

    /** Move assignment operator */
    String& operator= (String&& o) noexcept {
        if (this != &o) {
            _str = std::move (o._str);
        }
        return *this;
    }

    String& operator= (const str_type& o) {
        _str = o;
        return *this;
    }

    /** Move assignment from std::string */
    String& operator= (str_type&& o) noexcept {
        _str = std::move (o);
        return *this;
    }

    String& operator= (const char* o) {
        _str = o;
        return *this;
    }

    /** Equality comparison with C string. */
    inline bool operator== (const char* o) const noexcept { return strcmp (_str.c_str(), o) == 0; }
    /** Equality comparison with another String. */
    inline bool operator== (const String& o) const noexcept { return _str == o._str; }
    /** Equality comparison with std::string. */
    inline bool operator== (const std::string& o) const noexcept { return _str == o; }

    /** Inequality comparison with C string. */
    inline bool operator!= (const char* o) const noexcept { return strcmp (_str.c_str(), o) != 0; }
    /** Inequality comparison with another String. */
    inline bool operator!= (const String& o) const noexcept { return _str != o._str; }
    /** Inequality comparison with std::string. */
    inline bool operator!= (const std::string& o) const noexcept { return _str != o; }

    /** Less-than comparison for sorting. */
    inline bool operator< (const String& a) const noexcept {
        return _str < a._str;
    }
    /** Greater-than comparison for sorting. */
    inline bool operator> (const String& a) const noexcept {
        return _str > a._str;
    }

    /** Returns the C string of this String. Do not free or modify the returned pointer. */
    const char* c_str() const noexcept { return _str.c_str(); }
    /** Implicit conversion to const char*. Do not free or modify the returned pointer. */
    operator const char*() const noexcept { return _str.c_str(); }
    /** Returns a const reference to the internal std::string. */
    const std::string& str() const noexcept { return _str; }
    /** Implicit conversion to const std::string&. */
    operator const std::string&() const { return str(); }

private:
    std::string _str;
};

#define APPEND(t)                                          \
    static inline String& operator<< (String& s1, t val) { \
        s1.append (val);                                   \
        return s1;                                         \
    }
APPEND (const char*)
APPEND (char)
APPEND (const String&)
APPEND (const std::string&)
APPEND (int)
APPEND (int64_t)
APPEND (float)
APPEND (double)
#undef APPEND

} // namespace lui
