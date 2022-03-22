#include <iostream>
#include "requester/requester.h"
#include "requester/globalqueue.h"
#include "util/util.h"
#include "util/fileparser.h"
#include <vector>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <getopt.h>

void time(std::chrono::time_point<std::chrono::high_resolution_clock> &start, std::string label) {
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
    std::cout << label << ": " << duration.count() / 1000.0 << " second duration" << std::endl;
    start = stop;
}

void printOption(std::string flag, std::string description) {
    std::cout << "  " << flag << "          " << description << std::endl;
}

void printHelp() {
    std::cout << "Options:" << std::endl; 
    printOption("-u", "URL or host to fuzz");
    printOption("-w", "Path to wordlist");
    printOption("-k", "Disable SSL verification");
    printOption("-t", "Number of threads (default: 10)");
    printOption("-g", "Use global queue mode (default: queue per thread)");
    printOption("-x", "Comma separated extensions to bruteforce (e.g. .php,.html,.js)");
    printOption("-b", "Comma separated tatus codes to ignore. Default: 404 (e.g. 404,403)");
    exit(EXIT_SUCCESS);
}

void checkOpts(ReqOptions &opts) {
    if(opts.target == "") {
        throw std::runtime_error("You must specify a URL to fuzz via -u");
    }
    if(opts.wordlist == "") {
        throw std::runtime_error("You must specify a wordlist via -w");
    }
    if(!(opts.threads > 0)) {
        throw std::runtime_error("Invalid thread quantity specified");
    }

    std::cout << "Fuzzing " << opts.target << " with " << opts.threads << " threads." << std::endl;
    if(!opts.ssl_verify) {
        std::cout << "SSL cert validation is turned off." << std::endl;
    }

    if(opts.req_type == ReqType::DIR) {
        std::cout << "Running in directory brute force mode." << std::endl;
    }
    // Not implemented
    else if(opts.req_type == ReqType::VHOST) {
        std::cout << "Running in subdomain / vhost brute force mode." << std::endl;
        throw std::runtime_error("VHOST mode not implemented!");
    }

    if(opts.queue_mode == QueueMode::GLOBAL) {
        std::cout << "Global queue mode. All threads share same word queue." << std::endl;
    }
    else if(opts.queue_mode == QueueMode::INDIVIDUAL) {
        std::cout << "Individual queue mode. Words evenly distributed between threads." << std::endl;
    }
}

void printOptions(ReqOptions &opts) {
    if(opts.blacklist_codes.size() > 0) {
        std::cout << "The following response codes are filtered: ";
        for(auto& code : opts.blacklist_codes) {
            std::cout << code << " ";
        }
        std::cout << std::endl;
    }
    if(opts.extensions.size() > 0) {
        std::cout << "The following extensions will be tried: ";
        for(auto& ext : opts.extensions) {
            std::cout << "." << ext << " ";
        }
        std::cout << std::endl;
    }
}

int main(int argc, char** argv) {
    auto full_start = std::chrono::high_resolution_clock::now();
    auto start = std::chrono::high_resolution_clock::now();
    
    ReqOptions opts;
    int c;

    while ((c = getopt (argc, argv, "h?u:gt:kw:b:x:s")) != -1) {
        switch(c)
        {
            case '?': 
            case 'h': {
                printHelp();
                break;
            }
            case 'u': {
                opts.target = optarg;
                break;
            }
            case 'g': {
                opts.queue_mode = QueueMode::GLOBAL;
                break;
            }
            case 'k': {
                opts.ssl_verify = false;
                break;
            }
            case 't': {
                opts.threads = std::stoi(optarg);
                break;
            }
            case 'w': {
                opts.wordlist = optarg;
                break;
            }
            case 'b': {
                opts.raw_blacklist_codes = optarg;
                break;
            }
            case 'x': {
                opts.raw_extensions = optarg;
                break;
            }
            case 's': {
                opts.req_type = ReqType::VHOST;
                break;
            }
            case -1: {
                break;
            }

        }
    }
    
    std::vector<Requester> workers;
    std::vector<std::thread> threads;
    try {
        
        checkOpts(opts);

        // Get address info
        Util::parseTarget(opts);
        // Parse comma separated options
        Util::parseCommaSeparatedString(opts.raw_blacklist_codes, opts.blacklist_codes);
        Util::parseCommaSeparatedString(opts.raw_extensions, opts.extensions);
        printOptions(opts);

        // Find a valid IP:Port combination
        std::vector<addrinfo> hosts = Util::getAddress(opts.host, opts.port);
        Util::getValidHost(hosts, opts);

        // Create worker threads
        for(int i = 0; i < opts.threads; i++) {
            workers.push_back(Requester(opts, i));
        }

        // Create word queue
        FileParser::createQueueFromWordlist(workers, opts);

        std::cout << "Checking a total of " << opts.total_count << " words." << std::endl;

        // Run queue
        for(auto & requestor : workers) {
            threads.push_back(std::thread(&Requester::runQueue, &requestor));
        }
        for(auto & th : threads) {
            th.join();
        }

        free(opts.address.ai_addr);
    }
    catch(std::runtime_error e) {
        std::cerr << "Fatal Error:" << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << "Run program with -h for help text" << std::endl;
        exit(EXIT_FAILURE);
    }
    time(full_start, "Program Complete");
}
