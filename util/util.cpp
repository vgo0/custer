#include "util.h"

namespace Util {
    std::vector<addrinfo> getAddress(std::string hostname, std::string port) {
        struct addrinfo hints;
        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = 0;
        hints.ai_flags = AI_NUMERICSERV;
        struct addrinfo *result, *rp;
        int s = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &result);
        if(s != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
            throw std::runtime_error("Unable to resolve hostname");
        }
        std::vector<addrinfo> resolved;
        for(rp = result; rp != NULL; rp = rp->ai_next) {
            resolved.push_back(addrinfo(*rp));
        }

        freeaddrinfo(result);
        return resolved;
    }

    static inline void ltrim(std::string &s) {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    }

    static inline void rtrim(std::string &s) {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    }

    void trim(std::string &s) {
        ltrim(s);
        rtrim(s);
    }

    static inline bool stringFound(size_t &seek) {
        return seek != std::string::npos;
    }

    void parseCommaSeparatedString(std::string &input, std::vector<std::string> &output) {
        if(input == "") {
            return;
        }

        size_t pos = input.find(",");

        // "404" "php"
        if(pos == std::string::npos) {
            cleanValue(input);
            output.push_back(input);
            return;
        }

        // "404,403" "php,html"
        do {
            std::string value = input.substr(0, pos);
            cleanValue(value);
            output.push_back(value);
            input.erase(0, pos + 1);
            pos = input.find(",");
        } while(pos != std::string::npos);

        // final entry
        cleanValue(input);
        output.push_back(input);
    }

    static inline void cleanValue(std::string &input) {
        trim(input);
        stripLeadingPeriod(input);
    }

    static inline void stripLeadingPeriod(std::string &input) {
        if(input == "")
            return;
        if(input.at(0) == '.') {
            input.erase(0, 1);
        }
    }

    void parseTarget(ReqOptions &opts) {
        const char https[] = "https://";
        const char http[] = "http://";
        const char port_delim[] = ":";
        const char slash[] = "/";
        trim(opts.target);

        size_t jump = 0;
        size_t seek = opts.target.find(https);
        size_t seek2 = opts.target.find(http);
        

        if(stringFound(seek)) {
            jump = strlen(https);
            opts.ssl = true;
        }
        else if(stringFound(seek2)) {
            jump = strlen(http);
            opts.ssl = false;
        }
        else {
            throw std::runtime_error("Invalid URL specified, must begin with http:// or https://");
        }

        seek = opts.target.find(port_delim, jump);
        seek2 = opts.target.find(slash, jump);
        opts.port = "";

        if(stringFound(seek)) {
            if(stringFound(seek2)) {
                //http://localhost:8080/asd
                if(seek < seek2) {
                    opts.port = opts.target.substr(seek + 1, seek2 - (seek + 1));
                    opts.host = opts.target.substr(jump, seek - jump);
                }
                //http://localhost/asd:8080
                else {
                    opts.host = opts.target.substr(jump, seek2 - jump);
                }
                opts.path = opts.target.substr(seek2);
            }
            else {

                //http://localhost:8080
                opts.port = opts.target.substr(seek + 1);
                opts.path = "/";
                opts.target += "/";
            }
        }
        else {
            //http://localhost/asd
            if(stringFound(seek2)) {
                opts.host = opts.target.substr(jump, seek2 - jump);
                opts.path = opts.target.substr(seek2);
            }
            //http://localhost
            else {
                opts.host = opts.target.substr(jump);
                opts.path = "/";
                opts.target += "/";
            }
        }

        if(opts.port == "") {
            if(opts.ssl) {
                opts.port = "443";
            }
            else {
                opts.port = "80";
            }
        }


    }

    void getValidHost(std::vector<addrinfo> &hosts, ReqOptions &opts) {
        int s;
        for(auto& host : hosts) {
            s = socket(host.ai_family, host.ai_socktype, host.ai_protocol);
            if(s < 0) {
                throw std::runtime_error("Unable to create socket.");
            }
            
            int r = connect(s, host.ai_addr, host.ai_addrlen);
            if(r != 0) {
                close(s);
                char host_str[INET_ADDRSTRLEN];
                std::string port_str;
                switch(host.ai_family) {
                    case AF_INET: {
                        struct sockaddr_in *addr = (struct sockaddr_in *)host.ai_addr;
                        inet_ntop(host.ai_family, &addr->sin_addr, host_str, INET_ADDRSTRLEN);
                        port_str = std::to_string(ntohs(addr->sin_port));
                        break;
                    }
                    case AF_INET6: {
                        struct sockaddr_in6 *addr = (struct sockaddr_in6 *)host.ai_addr;
                        inet_ntop(host.ai_family, &addr->sin6_addr, host_str, INET_ADDRSTRLEN);
                        port_str = std::to_string(ntohs(addr->sin6_port));
                        break;
                    }
                    default: {
                        continue;
                    }
                }
                std::cout << "Unable to connect to " << host_str << ":" << port_str << std::endl;
            }
            else {
                close(s);
                opts.address = addrinfo(host);
                opts.address.ai_addr = new sockaddr(*host.ai_addr);

                return;
            }
        }
        close(s);
        throw std::runtime_error("Unable to find any valid hosts to connect to");
    }
}