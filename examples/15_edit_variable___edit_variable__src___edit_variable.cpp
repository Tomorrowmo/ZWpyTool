/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include "zwapi_cmd_paramdefine_param.h"
#include "zwapi_cmd_paramdefine_tpl.h"

#include "zwapi_general_ent.h"
#include "zwapi_part_var.h"
#include <vector>
#include "zwapi_variable.h"
#include "zwapi_part_history.h"
#include "zwapi_ui_tablewidget.h"
#include "zwapi_entity.h"

/******************************************************************/
/* 应用程序包含 */
#include "../inc/EditVariablePr.h"

/******************************************************************/
/*  全局变量声明 */
#define FORM_NAME "EditVariable"
#define SUB_FORM_NAME "Variable"
static int totalVarNum = 0;
std::vector<szwVariableData> allVaribleList{}; //当前部分的所有变量

/******************************************************************/
/* 数据类型定义 */

/* DESCRIPTION: EditVariable的表单字段 */
typedef enum EditVariableField
{
    ZW_FEATURE = 1,       // 特征
    ZW_EXPRESSION = 2,    // 表达式
    ZW_VARIABLE_LIST = 3, // 变量列表
} EditVariableField;

/* DESCRIPTION: 子表单Variable的字段 */
typedef enum VariableField
{
    ZW_VARIABLE_NAME = 1,  // 变量名
    ZW_VARIABLE_VALUE = 2, // 变量值
} VariableField;

/******************************************************************/
/* 函数声明 */
static void EditVariableInit(int idData);
static int EditVariable(int idData);
static int EditVariableCb(char *form, int idField, int item);
static int VariableCb(char *form, int idField, int item);
static void VariableEcho(int idData, void *ohEcho);


