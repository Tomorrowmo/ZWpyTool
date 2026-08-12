/***
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/


#include "stdio.h"
#include "float.h"
#include "VxApi.h"

int FilletBox(int idData);

int FilletBoxInit
(
int format,
void *data
)
{
    vxPath ApiPath;
    cvxCmdFunc("FilletBox", (void *)FilletBox, VX_CODE_GENERAL);

    /*** 注册命令模板文件 */
    cvxPathApiLib("FilletBox", ApiPath);
    cvxPathAdd(ApiPath);


    return (0);
}

int FilletBoxExit
(
void
)
{
    /*** 在这里放置您的清理代码 */
    cvxCmdFuncUnload("FilletBox");
    return (0);
}

int FilletBox
(
int idData
)
{
    int idShape = -1, idPlane = -1, idFilletBox = -1, idFilletFace = -1;
    int ret = 0, i = 0, nOp = -1, nBoxFaceCnt = 0, nEdgeCnt = 0;
    int *listFaces = NULL, *listEdges = NULL;

    double dRadius = 0.0, dDistance = 0.0, dMinDist = DBL_MAX;

    svxPoint *pListPrjPnts = NULL;
    svxPoint zSeedPnt;
    svxMatrix zPlaneMat;
    svxBoxData zBoxData;

    zBoxData.Combine = VX_BOOL_NONE;
    zBoxData.idPlane = -1;

    nOp = cvxOpCount();

    /*** 获取输入数据*/
    ret = cvxDataGetNum(idData, 1, &zBoxData.Center.x);
    ret = cvxDataGetNum(idData, 2, &zBoxData.Center.y);
    ret = cvxDataGetNum(idData, 3, &zBoxData.Center.z);
    ret = cvxDataGetNum(idData, 4, &zBoxData.X);
    ret = cvxDataGetNum(idData, 5, &zBoxData.Y);
    ret = cvxDataGetNum(idData, 6, &zBoxData.Z);
    ret = cvxDataGetNum(idData, 7, &dRadius);
    if (ret = cvxDataGetEnt(idData, 8, &idPlane, NULL))
        idPlane = 0;

    zBoxData.idPlane = idPlane;

    /*** 创建一个盒子 */
    if (ret = cvxPartBox(&zBoxData, &idShape))
    {
        cvxMsgDisp("无法创建盒子");
    }

    /*** 获取靠近顶面的点 */
    if (0 < idPlane)
    {
        if (ret = cvxEntMatrix(idPlane, &zPlaneMat))
        {
            cvxMsgDisp("无法获取矩阵");
        }
        zPlaneMat.xt = 0.0;
        zPlaneMat.yt = 0.0;
        zPlaneMat.zt = 0.0;
        zSeedPnt.x = zBoxData.Center.x;
        zSeedPnt.y = zBoxData.Center.y;
        zSeedPnt.z = zBoxData.Center.z + (zBoxData.Z) / 2;
        cvxPntTransform(&zPlaneMat, &zSeedPnt);
    }
    else
    {
        zSeedPnt.x = zBoxData.Center.x;
        zSeedPnt.y = zBoxData.Center.y;
        zSeedPnt.z = zBoxData.Center.z + (zBoxData.Z) / 2;
    }

    /*** 将点投影到每个面上 */
    if (ret = cvxPartInqShapeFaces(idShape, &nBoxFaceCnt, &listFaces))
    {
        cvxMsgDisp("无法获取面");
    }
    if (ret = cvxMemAlloc(sizeof(svxPoint) * nBoxFaceCnt, (void **)&pListPrjPnts))
    {
        cvxMsgDisp("无法为投影点分配内存");
    }

    /*** 查找顶面 */
    for (i = 0; i < nBoxFaceCnt; i++)
    {
        if (ret = cvxPntProject(&zSeedPnt, listFaces[i], &pListPrjPnts[i]))
        {
            cvxMsgDisp("无法获取投影点");
        }

        dDistance = cvxPntDist(&zSeedPnt, &pListPrjPnts[i]);

        /*** 获取最近的面 */
        if (dDistance < dMinDist)
        {
            idFilletFace = listFaces[i];
            dMinDist = dDistance;
        }
    }

    /*** 获取边 */
    if (ret = cvxPartInqFaceEdges(idFilletFace, &nEdgeCnt, &listEdges))
    {
        cvxMsgDisp("无法获取顶面的边");
    }

    /*** 倒角操作 */
    if (ret = cvxPartFillet(nEdgeCnt, listEdges, dRadius))
    {
        cvxMsgDisp("倒角失败");
    }

    /*** 获取形状ID */
    idFilletBox = cvxEntNew(nOp, VX_ENT_SHAPE);

    cvxMemFree((void **)&listFaces);
    cvxMemFree((void **)&listEdges);
    cvxMemFree((void **)&pListPrjPnts);
    return ret;
}