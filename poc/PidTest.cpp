/*
 *  PidTest.cpp —— 持久标识存活性验证
 *
 *  ⚠️ 编译前请核对以下三点（都能在 apiHelpDoc_chs.chm 里查到）：
 *     1. 每个函数页顶部的 "#include <xxx.h>" 行，与本文件的 include 是否一致
 *     2. ezwEntityType 中"面"的枚举名（本文件假设为 ZW_ENTITY_FACE）
 *     3. szwEntityIdentifier 是否有配套的释放接口（本文件按"需自行深拷贝"处理）
 *     凡是标 [TODO-VERIFY] 的地方都是我未能从 chm 完全确认的，装好后请核对。
 */

#include "PidTest.h"

#include "zwapi_entity.h"           /* ZwEntity* : Name / UniqueId / Type / Highlight / Handle */
#include "zwapi_shape.h"            /* ZwShapeListGet / ZwShapeFaceListGet                     */
#include "zwapi_face.h"             /* ZwFaceCylinderialCheck / ...GeometricInformationGet     */
#include "zwapi_cmd_direct_edit.h"  /* cvxPartDERad / cvxPartDERadInit                         */
#include "zwapi_part_hole.h"        /* cvxPartInqHoles / cvxPartFreeHoles                      */
#include "zwapi_memory.h"           /* ZwMemoryFree                                            */

#include <vector>
#include <string>
#include <cstdio>
#include <cstring>
#include <cstdlib>

/*====================================================================*/
/*  记录表                                                             */
/*====================================================================*/

struct PidRecord
{
    int                 seq;          /* 序号                       */
    char                name[32];     /* 打上去的名字（zwString32） */
    int                 isCylinder;   /* 是否圆柱面（≈孔）          */
    double              radius;       /* 圆柱半径（非圆柱面为 -1）  */
    szwEntityIdentifier uid;          /* 唯一 ID（深拷贝）          */
    int                 uidValid;     /* uid 是否成功取到           */
};

static std::vector<PidRecord> g_records;

static const char *kCsvPath = "C:\\Temp\\pid_test.csv";   /* 结果落盘位置，按需改 */

/*--------------------------------------------------------------------*/
/*  szwEntityIdentifier 深拷贝                                         */
/*  结构体： char *innerData; int dataLen; int idType;                 */
/*  接口返回的 innerData 可能指向内部缓冲，编辑后失效，必须自己留一份。 */
/*--------------------------------------------------------------------*/
static int UidClone(const szwEntityIdentifier *src, szwEntityIdentifier *dst)
{
    if (src == nullptr || dst == nullptr || src->innerData == nullptr || src->dataLen <= 0)
        return 0;
    dst->innerData = (char *)malloc(src->dataLen);
    if (dst->innerData == nullptr) return 0;
    memcpy(dst->innerData, src->innerData, src->dataLen);
    dst->dataLen = src->dataLen;
    dst->idType  = src->idType;
    return 1;
}

static void UidRelease(szwEntityIdentifier *id)
{
    if (id && id->innerData) { free(id->innerData); id->innerData = nullptr; }
    if (id) { id->dataLen = 0; id->idType = 0; }
}

static void ClearRecords(void)
{
    for (size_t i = 0; i < g_records.size(); i++)
        if (g_records[i].uidValid) UidRelease(&g_records[i].uid);
    g_records.clear();
}

/*====================================================================*/
/*  遍历当前零件的所有面                                               */
/*====================================================================*/
static int CollectAllFaces(std::vector<szwEntityHandle> &faces)
{
    faces.clear();

    int              shapeCount = 0;
    szwEntityHandle *shapes     = nullptr;

    if (ZwShapeListGet(&shapeCount, &shapes) != ZW_API_NO_ERROR)
    {
        cvxMsgDisp("PidTest: ZwShapeListGet 失败（当前是否有激活的零件？）");
        return 1;
    }

    for (int i = 0; i < shapeCount; i++)
    {
        int              faceCount = 0;
        szwEntityHandle *faceList  = nullptr;

        if (ZwShapeFaceListGet(shapes[i], &faceCount, &faceList) == ZW_API_NO_ERROR)
        {
            for (int j = 0; j < faceCount; j++)
                faces.push_back(faceList[j]);       /* 句柄转移，稍后统一释放 */
            /* 只释放数组本身，不释放已转移的句柄 */
            if (faceList) ZwMemoryFree((void **)&faceList);
        }
    }

    ZwEntityHandleListFree(shapeCount, &shapes);
    return 0;
}

