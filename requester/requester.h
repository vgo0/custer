#pragma once
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "globalqueue.h"
#include <stdexcept>
#include <queue>
#include <thread>
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

enum ReqType {
    DIR,
    VHOST
};

enum QueueMode {
    INDIVIDUAL,
    GLOBAL
};

struct ReqData {
    const unsigned int BUF_SIZE = 8096;
    int total = 0;
    int content_length = 0;
    bool chunked = false;
    bool done = false;
    std::vector<unsigned char> resp_buffer;
    std::string resp_string;
    ReqData() : resp_buffer(BUF_SIZE) {

    }
};

struct ReqOptions {
    struct addrinfo address;
    ReqType req_type = ReqType::DIR;
    QueueMode queue_mode = QueueMode::INDIVIDUAL;
    std::vector<std::string> blacklist_codes;
    std::vector<std::string> extensions;
    GlobalQueue q;
    std::string raw_blacklist_codes = "404";
    std::string raw_extensions;
    std::string target;
    std::string host;
    std::string port;
    std::string path;
    std::string wordlist;
    int threads = 10;
    int total_count = 0;
    bool ssl;
    bool ssl_verify = true;
};

class Requester {
public:
    //Requester(ReqOptions opts, int id);
    Requester(ReqOptions& opts, int id);
    void addWord(std::string word);
    void runQueue();
    void printTimes();
private:
    int sockfd;
    int id;
    bool connected;
    SSL_CTX *sslctx;
    SSL *ssl;
    ReqOptions& opts;
    std::string base_buf;
    std::queue<std::string> words;
    void _runQueue();
    void connectSocket();
    void connectSSL(int &sockfd);
    void initSSLCTX();
    void cleanupSSL();
    void makeReq(std::string fuzz);
    bool keepAlive(std::string &resp);
};