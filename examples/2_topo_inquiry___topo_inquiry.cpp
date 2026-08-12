
#include "stdio.h"
#include <cstring>
#include <set>
#include "VxApi.h"

#define BUFFER 256
#define INQ_OUTPUT_FORM "InqOutputWindow"

int InqPartShpFace(int idData);
int InqPartShpEdge(int idData);
int InqFaceLoop(int idData);
int InqEdgeUVCrv(int idData);

int TopoInquiryInit
(
int format,
void *data
)
{
    vxPath ApiPath;

    cvxCmdFunc("InqPartShpFace", (void *)InqPartShpFace, VX_CODE_GENERAL);
    cvxCmdFunc("InqPartShpEdge", (void *)InqPartShpEdge, VX_CODE_GENERAL);
    cvxCmdFunc("InqFaceLoop", (void *)InqFaceLoop, VX_CODE_GENERAL);
    cvxCmdFunc("InqEdgeUVCrv", (void *)InqEdgeUVCrv, VX_CODE_GENERAL);

    cvxPathApiLib("TopoInquiry", ApiPath);
    cvxPathAdd(ApiPath);

    return 0;
}

int TopoInquiryExit
(
void
)
{
    cvxCmdFuncUnload("InqPartShpFace");
    cvxCmdFuncUnload("InqPartShpEdge");
    cvxCmdFuncUnload("InqFaceLoop");
    cvxCmdFuncUnload("InqEdgeUVCrv");
    return 0;
}

int InqPartShpFace
(
int idData
)
{
    int i, iRet = 0;
    int idShape;
    int iCntFace = 0, *idFaceList = NULL;
    char sBuf[BUFFER];

    iRet = cvxFormCreate(INQ_OUTPUT_FORM, 0);

    cvxItemDel(INQ_OUTPUT_FORM, 1, -1);

    iRet = cvxDataGetEnt(idData, 1, &idShape, NULL);
    if (iRet)
    {
        cvxItemAdd(INQ_OUTPUT_FORM, 1, "获取造型失败。");
        cvxFormShow(INQ_OUTPUT_FORM);
    }

    iRet = cvxPartInqShapeFaces(idShape, &iCntFace, &idFaceList);

    for (i = 0; i < iCntFace; i++)
    {
        //sprintf_s(sBuf, BUFFER, "面%d属于造型%d", idFaceList[i], idShape);
        cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
    }

    cvxFormShow(INQ_OUTPUT_FORM);

    cvxMemFree((void **)&idFaceList);

    return iRet;
}

int InqPartShpEdge
(
int idData
)
{
    int i, iRet = 0;
    int idShape;
    int iCntEdge = 0, *idEdgeList = NULL;
    char sBuf[BUFFER];

    iRet = cvxFormCreate(INQ_OUTPUT_FORM, 0);
    if (iRet)
        return iRet;

    cvxItemDel(INQ_OUTPUT_FORM, 1, -1);

    iRet = cvxDataGetEnt(idData, 1, &idShape, NULL);
    if (iRet)
    {
        cvxItemAdd(INQ_OUTPUT_FORM, 1, "获取造型失败。");
        cvxFormShow(INQ_OUTPUT_FORM);
        return iRet;
    }

    iRet = cvxPartInqShapeEdges(idShape, &iCntEdge, &idEdgeList);
    if (iRet)
        return iRet;

    for (i = 0; i < iCntEdge; i++)
    {
        //sprintf_s(sBuf, BUFFER, "边%d属于造型%d", idEdgeList[i], idShape);
        cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
    }

    cvxFormShow(INQ_OUTPUT_FORM);

    cvxMemFree((void **)&idEdgeList);

    return iRet;
}

