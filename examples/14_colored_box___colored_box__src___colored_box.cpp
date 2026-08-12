/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

/******************************************************************/
/* ZW3D API 包含 */
#include "zwapi_cmd_paramdefine_param.h"
#include "zwapi_cmd_paramdefine_tpl.h"
#include "zwapi_feature_shape.h"
#include "zwapi_matrix_data.h"
#include "zwapi_general_ent.h"
#include "zwapi_part_facets.h"
#include "zwapi_face.h"
#include "zwapi_display.h"

/******************************************************************/
/* 应用程序包含 */
#include "..\inc\ColoredBoxPr.h"

/******************************************************************/
/* 数据类型定义 */
/* DESCRIPTION: 表单字段ID */
enum BoxedColorField
{
    Field_Length = 1, /* 盒子的长度 */
    Field_Width = 2,  /* 盒子的宽度 */
    Field_Height = 3, /* 盒子的高度 */
    Field_Origin = 4, /* 盒子的原点 */
    Field_Color = 5,  /* 盒子的颜色 */
};

/******************************************************************/
/* 函数声明 */
static int ColoredBox(int idData);
static void ColoredBoxEcho(int idData, void *ohEcho);
static int ColoredBoxCb(char *form, int idField, int item);

/******************************************************************/
/* 函数定义 */
int RegisterColoredBox
(
void
)
/*
DESCRIPTION:
   注册模板命令的回调函数。
*/
{
    /* 通过输入命令字符串 "!ColoredBox" 启动命令 */
    ZwCommandFunctionLoad("ColoredBox", (void *)ColoredBox, ZW_LICENSE_CODE_GENERAL);
    ZwCommandCallbackLoad("ColoredBoxEcho", (void *)ColoredBoxEcho);

    /* 注册表单的回调函数 */
    ZwCommandCallbackLoad("ColoredBoxCb", (void *)ColoredBoxCb);
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UnloadColoredBox
(
void
)
/*
DESCRIPTION:
   卸载模板命令的回调函数。
*/
{
    ZwCommandFunctionUnload("ColoredBox");
    ZwCommandFunctionUnload("ColoredBoxEcho");
    ZwCommandFunctionUnload("ColoredBoxCb");
    return 0;
}

/******************************************************************/
/* 函数定义 */
int ColoredBox
(
int idData /* I: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令的执行函数。当命令在OK或APPLY按钮上被点击时调用该函数。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("ColoredBox");
    cvxEchoEnd();
    /* 查询数据容器的数据 */
    svxData filedData[4] = {};
    szwCenterBoxData boxData{};
    if (ZwFeatureCenterBoxDataInit(&boxData))
        return 1;

    for (int i = Field_Length; i < Field_Color; i++)
    {
        if (cvxDataGet(idData, i, &filedData[i - 1]))
            return 1;
        switch (i)
        {
        case Field_Length:
            boxData.x = filedData[i - 1].Num;
            break;
        case Field_Width:
            boxData.y = filedData[i - 1].Num;
            break;
        case Field_Height:
            boxData.z = filedData[i - 1].Num;
            break;
        case Field_Origin:
            boxData.center = filedData[i - 1].Pnt;
            break;
        default:
            break;
        }
    }

    int count = 0;
    szwEntityHandle *boxList = nullptr;

    if (ZwFeatureBoxCreateByCenter(boxData, &count, &boxList))
        return 1;

    szwFaceDisplayAttribute dispAt{};
    if (ZwFaceDisplayAttributeGet(boxList[0], &dispAt))
        return 1;
    svxColor color{};
    cvxDataGetColor(idData, Field_Color, &color);
    dispAt.frontColor = color;

    if (ZwFaceDisplayAttributeSet(count, boxList, dispAt))
        return 1;

    ZwEntityHandleListFree(count, &boxList);

    return 0;
}

/******************************************************************/
/* 函数定义 */
void ColoredBoxEcho
(
int idData,  /* I: 数据容器的索引 */
void *ohEcho /* I: 预览对象的句柄 */
)
/*
DESCRIPTION:
   命令的预览回调函数，当命令的参数改变时调用该函数。
*/
{
    cvxEchoStart();

    // TODO: 执行一些操作
    cvxMsgDisp("ColoredBoxEcho");
    /* 查询数据容器的数据 */
    svxData filedData[4] = {};
    szwCenterBoxData boxData{};
    if (ZwFeatureCenterBoxDataInit(&boxData))
        return;

    for (int i = Field_Length; i < Field_Color; i++)
    {
        if (cvxDataGet(idData, i, &filedData[i - 1]))
            return;
        switch (i)
        {
        case Field_Length:
            boxData.x = filedData[i - 1].Num;
            break;
        case Field_Width:
            boxData.y = filedData[i - 1].Num;
            break;
        case Field_Height:
            boxData.z = filedData[i - 1].Num;
            break;
        case Field_Origin:
            boxData.center = filedData[i - 1].Pnt;
            break;
        default:
            break;
        }
    }

    int count = 0;
    szwEntityHandle *boxList = nullptr;

    if (ZwFeatureBoxCreateByCenter(boxData, &count, &boxList))
        return;

    ZwEntityHandleListFree(count, &boxList);

    cvxEchoEnd();
    return;
}

/******************************************************************/
/* 函数定义 */
int ColoredBoxCb
(
char *formName, /* I: 表单名称 */
int idField,    /* I: 命令参数字段的索引 */
int idData      /* I: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令参数字段的回调函数，当指定参数字段的值改变时调用该函数。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("ColoredBoxCb");

    int idxIn(idData);
    int ret(0);
    int iCmpNoAtFlag = false;
    if (idField == Field_Color)
    {
        svxColor color{};
        if (cvxDataGetColor(idData, Field_Color, &color))
            return 1;
        cvxDispRgbSet(VX_DISP_FACE, &color);
        cvxEchoDraw();
    }
    return 0;
}