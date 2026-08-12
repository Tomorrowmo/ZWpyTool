/***
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include <stdio.h>
#include <string.h>
#include "VxApi.h"

/*** 声明你的函数 */
int FileExportImg(void);
int FileExportPdf(void);

int FileExportInit
(
int format,
void *data
)
{
    /*** 将你的函数注册到ZW3D中。 */
    cvxCmdFunc("FileExportImg", (void *)FileExportImg, VX_CODE_GENERAL);
    cvxCmdFunc("FileExportPdf", (void *)FileExportPdf, VX_CODE_GENERAL);

    return 0;
}

int FileExportExit
(
void
)
{
    /*** 在这里放置你的清理代码。 */
    cvxCmdFuncUnload("FileExportImg");
    cvxCmdFuncUnload("FileExportPdf");
    return 0;
}

/*** 将当前文件保存为"png"格式。 */
/*** 如果函数失败则返回1，否则返回0。 */
int FileExportImg(void)
{
    int iRet = 0;
    vxName FileName;
    vxName ExportName;
    vxPath FilePath;
    svxImgData ImgData;

    /*** 确保有一个当前文件。 */
    cvxFileInqActive(FileName, sizeof(FileName));
    if (!FileName[0])
    {
        cvxMsgDisp("没有当前文件。");
        return 1;
    }

    /*** 获取当前文件的源目录。 */
    cvxFileDirectory(FilePath);

    /*** 设置输出文件路径。 */
    iRet = strcpy_s(ExportName, "test.png");

    if (FilePath[0])
        iRet = cvxPathCompose(FilePath, ExportName);
    else
        iRet = strcpy_s(FilePath, "test.png");

    if (iRet)
    {
        cvxMsgDisp("找不到导出路径。");
        return iRet;
    }

    /*** 初始化输出数据。 */
    cvxMemZero((void *)&ImgData, sizeof(ImgData));

    /*** 输出数据的格式 */
    ImgData.Type = VX_EXPORT_IMG_TYPE_PNG;

    /*** 背景颜色 */
    //ImgData.Background.r = 0;
    //ImgData.Background.g = 0;
    //ImgData.Background.b = 0;

    /*** 背景模式 */
    ImgData.BkgndMode = VX_EXPORT_IMG_BKGND_MODE_CURRENT;

    /*** 颜色模式 */
    ImgData.ColorMode = VX_EXPORT_IMG_COLOR_MODE_24BITS;

    /*** 范围模式 */
    ImgData.RangeMode = VX_EXPORT_IMG_RANGE_MODE_NORMAL;

    /*** 图像大小（像素）*/
    int height = 0, width = 0;
    cvxDispWindowRectGet(0, 1, NULL, NULL, &height, &width);
    ImgData.Height = height;
    ImgData.Width = width;

    /*** 压缩率（仅限JPEG） */
    //pImgData.Quality = 10;

    /*** 导出文件。 */
    iRet = cvxFileExport(VX_EXPORT_TYPE_IMG, FilePath, (void *)&ImgData);
    if (iRet)
        cvxMsgDisp("保存为png格式失败。");
    else
        cvxMsgDisp("文件保存成功。");

    return iRet;
};

/*** 将当前文件保存为"pdf"格式。 */
/*** 如果函数失败则返回1，否则返回0。 */
int FileExportPdf(void)
{
    int iRet = 0;
    vxName FileName;
    vxName ExportName;
    vxPath FilePath;
    svxPdfData PdfData;

    /*** 确保有一个当前文件。 */
    cvxFileInqActive(FileName, sizeof(FileName));
    if (!FileName[0])
    {
        cvxMsgDisp("没有当前文件。");
        return 1;
    }

    /*** 获取当前文件的源目录。 */
    cvxFileDirectory(FilePath);

    /*** 设置输出文件路径。 */
    iRet = strcpy_s(ExportName, "test.pdf");

    if (FilePath[0])
        iRet = cvxPathCompose(FilePath, ExportName);
    else
        iRet = strcpy_s(FilePath, "test.pdf");

    if (iRet)
    {
        cvxMsgDisp("找不到导出路径。");
        return iRet;
    }

    /*** 初始化输出数据。 */
    cvxMemZero((void *)&PdfData, sizeof(PdfData));

    /*** 作者 */
    //PdfData.Author = ;

    /*** 颜色 */
    PdfData.Color = 3;

    /*** 描述 */
    //PdfData.Description = ;

    /*** 每英寸点数 */
    PdfData.Dpi = 72;

    /*** 字体 */
    //PdfData.Font = ;

    /*** 纸张高度（像素） */
    PdfData.PaperHeight = 297;

    /*** 纸张宽度（像素） */
    PdfData.PaperWidth = 210;

    /*** 密码 */
    //PdfData.Password = ;

    /*** 范围模式 */
    PdfData.RangeMode = VX_EXPORT_PDF_RANGE_MODE_NORMAL;

    /*** 主题 */
    //PdfData.Subject = ;

    /*** 标题 */
    //PdfData.Title = ;

    /*** pdf类型 */
    PdfData.Type = VX_EXPORT_PDF_TYPE_RASTER;

    /*** 导出文件。 */
    iRet = cvxFileExport(VX_EXPORT_TYPE_PDF, FilePath, (void *)&PdfData);

    if (iRet)
        cvxMsgDisp("保存为pdf格式失败。");
    else
        cvxMsgDisp("文件保存成功。");

    return iRet;
};