int InqFaceLoop
(
int idData
)
{
    char sBuf[BUFFER];
    int i, j, iRet = 0;
    int idFace = -1, iCntLoop = 0, *idLoops = NULL;
    int iCntEdge = 0, *idEdges = NULL;

    iRet = cvxFormCreate(INQ_OUTPUT_FORM, 0);
    if (iRet)
        return iRet;

    cvxItemDel(INQ_OUTPUT_FORM, 1, -1);

    iRet = cvxDataGetEnt(idData, 1, &idFace, NULL);
    if (iRet)
    {
        cvxItemAdd(INQ_OUTPUT_FORM, 1, "获取面失败。");
        cvxFormShow(INQ_OUTPUT_FORM);
        return iRet;
    }

    iRet = cvxPartInqFaceLoops(idFace, 1, &iCntLoop, &idLoops);

    for (i = 0; i < iCntLoop; i++)
    {
        //sprintf_s(sBuf, BUFFER, "环%d属于面%d", idLoops[i], idFace);
        cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);

        iRet = cvxPartInqLoopEdges(idLoops[i], &iCntEdge, &idEdges);

        std::set<int> idEdgeSet;
        std::set<int>::iterator iter;

        for (j = 0; j < iCntEdge; j++)
        {
            idEdgeSet.insert(idEdges[j]);
        }

        for (iter = idEdgeSet.begin(); iter != idEdgeSet.end(); iter++)
        {
            //sprintf_s(sBuf, BUFFER, "边%d属于环%d", *iter, idLoops[i]);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
        }

        cvxMemFree((void **)&idEdges);
    }

    cvxFormShow(INQ_OUTPUT_FORM);

    cvxMemFree((void **)&idLoops);
    cvxMemFree((void **)&idEdges);

    return iRet;
}

