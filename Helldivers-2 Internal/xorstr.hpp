#ifndef XORSTR_H
#define XORSTR_H

#include <Windows.h>
#include <stdlib.h>

// Inline functions and constants
#define XORSTR_INLINE	__forceinline
#define XORSTR_NOINLINE __declspec(noinline)
#define XORSTR_CONST	constexpr
#define XORSTR_VOLATILE volatile

#define XORSTR_CONST_INLINE XORSTR_INLINE XORSTR_CONST
#define XORSTR_CONST_NOINLINE XORSTR_NOINLINE XORSTR_CONST

// Constants for FNV hash function
#define XORSTR_FNV_OFFSET_BASIS 0xCBF29CE484222325
#define XORSTR_FNV_PRIME 0x100000001B3

// Macros for byte and nibble operations
#define XORSTR_BYTE(_VALUE, _IDX) ((_VALUE >> (__min(_IDX, (XORSTR_TYPE_SIZEOF(_VALUE)) - 1) * 8)) & 0xFF)
#define XORSTR_NIBBLE(_VALUE, _IDX) ((_VALUE >> (__min(_IDX, (XORSTR_TYPE_SIZEOF(_VALUE) * 2) - 1) * 4)) & 0xF)

// Helper struct for integer sequences
template <typename _Ty, _Ty... Types>
struct XORSTR_INT_SEQ {};

// Function to convert a character to an integer
XORSTR_CONST_NOINLINE
INT XORSTR_ATOI8(IN CHAR Character) noexcept {
    return (Character >= '0' && Character <= '9') ? (Character - '0') : NULL;
}

// Function to calculate key hash using FNV hash
XORSTR_CONST_NOINLINE
UINT64 XORSTR_KEY(IN SIZE_T CryptStrLength) noexcept {
    UINT64 KeyHash = XORSTR_FNV_OFFSET_BASIS;

    for (SIZE_T i = NULL; i < sizeof(__TIME__); i++) {
        KeyHash = KeyHash ^ (XORSTR_ATOI8(__TIME__[i]) + (CryptStrLength * i)) & 0xFF;
        KeyHash = KeyHash * XORSTR_FNV_PRIME;
    }

    return KeyHash;
}

// Class for XOR encryption of strings
template <typename _CHAR_TYPE_, SIZE_T _STR_LENGTH_>
class _XORSTR_ {
public:
    // Constructor
    XORSTR_CONST_INLINE _XORSTR_(IN _CHAR_TYPE_ CONST(&String)[_STR_LENGTH_])
        : _XORSTR_(String, XORSTR_MAKE_INTEGER_SEQUENCE(_STR_LENGTH_)) {}

    // Decrypts the string and returns it
    XORSTR_INLINE CONST _CHAR_TYPE_* String(VOID) {
        for (SIZE_T i = NULL; i < _STR_LENGTH_; i++) {
            StringData[i] = CRYPT_CHAR(StringData[i], i);
        }
        return (_CHAR_TYPE_*)(StringData);
    }

private:
    // Key for encryption
    static XORSTR_CONST UINT64 Key = XORSTR_KEY(_STR_LENGTH_);

    // Decrypts a single character
    static XORSTR_CONST_INLINE _CHAR_TYPE_ CRYPT_CHAR(IN _CHAR_TYPE_ Character, IN SIZE_T KeyIndex) {
        return (Character ^ ((Key + KeyIndex) ^ (XORSTR_NIBBLE(Key, KeyIndex % 16))));
    }

    // Constructor with integer sequence
    template <SIZE_T... _INDEX_>
    XORSTR_CONST_INLINE _XORSTR_(IN _CHAR_TYPE_ CONST(&String)[_STR_LENGTH_], IN XORSTR_INTEGER_SEQUENCE(_INDEX_) IntSeq)
        : StringData{ CRYPT_CHAR(String[_INDEX_], _INDEX_)... } {}

    XORSTR_VOLATILE _CHAR_TYPE_ StringData[_STR_LENGTH_];
};

// Function to create _XORSTR_ object for char array
template <SIZE_T _STR_LEN_>
XORSTR_CONST_INLINE _XORSTR_<CHAR, _STR_LEN_> XorStr(IN CHAR CONST(&String)[_STR_LEN_]) {
    return _XORSTR_<CHAR, _STR_LEN_>(String);
}

// Function to create _XORSTR_ object for wchar_t array
template <SIZE_T _STR_LEN_>
XORSTR_CONST_INLINE _XORSTR_<WCHAR, _STR_LEN_> XorStr(IN WCHAR CONST(&String)[_STR_LEN_]) {
    return _XORSTR_<WCHAR, _STR_LEN_>(String);
}

// Function to create _XORSTR_ object for char32_t array
template <SIZE_T _STR_LEN_>
XORSTR_CONST_INLINE _XORSTR_<char32_t, _STR_LEN_> XorStr(IN char32_t CONST(&String)[_STR_LEN_]) {
    return _XORSTR_<char32_t, _STR_LEN_>(String);
}

// Macro for simplified usage
#define _XOR_(_STR_) XorStr(_STR_).String()

#endif
