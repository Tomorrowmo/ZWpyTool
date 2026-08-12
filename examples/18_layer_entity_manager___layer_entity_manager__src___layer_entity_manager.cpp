/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

/******************************************************************/
/* ZW3D API 包含 */
#include "zwapi_cmd_paramdefine_param.h"
#include "zwapi_cmd_paramdefine_tpl.h"
#include "zwapi_layer.h"
#include "zwapi_ui_form.h"
#include "zwapi_entity.h"
#include "zwapi_global_apply.h"

/******************************************************************/
/* 应用程序包含 */
#include "..\inc\LayerEntityManagerPr.h"
#include <string>
#include <vector>

/******************************************************************/
/* 应用程序包含 */
#define FormName "LayerEntityManager"

/******************************************************************/
/* 数据类型定义 */
/* DESCRIPTION: 表单字段ID */
enum LayerEntityManagerField
{
    Field_Layer = 1,      /* 图层 */
    Field_EntityList = 2, /* 实体列表 */
    Field_Color = 3,      /* 实体颜色 */
};

/******************************************************************/
/* 函数声明 */
static int LayerEntityManager(int idData);
static int LayerEntityManagerInitA(int idData);

/******************************************************************/
/* 函数定义 */
int RegisterLayerEntityManager
(
void
)
/*
DESCRIPTION:
   注册模板命令的回调函数。
*/
{
    /* 通过输入命令字符串 "!LayerEntityManager" 启动命令 */
    ZwCommandFunctionLoad("LayerEntityManager", (void *)LayerEntityManager, ZW_LICENSE_CODE_GENERAL);
    ZwCommandCallbackLoad("LayerEntityManagerInitA", (void *)LayerEntityManagerInitA);
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UnloadLayerEntityManager
(
void
)
/*
DESCRIPTION:
   卸载模板命令的回调函数。
*/
{
    ZwCommandFunctionUnload("LayerEntityManager");
    ZwCommandFunctionUnload("LayerEntityManagerInitA");
    return 0;
}

/******************************************************************/
/* 函数定义 */
int LayerEntityManager
(
int idData /* I: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令的执行函数。当命令在OK或APPLY按钮上被点击时调用该函数。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("LayerEntityManager");
    /*ui数据获取 -- 图层*/
    int idOpt = cvxDataGetOpt(idData, Field_Layer);
    zwString64 layerName{};
    if (ZwUiFormItemGet(FormName, Field_Layer, idOpt, sizeof(zwString64), layerName))
        return 1;
    /*ui数据获取 -- 实体列表*/
    int nEnts = 0, *idEnts = 0;
    if (cvxDataGetEnts(idData, Field_EntityList, &nEnts, &idEnts))
        return 1;
    szwEntityHandle *ents = nullptr;
    ZwMemoryAlloc(nEnts * sizeof(szwEntityHandle), (void **)&ents);
    if (ZwEntityIdTransfer(nEnts, idEnts, ents))
        return 1;
    /*ui数据获取 -- 颜色*/
    svxColor inputRGB{};
    if (cvxDataGetColor(idData, Field_Color, &inputRGB))
        return 1;

    /*将实体分配到选定的图层*/
    if (cvxLayerAssign(layerName, nEnts, idEnts))
        return 1;
    /*设置实体颜色*/
    if (ZwEntityColorRgbSet(inputRGB, nEnts, ents))
        return 1;

    ZwMemoryFree((void **)&idEnts);
    for (int i = 0; i < nEnts; i++)
        ZwEntityHandleFree(&ents[i]);
    return 0;
}

/******************************************************************/
/* 函数定义 */
int LayerEntityManagerInitA
(
int idData /* I: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令的执行函数。当命令在OK或APPLY按钮上被点击时调用该函数。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("LayerEntityManagerInitA");

    /*查询图层列表*/
    int numLayer = 0;
    vxName *layerList = nullptr;
    if (cvxLayerList(&numLayer, &layerList))
        return 1;

    for (int i = 0; i < numLayer; i++)
    {
        cvxItemAdd(FormName, Field_Layer, layerList[i]);
    }
    cvxMemFree((void **)&layerList);
    return 0;
}