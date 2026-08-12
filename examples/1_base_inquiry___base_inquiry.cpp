/*
**  (C) Copyright 2022, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include "stdio.h"
#include <cstring>
#include "VxApi.h"

#define BUFFER 256
#define INQ_OUTPUT_FORM "InqOutputWindow"

/* 声明您的函数 */
int InqRoot(int idData);
int InqPartComp(int idData);
int InqPartVar(int idData);

int BaseInquiryInit
(
int format,
void *data
)
{
    vxPath ApiPath;

    /* 将您的函数注册到ZW3D中。 */
    cvxCmdFunc("InqRoot", (void *)InqRoot, VX_CODE_GENERAL);
    cvxCmdFunc("InqPartComp", (void *)InqPartComp, VX_CODE_GENERAL);
    cvxCmdFunc("InqPartVar", (void *)InqPartVar, VX_CODE_GENERAL);

    /* 获取包含"BaseInquiry.dll"的"apilibs"文件夹的路径，
       并将该文件夹目录添加到搜索路径列表中 */
    cvxPathApiLib("BaseInquiry", ApiPath);
    cvxPathAdd(ApiPath);

    return 0;
}

int BaseInquiryExit
(
void
)
{
    /* 在这里放置您的清理代码。 */
    cvxCmdFuncUnload("InqRoot");
    cvxCmdFuncUnload("InqPartComp");
    cvxCmdFuncUnload("InqPartVar");
    return 0;
}

/* 查询激活的根对象信息 */
/* 如果函数失败则返回1，否则返回0。 */
int InqRoot
(
int idData
)
{
    int i, idObj, nObjs = 0, iRet = 0;
    char sBuf[BUFFER];
    vxName ActFile;
    vxLongName *ObjList = NULL;
    evxRootType ObjType;

    iRet = cvxFormCreate(INQ_OUTPUT_FORM, 0);

    cvxItemDel(INQ_OUTPUT_FORM, 1, -1);

    /* 确保有一个激活的文件。 */
    cvxFileInqActive(ActFile, sizeof(ActFile));
    if (!ActFile[0])
    {
        cvxItemAdd(INQ_OUTPUT_FORM, 1, "没有激活的根对象");
        /* 在GUI窗口中显示信息。 */
        cvxFormShow(INQ_OUTPUT_FORM);
    }

    /* 获取根对象列表。 */
    iRet = cvxRootList(NULL, &nObjs, &ObjList);

    //sprintf_s(sBuf, BUFFER, "根对象数量: %d。", nObjs);
    cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);

    for (i = 0; i < nObjs; i++)
    {
        if (cvxRootId(ObjList[i], &idObj, &ObjType))
            continue;

        /* 应用程序必须可靠地成对调用cvxRootActivate2()函数 */
        iRet = cvxRootActivate2(ActFile, ObjList[i]);

        //sprintf_s(sBuf, BUFFER, "根对象%s的ID是%d。", ObjList[i], idObj);
        cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);

        iRet = cvxRootActivate2(NULL, NULL);
    }

    /* 在GUI窗口中显示信息。 */
    cvxFormShow(INQ_OUTPUT_FORM);

    /* 释放列表的内存。 */
    cvxMemFree((void **)&ObjList);

    return iRet;
}

/*
获取属于激活的部件的组件的路径和根对象名称列表。
如果函数失败则返回1，否则返回0。
*/
int InqPartComp
(
int idData
)
{
    int i, nComps = 0, iRet = 0;
    char sBuf[BUFFER];
    vxPath *CompPath = NULL;
    vxName ActFile;
    vxLongName *CompList = NULL;

    iRet = cvxFormCreate(INQ_OUTPUT_FORM, 0);

    cvxItemDel(INQ_OUTPUT_FORM, 1, -1);

    /* 确保有一个激活的文件。 */
    cvxFileInqActive(ActFile, sizeof(ActFile));
    if (!ActFile[0])
    {
        cvxItemAdd(INQ_OUTPUT_FORM, 1, "没有激活的根对象。");
        /* 在GUI窗口中显示信息。 */
        cvxFormShow(INQ_OUTPUT_FORM);
    }

    /* 将根对象信息列表添加到GUI窗口中。 */
    if (iRet = cvxPartInqCompsInfo(NULL, NULL, &nComps, &CompPath, &CompList))
    {
    }
    else
    {
        iRet = cvxFormCreate(INQ_OUTPUT_FORM, 0);

        cvxItemDel(INQ_OUTPUT_FORM, 1, -1);

        if (nComps == 0)
        {
            cvxItemAdd(INQ_OUTPUT_FORM, 1, "没有组件。");
        }
        else
        {
            for (i = 0; i < nComps; i++)
            {
                //sprintf_s(sBuf, BUFFER, "组件%s的路径:", CompList[i]);
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
                cvxItemAdd(INQ_OUTPUT_FORM, 1, CompPath[i]);
            }
        }
    }

    /* 在GUI窗口中显示信息。 */
    cvxFormShow(INQ_OUTPUT_FORM);

    /* 释放列表的内存。 */
    cvxMemFree((void **)&CompPath);
    cvxMemFree((void **)&CompList);

    return iRet;
}

/*
获取属于指定部件的变量列表。
注意：只有使用"!CdPartEqnNew"或"!CdVarDis"命令创建的变量才能被查询。
如果函数失败则返回1，否则返回0。
*/
int InqPartVar
(
int idData
)
{
    int i, iRet = 0, nVars = 0;
    char sBuf[BUFFER];
    svxVariable *Vars = NULL;

    /* 获取属于激活的部件的变量列表。 */
    if (iRet = cvxPartInqVars(NULL, NULL, &nVars, &Vars))
    {
    }
    else
    {
        iRet = cvxFormCreate(INQ_OUTPUT_FORM, 0);

        cvxItemDel(INQ_OUTPUT_FORM, 1, -1);

        /* 在消息区域显示信息。 */
        if (!nVars)
        {
            cvxItemAdd(INQ_OUTPUT_FORM, 1, "没有变量。");
        }
        else
        {
            for (i = 0; i < nVars; i++)
            {
                //sprintf_s(sBuf, BUFFER, "%s = %lf", Vars[i].Name, Vars[i].Value);
                cvxItemAdd(INQ_OUTPUT_FORM, 1, sBuf);
                if (!Vars[i].Expression)
                    cvxItemAdd(INQ_OUTPUT_FORM, 1, Vars[i].Expression);
            }
        }
        /* 在GUI窗口中显示信息。 */
        cvxFormShow(INQ_OUTPUT_FORM);
    }

    /* 释放列表的内存。 */
    cvxMemFree((void **)&Vars);

    return iRet;
}