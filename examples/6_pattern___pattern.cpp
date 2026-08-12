/***
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include "stdio.h"
#include "VxApi.h"
#include <tuple>

// 线性阵列工具函数
int LinearPatternUtil(int idData);
// 线性阵列函数
int LinearPattern(int idData);
// 线性阵列结束响应鼠标移动消息
void LinearPatternEO(int idData, void *echoObj);

// 模式初始化函数
int PatternInit
(
int format,
void *data
)
{
    vxPath ApiPath;

    // 注册命令模板函数
    cvxCmdFunc("Pattern", (void *)LinearPattern, VX_CODE_GENERAL);
    cvxCmdCallback("PatternEO", (void *)LinearPatternEO);

    // 注册命令模板文件
    cvxPathApiLib("Pattern", ApiPath);
    cvxPathAdd(ApiPath);
    return (0);
}

// 模式退出函数
int PatternExit
(
void
)
{
    // 在此处放置清理代码
    cvxCmdFuncUnload("Pattern");
    return (0);
}

// 线性阵列工具函数实现
int LinearPatternUtil
(
int idData
)
{
    int iRet = 0;
    int nEnts = -1;
    int *listEnts = NULL;
    svxData zDirData;
    svxVector zDir = {1.0, 0.0, 0.0};
    svxAxis zSrc, zDst;
    double dSpace = 1.0;
    double nInstance = 1.0;

    /*** 初始化数据结构 */
    cvxMemZero((void *)&zDirData, sizeof(svxData));
    cvxMemZero((void *)&zSrc, sizeof(svxAxis));
    cvxMemZero((void *)&zDst, sizeof(svxAxis));

    /*** 获取实体 */
    iRet = cvxDataGetEnts(idData, 1, &nEnts, &listEnts);

    /*** 获取方向 */
    iRet = cvxDataGet(idData, 2, &zDirData);

    if (zDirData.isDirection)
    {
        zDir = zDirData.Dir;
    }
    else
    {
        iRet = 1;
    }

    /*** 获取沿方向上的间距 */
    iRet = cvxDataGetNum(idData, 3, &dSpace);

    /*** 获取实例数量 */
    iRet = cvxDataGetNum(idData, 4, &nInstance);

    /*** 对选定的实体进行阵列 */
    zSrc.Dir.x = zDst.Dir.x = 1.0;

    for (int i = 1; i < (int)nInstance; i++)
    {
        zDst.Pnt.x = zSrc.Pnt.x + zDir.x * i * dSpace;
        zDst.Pnt.y = zSrc.Pnt.y + zDir.y * i * dSpace;
        zDst.Pnt.z = zSrc.Pnt.z + zDir.z * i * dSpace;

        for (int j = 0; j < nEnts; j++)
        {
            int idCopyEnt = -1;
            iRet = cvxPartCopyPntToPnt(listEnts[j], &zSrc, &zDst, &idCopyEnt);
        }
    }

    cvxMemFree((void **)&listEnts);
    return iRet;
}

// 线性阵列函数实现
int LinearPattern
(
int idData
)
{
    return LinearPatternUtil(idData);
}

// 线性阵列结束函数实现
void LinearPatternEO
(
int idData,
void *echoObj /*** 内部使用 */
)
{
    cvxEchoStart();
    std::ignore = LinearPatternUtil(idData);
    cvxEchoEnd();
}