安装说明
=======

## 构建

XPR 环境构建命令

    make [BUILD_OPTIONS] [XPR_MODULE_OPTIONS] [XPR_MODULE_DRIVER_OPTIONS]

### BUILD_OPTIONS

- BUILD_ALL = [0/1] 全局开关
- BUILD_SRC = [0/1] 指定是否构建 xpr 库
- BUILD_EXAMPLES = [0/1] 指定是否构建示例程序
- BUILD_TESTS = [0/1] 指定是否构建测试程序
- BUILD_DOCS = [0/1] 指定是否构建帮助文档

### XPR_MODULE_OPTIONS

- XPR_ALL = [0/1] 指定是否构建所有模块
- XPR_AVFRAME = [0/1] 指定是否构建 AVFRAME 模块
- XPR_BASE64 = [0/1]
- XPR_DEVCAPS = [0/1]
- XPR_DPU = [0/1]
- XPR_FIFO = [0/1]
- XPR_JSON = [0/1]
- XPR_MD5 = [0/1]
- XPR_MEM = [0/1]
- XPR_PLUGIN = [0/1]
- XPR_STREAMBLOCK = [0/1]
- XPR_SYNC = [0/1]
- XPR_SYS = [0/1]
- XPR_THREAD = [0/1]
- XPR_UPS = [0/1]
- XPR_URL = [0/1]
- XPR_UTILS = [0/1]

### XPR_MODULE_DRIVER_OPTIONS

- XPR_DPU_DRIVER_A5SVIDEO = [0/1] 指定是否集成 A5SVIDEO 驱动
- XPR_DPU_DRIVER_A5SYUV = [0/1] 指定是否集成 A5SYUV 驱动
- XPR_DPU_DRIVER_ALSAPCM = [0/1]
- XPR_DPU_DRIVER_MDSD = [0/1]

### 使用示例

只编译 XPR 库

    make BUILD_ALL=0 BUILD_SRC=1

只编译帮助文档

    make BUILD_ALL=0 BUILD_DOCS=1

不集成 ALSAPCM DPU 驱动

    make XPR_DPU_DRIVER_ALSAPCM=0

