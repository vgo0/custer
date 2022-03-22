
# c-buster (custer)
A basic C++ directory / file brute forcer. It uses a minimal (and probably poor) HTTP 1 / 1.1 implementation directly with TCP sockets to be extremely quick. It does not gracefully handle things like retries, rate limiting, etc... This was made mostly for fun and speed. It does have SSL capabilities. It does not attempt to URL encode the word list.

# Queue Modes
It offers 2 different queue modes. The default splits the wordlist up between each thread, and each thread has its own queue it works through. The global queue `-g` mode uses a central queue for all threads. The global queue has a lock between popping for thread safety which would be slower than the queue-per-thread. However, this allows all threads to work against the same queue. With a queue per thread a thread could clear its own queue and stop working / contributing.

# Using
```
./custer -h
Options:
  -u          URL or host to fuzz
  -w          Path to wordlist
  -k          Disable SSL verification
  -t          Number of threads (default: 10)
  -g          Use global queue mode (default: queue per thread)
  -x          Comma separated extensions to bruteforce (e.g. .php,.html,.js)
  -b          Comma separated tatus codes to ignore. Default: 404 (e.g. 404,403)
```

# Building
```
sudo apt-get update
sudo apt-get install build-essential libssl-dev

Compile:
Using clang++-11 / clang-11
./compile.sh 

cd build

./custer -h
```