/*====================================================================*/
/*  ~PidMark —— 打标                                                   */
/*====================================================================*/
int PidMark(void)
{
    cvxMsgDisp("PidTest: 开始打标...");
    ClearRecords();

    std::vector<szwEntityHandle> faces;
    if (CollectAllFaces(faces)) return 1;

    if (faces.empty())
    {
        cvxMsgDisp("PidTest: 未找到任何面。请先打开/新建一个有实体的零件。");
        return 1;
    }

    int okName = 0, okUid = 0, cylCount = 0;

    for (size_t i = 0; i < faces.size(); i++)
    {
        PidRecord rec;
        memset(&rec, 0, sizeof(rec));
        rec.seq    = (int)i + 1;
        rec.radius = -1.0;
        snprintf(rec.name, sizeof(rec.name), "PID_F%04d", rec.seq);

        /* --- A 路：命名 --- */
        if (ZwEntityNameSet(faces[i], rec.name) == ZW_API_NO_ERROR) okName++;

        /* --- B 路：唯一 ID --- */
        szwEntityIdentifier tmp;
        memset(&tmp, 0, sizeof(tmp));
        if (ZwEntityUniqueIdGet(faces[i], &tmp) == ZW_API_NO_ERROR)
        {
            if (UidClone(&tmp, &rec.uid)) { rec.uidValid = 1; okUid++; }
            /* [TODO-VERIFY] tmp.innerData 是否需要调用方释放？chm 未见配套 Free 接口。
               若确认需要释放，此处补 ZwMemoryFree((void**)&tmp.innerData); */
        }

        /* --- 顺带记录几何语义：是不是圆柱面（≈孔） --- */
        int flag = 0;
        if (ZwFaceCylinderialCheck(faces[i], &flag) == ZW_API_NO_ERROR && flag)
        {
            rec.isCylinder = 1;
            cylCount++;
            double   radius = 0.0, height = 0.0;
            szwPoint pBot{}, pTop{}, dir{};
            if (ZwFaceCylinderGeometricInformationGet(faces[i], &radius, &height,
                                                      &pBot, &pTop, &dir) == ZW_API_NO_ERROR)
                rec.radius = radius;
        }

        g_records.push_back(rec);
    }

    /* 释放句柄 */
    for (size_t i = 0; i < faces.size(); i++)
        ZwEntityHandleFree(&faces[i]);

    /* 顺带跑一次孔查询，看看 cvxPartInqHoles 在这个模型上认出几个孔 */
    int          holeCount = 0;
    svxHoleData *holes     = nullptr;
    if (cvxPartInqHoles(nullptr, nullptr, &holeCount, &holes) == ZW_API_NO_ERROR)
        cvxPartFreeHoles(holeCount, &holes);

    char msg[256];
    snprintf(msg, sizeof(msg),
             "PidMark: 面 %d 个 | 命名成功 %d | UniqueId 成功 %d | 圆柱面 %d | cvxPartInqHoles 认出 %d 个孔",
             (int)faces.size(), okName, okUid, cylCount, holeCount);
    cvxMsgDisp(msg);

    /* 落盘，便于对比 */
    FILE *fp = fopen(kCsvPath, "w");
    if (fp)
    {
        fprintf(fp, "seq,name,isCylinder,radius,uidValid,uidLen\n");
        for (size_t i = 0; i < g_records.size(); i++)
            fprintf(fp, "%d,%s,%d,%.4f,%d,%d\n",
                    g_records[i].seq, g_records[i].name, g_records[i].isCylinder,
                    g_records[i].radius, g_records[i].uidValid,
                    g_records[i].uidValid ? g_records[i].uid.dataLen : 0);
        fclose(fp);
        cvxMsgDisp("PidMark: 已写入 " );
        cvxMsgDisp((char *)kCsvPath);
    }
    return 0;
}