int InqEdgeUVCrv
(
int idData
)
{
    int i, j, iRet = 0;
    char sBuf[BUFFER];
    int idEdge = -1, iCntFace = 0, *idFaceList = NULL;
    svxCurve Crv;

    iRet = cvxFormCreate(INQ_OUTPUT_FORM, 0);
    if (iRet)
        return iRet;

    cvxItemDel(INQ_OUTPUT_FORM, 1, -1);

    iRet = cvxDataGetEnt(idData, 1, &idEdge, NULL);
    if (iRet)
    {
        cvxItemAdd(INQ_OUTPUT_FORM, 1, "获取边失败。");
        cvxFormShow(INQ_OUTPUT_FORM);
        return iRet;
    }

    iRet = cvxPartInqEdgeFaces(idEdge, &iCntFace, &idFaceList);

    for (i = 0; i < iCntFace; i++)
    {
        //sprintf_s(sBuf, BUFFER, "边%d属于面%d", idEdge, idFaceList[i]);
        cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);

        memset((void *)&Crv, 0, sizeof(svxCurve));
        cvxMatInit(&Crv.Frame);

        iRet = cvxPartInqEdgeCrv(idEdge, idFaceList[i], &Crv);

        switch (Crv.Type)
        {
        case VX_CRV_LINE:
            //sprintf_s(sBuf, BUFFER, "曲线类型是直线。");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "线起点：(%lf, %lf, %lf)。", Crv.P1.x, Crv.P1.y, Crv.P1.z);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "线终点：(%lf, %lf, %lf)。", Crv.P2.x, Crv.P2.y, Crv.P2.z);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            break;

        case VX_CRV_ARC:
            //sprintf_s(sBuf, BUFFER, "曲线类型是圆弧。");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            if (Crv.P1.x || Crv.P1.y || Crv.P1.z)
            {
                //sprintf_s(sBuf, BUFFER, "圆弧中心点：(%lf, %lf, %lf)。", Crv.P1.x, Crv.P1.y, Crv.P1.z);
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            }
            //sprintf_s(sBuf, BUFFER, "圆弧起始角度（度）：%lf", Crv.A1);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "圆弧终止角度（度）：%lf", Crv.A2);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "圆弧半径：%lf", Crv.R);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            break;

        case VX_CRV_CIRCLE:
            //sprintf_s(sBuf, BUFFER, "曲线类型是圆。");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            if (Crv.P1.x || Crv.P1.y)
            {
                //sprintf_s(sBuf, BUFFER, "圆心点：(%lf, %lf, %lf)。", Crv.P1.x, Crv.P1.y, Crv.P1.z);
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            }
            //sprintf_s(sBuf, BUFFER, "圆半径：%lf", Crv.R);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            break;

        case VX_CRV_NURB:
            //sprintf_s(sBuf, BUFFER, "曲线类型是NURBS曲线。");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            if (Crv.T.closed)
            {
                //sprintf_s(sBuf, BUFFER, "闭合曲线");
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            }
            else
            {
                //sprintf_s(sBuf, BUFFER, "开放曲线");
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            }
            //sprintf_s(sBuf, BUFFER, "次数：%d", Crv.T.degree);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "参数空间的边界 [%lf, %lf]", Crv.T.bnd.min, Crv.T.bnd.max);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "节点数：%d", Crv.T.num_knots);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "节点值数组：");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            for (j = 0; j < Crv.T.num_knots; j++)
            {
                //sprintf_s(sBuf, BUFFER, "%lf", Crv.T.knots[j]);
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            }

            if (Crv.P.rat)
            {
                //sprintf_s(sBuf, BUFFER, "有理控制点");
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            }
            else
            {
                //sprintf_s(sBuf, BUFFER, "非有理控制点");
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            }

            //sprintf_s(sBuf, BUFFER, "控制点超平面类型：%d", Crv.P.plane);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);

            //sprintf_s(sBuf, BUFFER, "控制点边界框：");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "X [%f, %f]", Crv.P.box.X.min, Crv.P.box.X.max);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "Y [%f, %f]", Crv.P.box.Y.min, Crv.P.box.Y.max);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "Z [%f, %f]", Crv.P.box.Z.min, Crv.P.box.Z.max);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);

            //sprintf_s(sBuf, BUFFER, "每个控制点的坐标数：%d", Crv.P.dim);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);

            //sprintf_s(sBuf, BUFFER, "控制点数：%d", Crv.P.num_cp);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);

            //sprintf_s(sBuf, BUFFER, "控制点坐标：");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            for (j = 0; j < (Crv.P.num_cp * Crv.P.dim); j++)
            {
                //sprintf_s(sBuf, BUFFER, "%lf", Crv.P.coord[j]);
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            }
            break;

        default:
            cvxItemAdd(INQ_OUTPUT_FORM, 1, "未知的曲线类型");
            break;
        }

        //sprintf_s(sBuf, BUFFER, "局部坐标系：");
        cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
        if (Crv.Frame.identity)
        {
            //sprintf_s(sBuf, BUFFER, "单位矩阵");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
        }
        else
        {
            //sprintf_s(sBuf, BUFFER, "X轴的余弦和原点X (xt)");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "%lf, %lf, %lf, %lf", Crv.Frame.xx, Crv.Frame.yx, Crv.Frame.zx, Crv.Frame.xt);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "Y轴的余弦和原点Y (yt)");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "%lf, %lf, %lf, %lf", Crv.Frame.xy, Crv.Frame.yy, Crv.Frame.zy, Crv.Frame.yt);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "Z轴的余弦和原点Z (zt)");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
            //sprintf_s(sBuf, BUFFER, "%lf, %lf, %lf, %lf", Crv.Frame.xz, Crv.Frame.yz, Crv.Frame.zz, Crv.Frame.zt);
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
        }
        if (i < iCntFace - 1)
        {
            //sprintf_s(sBuf, BUFFER, "==================================");
            cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
        }

        cvxCurveFree(&Crv);
    }
    cvxFormShow(INQ_OUTPUT_FORM);

    cvxMemFree((void **)&idFaceList);
    cvxCurveFree(&Crv);

    return iRet;
}