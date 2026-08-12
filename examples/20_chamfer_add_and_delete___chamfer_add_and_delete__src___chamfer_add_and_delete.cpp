
/*******************************************************************/
#include "zwapi_cmd_paramdefine_param.h"
#include "zwapi_cmd_paramdefine_tpl.h"
#include "zwapi_edge.h"
#include "zwapi_entity.h"
#include "zwapi_cmd_shape.h"
#include "zwapi_util.h"
#include "zwapi_history.h"
#include "zwapi_command.h"
#include "zwapi_message.h"
#include "zwapi_memory.h"
#include "zwapi_part_history.h"

/*******************************************************************/
#include <vector>
#include <set>
#include "..\inc\ChamferAddAndDeletePr.h"

/*******************************************************************/
enum ChamferAddAndDeleteField
{
    Field_Edge_List = 2,
    Field_Distance = 3,
    Field_Same_Face_Flag = 4,

    Field_Feature_List = 6,
    Field_Limit_Distance_Flag = 7,
    Field_Distance_Limit = 8,
};

#define CHAMFER_FIELD_CHAMFERLIST 2
#define CHAMFERLIST_FIELD_SETBACK 2

/*******************************************************************/
/* 全局变量声明 */
int dataId = 0;
std::set<int> faceList{};

/******************************************************************/
/* 函数声明 */
static int ChamferAddAndDelete(int idData);
static void ChamferAddAndDeleteEcho(int idData, void *ohEcho);
static int ChamferAddAndDeleteCb(char *form, int idField, int item);
static int SameFaceEdge(int idEdge);
static int LimitDistance(int idFeature);
static std::set<int> SameFaceList(const int cntFace, szwEntityHandle *curfaces, const std::set<int> faceList);

/******************************************************************/
/* 函数定义 */
int RegisterChamferAddAndDelete
(
void
)
/*
DESCRIPTION:
   注册模板命令的回调函数。
*/
{
    /* 通过输入命令字符串 "!ChamferAddAndDelete" 启动命令 */
    ZwCommandFunctionLoad("ChamferAddAndDelete", (void *)ChamferAddAndDelete, ZW_LICENSE_CODE_GENERAL);
    ZwCommandCallbackLoad("ChamferAddAndDeleteEcho", (void *)ChamferAddAndDeleteEcho);
    ZwCommandCallbackLoad("ChamferAddAndDeleteCb", (void *)ChamferAddAndDeleteCb);
    ZwCommandCallbackLoad("LimitDistance", (void *)LimitDistance);
    ZwCommandCallbackLoad("SameFaceEdge", (void *)SameFaceEdge);
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UnloadChamferAddAndDelete
(
void
)
/*
DESCRIPTION:
   卸载模板命令的回调函数。
*/
{
    ZwCommandFunctionUnload("ChamferAddAndDelete");
    ZwCommandFunctionUnload("ChamferAddAndDeleteEcho");
    ZwCommandFunctionUnload("ChamferAddAndDeleteCb");
    ZwCommandFunctionUnload("LimitDistance");
    ZwCommandFunctionUnload("SameFaceEdge");
    return 0;
}

/******************************************************************/
/* 函数定义 */
int ChamferAddAndDelete
(
int idData /* I: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令的执行函数。当命令在OK或APPLY按钮上被点击时调用该函数。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("ChamferAddAndDelete");
    cvxEchoEnd();
    int addFlag = 1; /*flag for add or del, 1:add, 0,:del*/

    /* 查询数据容器中的数据 */
    int cntEdges = 0;
    int *idEdge = nullptr;
    cvxDataGetEnts(idData, Field_Edge_List, &cntEdges, &idEdge);

    int cntFea = 0;
    int *idFea = nullptr;
    cvxDataGetEnts(idData, Field_Feature_List, &cntFea, &idFea);

    if (cntEdges)
    {
        /*添加倒角*/
        svxChamFlltData chamfer{};
        cvxPartChamferAllInit(&chamfer);
        chamfer.type = VX_CHAMFLLT_SYMMETRIC;

        /* 初始化设置列表数据 */
        svxChamFlltSetData chamSet = {0};
        if (cvxChamFlltSetDataInit(&chamSet))
            return 1;
        double dis = 0.0;
        if (cvxDataGetNum(idData, Field_Distance, &dis))
            return 1;
        chamSet.setback1 = dis;
        chamSet.idEdge = idEdge;
        chamSet.nEdges = cntEdges;
        chamfer.cntList = 1;
        chamfer.pChamFlltDataList = &chamSet;

        int idChamfer = 0;
        if (cvxPartChamferAll(&chamfer, &idChamfer))
            return 1;
    }

    if (cntFea)
    {
        /*删除倒角特征*/
        szwEntityHandle *feature = nullptr;
        ZwMemoryAlloc(cntFea * sizeof(szwEntityHandle), (void **)&feature);
        if (ZwEntityIdTransfer(cntFea, idFea, feature))
            return 1;
        if (ZwHistoryOperationDelete(cntFea, feature, ZW_DELETE_ASSOCIATED_OPERATION))
            return 1;
        ZwEntityHandleListFree(cntFea, &feature);
    }

    return 0;
}

/******************************************************************/
/* 函数定义 */
void ChamferAddAndDeleteEcho
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

    cvxMsgDisp("ChamferAddAndDeleteEcho");
    /*------------------添加------------------------------*/

    int cntEdges = 0;
    int *idEdge = nullptr;
    cvxDataGetEnts(idData, Field_Edge_List, &cntEdges, &idEdge);
    if (cntEdges == 0)
        return;

    /*倒角*/
    svxChamFlltData chamfer{};
    cvxPartChamferAllInit(&chamfer);
    chamfer.type = VX_CHAMFLLT_SYMMETRIC;

    /* 设置列表数据 */
    svxChamFlltSetData chamSet = {0};
    if (cvxChamFlltSetDataInit(&chamSet))
        return;

    chamSet.idEdge = idEdge;
    chamSet.nEdges = cntEdges;
    double dis = 0.0;
    if (cvxDataGetNum(idData, Field_Distance, &dis))
        return;
    chamSet.setback1 = dis;

    chamfer.cntList = 1;
    chamfer.pChamFlltDataList = &chamSet;


    int idChamfer = 0;
    if (cvxPartChamferAll(&chamfer, &idChamfer))
        return;

    cvxEchoEnd();
}


