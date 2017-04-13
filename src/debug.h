#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <errno.h>
#include "util.h"

#define DEBUG(M, ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define ERRNO_INFO() (errno == 0 ? "None" : strerror(errno))

#define LOG_ERROR(M, ...) fprintf(stderr, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, ERRNO_INFO(), ##__VA_ARGS__)

#define LOG_WARN(M, ...) fprintf(stderr, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, ERRNO_INFO(), ##__VA_ARGS__)

#define LOG_INFO(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define CHECK(A, M, ...) if(!(A)) { LOG_ERROR(M "\n", ##__VA_ARGS__);  }

#define CHEAK_EXIT(A, M, ...) if(!(A)) { LOG_ERROR(M "\n", ##__VA_ARGS__); exit(1);}

#define check_debug(A, M, ...) if(!(A)) { debug(M "\n", ##__VA_ARGS__); /* exit(1); */}
#endif