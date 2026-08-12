/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

/******************************************************************/
/* ZW3D API 包含 */
#include "zwapi_cmd_paramdefine_param.h"
#include "zwapi_cmd_paramdefine_tpl.h"

#include "zwapi_feature_shape.h"
#include "zwapi_cmd_shape.h"
#include "zwapi_cmd_freeform.h"
#include "zwapi_cmd_wireframe.h"
#include "zwapi_general_ent.h"
#include "zwapi_feature_wireframe.h"
#include "zwapi_entity.h"

/******************************************************************/
/* 应用程序包含 */
#include "..\inc\ExTrudewithPreviewPr.h"
#include <vector>

/******************************************************************/
/* 数据类型定义 */
/* DESCRIPTION: 表单字段ID */
enum ExTrudewithPreviewField
{
    Field_PROFILE = 1,        // 轮廓
    Field_START_DISTANCE = 2, // 起始距离
    Field_END_DISTANCE = 3,   // 结束距离
    Field_DIRECTION = 4,      // 方向
};

/******************************************************************/
/* 函数声明 */
static int ExTrudewithPreview(int idData);
static void ExTrudewithPreviewInit(int idData);
static void ExTrudewithPreviewEcho(int idData, void *ohEcho);

int RegisterExTrudewithPreview
(
void
)
/*
DESCRIPTION:
   注册模板命令的回调函数。
*/
{
    /* 通过输入命令字符串 "!ExTrudewithPreview" 启动命令 */
    ZwCommandFunctionLoad("ExTrudewithPreview", (void *)ExTrudewithPreview, ZW_LICENSE_CODE_GENERAL);
    ZwCommandCallbackLoad("ExTrudewithPreviewInit", (void *)ExTrudewithPreviewInit);
    ZwCommandCallbackLoad("ExTrudewithPreviewEcho", (void *)ExTrudewithPreviewEcho);
    return 0;
}

int UnloadExTrudewithPreview
(
void
)
/*
DESCRIPTION:
   卸载模板命令的回调函数。
*/
{
    ZwCommandFunctionUnload("ExTrudewithPreview");
    ZwCommandFunctionUnload("ExTrudewithPreviewInit");
    ZwCommandFunctionUnload("ExTrudewithPreviewEcho");
    return 0;
}

int ExTrudewithPreview
(
int idData /* I: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令的执行函数。当命令在OK或APPLY按钮上被点击时调用该函数。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("ExTrudewithPreview");

    /* 查询数据容器的数据 */
    svxFldData *fldData = nullptr;
    int numField = 0;
    if (cvxDataGetAll(idData, &numField, &fldData))
        return 1;

    if (numField >= 3 && fldData[0].count > 0)
    {
        std::vector<szwEntityHandle> curves{};
        std::vector<szwEntityHandle> curveList{};
        std::vector<szwEntityHandle> sketches{};
        std::vector<szwEntityHandle> sketch3Ds{};
        int entCount = fldData[0].count;

        /*分类*/
        for (int i = 0; i < entCount; i++)
        {
            int idEnt = 0;
            idEnt = fldData[0].fld_data[i].idEntity;
            szwEntityHandle tempEnt{};
            ZwEntityIdTransfer(1, &idEnt, &tempEnt);

            int isCurve = 0;
            ezwEntityType type = ZW_ENTITY_ALL;
            if (ZwEntityTypeNumberGet(tempEnt, &type))
                return 1;
            ZwEntityCurveCheck(tempEnt, &isCurve);
            if (type == ZW_ENTITY_CURVE_LIST)
            {
                curveList.push_back(tempEnt);
            }
            else if (type != ZW_ENTITY_CURVE_LIST && isCurve)
            {
                curves.push_back(tempEnt);
            }
            else if (type == ZW_ENTITY_SKETCH)
            {
                sketches.push_back(tempEnt);
            }
            else if (type == ZW_ENTITY_SKETCH_3D)
            {
                sketch3Ds.push_back(tempEnt);
            }
        }

        /*初始化*/
        szwExtrudeData extrudeData{};
        if (ZwFeatureExtrudeInit(&extrudeData))
            return 1;
        extrudeData.startS = fldData[1].fld_data->Num;
        extrudeData.endE = fldData[2].fld_data->Num;
        if (numField == 4)
            extrudeData.direction = (szwVector)fldData[3].fld_data->Dir;

        /*拉伸*/
        if (curves.size() > 0)
        {
            szwEntityHandle crvList{};
            if (ZwFeatureCurvelistCreate(curves.size(), curves.data(), &crvList))
                return 1;

            extrudeData.profileHandle = crvList;
            szwEntityHandle shape{};
            if (ZwFeatureExtrudeCreate(extrudeData, &shape))
                return 1;
            ZwEntityHandleFree(&crvList);
            ZwEntityHandleFree(&shape);
        }

        for (int i = 0; i < curveList.size(); i++)
        {
            extrudeData.profileHandle = curveList[i];
            szwEntityHandle shape{};
            if (ZwFeatureExtrudeCreate(extrudeData, &shape))
                return 1;
            ZwEntityHandleFree(&shape);
            ZwEntityHandleFree(&curves[i]);
        }

        for (int i = 0; i < sketches.size(); i++)
        {
            extrudeData.profileHandle = sketches[i];
            szwEntityHandle shape{};
            if (ZwFeatureExtrudeCreate(extrudeData, &shape))
                return 1;
            ZwEntityHandleFree(&shape);
            ZwEntityHandleFree(&sketches[i]);
        }

        for (int i = 0; i < sketch3Ds.size(); i++)
        {
            extrudeData.profileHandle = sketch3Ds[i];
            szwEntityHandle shape{};
            if (ZwFeatureExtrudeCreate(extrudeData, &shape))
                return 1;
            ZwEntityHandleFree(&shape);
            ZwEntityHandleFree(&sketch3Ds[i]);
        }
    }

    return 0;
}

