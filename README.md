X Portable Runtime
==================

基于 C/C++ 实现的跨平台运行库，主要针对基于 Linux 的嵌入式设备而设计，同时也可以用于桌面系统。

组件说明
--------

- [ADC - Analog/Digital Converter](docs/adc.md)
- [ARP - Address Resolve Protocol](docs/arp.md)
- [ARR - Audio Rate Resampler](docs/arr.md)
- [AVFRAME - Audio/Video Raw Frame Container](docs/avframe.md)
- [ATOMIC - Atomic Instructions](docs/atomic.md)
- [BASE64 - Base64 Encode/Decode](docs/base64.md)
- [BITVECTOR - Bit Vector](docs/bitvector.md)
- [CRC - Cyclic Redundancy Check](docs/crc.md)
- [DEVCAPS - Device Capabilties](docs/devcaps.md)
- [DPU - Data Processing Unit](docs/dpu.md)
- [DRM - Digital Right Manager](docs/drm.md)
- [FIFO - First In First Out Queue](docs/fifo.md)
- [FILE - Filesytem](docs/file.md)
- [FQ - Flex Queue](docs/fq.md)
- [GPIO - General Purpose Input/Output](docs/gpio.md)
- [H264 - MPEG-4/H264 Helper](docs/h264.md)
- [HASHTABLE - Hash Table](docs/hashtable.md)
- [JSON - Javascript Object Notiation](docs/json.md)
- [ICMP - Internet Control Messages Protocol](docs/icmp.md)
- [LIST - Double/Singly Linked List](docs/list.md)
- [MD5 - MD5 Digest](docs/md5.md)
- [MCDEC - Multi-Channels Decoder](docs/mcdec.md)
- [MCVR - Multi-Channels Video Renderer](docs/mcvr.md)
- [MEM - Memory Mangement](docs/mem.md)
- [META - Meta Data](docs/meta.md)
- [ONVIF - ONVIF Protocol Supports](docs/onvif.md)
- [OSD - On Screen Display](docs/osd.md)
- [PLUGIN - Plugin Framework](docs/plugin.md)
- [REGEX - RegExp Basic Supports](docs/regex.md)
- [RTSP - RTSP Server/Client](docs/rtsp.md)
- [SERIAL - Serial Port](docs/serial.md)
- [SHA1 - SHA1 Digest](docs/sha1.md)
- [STREAMBLOCK - Bytestream Container](docs/streamblock.md)
- [SYNC - Synchronization](docs/sync.md)
- [SYS - Operation System](docs/sys.md)
- [TIMER - Timer and Timer Queue](docs/timer.md)
- [THREAD - Multithreading](docs/thread.md)
- [UIO - Universal Input/Output](docs/ups.md)
- [UPS - Universal Parameters Settings](docs/ups.md)
- [URL - Url Parser](docs/url.md)
- [UTILS - Utilities](docs/utils.md)
- [XML - XML Stream Reader/Writer](docs/xml.md)

构建说明
--------

本项目支持 CMake 及 GNU Make 两种编译方式，推荐使用 CMake 方式。

### CMake 编译

```sh
mkdir -p build # [Optional]
cd build # Change to build directory
cmake -DBUILD_WITH_HUNTER=ON .. # 配置 CMake 脚本
make # 开始编译
```

- `BUILD_WITH_HUNTER=ON` - 表示使用 `hunter` 包管理脚本来解决依赖问题，可视需求开启。

除此之外还支持以下选项：

- `BUILD_SRC` - 配置是否编译主代码。
- `BUILD_DOCS` - 配置是否编译文档。
- `BUILD_BENCHMARKS` - 配置是否编译性能测试例程。
- `BUILD_EXAMPLES` - 配置是否编译演示例程。
- `BUILD_TESTS` - 配置是否编译功能测试例程。
- `BUILD_TOOLS` - 配置是否编译辅助工具。
- `BUILD_SHARED_LIBS` - 配置是否编译动态库。

当需要使用手工编译的 live 库时，你可以指定 LIVE_ROOT_FORCE 参数来实现，例如：

```sh
# 指定 live 的头文件及库的根目录
cmake -DLIVE_ROOT_FORCE=/../live/install/usr/local ..
```

### GNU Make 编译

```sh
make CROSS_COMPILER_PREFIX=arm-linux-gnueabi-
```

- `CROSS_COMPILER_PREFIX` 为交叉编译的工具链前缀，可以包含绝对路径，
  你可以根据你的需求来设定工具链路径。
