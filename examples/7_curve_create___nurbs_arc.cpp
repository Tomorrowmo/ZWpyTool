/***
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include "stdio.h"
#include "VxApi.h"

/*** 声明你的函数 */
int NurbsArc(void);

int NurbsArcInit
(
int format,
void *data
)
{
    /*** 在ZW3D中注册你的函数。 */
    cvxCmdFunc("NurbsArc", (void *)NurbsArc, VX_CODE_GENERAL);

    return 0;
}

int NurbsArcExit
(
void
)
{
    /*** 在这里放置你的清理代码。 */
    cvxCmdFuncUnload("NurbsArc");
    return 0;
}

/*** 创建一个3D NURBS圆弧。 */
/*** 如果函数失败则返回1，否则返回0。 */
int NurbsArc
(
void
)
{
    svxCurve zCrv;
    int idEnt = -1;
    int iRet = 0;

    /*** 圆弧的节点 */
    double knot[] = {0.0000000000, 0.0000000000, 0.0000000000, 0.3333333333, 0.3333333333,
                     0.6666666667, 0.6666666667, 1.0000000000, 1.0000000000, 1.0000000000};

    /*** 控制点的坐标 */
    double coord[] = {40.0000000000,  0.0000000000,   0.0000000000,   1.0000000000,   28.2842712475,  28.2842712475,
                      0.0000000000,   0.7071067812,   0.0000000000,   40.0000000000,  0.0000000000,   1.0000000000,
                      -28.2842712475, 28.2842712475,  0.0000000000,   0.7071067812,   -40.0000000000, 0.0000000000,
                      0.0000000000,   1.0000000000,   -28.2842712475, -28.2842712475, 0.0000000000,   0.7071067812,
                      0.0000000000,   -40.0000000000, 0.0000000000,   1.0000000000};

    /*** 初始化圆弧 */
    cvxMemZero((void **)&zCrv, sizeof(svxCurve));

    /*** 曲线类型 */
    zCrv.Type = VX_CRV_NURB;

    /*** 局部坐标系（原点是圆弧中心） */
    zCrv.Frame.identity = 1;
    zCrv.Frame.xx = 1;
    zCrv.Frame.yy = 1;
    zCrv.Frame.zz = 1;

    /*** NURB参数空间数据 */
    zCrv.T.bnd.max = 1.0;
    zCrv.T.bnd.min = 0.0;
    zCrv.T.closed = 0;
    zCrv.T.degree = 2;
    zCrv.T.num_knots = 10;
    zCrv.T.knots = knot;
    zCrv.T.free_mem = 0;

    /*** NURB控制点数据（毫米） */
    zCrv.P.coord = coord;
    zCrv.P.dim = 4;
    zCrv.P.num_cp = 7;
    zCrv.P.plane = 3;
    zCrv.P.rat = 1;
    zCrv.P.free_mem = 0;

    /*** 将圆弧添加到活动部分 */
    iRet = cvxPartCurve(&zCrv, &idEnt);

    if (iRet)
        cvxMsgDisp("无法显示曲线。");

    return iRet;
}