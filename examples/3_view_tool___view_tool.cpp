/***
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/


#include "stdio.h"
#include "VxApi.h"

#define Buffer 256
#define InformationBox "InformationBox"

int gsVisible = 0;
int gsFrozen = 0;
int gsDispMode = 1;
int gsDispAttribute = 0;

/*** 命令函数声明 */
int LayerInquire(int idData);
int LayerSet(int idData);
int LayerAdd(int idData);
int LayerDel(int idData);
int EntColorGet(int idData);
int EntColorSet(int idData);
void ViewGet(void);
int ViewSet(int idData);
int DispModeSet(int idData);
int DispColorSet(int idData);
void MsgAreaClose(void);
void MsgAreaOpen(void);
void PromptEnable(void);
void PromptDisable(void);

int ViewToolInit
(
int format,
void *data
)
{
    vxPath ApiPath;

    /*** 将您的函数注册到ZW3D中 */
    cvxCmdFunc("LayerInquire", (void *)LayerInquire, VX_CODE_GENERAL);
    cvxCmdFunc("LayerSet", (void *)LayerSet, VX_CODE_GENERAL);
    cvxCmdFunc("LayerAdd", (void *)LayerAdd, VX_CODE_GENERAL);
    cvxCmdFunc("LayerDel", (void *)LayerDel, VX_CODE_GENERAL);
    cvxCmdFunc("EntColorGet", (void *)EntColorGet, VX_CODE_GENERAL);
    cvxCmdFunc("EntColorSet", (void *)EntColorSet, VX_CODE_GENERAL);
    cvxCmdFunc("ViewGet", (void *)ViewGet, VX_CODE_GENERAL);
    cvxCmdFunc("ViewSet", (void *)ViewSet, VX_CODE_GENERAL);
    cvxCmdFunc("DispModeSet", (void *)DispModeSet, VX_CODE_GENERAL);
    cvxCmdFunc("DispColorSet", (void *)DispColorSet, VX_CODE_GENERAL);
    cvxCmdFunc("MsgAreaClose", (void *)MsgAreaClose, VX_CODE_GENERAL);
    cvxCmdFunc("MsgAreaOpen", (void *)MsgAreaOpen, VX_CODE_GENERAL);
    cvxCmdFunc("PromptEnable", (void *)PromptEnable, VX_CODE_GENERAL);
    cvxCmdFunc("PromptDisable", (void *)PromptDisable, VX_CODE_GENERAL);

    /*** 注册全局变量 */
    cvxCmdVariable("gsVisible", (void *)&gsVisible);
    cvxCmdVariable("gsFrozen", (void *)&gsFrozen);
    cvxCmdVariable("gsDispMode", (void *)&gsDispMode);
    cvxCmdVariable("gsDispAttribute", (void *)&gsDispAttribute);

    /*** 获取包含"ViewTool.dll"的"apilibs"文件夹的路径，
       并将该文件夹目录添加到搜索路径列表中 */
    cvxPathApiLib("ViewTool", ApiPath);
    cvxPathAdd(ApiPath);

    return (0);
}

int ViewToolExit
(
void
)
{
    /*** 在这里放置您的清理代码 */
    cvxCmdFuncUnload("LayerInquire");
    cvxCmdFuncUnload("LayerSet");
    cvxCmdFuncUnload("LayerAdd");
    cvxCmdFuncUnload("LayerDel");
    cvxCmdFuncUnload("EntColorGet");
    cvxCmdFuncUnload("EntColorSet");
    cvxCmdFuncUnload("ViewGet");
    cvxCmdFuncUnload("ViewSet");
    cvxCmdFuncUnload("DispModeSet");
    cvxCmdFuncUnload("DispColorSet");
    cvxCmdFuncUnload("MsgAreaClose");
    cvxCmdFuncUnload("MsgAreaOpen");
    cvxCmdFuncUnload("PromptEnable");
    cvxCmdFuncUnload("PromptDisable");
    return (0);
}

