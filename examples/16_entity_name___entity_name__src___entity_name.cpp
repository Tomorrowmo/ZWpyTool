/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

/******************************************************************/
/* ZW3D API 包含 */
#include "zwapi_cmd_paramdefine_param.h"
#include "zwapi_cmd_paramdefine_tpl.h"
#include "zwapi_layer.h"
#include "zwapi_general_ent.h"

/******************************************************************/
/* 应用程序包含 */
#include <string>
#include "stdlib.h"
#include "..\inc\EntityNamePr.h"

/******************************************************************/
/* 数据类型定义 */
/* DESCRIPTION: 表单字段ID */
enum EntityNameField
{
    Field_Number_Start = 4, /* 图层的起始编号 */
    Field_Number_End = 5,   /* 图层的结束编号 */
    Field_Name_Label = 6,   /* 名称标签 */
    Field_Name_Number = 7,  /* 名称编号 */
};

/******************************************************************/
/* 函数声明 */
static int EntityName(int idData);

/******************************************************************/
/* 函数定义 */
int RegisterEntityName
(
void
)
/*
DESCRIPTION:
   注册模板命令的回调函数。
*/
{
    /* 通过输入命令字符串 "!EntityName" 启动命令 */
    ZwCommandFunctionLoad("EntityName", (void *)EntityName, ZW_LICENSE_CODE_GENERAL);
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UnloadEntityName
(
void
)
/*
DESCRIPTION:
   卸载模板命令的回调函数。
*/
{
    ZwCommandFunctionUnload("EntityName");
    return 0;
}

/******************************************************************/
/* 函数定义 */
int EntityName
(
int idData /* I: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令的执行函数。当命令在OK或APPLY按钮上被点击时调用该函数。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("EntityName");

    double startLayer = 0, endLayer = 0, nameNumber = 0;
    if (cvxDataGetNum(idData, Field_Number_Start, &startLayer))
        return 1;
    if (cvxDataGetNum(idData, Field_Number_End, &endLayer))
        return 1;
    if (cvxDataGetNum(idData, Field_Name_Number, &nameNumber))
        return 1;

    svxData label{};
    if (cvxDataGet(idData, Field_Name_Label, &label))
        return 1;

    int exitLayerCnt = 0;
    vxName *layerName = nullptr;
    if (cvxLayerList(&exitLayerCnt, &layerName))
        return 1;

    ezwErrors err = ZW_API_NO_ERROR;
    for (int i = startLayer; i <= endLayer; i++)
    {
        if (cvxLayerExists2(i))
        {
            int numEntInCurLayer = 0;
            svxEntPath *entPathList = nullptr;
            cvxLayerInqEnts(i, &numEntInCurLayer, &entPathList);

            szwEntityHandle *entityList = nullptr;
            ZwMemoryAlloc(numEntInCurLayer * sizeof(szwEntityHandle), (void **)&entityList);
            if (ZwEntityPathTransfer(numEntInCurLayer, entPathList, entityList))
                return 1;

            for (int j = 0; j < numEntInCurLayer; j++)
            {
                std::string strname = label.Text + std::to_string((int)nameNumber);
                if (ZwEntityNameSet(entityList[j], strname.c_str()))
                    return 1;
                if (ZwEntityNameTagSet(entityList[j], strname.c_str()))
                    return 1;
            }
            for (int j = 0; j < numEntInCurLayer; j++)
                ZwEntityHandleFree(&entityList[j]);
        }
        nameNumber++;
    }

    return 0;
}