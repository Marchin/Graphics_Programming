#ifndef UTILS_H
#define UTILS_H

enum FIRST_LAST {
    FIRST,
    LAST,
};

inline b32
findCharacter(const char* pString, char toFind, FIRST_LAST from, size_t* pIndex) {
    size_t length = strlen(pString)/sizeof(char);
    switch (from) {
        case FIRST: {
            for (size_t i = 0; i < length; ++i) {
                if (pString[i] == toFind) {
                    *pIndex = i;
                    return true;
                }
            }
        } break;
        case LAST: {
            for (size_t i = length - 1; i >= 0; --i) {
                if (pString[i] == toFind) {
                    *pIndex = i;
                    return true;
                }
            }
        } break;
    }
    return false;
}

inline b32
haveSameSign(f32 a, f32 b) {
    return ((a >= 0) == (b >= 0));
}

struct ENGINE_API Plane {
    hmm_vec3 normal;
    hmm_vec3 dot;
    f32 d; // ax + by + cz + d
};

#endif //UTILS_H