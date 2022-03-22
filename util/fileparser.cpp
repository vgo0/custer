#include "fileparser.h"

namespace FileParser {
    void createQueueFromWordlist(std::vector<Requester> &workers, ReqOptions &opts) {
        //fix for relative home path
        if(opts.wordlist.at(0) == '~') {
            opts.wordlist.replace(0, 1, getenv("HOME"));
        }

        //add blank extension
        opts.extensions.push_back("");

        std::cout << "Fetching wordlist from " << opts.wordlist << std::endl;
        std::ifstream read_wordlist(opts.wordlist.c_str());
        if(read_wordlist.fail()) {
            throw std::runtime_error("Unable to read / access wordlist");
        }

        size_t count = 0;
        size_t max = workers.size();
        std::string line;
        while(std::getline(read_wordlist, line)) {
            //getline can leave carriage return etc
            Util::trim(line);

            for(auto & extension : opts.extensions) {
                std::string word = line;
                if(extension != "") {
                    word = word + "." + extension;
                }
                if(opts.queue_mode == QueueMode::INDIVIDUAL) {
                    workers[count].addWord(word);
                    count++;
                    if(count == max)
                        count = 0;
                }
                else {
                    opts.q.addWord(word);
                }

                opts.total_count++;
            }
            
            
        }
        read_wordlist.close();
    }
}