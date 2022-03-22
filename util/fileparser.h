#pragma once
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>
#include <iostream>
#include <vector>
#include "../requester/requester.h"
#include "../requester/globalqueue.h"
#include "util.h"

namespace FileParser {
    void createQueueFromWordlist(std::vector<Requester> &workers, ReqOptions &opts);
}
