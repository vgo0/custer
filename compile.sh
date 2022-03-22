rm -rf build/

cmake --no-warn-unused-cli -DCMAKE_EXPORT_COMPILE_COMMANDS:BOOL=TRUE -DCMAKE_BUILD_TYPE:STRING=Release -DCMAKE_C_COMPILER:FILEPATH=/bin/clang-11 -DCMAKE_CXX_COMPILER:FILEPATH=/bin/clang++-11 -B build/ -G "Unix Makefiles"

cd build

make