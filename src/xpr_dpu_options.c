#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "xpr_dpu_options.h"

static const XPR_DPU_Option* nextOption(XPR_DPU* ctx, const XPR_DPU_Option* last)
{
    if (!ctx)
        return NULL;

    const XPR_DPU_Driver* drv = ctx->driver;

    if (!drv)
        return NULL;

    if (!last && drv->options && drv->options[0].name)
        return drv->options;

    if (last && last[1].name)
        return ++last;

    return NULL;
}

static const XPR_DPU_Option* findOption(XPR_DPU* ctx, const char* name)
{
    if (!ctx || !name)
        return NULL;

    const XPR_DPU_Option* opt = NULL;

    while ((opt = nextOption(ctx, opt)) != NULL) {
        if (strcmp(opt->name, name) == 0)
            return opt;
    }

    return NULL;
}

static int setFloat(XPR_DPU* ctx, const XPR_DPU_Option* opt, float value)
{
    if (ctx && ctx->privateData && opt)
        *((float*)(((char*)ctx->privateData)) + opt->offset) = value;

    return 0;
}

static int setDouble(XPR_DPU* ctx, const XPR_DPU_Option* opt, double value)
{
    if (ctx && ctx->privateData && opt)
        *((double*)(((char*)ctx->privateData)) + opt->offset) = value;

    return 0;
}

static int setInt(XPR_DPU* ctx, const XPR_DPU_Option* opt, int value)
{
    if (ctx && ctx->privateData && opt)
        *((int*)(((char*)ctx->privateData) + opt->offset)) = value;

    return 0;
}

static int setInt64(XPR_DPU* ctx, const XPR_DPU_Option* opt, int64_t value)
{
    if (ctx && ctx->privateData && opt)
        *((int64_t*)(((char*)ctx->privateData) + opt->offset)) = value;

    return 0;
}

static int setString(XPR_DPU* ctx, const XPR_DPU_Option* opt, const char* value)
{
    if (ctx && ctx->privateData && opt) {
        char* tmp = strdup(value);
        free((char**)(((char*)ctx->privateData) + opt->offset));
        *((char**)(((char*)ctx->privateData) + opt->offset)) = tmp;
    }

    return 0;
}

int XPR_DPU_SetDoubleOption(XPR_DPU* ctx, const char* name, double value)
{
    const XPR_DPU_Option* opt = findOption(ctx, name);

    if (!opt)
        return -1;

    return setDouble(ctx, opt, value);
}

int XPR_DPU_SetIntOption(XPR_DPU* ctx, const char* name, int value)
{
    const XPR_DPU_Option* opt = findOption(ctx, name);

    if (!opt)
        return -1;

    return setInt(ctx, opt, value);
}

int XPR_DPU_SetInt64Option(XPR_DPU* ctx, const char* name, int64_t value)
{
    const XPR_DPU_Option* opt = findOption(ctx, name);

    if (!opt)
        return -1;

    return setInt64(ctx, opt, value);
}

int XPR_DPU_SetStringOption(XPR_DPU* ctx, const char* name, const char* value)
{
    const XPR_DPU_Option* opt = findOption(ctx, name);

    if (!opt)
        return -1;

    return setString(ctx, opt, value);
}

int XPR_DPU_SetOption(XPR_DPU* ctx, const char* name, const void* data, int length)
{
    const XPR_DPU_Option* opt = findOption(ctx, name);

    if (!opt)
        return -1;

    switch (opt->type) {
    case XPR_DPU_OPT_CONST:
        /* Nothing to be done here */
        break;

    case XPR_DPU_OPT_FLAGS:
    case XPR_DPU_OPT_INT:
        if (length == sizeof(int))
            setInt(ctx, opt, *((int*)data));
        else if (length == 0)
            setInt(ctx, opt, (int)data);

        break;

    case XPR_DPU_OPT_INT64:
        if (length == sizeof(int64_t))
            setInt64(ctx, opt, *((int64_t*)data));
        else if (length == 0)
            setInt64(ctx, opt, (int64_t)(int)data);

        break;

    case XPR_DPU_OPT_DOUBLE:
        if (length == sizeof(double))
            setDouble(ctx, opt, *((double*)data));
        else if (length == 0)
            setDouble(ctx, opt, (double)(int)data);

        break;

    case XPR_DPU_OPT_FLOAT:
        if (length == sizeof(float))
            setFloat(ctx, opt, *((float*)data));
        else if (length == 0)
            setFloat(ctx, opt, (float)(int)data);

        break;

    case XPR_DPU_OPT_RATIONAL:
        //FIXME
        break;

    case XPR_DPU_OPT_STRING:
        setString(ctx, opt, (const char*)data);
        break;

    case XPR_DPU_OPT_BINARY:
        /* Cannot set default for binary */
        break;

    default:
        fprintf(stderr, "XPR_DPU_Option type %d of option %s not implemented yet\n", opt->type, opt->name);
        break;
    }

    return 0;
}

