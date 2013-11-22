#ifndef _EXAMPLES_NFC_UTILS_H_
#  define _EXAMPLES_NFC_UTILS_H_

#  include <stdlib.h>
#  include <string.h>
#  include <err.h>
#  include <stdint.h>
#  include <stdbool.h>
/**
 * @macro DBG
 * @brief Print a message of standard output only in DEBUG mode
 */
#ifdef DEBUG
#  define DBG(...) do { \
    warnx ("DBG %s:%d", __FILE__, __LINE__); \
    warnx ("    " __VA_ARGS__ ); \
  } while (0)
#else
#  define DBG(...) {}
#endif

/**
 * @macro WARN
 * @brief Print a warn message
 */
#ifdef DEBUG
#  define WARN(...) do { \
    warnx ("WARNING %s:%d", __FILE__, __LINE__); \
    warnx ("    " __VA_ARGS__ ); \
  } while (0)
#else
#  define WARN(...) warnx ("WARNING: " __VA_ARGS__ )
#endif

/**
 * @macro ERR
 * @brief Print a error message
 */
#ifdef DEBUG
#  define ERR(...) do { \
    warnx ("ERROR %s:%d", __FILE__, __LINE__); \
    warnx ("    " __VA_ARGS__ ); \
  } while (0)
#else
#  define ERR(...)  warnx ("ERROR: " __VA_ARGS__ )
#endif
void ndef_transfer_interface(char INPUT[][1000],uint8_t final_result[]);


#endif
