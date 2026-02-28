conan install . --output-folder=build --build=missing
mkdir -p build && cd build
cmake ..
make