XPR 开发说明
===========


## 模块及驱动

### 模块相关

在 ``xpr_config.h`` 中定义了一系列用于判定某个模块是否已经启用的宏定义，
其格式为 ``HAVE_XPR_<MODULE>`` , 在模块代码中你可以使用 ``#if defined(HAVE_XPR_<MODULE>)`` 
代码来判断是否已经启用了该模块。

例如：

```
#if defined(HAVE_XPR_DPU)
XPR_DPU_Open(...);
...
#endif
```

### 模块驱动相关

在 ``xpr_config.h`` 中定义了一系列用于判定某个模块中某个驱动是否已经启用的宏定义，
其格式为 ``HAVE_XPR_<MODULE>_DRIVER_<DRIVERNAME>`` , 在模块代码中你可以使用
``#if defined(HAVE_XPR_<MODULE>_DRIVER_<DRIVERNAME>)`` 代码来判断是否已经启用了该模块。

例如：

```
#if defined(HAVE_XPR_DPU_DRIVER_ALSAPCM)
snd_pcm_open(...)
...
#endif
```