/*** 查询指定图层信息 */
int LayerInquire
(
int idData
)
{
    int ret = 0;
    int IsVisible = 0, IsFrozen = 0;
    vxName LyrName = "";

    if (ret = cvxFormCreate(InformationBox, 0))
        return 0;

    cvxItemDel(InformationBox, 1, -1);

    if (ret = cvxDataGetText(idData, 1, 32, LyrName))
    {
        cvxMsgDisp("图层不存在");
        return 0;
    }

    if (ret = cvxLayerStateGet(LyrName, &IsVisible, &IsFrozen))
    {
        cvxMsgDisp("查询图层状态失败");
        return 0;
    }

    if (1 == IsVisible)
    {
        if (1 == IsFrozen)
        {
            cvxItemAdd(InformationBox, 1, "图层可见且已冻结");
        }
        else
        {
            cvxItemAdd(InformationBox, 1, "图层可见且处于激活的状态");
        }
    }
    if (0 == IsVisible)
    {
        if (1 == IsFrozen)
        {
            cvxItemAdd(InformationBox, 1, "图层不可见且已冻结");
        }
        else
        {
            cvxItemAdd(InformationBox, 1, "图层不可见且已冻结");
        }
    }
    cvxFormShow(InformationBox);
    return 0;
}

/*** 设置指定图层状态 */
int LayerSet
(
int idData
)
{
    int ret = 0;
    vxName LyrName;

    if (ret = cvxDataGetText(idData, 1, 32, LyrName))
    {
        cvxMsgDisp("无法获取名称");
        return 0;
    }

    if (ret = cvxLayerStateSet(LyrName, gsVisible, gsFrozen))
    {
        cvxMsgDisp("无法设置状态");
        return 0;
    }
    return 0;
}

/*** 创建图层 */
int LayerAdd
(
int idData
)
{
    int ret = 0;
    vxName LyrName;
    if (ret = cvxDataGetText(idData, 1, 32, LyrName))
    {
        cvxMsgDisp("无法获取图层");
        return 0;
    }
    if (ret = cvxLayerAdd(LyrName))
    {
        cvxMsgDisp("无法添加图层");
        return 0;
    }
    return 0;
}

/*** 删除指定图层 */
int LayerDel
(
int idData
)
{
    int ret = 0;
    vxName LyrName;
    if (ret = cvxDataGetText(idData, 1, 32, LyrName))
    {
        cvxMsgDisp("无法获取数据");
        return 0;
    }
    if (ret = cvxLayerDel(LyrName))
    {
        cvxMsgDisp("无法删除图层");
        return 0;
    }
    return 0;
}