/*====================================================================*/
/*  ~PidEdit —— 做一次直接编辑，制造拓扑变化                           */
/*  找第一个圆柱面，半径 +1（即孔径 +2）                               */
/*====================================================================*/
int PidEdit(void)
{
    std::vector<szwEntityHandle> faces;
    if (CollectAllFaces(faces)) return 1;

    int targetId = 0; double oldR = 0.0;

    for (size_t i = 0; i < faces.size() && targetId == 0; i++)
    {
        int flag = 0;
        if (ZwFaceCylinderialCheck(faces[i], &flag) == ZW_API_NO_ERROR && flag)
        {
            double   radius = 0.0, height = 0.0;
            szwPoint pBot{}, pTop{}, dir{};
            if (ZwFaceCylinderGeometricInformationGet(faces[i], &radius, &height,
                                                      &pBot, &pTop, &dir) == ZW_API_NO_ERROR)
            {
                int id = 0;
                if (ZwEntityIdGet(1, &faces[i], &id) == ZW_API_NO_ERROR && id > 0)
                { targetId = id; oldR = radius; }
            }
        }
    }
    for (size_t i = 0; i < faces.size(); i++) ZwEntityHandleFree(&faces[i]);

    if (targetId == 0)
    {
        cvxMsgDisp("PidEdit: 模型上没找到圆柱面。请先在零件上打个孔。");
        return 1;
    }

    svxDERad data;
    cvxPartDERadInit(&data);            /* 默认值不全为 0，必须先 Init */
    data.numEnt = 1;
    data.idEnts = &targetId;
    data.R      = oldR + 1.0;           /* 半径 +1 = 直径 +2 */

    int idRad = 0;
    if (cvxPartDERad(&data, &idRad) != ZW_API_NO_ERROR)
    {
        cvxMsgDisp("PidEdit: cvxPartDERad 执行失败");
        return 1;
    }

    char msg[160];
    snprintf(msg, sizeof(msg), "PidEdit: 圆柱面半径 %.3f -> %.3f，生成特征 id=%d。请接着运行 ~PidCheck",
             oldR, data.R, idRad);
    cvxMsgDisp(msg);
    return 0;
}

/*====================================================================*/
/*  ~PidCheck —— 按标识查回，统计存活率                                */
/*====================================================================*/
int PidCheck(void)
{
    if (g_records.empty())
    {
        cvxMsgDisp("PidCheck: 记录为空，请先运行 ~PidMark");
        return 1;
    }

    int byName = 0, byUid = 0, both = 0, neither = 0;

    for (size_t i = 0; i < g_records.size(); i++)
    {
        int hitName = 0, hitUid = 0;

        /* --- A 路：按名字查 --- */
        szwEntityHandle h1{};
        /* [TODO-VERIFY] ezwEntityType 中"面"的枚举名，chm 里确认后替换 */
        if (ZwEntityGetByNameAndType(g_records[i].name, ZW_ENTITY_FACE, &h1) == ZW_API_NO_ERROR)
        {
            int exist = 0;
            if (ZwEntityExistenceCheck(h1, ZW_ENTITY_FACE, &exist) == ZW_API_NO_ERROR && exist)
                hitName = 1;
            ZwEntityHandleFree(&h1);
        }

        /* --- B 路：按唯一 ID 查 --- */
        if (g_records[i].uidValid)
        {
            szwEntityHandle h2{};
            if (ZwEntityGetByUniqueId(g_records[i].uid, &h2) == ZW_API_NO_ERROR)
            {
                int exist = 0;
                if (ZwEntityExistenceCheck(h2, ZW_ENTITY_FACE, &exist) == ZW_API_NO_ERROR && exist)
                    hitUid = 1;
                ZwEntityHandleFree(&h2);
            }
        }

        if (hitName) byName++;
        if (hitUid)  byUid++;
        if (hitName && hitUid) both++;
        if (!hitName && !hitUid) neither++;
    }

    int total = (int)g_records.size();
    char msg[320];
    snprintf(msg, sizeof(msg),
             "PidCheck: 共 %d 个面 | 按名字存活 %d (%.1f%%) | 按UniqueId存活 %d (%.1f%%) | 两者都在 %d | 全丢 %d",
             total,
             byName, total ? 100.0 * byName / total : 0.0,
             byUid,  total ? 100.0 * byUid  / total : 0.0,
             both, neither);
    cvxMsgDisp(msg);

    FILE *fp = fopen("C:\\Temp\\pid_result.txt", "a");
    if (fp) { fprintf(fp, "%s\n", msg); fclose(fp); }
    return 0;
}

/*====================================================================*/
/*  插件生命周期                                                       */
/*====================================================================*/
int PidTestInit(int format, void *data)
{
    cvxCmdFunc("PidMark",  (void *)PidMark,  VX_CODE_GENERAL);
    cvxCmdFunc("PidEdit",  (void *)PidEdit,  VX_CODE_GENERAL);
    cvxCmdFunc("PidCheck", (void *)PidCheck, VX_CODE_GENERAL);
    cvxMsgDisp("PidTest 已加载：~PidMark -> ~PidEdit -> ~PidCheck");
    return 0;
}

int PidTestExit(void)
{
    ClearRecords();
    cvxCmdFuncUnload("PidMark");
    cvxCmdFuncUnload("PidEdit");
    cvxCmdFuncUnload("PidCheck");
    return 0;
}
