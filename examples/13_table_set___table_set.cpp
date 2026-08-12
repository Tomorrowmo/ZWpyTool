/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

#include <Windows.h>
#include <time.h>
#include <string>
#include "VxApi.h"

char gTemplate[24] = "uiTableForm";
const int giTableFiled = 2;
const int giBtnFiled = 3;

int ShowForm(void);
int TableSet(int, int *);
int TableSetCb(char *, int, int);

int TableSetInit
(
void
)
{
    cvxCmdFunc("ShowForm", (void *)ShowForm, VX_CODE_GENERAL);
    cvxCmdFunc("TableSet", (void *)TableSet, VX_CODE_GENERAL);
    cvxCmdCallback("TableSetCb", (void *)TableSetCb);

   /* 获取包含"TableSet.dll"的文件夹的路径
   并将文件夹目录添加到搜索路径列表中。 */
    vxPath ApiPath, ApiDir;
    TCHAR szBuff[MAX_PATH];
    HMODULE module = GetModuleHandle(L"TableSet.dll");
    GetModuleFileName(module, szBuff, sizeof(szBuff));
    int nLength = WideCharToMultiByte(CP_ACP, 0, szBuff, -1, NULL, 0, NULL, NULL);
    WideCharToMultiByte(CP_ACP, 0, szBuff, -1, ApiPath, nLength, NULL, NULL);
    cvxPathDir(ApiPath, ApiDir);
    cvxPathAdd(ApiDir);

    return 0;
}

int TableSetExit
(
void
)
{
    cvxCmdFuncUnload("ShowForm");
    cvxCmdFuncUnload("TableSet");
    cvxCmdFuncUnload("TableSetCb");

    return 0;
}

int ShowForm
(
void
)
{
    cvxFormCreate("uiTableForm", 1);
    cvxFormShow("uiTableForm");

    return 0;
}

int TableSet
(
int idIn,
int *idOut
)
{
    char szText[64] = "\0";

    switch (idIn)
    {
    case VX_FORM_INIT:
        {
            /* 移除所有行 */
            int iRowCnts = cvxTableRowCnt(gTemplate, giTableFiled);
            for (int i = iRowCnts - 1; i >= 0; i--)
                cvxTableRowRemove(gTemplate, giTableFiled, i);

            /* 初始化表格数据 */
            for (int i = 0; i < 5; i++)
            {
                cvxTableRowInsert(gTemplate, giTableFiled, cvxTableRowCnt(gTemplate, giTableFiled));
                //sprintf_s(szText, sizeof(szText), "ID%02d", i + 1);
                cvxTableCellTextSet(gTemplate, giTableFiled, i, 0, szText);
                cvxTableComboSet2(gTemplate, giTableFiled, i, 1, "Picture1,Picture2,Picture3", "1.png,2.png,3.png");
                svxColor clr = {0, 0, 255};
                cvxTableColorSet(gTemplate, giTableFiled, i, 2, &clr, 0, 0);
                cvxTableDateSet(gTemplate, giTableFiled, i, 3, ((long)time((time_t *)NULL)), VX_DATE_ONLY);
            }
        }
        break;
    case VX_FORM_CANCEL:
        cvxGetResponse(1, "窗口将被关闭！");
        break;
    default:
        break;
    }
    return 0;
}

int TableSetCb
(
char *TemplateName,
int Filed,
int Item
)
{
    svxTableCallBackParam vsData;
    cvxMemZero(&vsData, sizeof(svxTableCallBackParam));
    char szText[128] = "\0";

    switch (abs(Filed))
    {
    case giTableFiled:
        {
            cvxTableCallBackParamGet(gTemplate, giTableFiled, &vsData);
            /* 如果按下ctrl，shift或alt键，则不做任何操作。 */
            if (vsData.key.modifier != 0)
                break;

            switch (vsData.evt)
            {
            case VX_RIGHT_CLICK:
                cvxPopupShow("AsmPopMulAln");
                break;
            case VX_LEFT_CLICK:
                {
                    evxTableCellType eTblType;
                    cvxTableTypeGet(gTemplate, giTableFiled, vsData.iRow, vsData.iCol, &eTblType);
                    switch (eTblType)
                    {
                    case VX_TABLECELL_TYPE_TEXT:
                        cvxTableCellTextGet(gTemplate, giTableFiled, vsData.iRow, vsData.iCol, szText, sizeof(szText));
                        break;
                    default:
                        break;
                    }
                    cvxItemSet(gTemplate, 1, 1, szText);
                    break;
                }
            case VX_SORT:
                if (vsData.aSortSequence)
                    for (int i = 0; i < vsData.iSortSequenceSize; i++)
                    {
                        //sprintf_s(szText, sizeof(szText), "表格排序：第%d行变为第%d行。", i, vsData.aSortSequence[i]);
                        cvxMsgDisp(szText);
                    }
                break;
            default:
                break;
            }
        }
        break;
    case giBtnFiled:
        {
            int SelCnt = 0;
            svxRectangle *Rects = NULL;
            cvxTableGetSelRange(gTemplate, giTableFiled, &SelCnt, &Rects);
            switch (Item)
            {
            case 1:
                {
                    int InsRow = 0;
                    if (SelCnt)
                        InsRow = Rects[SelCnt - 1].bottom + 1;
                    else
                        InsRow = cvxTableRowCnt(gTemplate, giTableFiled);
                    cvxTableRowInsert(gTemplate, giTableFiled, InsRow);
                    //sprintf_s(szText, sizeof(szText), "ID%02d", InsRow + 1);
                    cvxTableCellTextSet(gTemplate, giTableFiled, InsRow, 0, szText);
                }
                break;
            case 2:
                {
                    for (int i = SelCnt - 1; i >= 0; i--)
                        for (int j = Rects[i].bottom; j >= Rects[i].top; j--)
                            cvxTableRowRemove(gTemplate, giTableFiled, j);
                    int SelRow = cvxTableRowCnt(gTemplate, giTableFiled) - 1;

                    svxRectangle rect = {0};
                    rect.left = 0;
                    rect.top = SelRow;
                    rect.right = cvxTableColCnt(gTemplate, giTableFiled) - 1;
                    rect.left = SelRow;
                    cvxTableSetSelRange(gTemplate, giTableFiled, &rect, 1);
                    cvxFieldFocusSet(gTemplate, giTableFiled);
                }
                break;
            default:
                break;
            }
        }
        break;
    default:
        break;
    }

    cvxTableCallBackParamFree(&vsData);
    return 0;
}