/*** 获取实体颜色 */
int EntColorGet
(
int idData
)
{
    int ret = 0;
    int idEntity;
    evxColor Color;

    if (ret = cvxFormCreate(InformationBox, 0))
        return 0;

    cvxItemDel(InformationBox, 1, -1);

    if (ret = cvxDataGetEnt(idData, 1, &idEntity, NULL))
    {
        cvxMsgDisp("无法获取实体");
        return 0;
    }
    if (ret = cvxEntColorGet(idEntity, &Color))
    {
        cvxMsgDisp("无法获取颜色");
        return 0;
    }

    switch (Color)
    {
    case VX_COLOR_RGB:
        cvxMsgDisp("颜色是RGB");
        cvxItemAdd(InformationBox, 1, "颜色是RGB");
        break;

    case VX_COLOR_NULL:
        cvxMsgDisp("颜色是空");
        cvxItemAdd(InformationBox, 1, "颜色是空");
        break;

    case VX_COLOR_GREEN:
        cvxMsgDisp("颜色是绿色");
        cvxItemAdd(InformationBox, 1, "颜色是绿色");
        break;

    case VX_COLOR_RED:
        cvxMsgDisp("颜色是红色");
        cvxItemAdd(InformationBox, 1, "颜色是红色");
        break;

    case VX_COLOR_BROWN:
        cvxMsgDisp("颜色是棕色");
        cvxItemAdd(InformationBox, 1, "颜色是棕色");
        break;

    case VX_COLOR_GOLDENROD:
        cvxMsgDisp("颜色是金rod");
        cvxItemAdd(InformationBox, 1, "颜色是金rod");
        break;

    case VX_COLOR_MED_BLUE:
        cvxMsgDisp("颜色是中蓝色");
        cvxItemAdd(InformationBox, 1, "颜色是中蓝色");
        break;

    case VX_COLOR_DARK_MAGENTA:
        cvxMsgDisp("颜色是深品红色");
        cvxItemAdd(InformationBox, 1, "颜色是深品红色");
        break;

    case VX_COLOR_DARK_GREY:
        cvxMsgDisp("颜色是深灰色");
        cvxItemAdd(InformationBox, 1, "颜色是深灰色");
        break;

    case VX_COLOR_DARK_BLUE:
        cvxMsgDisp("颜色是深蓝色");
        cvxItemAdd(InformationBox, 1, "颜色是深蓝色");
        break;

    case VX_COLOR_VIOLET:
        cvxMsgDisp("颜色是紫色");
        cvxItemAdd(InformationBox, 1, "颜色是紫色");
        break;

    case VX_COLOR_LIGHT_GREEN:
        cvxMsgDisp("颜色是浅绿色");
        cvxItemAdd(InformationBox, 1, "颜色是浅绿色");
        break;

    case VX_COLOR_LIGHT_BLUE:
        cvxMsgDisp("颜色是浅蓝色");
        cvxItemAdd(InformationBox, 1, "颜色是浅蓝色");
        break;

    case VX_COLOR_ROSE:
        cvxMsgDisp("颜色是玫瑰色");
        cvxItemAdd(InformationBox, 1, "颜色是玫瑰色");
        break;

    case VX_COLOR_LIGHT_MAGENTA:
        cvxMsgDisp("颜色是浅品红色");
        cvxItemAdd(InformationBox, 1, "颜色是浅品红色");
        break;

    case VX_COLOR_LIGHT_GREY:
        cvxMsgDisp("颜色是浅灰色");
        cvxItemAdd(InformationBox, 1, "颜色是浅灰色");
        break;

    case VX_COLOR_BLACK:
        cvxMsgDisp("颜色是黑色");
        cvxItemAdd(InformationBox, 1, "颜色是黑色");
        break;

    case VX_COLOR_WHITE:
        cvxMsgDisp("颜色是白色");
        cvxItemAdd(InformationBox, 1, "颜色是白色");
        break;

    case VX_COLOR_YELLOW:
        cvxMsgDisp("颜色是黄色");
        cvxItemAdd(InformationBox, 1, "颜色是黄色");
        break;

    case VX_COLOR_MED_GREY:
        cvxMsgDisp("颜色是中灰色");
        cvxItemAdd(InformationBox, 1, "颜色是中灰色");
        break;

    default:
        break;
    }
    cvxFormShow(InformationBox);
    return 0;
}

/*** 设置指定实体颜色 */
int EntColorSet
(
int idData
)
{
    int ret = 0;
    int nCnt = 0;
    int *idEntities = NULL;
    double ColorNum;

    if (ret = cvxDataGetEnts(idData, 1, &nCnt, &idEntities))
    {
        cvxMsgDisp("无法获取数据");
        return 0;
    }
    if (ret = cvxDataGetNum(idData, 2, &ColorNum))
    {
        cvxMsgDisp("默认颜色");
        ColorNum = VX_COLOR_BLACK;
    }

    if (ret = cvxEntColorSet((evxColor)ColorNum, nCnt, idEntities))
    {
        cvxMsgDisp("无法设置颜色");
        return 0;
    }

    return 0;
}

/*** 获取视图信息 */
void ViewGet
(
void
)
{
    int ret = 0;
    svxMatrix matrix;
    double dViewExtent = 200.0;
    char sViewExtent[Buffer] = {""};
    char sOrigin[Buffer] = {""};
    char sAxis[Buffer] = {""};

    if (ret = cvxFormCreate(InformationBox, 0))
        return;

    cvxItemDel(InformationBox, 1, -1);

    /*** 输出视图范围信息 */
    cvxViewGet(&matrix, &dViewExtent);
    //sprintf_s(sViewExtent, Buffer, "视图范围是 : %lf 毫米", dViewExtent);
    cvxItemAdd(InformationBox, 1, sViewExtent);

    /*** 输出激活的视图的原点 */
    //sprintf_s(sOrigin, Buffer, "视图原点是 (%lf, %lf, %lf)", matrix.xt, matrix.yt, matrix.zt);
    cvxItemAdd(InformationBox, 1, sOrigin);

    /*** 输出激活的视图的X轴 */
    //sprintf_s(sAxis, Buffer, "视图X轴是 (%lf, %lf, %lf)", matrix.xx, matrix.yx, matrix.zx);
    cvxItemAdd(InformationBox, 1, sAxis);

    /*** 输出激活的视图的Y轴 */
    //sprintf_s(sAxis, Buffer, "视图Y轴是 (%lf, %lf, %lf)", matrix.xy, matrix.yy, matrix.zy);
    cvxItemAdd(InformationBox, 1, sAxis);

    /*** 输出激活的视图的Z轴 */
    //sprintf_s(sAxis, Buffer, "视图Z轴是 (%lf, %lf, %lf)", matrix.xz, matrix.yz, matrix.zz);
    cvxItemAdd(InformationBox, 1, sAxis);

    /*** 显示信息窗口 */
    cvxFormShow(InformationBox);
}

