#include "globalqueue.h"

GlobalQueue::GlobalQueue() {

}

void GlobalQueue::addWord(std::string word) {
    words.push(word);
}

std::string GlobalQueue::getTerm() {
    std::lock_guard<std::mutex> guard(pop_mutex);
    while(!words.empty()) {
        std::string ret = words.front();
        words.pop();
        if(ret == "")
            continue;
        return ret;
    }

    return "";
}