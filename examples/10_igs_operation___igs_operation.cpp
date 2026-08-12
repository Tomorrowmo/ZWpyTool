/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include <stdio.h>
#include <string.h>
#include "VxApi.h"

/* 声明你的函数 */
int FileLoadIgs(void);
int FileSaveIgs(void);

int IgsOperationInit
(
int format,
void *data
)
{
    /* 将你的函数注册到ZW3D中。 */
    cvxCmdFunc("FileSaveIgs", (void *)FileSaveIgs, VX_CODE_GENERAL);
    cvxCmdFunc("FileLoadIgs", (void *)FileLoadIgs, VX_CODE_GENERAL);

    return 0;
}

int IgsOperationExit
(
void
)
{
    /* 在这里放置你的清理代码。 */
    cvxCmdFuncUnload("FileSaveIgs");
    cvxCmdFuncUnload("FileLoadIgs");
    return 0;
}

/* 将当前文件保存为"igs"格式。 */
/* 如果函数失败则返回1，否则返回0。 */
int FileSaveIgs
(
void
)
{
    int iRet = 0;
    vxName FileName, ExportName;
    vxPath FilePath;

    /* 确保有一个当前文件。 */
    cvxFileInqActive(FileName, sizeof(vxName));
    if (!FileName[0])
    {
        cvxMsgDisp("没有当前文件。");
        return 1;
    }

    /* 获取当前文件的源目录。 */
    cvxFileDirectory(FilePath);

    /* 设置输出文件路径。 */
    iRet = strcpy_s(ExportName, "test.igs");

    if (FilePath[0])
        iRet = cvxPathCompose(FilePath, ExportName);
    else
        iRet = strcpy_s(FilePath, "E:\\test.igs");

    if (iRet)
    {
        cvxMsgDisp("找不到导出路径。");
        return iRet;
    }

    /* 使用宏命令将当前文件保存为"igs"格式。 */
    char sMacro[sizeof(vxPath) + sizeof(vxName) + 128] = {'\0'};
    iRet = sprintf_s(sMacro, sizeof(vxPath) + sizeof(vxName) + 128, "[vxSend,\"!CdFileSaveAs2\", \"%s\"]", FilePath);

    if (iRet < 0)
    {
        cvxMsgDisp("保存为igs格式失败。");
        return 1;
    }

    sMacro[strlen(sMacro)] = '\0';

    iRet = cvxCmdMacro(sMacro, NULL);

    if (iRet)
        cvxMsgDisp("保存为igs格式失败。");
    else
        cvxMsgDisp("文件保存成功。");

    return iRet;
}

/* 加载指定的"igs"文件。 */
/* 如果函数失败则返回1，否则返回0。 */
int FileLoadIgs
(
void
)
{
    int iRet = 0;
    vxName FileName;
    char PartName[] = {"IgsPart"};

    /* 确保有一个当前文件。 */
    cvxFileInqActive(FileName, sizeof(vxName));
    if (!FileName[0])
    {
        cvxMsgDisp("没有当前文件。");
        return 1;
    }

    /* 使用宏命令加载指定的"igs"文件。 */
    char sMacro[sizeof(vxPath) + sizeof(vxName) + 128] = {'\0'};
    iRet = strcpy_s(sMacro, "[vxSend,\"!CdFileOpen2\", \"E:\\test.igs\"]");

    if (iRet)
        return iRet;

    sMacro[strlen(sMacro)] = '\0';

    iRet = cvxCmdMacro(sMacro, NULL);
    if (iRet)
    {
        cvxMsgDisp("加载文件失败。");
        return iRet;
    }

    return iRet;
}