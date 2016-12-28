安装说明
=======

## 构建

XPR 环境构建命令

    make [BUILD_OPTIONS] [XPR_MODULE_OPTIONS] [XPR_MODULE_DRIVER_OPTIONS]

### BUILD_OPTIONS

- BUILD_ALL = [n/y] 全局开关
- BUILD_SRC = [n/y] 指定是否构建 xpr 库
- BUILD_EXAMPLES = [n/y] 指定是否构建示例程序
- BUILD_TESTS = [n/y] 指定是否构建测试程序
- BUILD_DOCS = [n/y] 指定是否构建帮助文档

实际选项可参见 ./Makefile 文件中相关代码

### XPR_MODULE_OPTIONS

- XPR_ALL = [n/y] 指定是否构建所有模块
- XPR_AVFRAME = [n/y] 指定是否构建 AVFRAME 模块
- XPR_BASE64 = [n/y]
- XPR_DEVCAPS = [n/y]
- XPR_DPU = [n/y]
- XPR_FIFO = [n/y]
- XPR_JSON = [n/y]
- XPR_MD5 = [n/y]
- XPR_MEM = [n/y]
- XPR_PLUGIN = [n/y]
- XPR_STREAMBLOCK = [n/y]
- XPR_SYNC = [n/y]
- XPR_SYS = [n/y]
- XPR_THREAD = [n/y]
- XPR_UPS = [n/y]
- XPR_URL = [n/y]
- XPR_UTILS = [n/y]

实际选项可参见 ./src/Modules.mk 文件中相关代码，你也可以以上参数写在 ./src/Modules.local 文件中。

### XPR_MODULE_DRIVER_OPTIONS

- XPR_DPU_DRIVER_A5SVIDEO = [n/y] 指定是否集成 A5SVIDEO 驱动
- XPR_DPU_DRIVER_A5SYUV = [n/y] 指定是否集成 A5SYUV 驱动
- XPR_DPU_DRIVER_ALSAPCM = [n/y]
- XPR_DPU_DRIVER_MDSD = [n/y]

实际选项可参见 ./src/Modules.mk 文件中相关代码

### 使用示例

只编译 XPR 库

    make BUILD_ALL=n BUILD_SRC=y

只编译帮助文档

    make BUILD_ALL=n BUILD_DOCS=y

不集成 ALSAPCM DPU 驱动

    make XPR_DPU_DRIVER_ALSAPCM=n

