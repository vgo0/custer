#pragma once
#include <queue>
#include <thread>
#include <mutex>

class GlobalQueue {
public:
    GlobalQueue();
    std::string getTerm();
    void addWord(std::string word);
private:
    std::queue<std::string> words;
    std::mutex pop_mutex;
};