void ExTrudewithPreviewEcho
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

    cvxMsgDisp("ExTrudewithPreviewEcho");
    svxFldData *fldData = nullptr;
    int numField = 0;
    if (cvxDataGetAll(idData, &numField, &fldData))
        return;

    if (numField >= 3 && fldData[0].count > 0)
    {
        std::vector<int> curves{};
        std::vector<szwEntityHandle> curveList{};
        std::vector<szwEntityHandle> sketches{};
        std::vector<szwEntityHandle> sketch3Ds{};
        int entCount = fldData[0].count;

        /*分类*/
        for (int i = 0; i < entCount; i++)
        {
            int idEnt = 0;
            idEnt = fldData[0].fld_data[i].idEntity;
            szwEntityHandle tempEnt{};
            ZwEntityIdTransfer(1, &idEnt, &tempEnt);

            int isCurve = 0;
            ezwEntityType type = ZW_ENTITY_ALL;
            if (ZwEntityTypeNumberGet(tempEnt, &type))
                return;
            ZwEntityCurveCheck(tempEnt, &isCurve);
            if (type == ZW_ENTITY_CURVE_LIST)
            {
                curveList.push_back(tempEnt);
            }
            else if (type != ZW_ENTITY_CURVE_LIST && isCurve)
            {
                curves.push_back(idEnt);
            }
            else if (type == ZW_ENTITY_SKETCH)
            {
                sketches.push_back(tempEnt);
            }
            else if (type == ZW_ENTITY_SKETCH_3D)
            {
                sketch3Ds.push_back(tempEnt);
            }
        }


        szwExtrudeData extrudeData{};
        if (ZwFeatureExtrudeInit(&extrudeData))
            return;
        extrudeData.startS = fldData[1].fld_data->Num;
        extrudeData.endE = fldData[2].fld_data->Num;
        if (numField == 4)
            extrudeData.direction = (szwVector)fldData[3].fld_data->Dir;

        /*拉伸*/
        if (curves.size() > 0)
        {
            szwEntityHandle crvList{};
            int idCrvlist = 0;
            if (cvxPartCrvList(curves.size(), curves.data(), &idCrvlist))
                return;

            if (ZwEntityIdTransfer(1, &idCrvlist, &crvList))
                return;

            extrudeData.profileHandle = crvList;
            if (ZwFeatureExtrudeCreate(extrudeData, NULL))
                return;

            ZwEntityHandleFree(&crvList);
        }

        for (int i = 0; i < curveList.size(); i++)
        {
            extrudeData.profileHandle = curveList[i];
            if (ZwFeatureExtrudeCreate(extrudeData, NULL))
                return;
        }

        for (int i = 0; i < sketches.size(); i++)
        {
            extrudeData.profileHandle = sketches[i];
            if (ZwFeatureExtrudeCreate(extrudeData, NULL))
                return;
            ZwEntityHandleFree(&sketches[i]);
        }

        for (int i = 0; i < sketch3Ds.size(); i++)
        {
            extrudeData.profileHandle = sketch3Ds[i];
            if (ZwFeatureExtrudeCreate(extrudeData, NULL))
                return;
            ZwEntityHandleFree(&sketch3Ds[i]);
        }
    }

    cvxEchoEnd();
}

void ExTrudewithPreviewInit
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
    svxData endDis{};
    endDis.isNumber = 1;
    endDis.Num = 15;
    endDis.NumType = VX_DST;
    cvxDataSet(idData, Field_END_DISTANCE, &endDis);

    svxData stDis{};
    stDis.isNumber = 1;
    stDis.Num = 0;
    stDis.NumType = VX_DST;
    cvxDataSet(idData, Field_START_DISTANCE, &stDis);

    cvxMsgDisp("ExTrudewithPreviewInit");
}