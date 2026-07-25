#ifndef _SEARCH_DIRECTION_H
#define _SEARCH_DIRECTION_H

enum class SearchDirection {
    FORWARD,
    BACKWARD
};

inline const char* get_direction_name(SearchDirection dir) {
    return dir == SearchDirection::FORWARD ? "FORWARD" : "BACKWARD";
}

#endif // _SEARCH_DIRECTION_H
