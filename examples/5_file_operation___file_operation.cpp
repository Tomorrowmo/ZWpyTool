/***
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/


#include "stdio.h"
#include "VxApi.h"

/*** 声明您的函数 */
int FileCreate(void);
int FileCopy(void);

int FileOperationInit
(
int format,
void *data
)
{
    /*** 将您的函数注册到ZW3D中。 */
    cvxCmdFunc("FileCreate", (void *)FileCreate, VX_CODE_GENERAL);
    cvxCmdFunc("FileCopy", (void *)FileCopy, VX_CODE_GENERAL);

    return 0;
}

int FileOperationExit
(
void
)
{
    /*** 在这里放置您的清理代码。 */
    cvxCmdFuncUnload("FileCreate");
    cvxCmdFuncUnload("FileCopy");
    return 0;
}

/*** 创建一个文件并向其中添加一个零件和一张工程图。 */
/*** 如果函数失败则返回1，否则返回0。 */
int FileCreate
(
void
)
{
    int iRet = 0;
    char FileName[] = {"APITestFile.Z3"};
    char PartName[] = {"APITestPart"};
    char DrawName[] = {"APITestDwg"};

    /*** 使用指定名称创建一个新文件并使其处于激活的状态。 */
    iRet = cvxFileNew(FileName);
    if (iRet)
    {
        cvxMsgDisp("创建新文件失败。");
        return iRet;
    }

    /*** 在激活的文件中创建并插入一个零件对象。 */
    iRet = cvxRootAdd(VX_ROOT_PART, PartName, NULL);
    if (iRet)
    {
        cvxMsgDisp("添加部件失败。");
        return iRet;
    }

    /*** 退出激活的零件。 */
    cvxPartExit();

    /*** 在激活的文件中创建并插入一张工程图对象。 */
    iRet = cvxRootAdd(VX_ROOT_SHEET, DrawName, NULL);
    if (iRet)
    {
        cvxMsgDisp("添加工程图失败。");
        return iRet;
    }

    /*** 退出激活的工程图。 */
    cvxPartExit();

    /*** 保存文件。 */
    iRet = cvxFileSaveAs(FileName);
    if (iRet)
    {
        cvxMsgDisp("保存失败。");
        return iRet;
    }

    /*** 关闭激活的文件。 */
    cvxFileClose();
    return iRet;
}

/*** 复制激活的文件。 */
/*** 如果函数失败则返回1，否则返回0。 */
int FileCopy
(
void
)
{
    int iRet = 0;
    char SourceFile[] = {"APISourceFile.Z3"};
    char SourceRoot[] = {"APISourcePart"};
    char DestFile[] = {"APICopyFile.Z3"};
    char DestRoot[] = {"APICopyPart"};
    svxBoxData Box;
    int idShape = -1;

    /*** 初始化内存 */
    cvxMemZero((void *)&Box, sizeof(Box));

    Box.Center.x = 0.0;
    Box.Center.y = 0.0;
    Box.Center.z = 0.0;
    Box.X = 10.0;
    Box.Y = 10.0;
    Box.Z = 10.0;
    Box.Combine = VX_BOOL_ADD;
    Box.idPlane = 0;

    /*** 使用指定名称创建一个新文件并使其处于激活的状态。 */
    iRet = cvxFileNew(SourceFile);
    if (iRet)
    {
        cvxMsgDisp("创建源文件失败。");
        return iRet;
    }

    /*** 在源文件中创建并插入一个部件对象。 */
    iRet = cvxRootAdd(VX_ROOT_PART, SourceRoot, NULL);
    if (iRet)
    {
        cvxMsgDisp("添加部件失败。");
        return iRet;
    }

    /*** 创建一个盒子形状并将其添加到源文件中。 */
    iRet = cvxPartBox(&Box, &idShape);
    if (iRet)
    {
        cvxMsgDisp("创建盒子失败。");
        return iRet;
    }

    /*** 退出激活的部件。 */
    cvxPartExit();

    /*** 保存文件。 */
    iRet = cvxFileSaveAs(SourceFile);
    if (iRet)
    {
        cvxMsgDisp("保存失败。");
        return iRet;
    }

    cvxFileClose();

    /*** 创建另一个文件。 */
    iRet = cvxFileNew(DestFile);
    if (iRet)
    {
        cvxMsgDisp("创建目标文件失败。");
        return iRet;
    }

    /*** 将部件复制到新创建的文件中。 */
    iRet = cvxRootCopy(SourceFile, SourceRoot, DestFile, DestRoot, 0, 1);
    if (iRet)
    {
        cvxMsgDisp("复制根对象失败。");
        return iRet;
    }

    /*** 退出激活的部件。 */
    cvxPartExit();

    /*** 保存文件。 */
    iRet = cvxFileSaveAs(DestFile);
    if (iRet)
    {
        cvxMsgDisp("保存失败。");
        return iRet;
    }

    return iRet;
}