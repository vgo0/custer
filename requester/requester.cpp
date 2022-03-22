#include "requester.h"
#include <cerrno>
#include <cstring>

Requester::Requester(ReqOptions& opts, int id) : opts(opts) {
    this->id = id;
    connected = false;
    base_buf = "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n";
    base_buf += "Host: " + opts.host + "\r\n";
    base_buf += "Accept-Language: en-us\r\n";
    base_buf += "Accept-Encoding: *\r\n";
    base_buf += "Connection: Keep-Alive\r\n\r\n";
    if(opts.ssl) {
        initSSLCTX();
    }
}

void Requester::runQueue() {
    try {
        _runQueue();
    }
    catch(std::runtime_error e) {
        std::cerr << "Fatal Error:" << std::endl;
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void Requester::_runQueue() {
    std::string word;
    if(opts.queue_mode == QueueMode::GLOBAL) {
        while(true) {
            word = opts.q.getTerm();
            if(word == "")
                break;
            makeReq(word);
        }
    }
    else {
        while(!words.empty()) {
            word = words.front();
            words.pop();
            makeReq(word);
        }
    }


    close(sockfd);
}

void Requester::makeReq(std::string fuzz) {
    std::string buf = "GET " + opts.path + fuzz + " HTTP/1.1\r\n" + base_buf;
    if(!connected) {
        connectSocket();
    }
    int size;
    do {
        if(opts.ssl) {
            size = SSL_write(ssl, buf.c_str(), buf.size());
        }
        else {
            size = send(sockfd, buf.c_str(), buf.size(), 0);
        }
        if(size < 0) {
            connectSocket();
        }
    } while(size < 0);
    
    ReqData response;
    do {
        if(opts.ssl) {
            size = SSL_read(ssl, response.resp_buffer.data(), response.BUF_SIZE);
        }
        else {
            size = recv(sockfd, response.resp_buffer.data(), response.BUF_SIZE, 0);
        }
        
        if(size == 0) {
            continue;
        }
        else if(size < 0) {
            connectSocket();
        }

        for(int i = 0; i < size; i++) {
            response.resp_string += response.resp_buffer[i];
        }
        if(response.total == 0) {
            response.chunked = response.resp_string.find("Transfer-Encoding: chunked\r\n") != std::string::npos;
            if(!response.chunked) {
                size_t search = response.resp_string.find("Content-Length: ");
                if(search == std::string::npos) {
                    response.content_length = 0;
                }
                else {
                    search += 16;
                    std::string size_str;
                    while(response.resp_string.at(search) != '\r') {
                        size_str += response.resp_string.at(search);
                        search++;
                    }
                    response.content_length = std::stoi(size_str);
                    search = response.resp_string.find("\r\n\r\n");
                    response.total -= search + 4;
                }
            }
            connected = response.resp_string.find("HTTP/1.0") == std::string::npos;
            if(connected) {
                connected = response.resp_string.find("Connection: close") == std::string::npos;
            }
            if(!connected) {
                close(sockfd);
            }
            size_t space_pos = response.resp_string.find(" ");
            std::string status = response.resp_string.substr(space_pos + 1, 3);

            if(std::find(opts.blacklist_codes.begin(), opts.blacklist_codes.end(), status) == opts.blacklist_codes.end()) {
                std::cout << opts.target + fuzz << " [" << status << "]" << std::endl;
            }            
        }
            
        response.total += size;
        
        
        if(response.chunked) {
            response.resp_string = response.resp_string.substr(response.resp_string.length() - 5, 5);
            response.done = response.resp_string == "0\r\n\r\n";
        }
        else if(response.content_length > 0) {
            response.done = response.total >= response.content_length;
        }
        else {
            response.done = size < response.BUF_SIZE;
        }
        response.resp_buffer.clear();
    } while(!response.done);
}

void Requester::addWord(std::string word) {
    words.push(word);
}

void Requester::initSSLCTX() {
    OpenSSL_add_ssl_algorithms();
    const SSL_METHOD *method = TLS_client_method();
    sslctx = SSL_CTX_new(method);
    if(!opts.ssl_verify) {
        SSL_CTX_set_verify(sslctx, SSL_VERIFY_NONE, NULL);
    }
    else {
        SSL_CTX_set_verify(sslctx, SSL_VERIFY_PEER, NULL);
    }
}

void Requester::connectSSL(int &sockfd) {
    if(ssl != nullptr) {
        SSL_free(ssl);
    }

    ssl = SSL_new(sslctx);

    if(ssl == NULL) {
        throw std::runtime_error("Error initializing SSL");
    }

    SSL_set_fd(ssl, sockfd);
    int r = SSL_connect(ssl);
    
    if(r != 1) {
        r = SSL_get_error(ssl, r);
        BIO *bio = BIO_new(BIO_s_mem());
        ERR_print_errors(bio);
        char *buf;
        size_t len = BIO_get_mem_data(bio, &buf);
        std::string err(buf, len);
        BIO_free(bio);
        if(err.find("certificate verify failed") != std::string::npos) {
            throw std::runtime_error("SSL certificate verification failed. Consider disabling certificate validation (-k)");
        }
        else {
            throw std::runtime_error("SSL connect failed with error: " + err);
        }
    }
}

void Requester::connectSocket() {
    sockfd = socket(opts.address.ai_family, SOCK_STREAM, 0);
    int r = connect(sockfd, opts.address.ai_addr, opts.address.ai_addrlen);
    if(r != 0) {
        close(sockfd);
        fprintf(stderr, "%s\n", std::strerror(errno));
        throw std::runtime_error("Unable to connect to host");
    }

    if(opts.ssl) {
        connectSSL(sockfd);
    }
    connected = true;
}