/******************************************************************/
/* 函数定义 */
int RegisterEditVariable
(
void
)
/*
DESCRIPTION:
   注册模板命令的回调函数。
*/
{
    /* 通过输入命令字符串 "!EditVariable" 启动命令 */
    ZwCommandFunctionLoad("EditVariable", (void *)EditVariable, ZW_LICENSE_CODE_GENERAL);
    ZwCommandFunctionLoad("EditVariableInit", (void *)EditVariableInit, ZW_LICENSE_CODE_GENERAL);
    ZwCommandCallbackLoad("EditVariableCb", (void *)EditVariableCb);

    ZwCommandCallbackLoad("VariableEcho", (void *)VariableEcho);
    ZwCommandCallbackLoad("VariableCb", (void *)VariableCb);
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UnloadEditVariable
(
void
)
/*
DESCRIPTION:
   卸载模板命令的回调函数。
*/
{
    ZwCommandFunctionUnload("EditVariableInit");
    ZwCommandFunctionUnload("EditVariable");
    ZwCommandFunctionUnload("VariableEcho");
    ZwCommandFunctionUnload("EditVariableCb");
    ZwCommandFunctionUnload("VariableCb");
    return 0;
}

/******************************************************************/
/* 函数定义 */
void EditVariableInit
(
int idData /* I: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令的初始化回调函数。当命令初始化时调用该函数。
   在此回调函数中，您可以初始化命令的数据，
   但此时命令表单尚未创建，因此不应在此初始化命令表单。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("EditVaribleInit");
    /* 获取活动文件中的所有变量 */
    int allVarNum = 0;
    szwVariableData *varList = nullptr;
    if (ZwVariableListGet(NULL, &allVarNum, &varList))
        return;
    for (int i = 0; i < allVarNum; i++)
        allVaribleList.push_back(varList[i]);
    ZwMemoryFree((void **)&varList);
    return;
}

/******************************************************************/
/* 函数定义 */
int EditVariable
(
int idData /* I: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令的执行函数。当命令在OK或APPLY按钮上被点击时调用该函数。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("EditVariable");
    cvxEchoEnd();
    /* 设置变量列表并重新生成 */
    if (ZwVariableListSet((int)allVaribleList.size(), allVaribleList.data(), 1))
        return 1;
    ZwEntityAutoRegen(ZW_REGEN_FOR_OUTDATED_OBJECTS, 1);
    /* 查询数据容器的数据 */

    return 0;
}

/******************************************************************/
/* 函数定义 */
int EditVariableCb
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
    cvxMsgDisp("EditVariableCb");

    std::vector<svxData> varNameList{};
    std::vector<svxData> varValueList{};
    if (idField == ZW_FEATURE)
    {
        /* 获取当前特征的变量列表 */
        svxData featureDada{};
        if (cvxDataGet(idData, idField, &featureDada))
            return 1;
        int idDataFeature = 0;
        if (cvxPartInqFtrData(featureDada.idEntity, 1, &idDataFeature) || idDataFeature < 0)
            return 1;

        // 通过遍历所有字段找到包含变量的字段。
        int feaFieldNum = 0;
        svxFldData *feaFldDataList = nullptr;
        if (cvxDataGetAll(idDataFeature, &feaFieldNum, &feaFldDataList))
            return 1;
        for (int i = 0; i < feaFieldNum; i++)
        {
            if (feaFldDataList[i].fld_type == VX_FLD_PNT || feaFldDataList[i].fld_type == VX_FLD_ENT ||
                feaFldDataList[i].fld_type == VX_FLD_TXT || feaFldDataList[i].fld_type == VX_FLD_DATA)
                continue;
            if (feaFldDataList[i].fld_data->isText)
            {
                /* 变量名 */
                svxData varName{};
                varName.isText = 1;
                strcpy_s(varName.Text, feaFldDataList[i].fld_data->Text);
                varNameList.push_back(varName);
                /* 变量值 */
                svxData varValue{};
                varValue.isNumber = 1;
                varValue.Num = feaFldDataList[i].fld_data->Num;
                varValueList.push_back(varValue);
            }
        }

        /* 表格数据设置 */
        for (int i = 0; i < varNameList.size(); i++)
        {
            // 激活子表单，在相应字段中记录引用的原始变量名。
            int idSubData = 0;
            if (cvxDataInit(SUB_FORM_NAME, &idSubData))
                return 1;
            if (cvxDataSet(idSubData, ZW_VARIABLE_NAME, &varNameList[i]))
                return 1;
            if (cvxDataSet(idSubData, ZW_VARIABLE_VALUE, &varValueList[i]))
                return 1;
            int idOut = 0;

            svxData temp{};
            temp.idEntity = idSubData;
            if (cvxDataSet(idData, ZW_VARIABLE_LIST, &temp))
                return 1;
        }
        ZwMemoryFree((void **)&feaFldDataList);
    }

    return 0;
}

/******************************************************************/
/* 函数定义 */
int VariableCb
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
    cvxMsgDisp("VariableCb");

    if (idField == ZW_VARIABLE_VALUE)
    {
        /* 获取用户输入的变量名和值 */
        svxData valueData{}, nameData{};
        if (cvxDataGet(idData, ZW_VARIABLE_VALUE, &valueData))
            return 1;
        if (cvxDataGet(idData, ZW_VARIABLE_NAME, &nameData))
            return 1;

        /* 更新变量列表 */
        szwVariableData tempVar{};
        strcpy_s(tempVar.name, nameData.Text);
        tempVar.type = ZW_VARIABLE_NUMBER;
        tempVar.value.numberValue.number = valueData.Num;

        for (int j = 0; j < allVaribleList.size(); j++)
        {
            if (strcmp(allVaribleList[j].name, tempVar.name) == 0)
            {
                allVaribleList[j] = tempVar;
            }
        }
    }

    return 0;
}

/******************************************************************/
/* 函数定义 */
void VariableEcho
(
int idData,
void *ohEcho
)
{
    cvxEchoStart();
    cvxMsgDisp("VariableEcho");
    /* 查询数据容器的数据 */
    if (ZwVariableListSet((int)allVaribleList.size(), allVaribleList.data(), 0))
        return;
    cvxEchoEnd();
    return;
}