int XPR_DPU_GetOption(XPR_DPU* ctx, const char* name, void* buffer, int* size)
{
    const XPR_DPU_Option* opt = findOption(ctx, name);

    if (!opt)
        return -1;

    switch (opt->type) {
    case XPR_DPU_OPT_CONST:
        /* Nothing to be done here */
        break;

    case XPR_DPU_OPT_FLAGS:
    case XPR_DPU_OPT_INT:
        if (size && *size == sizeof(int64_t)) {
            *((int64_t*)buffer) = *((int*)(((char*)ctx->privateData) + opt->offset));
            *size = sizeof(int);
        }
        else
            *((int*)buffer) = *((int*)(((char*)ctx->privateData) + opt->offset));
        break;

    case XPR_DPU_OPT_INT64:
        if (size && *size == sizeof(int)) {
            *((int*)buffer) = *((int64_t*)(((char*)ctx->privateData) + opt->offset));
            *size = sizeof(int64_t);
        }
        else
            *((int64_t*)buffer) = *((int64_t*)(((char*)ctx->privateData) + opt->offset));
        break;

    case XPR_DPU_OPT_DOUBLE:
        if (size && *size == sizeof(float)) {
            *((float*)buffer) = *((double*)(((char*)ctx->privateData) + opt->offset));
            *size = sizeof(double);
        }
        else
            *((double*)buffer) = *((double*)(((char*)ctx->privateData) + opt->offset));
        break;

    case XPR_DPU_OPT_FLOAT:
        if (size && *size == sizeof(double)) {
            *((double*)buffer) = *((float*)(((char*)ctx->privateData) + opt->offset));
            *size = sizeof(float);
        }
        else
            *((float*)buffer) = *((float*)(((char*)ctx->privateData) + opt->offset));
        break;

    case XPR_DPU_OPT_RATIONAL:
        //FIXME
        break;

    case XPR_DPU_OPT_STRING:
        if (size && *size > 0) {
            strcpy((char*)buffer, *((const char**)(((char*)ctx->privateData) + opt->offset)));
        }
        else
            *((char**)buffer) = *((char**)(((char*)ctx->privateData) + opt->offset));
        break;

    case XPR_DPU_OPT_BINARY:
        /* Cannot set default for binary */
        break;

    default:
        fprintf(stderr, "XPR_DPU_Option type %d of option %s not implemented yet\n", opt->type, opt->name);
        break;
    }

    return 0;
}

void XPR_DPU_SetDefaultOptions(XPR_DPU* ctx)
{
    const XPR_DPU_Option* opt = NULL;

    while ((opt = nextOption(ctx, opt)) != NULL) {
        switch (opt->type) {
        case XPR_DPU_OPT_CONST:
            /* Nothing to be done here */
            break;

        case XPR_DPU_OPT_FLAGS:
        case XPR_DPU_OPT_INT:
            setInt(ctx, opt, (int)opt->default_val.i);
            break;

        case XPR_DPU_OPT_INT64:
            setInt64(ctx, opt, opt->default_val.i64);
            break;

        case XPR_DPU_OPT_DOUBLE:
            setFloat(ctx, opt, opt->default_val.flt);
            break;

        case XPR_DPU_OPT_FLOAT:
            setDouble(ctx, opt, opt->default_val.dbl);
            break;

        case XPR_DPU_OPT_RATIONAL:
            //FIXME
            break;

        case XPR_DPU_OPT_STRING:
            setString(ctx, opt, opt->default_val.str);
            break;

        case XPR_DPU_OPT_BINARY:
            /* Cannot set default for binary */
            break;

        default:
            fprintf(stderr, "XPR_DPU_Option type %d of option %s not implemented yet\n", opt->type, opt->name);
            break;
        }
    }
}

static const char* typeName(int type)
{
    switch (type) {
    case XPR_DPU_OPT_CONST:
        return "";

    case XPR_DPU_OPT_FLAGS:
        return "<flags>";

    case XPR_DPU_OPT_INT:
        return "<int>";

    case XPR_DPU_OPT_INT64:
        return "<int64>";

    case XPR_DPU_OPT_DOUBLE:
        return "<double>";

    case XPR_DPU_OPT_FLOAT:
        return "<float>";

    case XPR_DPU_OPT_RATIONAL:
        return "<rational>";

    case XPR_DPU_OPT_STRING:
        return "<string>";

    case XPR_DPU_OPT_BINARY:
        return "<binary>";

    default:
        break;
    }

    return "<unknow>";
}

void XPR_DPU_ShowOptions(XPR_DPU* ctx)
{
    const XPR_DPU_Option* opt = NULL;
    printf("XPR_DPU_Options:\n");
    printf("===============================================================\n");

    while ((opt = nextOption(ctx, opt)) != NULL) {
        printf("%-16s %-10s", opt->name, typeName(opt->type));

        switch (opt->type) {
        case XPR_DPU_OPT_CONST:
            /* Nothing to be done here */
            break;

        case XPR_DPU_OPT_FLAGS:
            printf("0x%08X", *((int*)(((char*)ctx->privateData) + opt->offset)));
            break;

        case XPR_DPU_OPT_INT:
            printf("%d", *((int*)(((char*)ctx->privateData) + opt->offset)));
            break;

        case XPR_DPU_OPT_INT64:
            printf("%lld", *((int64_t*)(((char*)ctx->privateData) + opt->offset)));
            break;

        case XPR_DPU_OPT_DOUBLE:
            printf("%g", *((double*)(((char*)ctx->privateData) + opt->offset)));
            break;

        case XPR_DPU_OPT_FLOAT:
            printf("%g", *((float*)(((char*)ctx->privateData) + opt->offset)));
            break;

        case XPR_DPU_OPT_RATIONAL:
            //FIXME
            break;

        case XPR_DPU_OPT_STRING:
            printf("%s", *((const char**)(((char*)ctx->privateData) + opt->offset)));
            break;

        case XPR_DPU_OPT_BINARY:
            /* Cannot set default for binary */
            break;

        default:
            break;
        }

        printf("\n");

        if (opt->help)
            printf("  [?] %s\n", opt->help);
    }

    printf("\n");
    printf("===============================================================\n");
}

