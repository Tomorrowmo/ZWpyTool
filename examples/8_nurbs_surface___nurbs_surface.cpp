/***
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include <Windows.h>
#include "stdio.h"
#include "VxApi.h"

/*** 声明你的函数 */
int NurbsSurface(void);

int NurbsSurfaceInit
(
int format,
void *data
)
{
    vxPath ApiPath;

    /*** 在ZW3D中注册你的函数。 */
    cvxCmdFunc("NurbsSurface", (void *)NurbsSurface, VX_CODE_GENERAL);

    /*** 获取包含"NurbsSurface.dll"的文件夹的路径
    并将文件夹目录添加到搜索路径列表中。 */
    TCHAR szBuff[MAX_PATH];
    HMODULE module = GetModuleHandle(L"NurbsSurface.dll");
    GetModuleFileName(module, szBuff, sizeof(szBuff));
    int nLength = WideCharToMultiByte(CP_ACP, 0, szBuff, -1, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, szBuff, -1, ApiPath, nLength, NULL, NULL);
    vxPath ApiDir;
    cvxPathDir(ApiPath, ApiDir);
    cvxPathAdd(ApiDir);

    return 0;
}

int NurbsSurfaceExit
(
void
)
{
    /*** 在这里放置你的清理代码。 */
    cvxCmdFuncUnload("NurbsSurface");
    return 0;
}

