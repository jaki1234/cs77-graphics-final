#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <set>
#include <tuple>


// Prints an error message and stops (printf style)
#define ERROR(msg, ...) do { \
fprintf(stderr, "error in function %s at %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
fprintf(stderr, msg , ## __VA_ARGS__ ); fprintf(stderr, "\n" ); \
fflush(stderr); abort(); \
} while(false)

// Prints an error message and stops if a condition does not hold (printf style)
#define ERROR_IF(cond, msg, ...) do { \
if(!(cond)) break; \
fprintf(stderr, "error (assertion failed) in function %s at %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
fprintf(stderr, "assert: !(%s)\n", #cond ); \
fprintf(stderr, msg , ## __VA_ARGS__ ); fprintf(stderr, "\n" ); \
fflush(stderr); abort(); \
} while(false)

// Prints an error message and stops if a condition does not hold (printf style)
#define ERROR_IF_NOT(cond, msg, ...) do { \
if((cond)) break; \
fprintf(stderr, "error (assertion failed) in function %s at %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
fprintf(stderr, "assert: %s\n", #cond ); \
fprintf(stderr, msg , ## __VA_ARGS__ ); fprintf(stderr, "\n" ); \
fflush(stderr); abort(); \
} while(false)

// Prints a warning message (printf style)
#define WARNING(msg, ...) do { \
fprintf(stderr, "warning in function %s at %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
fprintf(stderr, msg , ## __VA_ARGS__ ); fprintf(stderr, "\n" ); \
fflush(stderr); \
} while(false)

// Prints a warning message if a condition does not hold (printf style)
#define WARNING_IF(cond, msg, ...) do { \
if(!(cond)) break; \
fprintf(stderr, "warning (assertion failed) in function %s at %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
fprintf(stderr, "condition: !(%s)\n", #cond ); \
fprintf(stderr, msg , ## __VA_ARGS__ ); fprintf(stderr, "\n" ); \
fflush(stderr); \
} while(false)

// Prints a warning message if a condition does not hold (printf style)
#define WARNING_IF_NOT(cond, msg, ...) do { \
if((cond)) break; \
fprintf(stderr, "warning (assertion failed) in function %s at %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
fprintf(stderr, "condition: %s\n", #cond ); \
fprintf(stderr, msg , ## __VA_ARGS__ ); fprintf(stderr, "\n" ); \
fflush(stderr); \
} while(false)

// Error signaling unimplemented features
#define NOT_IMPLEMENTED_ERROR() do { \
fprintf(stderr, "error (not implemented) in function %s at %s:%d\n", __FUNCTION__, __FILE__, __LINE__); \
fflush(stderr); abort(); \
} while(false)




/// Error signaling unimplemented features
/// Declare function, then undefine and define again
void put_your_code_here(const char *txt);
#undef put_your_code_here
#define put_your_code_here(txt) \
    do { \
    static bool been_here = false; if(!been_here) { \
    been_here = true; \
    fprintf(stderr, "warning: %s() not (fully) implemented at %s:%d.\n    msg: %s\n", __FUNCTION__, __FILE__, __LINE__, txt); \
    fflush(stderr); \
    /*_put_your_code_here(__FILE__, __LINE__, __FUNCTION__, txt);*/ \
    } } while(0);


#endif
