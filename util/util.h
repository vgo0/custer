#pragma once
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../requester/requester.h"
#include <netdb.h>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <unistd.h>
#include <algorithm>

namespace Util {
    std::vector<addrinfo> getAddress(std::string hostname, std::string port);

    static inline void ltrim(std::string &s);

    static inline void rtrim(std::string &s);

    void trim(std::string &s);

    static inline bool stringFound(size_t &seek);

    void parseCommaSeparatedString(std::string &input, std::vector<std::string> &output);

    static inline void cleanValue(std::string &input);

    static inline void stripLeadingPeriod(std::string &input);

    void parseTarget(ReqOptions &opts);

    void getValidHost(std::vector<addrinfo> &hosts, ReqOptions &opts);
}