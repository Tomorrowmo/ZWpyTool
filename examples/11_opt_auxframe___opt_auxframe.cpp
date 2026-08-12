/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include "VxApi.h"
#include <stdlib.h>

#define FIELD_ENT 1
#define FIELD_POSITION 101
#define FIELD_X_AXIS 102
#define FIELD_Y_AXIS 103
#define FIELD_Z_AXIS 104
#define FIELD_MATRIX 201

/* 声明您的函数 */
static int OptAuxframe(int idData);
static int OptAuxframeEO(int idData, void *ohEcho);
static int OptAuxframeTerm(void);
static int OptAuxframeCb(char *form, int idField, int idData);


int OptAuxframeInit
(
void
)
/* 将您的函数注册到ZW3D中。 */
{
    cvxCmdFunc("OptAuxframe", (void *)OptAuxframe, VX_CODE_GENERAL);
    cvxCmdCallback("OptAuxframeEO", (void *)OptAuxframeEO);
    cvxCmdCallback("OptAuxframeTerm", (void *)OptAuxframeTerm);
    cvxCmdCallback("OptAuxframeCb", (void *)OptAuxframeCb);
    return 0;
}


int OptAuxframeExit
(
void
)
/* 在这里放置您的清理代码。 */
{
    cvxCmdFuncUnload("OptAuxframe");
    cvxCmdFuncUnload("OptAuxframeEO");
    cvxCmdFuncUnload("OptAuxframeTerm");
    cvxCmdFuncUnload("OptAuxframeCb");
    return 0;
}


int OptAuxframe
(
int idData
)
/*命令的执行函数。当在OK或APPLY按钮上点击命令时，将调用此函数。*/
{
    int idShape = -1;
    cvxDataGetEnt(idData, FIELD_ENT, &idShape, NULL);

    /* 获取移动句柄的矩阵 */
    svxMatrix matrix = {0};
    cvxDataGetMatrix(idData, FIELD_MATRIX, &matrix);

    /* 模态实体 */
    cvxPartMoveTransform(idShape, &matrix);

    /* 隐藏移动句柄 */
    OptAuxframeTerm();

    return 0;
}


int OptAuxframeEO
(
int idData,
void *ohEcho
)
/*命令窗口的预览回调函数，当命令的参数更改时，将调用此函数。*/
{
    cvxEchoStart();

    int idShape = -1;
    cvxDataGetEnt(idData, FIELD_ENT, &idShape, NULL);

    /* 获取移动句柄的矩阵 */
    svxMatrix matrix = {0};
    cvxDataGetMatrix(idData, FIELD_MATRIX, &matrix);

    /* 模态实体 */
    cvxPartMoveTransform(idShape, &matrix);

    cvxEchoEnd();
    return 0;
}


int OptAuxframeTerm
(
void
)
/*命令窗口的终止回调函数，当在CANCEL按钮上点击命令以退出时，将调用此函数。*/
{
    /* 隐藏移动句柄 */
    cvxAuxFrameShow(-1, 0, 0);

    return 0;
}


int OptAuxframeCb
(
char *form,
int idField,
int idData
)
/*命令字段的回调函数，当指定字段的值更改时，将调用此函数。*/
{
    if (idField == FIELD_ENT)
    {
        cvxMoveSetAuxFrameElem(0);
        cvxAuxFrameShow(idData, FIELD_ENT, FIELD_MATRIX);
        cvxAuxFrameSetDirXYZOrgBuddy(idData, FIELD_X_AXIS, FIELD_Y_AXIS, FIELD_Z_AXIS, FIELD_POSITION);
    }
    if (idField == FIELD_POSITION)
    {
        svxPoint origin = {};
        cvxDataGetPnt(idData, idField, &origin);
        cvxAuxFrameSetOrigin(&origin);
    }
    if (idField == FIELD_X_AXIS || idField == FIELD_Y_AXIS || idField == FIELD_Z_AXIS)
    {
        evxAxisType axisType = evxAxisType::VX_AXIS_X;
        if (idField == FIELD_Y_AXIS)
            axisType = VX_AXIS_Y;
        if (idField == FIELD_Z_AXIS)
            axisType = VX_AXIS_Z;

        svxData fldData = {};
        cvxDataGet(idData, idField, &fldData);
        if (fldData.isDirection)
        {
            svxVector axisDir = {};
            axisDir = fldData.Dir;
            cvxAuxFrameSetAxisVec(axisType, &axisDir);
        }
    }
    return 0;
}