/******************************************************************/
/* 函数定义 */
int ChamferAddAndDeleteCb
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

    cvxMsgDisp("ChamferAddAndDeleteCb");
    dataId = idData;
    if (idField <= Field_Same_Face_Flag)
    {
        double flag = 0;
        cvxDataGetNum(idData, Field_Same_Face_Flag, &flag);
        if ((int)flag == 1 && !cvxDataDelUnHi(idData, Field_Edge_List))
        {
            faceList.clear();
            return 0;
        }
    }
    else if (idField >= Field_Feature_List)
    {
        double flag = 0;
        cvxDataGetNum(idData, Field_Same_Face_Flag, &flag);
        if ((int)flag == 1)
        {
            int cntFeas = 0;
            int *idFea = nullptr;
            cvxDataGetEnts(idData, Field_Edge_List, &cntFeas, &idFea);
            if (cntFeas == 0)
                return 0;
            for (int i = 0; i < cntFeas; i++)
            {
                int sameDis = LimitDistance(idFea[i]);
                if (sameDis == -1)
                {
                    ZwMemoryFree((void **)&idFea);
                    return 1;
                }
                else if (sameDis == 0)
                {
                    cvxDataDelItem(idData, Field_Feature_List, idFea[i]);
                    szwEntityHandle tmpEntity{};
                    ZwEntityIdTransfer(1, &idFea[i], &tmpEntity);
                    ZwEntityUnHighlight(tmpEntity);
                }
            }
            ZwMemoryFree((void **)&idFea);
        }
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
int LimitDistance
(
int idFeature /* I: 特征的索引 */
)
/*
DESCRIPTION:
   命令参数字段的回调函数，当指定参数字段的值改变时调用该函数。
*/
{
    // TODO: 执行一些操作

    cvxMsgDisp("LimitDistance");
    double limitFlag = 0;
    cvxDataGetNum(dataId, Field_Same_Face_Flag, &limitFlag);
    if ((int)limitFlag != 1)
        return 1;

    zwString32 formname{};
    strcpy_s(formname, cvxDataName(idFeature));

    if (strcmp("FtChamfers2", formname) == 0)
    {
        /*查询特征数据*/
        double limitDis = 0, chamferDis = 0;
        int feaIdData = 0;
        if (cvxPartInqFtrData(idFeature, 0, &feaIdData))
            return -1;
        svxData *edgeListData = nullptr;
        int EdgeCount = 0;
        cvxDataGetList(feaIdData, CHAMFER_FIELD_CHAMFERLIST, &EdgeCount, &edgeListData);

        cvxDataGetNum(edgeListData[0].idEntity, CHAMFERLIST_FIELD_SETBACK, &chamferDis);

        ZwMemoryFree((void **)&edgeListData);

        /*获取限制距离*/
        if (cvxDataGetNum(dataId, Field_Distance_Limit, &limitDis))
            return -1;

        if (chamferDis == limitDis)
            return 1;
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
std::set<int> SameFaceList
(
const int cntFace,
szwEntityHandle *curfaces,
const std::set<int> faceList
)
{
    std::set<int> newFaceList{};
    for (int i = 0; i < cntFace; i++)
    {
        int curFaceId = 0;
        ZwEntityIdGet(1, &curfaces[i], &curFaceId);
        std::set<int>::iterator it;
        for (it = faceList.begin(); it != faceList.end(); it++)
        {
            if (curFaceId == *it)
            {
                newFaceList.insert(*it);
                break;
            }
        }
    }
    return newFaceList;
}

/******************************************************************/
/* 函数定义 */
int SameFaceEdge
(
int idEdge
)
{
    double sameFace = 0;
    cvxDataGetNum(dataId, Field_Same_Face_Flag, &sameFace);
    if ((int)sameFace != 1)
        return 1;

    szwEntityHandle curEdge{};
    ZwEntityIdTransfer(1, &idEdge, &curEdge);
    szwEntityHandle *faces = nullptr;
    int cntCurFace = 0;
    if (ZwEdgeFaceListGet(curEdge, &cntCurFace, &faces))
        return 1;
    ZwEntityHandleFree(&curEdge);

    /*如果只支持来自面的边且它不是第一条边，则检查当前边，
  否则将其推送到边列表并初始化面列表。 */
    int cntExitEdge = 0;
    int *edgeId = nullptr;
    cvxDataGetEnts(dataId, Field_Edge_List, &cntExitEdge, &edgeId);

    ZwMemoryFree((void **)&edgeId);

    if (!cntExitEdge)
    {
        faceList.clear();
        for (int i = 0; i < cntCurFace; ++i)
        {
            int curFaceId = 0;
            ZwEntityIdGet(1, &faces[i], &curFaceId);
            faceList.insert(curFaceId);
        }
        return 1;
    }
    else
    {
        std::set<int> tmpFaceList = faceList;
        tmpFaceList = SameFaceList(cntCurFace, faces, faceList);
        if (!tmpFaceList.size())
            return 0;
        else
            std::swap(faceList, tmpFaceList);
    }
}