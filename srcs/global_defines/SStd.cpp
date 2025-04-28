# include "global_defines/SStd.h"

 /**
 * @brief Abort, printing position in code, if (! x).
 * @param x Condition to check.
 * @param file File name.
 * @param line Line number.
 * @param message Message to print. Default is "Assertion failed.  Aborting.".
 */
void SStd::sassert(bool x, const char *file, int line, const char *message) {
    if (!x) {
        std::cerr << "File: " << file << ", Line: " << line << ":: ";
        if (message) {
            std::cerr << message << std::endl;
            Logger::error("File: {}, Line: {}:: {}", file, line, message);
        }
        else {
            std::cerr << "Assertion failed.  Aborting." << std::endl;
            Logger::error("File: {}, Line: {}:: Assertion failed.  Aborting.", file, line);
        }
        abort();
    }
}