/*** 设置视图 */
int ViewSet
(
int idData
)
{
    int ret = 0;
    double dViewExtent = 200.0;
    svxPoint origin = {0.0, 0.0, 0.0};

    svxData axisDir;
    svxMatrix matrix;

    /*** 初始化 */
    cvxMatInit(&matrix);

    /*** 获取视图原点 */
    if (ret = cvxDataGetPnt(idData, 1, &origin))
    {
        cvxMsgDisp("无法获取视图原点输入");
        return 0;
    }
    else
    {
        matrix.xt = origin.x;
        matrix.yt = origin.y;
        matrix.zt = origin.z;
    }

    /*** 获取视图X轴 */
    if (ret = cvxDataGet(idData, 2, &axisDir) || !axisDir.isDirection)
    {
        cvxMsgDisp("无法获取视图X轴输入");
        return 0;
    }
    else
    {
        matrix.xx = axisDir.Dir.x;
        matrix.yx = axisDir.Dir.y;
        matrix.zx = axisDir.Dir.z;
    }

    /*** 获取视图Y轴 */
    if (ret = cvxDataGet(idData, 3, &axisDir) || !axisDir.isDirection)
    {
        cvxMsgDisp("无法获取视图Y轴输入");
        return 0;
    }
    else
    {
        matrix.xy = axisDir.Dir.x;
        matrix.yy = axisDir.Dir.y;
        matrix.zy = axisDir.Dir.z;
    }

    /*** 获取视图Z轴 */
    if (ret = cvxDataGet(idData, 4, &axisDir) || !axisDir.isDirection)
    {
        cvxMsgDisp("无法获取视图Z轴输入");
        return 0;
    }
    else
    {
        matrix.xz = axisDir.Dir.x;
        matrix.yz = axisDir.Dir.y;
        matrix.zz = axisDir.Dir.z;
    }

    /*** 确保与矩阵关联的"identity"标志正确设置 */
    cvxMatSetIdentity(&matrix);

    /*** 获取视图范围 */
    if (ret = cvxDataGetNum(idData, 5, &dViewExtent))
    {
        cvxMsgDisp("无法获取视图范围输入");
        return 0;
    }

    /*** 设置视图 */
    cvxViewSet(&matrix, dViewExtent);

    return 0;
}

/*** 设置显示模式 */
int DispModeSet
(
int idData
)
{
    cvxDispModeSet((evxDispMode)gsDispMode);
    return 0;
}

/*** 设置显示颜色 */
int DispColorSet
(
int idData
)
{
    int ret = 0;
    evxColor color;
    double IdColor;
    if (ret = cvxDataGetNum(idData, 2, &IdColor))
    {
        cvxMsgDisp("设置默认颜色");
        IdColor = VX_COLOR_BLACK;
    }
    color = (evxColor)IdColor;
    cvxDispColorSet((evxDispAttrib)gsDispAttribute, color);
    return 0;
}

/*** 打开消息区域 */
void MsgAreaOpen
(
void
)
{
    cvxMsgAreaOpen();
}

/*** 关闭消息区域 */
void MsgAreaClose
(
void
)
{
    cvxMsgAreaClose();
}

/*** 启用命令提示显示 */
void PromptEnable
(
void
)
{
    cvxPromptEnable();
    cvxMsgDisp("命令提示已启用");
}

/*** 禁用命令提示显示 */
void PromptDisable
(
void
)
{
    cvxPromptDisable();
    cvxMsgDisp("命令提示已禁用");
}