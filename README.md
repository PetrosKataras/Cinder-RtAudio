### Cinder-RtAudio
AudioIn/Out through RtAudio in Cinder.

Currently only support for Cinder's CMake build system. Tested on Xubuntu 16.04 and macOS 10.11.2.

**Building**: 

`cd samples/BasicInOut/proj/cmake && mkdir build && cd build && cmake .. && make -j4`

**Notes**: 

Since RtAudio is also using CMake, it is compiled as part of the Cinder-RtAudio target compilation.