/*** 创建一个NURBS修剪曲面。 */
/*** 如果函数失败则返回1，否则返回0。 */
int NurbsSurface
(
void
)
{
    svxSurface zSurface;
    svxCurve *pTrimCurves = NULL;

    int idFace = -1;
    int iRet = 0;

    /*** 外环控制点的坐标 */
    double outCurve_cps[][4] = {{0.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 1.0, 0.0}, {1.0, 0.0, 0.0, 0.0}};

    /*** 外环曲线的边界框 */
    double outCurve_bnds[][4] = {
        {0.0, 0.0, 0.0, 1.0}, {0.0, 1.0, 1.0, 1.0}, {1.0, 1.0, 0.0, 1.0}, {0.0, 1.0, 0.0, 0.0}};

    /*** 外环的节点 */
    double outCurve_knots[] = {0.0, 0.0, 1.0, 1.0};

    /*** 内环控制点的坐标 */
    double inCurve_cps[] = {0.7000000000, 0.5000000000, 1.0000000000, 0.3500000000, 0.0767949192, 0.5000000000,
                            0.4000000000, 0.3267949192, 1.0000000000, 0.0500000000, 0.2500000000, 0.5000000000,
                            0.4000000000, 0.6732050808, 1.0000000000, 0.3500000000, 0.4232050808, 0.5000000000,
                            0.7000000000, 0.5000000000, 1.0000000000};

    /*** 内环的节点 */
    double inCurve_knots[] = {0.0000000000, 0.0000000000, 0.0000000000, 0.3333333333, 0.3333333333,
                              0.6666666667, 0.6666666667, 1.0000000000, 1.0000000000, 1.0000000000};

    /*** 曲面在U方向和V方向的节点 */
    double u_knots[] = {0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 1.0, 1.0};
    double v_knots[] = {0.0, 0.0, 1.0, 1.0};

    /*** 曲面控制点的坐标 */
    double surf_cps[] = {80,  40, 40, 80,  40, 0, 80,  -40, 0, 80,  -40, -40,
                         200, 40, 40, 200, 40, 0, 200, -40, 0, 200, -40, -40};

    /*** 初始化修剪曲线 */
    iRet = cvxMemAlloc(sizeof(svxCurve) * 5, (void **)&pTrimCurves);
    if (iRet)
    {
        cvxMsgDisp("内存分配失败。");
        return iRet;
    }

    cvxMemZero((void *)pTrimCurves, sizeof(svxCurve) * 5);

    /*** 设置修剪曲面。 */
    /*** 曲面的类型 */
    zSurface.Type = VX_SRF_EXTRUDE;

    /*** 曲面自然法线的方向 */
    zSurface.OutNormal = 0;

    /*** U方向的NURB参数空间数据 */
    zSurface.U.closed = 0;
    zSurface.U.degree = 3;
    zSurface.U.num_knots = 8;
    zSurface.U.knots = u_knots;
    zSurface.U.bnd.min = 0.0;
    zSurface.U.bnd.max = 1.0;

    /*** V方向的NURB参数空间数据 */
    zSurface.V.closed = 0;
    zSurface.V.degree = 1;
    zSurface.V.num_knots = 4;
    zSurface.V.knots = v_knots;
    zSurface.V.bnd.min = 0.0;
    zSurface.V.bnd.max = 1.0;

    /*** NURB控制点数据（毫米） */
    zSurface.P.dim = 3;
    zSurface.P.rat = 0;
    zSurface.P.plane = 4;
    zSurface.P.num_cp = 8;
    zSurface.P.coord = surf_cps;

    zSurface.P.box.X.min = 80;
    zSurface.P.box.X.max = 200;
    zSurface.P.box.Y.min = -40;
    zSurface.P.box.Y.max = 40;
    zSurface.P.box.Z.min = -40;
    zSurface.P.box.Z.max = 40;

    /*** 设置外环修剪曲线。 */
    for (int i = 0; i < 4; i++)
    {
        pTrimCurves[i].Type = VX_CRV_NURB;

        pTrimCurves[i].P.rat = 0;
        pTrimCurves[i].P.dim = 2;
        pTrimCurves[i].P.plane = 2;
        pTrimCurves[i].P.num_cp = 2;
        pTrimCurves[i].P.coord = outCurve_cps[i];

        pTrimCurves[i].P.box.X.min = outCurve_bnds[i][0];
        pTrimCurves[i].P.box.X.max = outCurve_bnds[i][1];
        pTrimCurves[i].P.box.Y.min = outCurve_bnds[i][2];
        pTrimCurves[i].P.box.Y.max = outCurve_bnds[i][3];
        pTrimCurves[i].P.box.Z.min = 0.0;
        pTrimCurves[i].P.box.Z.max = 0.0;

        pTrimCurves[i].T.closed = 0;
        pTrimCurves[i].T.degree = 1;
        pTrimCurves[i].T.num_knots = 4;
        pTrimCurves[i].T.knots = outCurve_knots;
        pTrimCurves[i].T.bnd.min = 0.0;
        pTrimCurves[i].T.bnd.max = 1.0;
    }

    /*** 设置内环修剪曲线（圆）。 */
    pTrimCurves[4].Type = VX_CRV_NURB;

    pTrimCurves[4].P.rat = 1;
    pTrimCurves[4].P.dim = 3;
    pTrimCurves[4].P.plane = 3;
    pTrimCurves[4].P.num_cp = 7;
    pTrimCurves[4].P.coord = inCurve_cps;

    pTrimCurves[4].P.box.X.min = 0.0;
    pTrimCurves[4].P.box.X.max = 1.0;
    pTrimCurves[4].P.box.Y.min = 0.0;
    pTrimCurves[4].P.box.Y.max = 1.0;
    pTrimCurves[4].P.box.Z.min = 0.0;
    pTrimCurves[4].P.box.Z.max = 0.0;

    pTrimCurves[4].T.closed = 1;
    pTrimCurves[4].T.degree = 2;
    pTrimCurves[4].T.num_knots = 10;
    pTrimCurves[4].T.knots = inCurve_knots;
    pTrimCurves[4].T.bnd.min = 0.0;
    pTrimCurves[4].T.bnd.max = 1.0;

    pTrimCurves[4].Frame.xx = 1;
    pTrimCurves[4].Frame.yy = 1;
    pTrimCurves[4].Frame.zz = 1;
    pTrimCurves[4].Frame.identity = 1;


    /*** 将修剪曲面添加到活动部分。 */
    iRet = cvxPartFace(&zSurface, 5, pTrimCurves, 1, 0, 0.0, &idFace);
    if (iRet)
    {
        cvxMsgDisp("无法将曲面添加到活动部分。");
    }

    /*** 释放曲线的内存。 */
    cvxMemFree((void **)&pTrimCurves);

    return iRet;
}