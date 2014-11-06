#ifndef XPR_GPIO_H
#define XPR_GPIO_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum XPR_GPIO_Mode {
    XPR_GPIO_MODE_INPUT,
    XPR_GPIO_MODE_OUTPUT,
    XPR_GPIO_MODE_HW,
} XPR_GPIO_Mode;

int XPR_GPIO_Init(void);

int XPR_GPIO_Fini(void);

int XPR_GPIO_Get(int port, int* value);

int XPR_GPIO_Set(int port, int value);

int XPR_GPIO_GetMode(int port, int* mode);

int XPR_GPIO_SetMode(int port, int mode);

#ifdef __cplusplus
}
#endif

#endif // XPR_GPIO_H

