/**
 * @file log.h
 * @brief Provides macros and functions for logging messages with different severity levels and optional annotations.
 * @version 0.1
 */

#ifndef LOG_H
#define LOG_H

#include <stdio.h>

// ANSI escape codes for colored log output
#define LOG_RESET   "\033[0m"   /**< Reset color to default */
#define LOG_RED     "\033[1;31m" /**< Red color for error messages */
#define LOG_GREEN   "\033[1;32m" /**< Green color for success messages */
#define LOG_YELLOW  "\033[1;33m" /**< Yellow color for warnings */
#define LOG_BLUE    "\033[1;34m" /**< Blue color for informational messages */
#define LOG_CYAN    "\033[1;36m" /**< Cyan color for debug messages */
#define LOG_WHITE   "\033[1;37m" /**< White color for general messages */

// Log levels and their respective display modes
#define LOG_ERR     0  /**< Log level for critical errors, always printed */
#define LOG_DBG     1  /**< Log level for debug messages, printed only in debug mode */
#define LOG_PRI     2  /**< Log level for regular messages, always printed without special annotations */

// Default colors for different log types
#define LOG_COLOR_ERR   LOG_RED    /**< Default color for error messages */
#define LOG_COLOR_DBG   LOG_CYAN  /**< Default color for debug messages */
#define LOG_COLOR_PRI   LOG_WHITE /**< Default color for regular messages */

// Debug mode configuration
#ifdef DEBUG
#define ANNOTATIONS 1      /**< Set to 1 to enable annotations in debug mode (file, function, line info) */
#else
#define DEBUG 0
#define ANNOTATIONS 0
#endif

// Configuration for annotations
#define ANNOTATIONS_INFO 1  /**< Set to 1 to include file, function, and line number annotations */

// Enable/disable specific annotation types
#define ANNOTATIONS_FILE 0 /**< Set to 1 to include file name in annotations */
#define ANNOTATIONS_FUNC 1 /**< Set to 1 to include function name in annotations */
#define ANNOTATIONS_LINE 1 /**< Set to 1 to include line number in annotations */

// Default output function for logging
#define LOG_OUT(...) printf(__VA_ARGS__)

// Constructs the annotation string with file, function, and line information
#define ANNOTATION_INFO_STRING do {\
    if (ANNOTATIONS_INFO) {\
        LOG_OUT(" ("); \
        if (ANNOTATIONS_FILE) printf("%s", __FILE__); \
        if (ANNOTATIONS_FILE && ANNOTATIONS_FUNC) printf(","); \
        if (ANNOTATIONS_FUNC) printf("%s", __func__); \
        if (ANNOTATIONS_FUNC && ANNOTATIONS_LINE) printf(","); \
        if (ANNOTATIONS_LINE) printf("%d", __LINE__); \
        LOG_OUT(") ");\
    }\
} while (0)

// Macro for logging messages with optional annotations and color
#define LOG(type, prefix, color, ...) \
    do { \
        if (DEBUG) {\
            LOG_OUT("%s%s%s: ", color, prefix, LOG_RESET); \
            ANNOTATION_INFO_STRING; \
            if (type == LOG_DBG) { \
                LOG_OUT(__VA_ARGS__); \
                break; \
            } \
        }\
        if (type == LOG_ERR || type == LOG_PRI) { LOG_OUT(__VA_ARGS__); } \
    } while (0)

#endif // LOG_H
