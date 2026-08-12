/*
**  (C) Copyright 2024, ZWSOFT Co., LTD. (Guangzhou) All Rights Reserved.
*/

/******************************************************************/
/* ZW3D API 包含 */
#include "zwapi_matrix.h"
#include "zwapi_ui_form.h"
#include "zwapi_math_vector.h"

/******************************************************************/
/* 应用程序包含 */
#include <stdio.h>
#include <stdlib.h>
#include "..\inc\MatrixOperationsPr.h"

/******************************************************************/
/* 数据类型定义 */
/* DESCRIPTION: 模拟序列 */
enum SimulationSequence
{
    Simulation_Multiplication = 1, /* 矩阵乘法 */
    Simulation_Addition,           /* 矩阵加法 */
    Simulation_Determinant,        /* 矩阵行列式计算 */
    Simulation_Transpose,          /* 矩阵转置 */
    Simulation_Vector_Multiply,    /* 向量乘法 */
    Simulation_X_direction,        /* 矩阵的X方向 */
    Simulation_Y_direction,        /* 矩阵的Y方向 */
    Simulation_Z_direction,        /* 矩阵的Z方向 */
};

/* DESCRIPTION: 数据类型 */
enum DataType
{
    Data_Matrix4,     /* 3 x 4 矩阵 */
    Data_Matrix3,     /* 3 x 3 矩阵 */
    Data_Vector,      /* 向量 */
    Data_Determinant, /* 行列式值 */
    Data_None,        /* 隐藏 */
};

/* DESCRIPTION: 表单矩阵项 */
enum MatrixItem
{
    Input_Matrix1, /* 输入矩阵1 */
    Input_Matrix2, /* 输入矩阵2 */
    Result_Matrix, /* 结果矩阵 */
};

/* DESCRIPTION: 表单字段ID */
enum MatrixOperationsField
{
    Field_Simulation_Sequence = 1,
    Field_Simulation_Execute = 2,
    Field_Simulation_Reset = 6,
    Field_Simulation_OK_HELP = 7,
};

/******************************************************************/
/* 全局变量声明 */
const char g_formName[] = "MatrixOperations";
SimulationSequence g_simulationSequence = Simulation_Multiplication;
DataType g_DataType[3] = {Data_Matrix4, Data_Matrix4, Data_Matrix4};

/******************************************************************/
/* 函数声明 */
static int ShowFormMatrixOperations(void);
static int MatrixOperations(int idIn, int *idOut);
static int MatrixOperationCb(char *form, int idField, int item);
static int UpdateData(int update, DataType dataType, double data[12], MatrixItem matrixItem);
static int UpdateMatrixData(int update, szwMatrix *matrix, MatrixItem matrixItem);
static int UpdateMatrix3Data(int update, szwMatrix3 *matrix, MatrixItem matrixItem);
static int UpdateVectorData(int update, szwVector *vector, MatrixItem matrixItem);
static int UpdateDeterminantData(int update, double *data, MatrixItem matrixItem);
static int UpdateData(int update, DataType dataType, double data[12], MatrixItem matrixItem);
static int UpdateMatrixForm(MatrixItem matrixItem, DataType dataType);
static szwMatrix MatrixAdd(szwMatrix matrix1, szwMatrix matrix2);
static int MatrixOperationsExecute();

/******************************************************************/
/* 函数定义 */
int RegisterFormCommand()
/*
DESCRIPTION:
   注册表单命令的回调函数。
*/
{
    /* 通过输入命令字符串 "~ShowFormMatrixOperations" 显示表单 */
    cvxCmdFunc("ShowFormMatrixOperations", (void *)ShowFormMatrixOperations, VX_CODE_GENERAL);

    /* 注册表单的回调函数 */
    cvxFormFunc(const_cast<char *>(g_formName), (void *)MatrixOperations, VX_CODE_GENERAL);
    cvxCmdCallback("MatrixOperationCb", (void *)MatrixOperationCb);
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UnloadFormCommand()
/*
DESCRIPTION:
   卸载表单命令的回调函数。
*/
{
    cvxCmdFuncUnload("ShowFormMatrixOperations");
    cvxCmdFuncUnload("MatrixOperations");
    cvxCmdFuncUnload("MatrixOperationCb");
    return 0;
}

/******************************************************************/
/* 函数定义 */
int ShowFormMatrixOperations
(
void
)
/*
DESCRIPTION:
   自定义命令显示表单。
*/
{
    /* 如果表单尚未创建，则创建表单 */
    int formState = cvxFormState(const_cast<char *>(g_formName));
    if (formState == 0 && cvxFormCreate(const_cast<char *>(g_formName), 0))
        return 1;

    /* 显示表单 */
    cvxFormShow(const_cast<char *>(g_formName));
    return 0;
}

/******************************************************************/
/* 函数定义 */
int MatrixOperations
(
int idIn,  /* I: 数据容器的索引 */
int *idOut /* O: 数据容器的索引 */
)
/*
DESCRIPTION:
   命令表单的执行函数。
*/
{
    switch (idIn)
    {
    case VX_FORM_INIT:
        {
            szwMatrix matrix = {};
            ZwMatrixInitByTranslation(0.0, 0.0, 0.0, &matrix);
            UpdateMatrixData(0, &matrix, Input_Matrix1);
            UpdateMatrixData(0, &matrix, Input_Matrix2);
        }
        break;
    default:
        break;
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
int MatrixOperationCb
(
char *form,  /* I: 表单名称 */
int idField, /* I: 字段的索引 */
int idItem   /* I: 项的索引 */
)
/*
DESCRIPTION:
   命令表单的回调函数。
*/
{
    switch (idField)
    {
    case Field_Simulation_Sequence:
        {
            DataType tempType[3] = {Data_Matrix4, Data_Matrix4, Data_Matrix4};
            g_simulationSequence = (SimulationSequence)idItem;
            switch (g_simulationSequence)
            {
            case Simulation_Determinant:
                tempType[0] = Data_Matrix3;
                tempType[1] = Data_None;
                tempType[2] = Data_Determinant;
                break;
            case Simulation_Transpose:
                tempType[1] = Data_None;
                break;
            case Simulation_Vector_Multiply:
                tempType[0] = Data_Vector;
                tempType[1] = Data_Vector;
                tempType[2] = Data_Vector;
                break;
            case Simulation_X_direction:
            case Simulation_Y_direction:
            case Simulation_Z_direction:
                tempType[0] = Data_Matrix4;
                tempType[1] = Data_None;
                tempType[2] = Data_Vector;
                break;
            default:
                break;
            }
            if (tempType[0] != g_DataType[0] || tempType[1] != g_DataType[1] || tempType[2] != g_DataType[2])
            {
                g_DataType[0] = tempType[0];
                g_DataType[1] = tempType[1];
                g_DataType[2] = tempType[2];
                UpdateMatrixForm(Input_Matrix1, g_DataType[0]);
                UpdateMatrixForm(Input_Matrix2, g_DataType[1]);
                UpdateMatrixForm(Result_Matrix, g_DataType[2]);
                if (g_simulationSequence != Simulation_Multiplication && g_simulationSequence != Simulation_Addition)
                    cvxFormAdjustSize(g_formName);
            }
        }
        break;
    case Field_Simulation_Execute:
        {
            MatrixOperationsExecute();
        }
        break;
    case Field_Simulation_Reset:
        {
            for (int i = 0; i < 3; i++)
            {
                switch (g_DataType[i])
                {
                case Data_Matrix4:
                    {
                        szwMatrix matrix = {};
                        ZwMatrixInitByTranslation(0.0, 0.0, 0.0, &matrix);
                        UpdateMatrixData(0, &matrix, (MatrixItem)i);
                    }
                    break;
                case Data_Matrix3:
                    {
                        szwMatrix3 matrix = {};
                        matrix.xx = 1.0;
                        matrix.yy = 1.0;
                        matrix.zz = 1.0;
                        UpdateMatrix3Data(0, &matrix, (MatrixItem)i);
                    }
                    break;
                case Data_Vector:
                    {
                        szwVector vector = {1.0, 0.0, 0.0};
                        UpdateVectorData(0, &vector, (MatrixItem)i);
                    }
                    break;
                case Data_Determinant:
                    {
                        double determinant = 0.0;
                        UpdateDeterminantData(0, &determinant, (MatrixItem)i);
                    }
                    break;
                default:
                    break;
                }
            }
        }
        break;
    case Field_Simulation_OK_HELP:
        {
            if (idItem == 1)
                cvxFormKill(const_cast<char *>(g_formName));
            if (idItem == 2)
                cvxMessageBox(2, 1, "Note", "Show help document", "OK", nullptr, nullptr);
        }
        break;
    default:
        break;
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UpdateData
(
int update,           /* I: 0: 更新UI; 1: 更新数据 */
DataType dataType,    /* I: 数据类型 */
double data[12],      /* I/O: 矩阵数据 */
MatrixItem matrixItem /* I: 矩阵起始字段ID */
)
/*
DESCRIPTION:
   更新数据。
*/
{
    int matrixStartField = 11;
    if (matrixItem == Input_Matrix2)
        matrixStartField = 31;
    if (matrixItem == Result_Matrix)
        matrixStartField = 51;

    const int nRow = 3;
    const int nCol = 4;
    zwString32 strData = "";
    if (!update)
    {
        /* 将数据设置到UI */
        for (int i = 0; i < nRow * nCol; i++)
        {
            int isVis = 0;
            cvxItemVisibleGet(g_formName, matrixStartField + i, 1, &isVis);
            if (!isVis)
                continue;

            //sprintf_s(strData, sizeof(strData), "%f", data[i]);
            cvxItemSet(const_cast<char *>(g_formName), matrixStartField + i, 1, strData);
        }
    }
    else
    {
        /* 从UI获取数据 */
        for (int i = 0; i < nRow * nCol; i++)
        {
            int isVis = 0;
            cvxItemVisibleGet(g_formName, matrixStartField + i, 1, &isVis);
            if (isVis)
            {
                cvxItemGet(const_cast<char *>(g_formName), matrixStartField + i, 1, strData);
                data[i] = strtod(strData, nullptr);
            }
            else
            {
                data[i] = 0.0;
            }
        }
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UpdateMatrixData
(
int update,           /* I: 0: 更新UI; 1: 更新数据 */
szwMatrix *matrix,    /* I/O: 矩阵数据 */
MatrixItem matrixItem /* I: 矩阵起始字段ID */
)
/*
DESCRIPTION:
   更新矩阵数据。
*/
{
    const int dataLength = 12;
    double data[dataLength] = {};
    if (!update)
    {
        int index = 0;
        data[index++] = matrix->xx;
        data[index++] = matrix->yx;
        data[index++] = matrix->zx;
        data[index++] = matrix->xt;
        data[index++] = matrix->xy;
        data[index++] = matrix->yy;
        data[index++] = matrix->zy;
        data[index++] = matrix->yt;
        data[index++] = matrix->xz;
        data[index++] = matrix->yz;
        data[index++] = matrix->zz;
        data[index++] = matrix->zt;
        UpdateData(update, Data_Matrix4, data, matrixItem);
    }
    else
    {
        UpdateData(update, Data_Matrix4, data, matrixItem);

        *matrix = {};
        int index = 0;
        matrix->xx = data[index++];
        matrix->yx = data[index++];
        matrix->zx = data[index++];
        matrix->xt = data[index++];
        matrix->xy = data[index++];
        matrix->yy = data[index++];
        matrix->zy = data[index++];
        matrix->yt = data[index++];
        matrix->xz = data[index++];
        matrix->yz = data[index++];
        matrix->zz = data[index++];
        matrix->zt = data[index++];
        matrix->scale = 1.0;
        ZwMatrixIdentityFlagRefresh(matrix);
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UpdateMatrix3Data
(
int update,           /* I: 0: 更新UI; 1: 更新数据 */
szwMatrix3 *matrix,   /* I/O: 矩阵数据 */
MatrixItem matrixItem /* I: 矩阵起始字段ID */
)
/*
DESCRIPTION:
   更新矩阵数据。
*/
{
    const int dataLength = 12;
    double data[dataLength] = {};
    if (!update)
    {
        int index = 0;
        data[index++] = matrix->xx;
        data[index++] = matrix->yx;
        data[index++] = matrix->zx;
        index++;
        data[index++] = matrix->xy;
        data[index++] = matrix->yy;
        data[index++] = matrix->zy;
        index++;
        data[index++] = matrix->xz;
        data[index++] = matrix->yz;
        data[index++] = matrix->zz;
        index++;
        UpdateData(update, Data_Matrix3, data, matrixItem);
    }
    else
    {
        UpdateData(update, Data_Matrix3, data, matrixItem);

        *matrix = {};
        int index = 0;
        matrix->xx = data[index++];
        matrix->yx = data[index++];
        matrix->zx = data[index++];
        index++;
        matrix->xy = data[index++];
        matrix->yy = data[index++];
        matrix->zy = data[index++];
        index++;
        matrix->xz = data[index++];
        matrix->yz = data[index++];
        matrix->zz = data[index++];
        index++;
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UpdateVectorData
(
int update,           /* I: 0: 更新UI; 1: 更新数据 */
szwVector *vector,    /* I/O: 向量数据 */
MatrixItem matrixItem /* I: 矩阵起始字段ID */
)
/*
DESCRIPTION:
   更新向量数据。
*/
{
    const int dataLength = 12;
    double data[dataLength] = {};
    if (!update)
    {
        int index = 0;
        data[index++] = vector->x;
        data[index++] = vector->y;
        data[index++] = vector->z;
        UpdateData(update, Data_Vector, data, matrixItem);
    }
    else
    {
        UpdateData(update, Data_Vector, data, matrixItem);

        *vector = {};
        int index = 0;
        vector->x = data[index++];
        vector->y = data[index++];
        vector->z = data[index++];
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UpdateDeterminantData
(
int update,           /* I: 0: 更新UI; 1: 更新数据 */
double *determinant,  /* I/O: 行列式数据 */
MatrixItem matrixItem /* I: 矩阵起始字段ID */
)
/*
DESCRIPTION:
   更新行列式数据。
*/
{
    const int dataLength = 12;
    double data[dataLength] = {};

    if (!update)
    {
        data[0] = *determinant;
        UpdateData(update, Data_Determinant, data, matrixItem);
    }
    else
    {
        UpdateData(update, Data_Determinant, data, matrixItem);
        *determinant = data[0];
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
int UpdateMatrixForm
(
MatrixItem matrixItem, /* I: 矩阵起始字段ID */
DataType dataType      /* I: 数据类型 */
)
/*
DESCRIPTION:
   更新矩阵表单显示状态。
*/
{
    int matrixStartField = 11;
    if (matrixItem == Input_Matrix2)
        matrixStartField = 31;
    if (matrixItem == Result_Matrix)
        matrixStartField = 51;

    if (matrixItem == Input_Matrix2)
        cvxFormFieldVisSet(g_formName, 4, dataType != Data_None ? 1 : 0);

    switch (dataType)
    {
    case Data_Matrix4:
        for (int i = 0; i < 16; i++)
            cvxFormFieldVisSet(g_formName, matrixStartField + i, 1);
        break;
    case Data_Matrix3:
        for (int i = 0; i < 16; i++)
            cvxFormFieldVisSet(g_formName, matrixStartField + i, (i + 1) % 4 == 0 ? 0 : 1);
        break;
    case Data_Vector:
        for (int i = 0; i < 16; i++)
            cvxFormFieldVisSet(g_formName, matrixStartField + i, i < 3 ? 1 : 0);
        break;
    case Data_Determinant:
        for (int i = 0; i < 16; i++)
            cvxFormFieldVisSet(g_formName, matrixStartField + i, i < 1 ? 1 : 0);
        break;
    case Data_None:
        break;
    default:
        break;
    }
    return 0;
}

/******************************************************************/
/* 函数定义 */
szwMatrix MatrixAdd
(
szwMatrix matrix1, /* I: 第一个矩阵 */
szwMatrix matrix2  /* I: 第二个矩阵 */
)
/*
DESCRIPTION:
   更新矩阵表单显示状态。
*/
{
    szwMatrix resultMatrix = {};
    resultMatrix.xx = matrix1.xx + matrix2.xx;
    resultMatrix.yx = matrix1.yx + matrix2.yx;
    resultMatrix.zx = matrix1.zx + matrix2.zx;
    resultMatrix.xt = matrix1.xt + matrix2.xt;
    resultMatrix.xy = matrix1.xy + matrix2.xy;
    resultMatrix.yy = matrix1.yy + matrix2.yy;
    resultMatrix.zy = matrix1.zy + matrix2.zy;
    resultMatrix.yt = matrix1.yt + matrix2.yt;
    resultMatrix.xz = matrix1.xz + matrix2.xz;
    resultMatrix.yz = matrix1.yz + matrix2.yz;
    resultMatrix.zz = matrix1.zz + matrix2.zz;
    resultMatrix.zt = matrix1.zt + matrix2.zt;
    ZwMatrixIdentityFlagRefresh(&resultMatrix);
    return resultMatrix;
}

/******************************************************************/
/* 函数定义 */
int MatrixOperationsExecute()
/*
DESCRIPTION:
   执行矩阵操作。
*/
{
    ezwErrors err = ZW_API_NO_ERROR;
    switch (g_simulationSequence)
    {
    case Simulation_Multiplication:
        {
            szwMatrix matrix1 = {}, matrix2 = {}, resultMatrix = {};
            UpdateMatrixData(1, &matrix1, Input_Matrix1);
            UpdateMatrixData(1, &matrix2, Input_Matrix2);
            err = ZwMatrixCrossProduct(matrix1, matrix2, &resultMatrix);
            UpdateMatrixData(0, &resultMatrix, Result_Matrix);
        }
        break;
    case Simulation_Addition:
        {
            szwMatrix matrix1 = {}, matrix2 = {};
            UpdateMatrixData(1, &matrix1, Input_Matrix1);
            UpdateMatrixData(1, &matrix2, Input_Matrix2);
            szwMatrix resultMatrix = MatrixAdd(matrix1, matrix2);
            UpdateMatrixData(0, &resultMatrix, Result_Matrix);
        }
        break;
    case Simulation_Determinant:
        {
            szwMatrix3 matrix = {};
            UpdateMatrix3Data(1, &matrix, Input_Matrix1);
            double determinant = 0.0;
            err = ZwMatrix3Determinant(matrix, &determinant);
            UpdateDeterminantData(0, &determinant, Result_Matrix);
        }
        break;
    case Simulation_Transpose:
        {
            szwMatrix matrix = {};
            UpdateMatrixData(1, &matrix, Input_Matrix1);
            szwMatrix resultMatrix = {};
            err = ZwMatrixTranspose(matrix, &resultMatrix);
            UpdateMatrixData(0, &resultMatrix, Result_Matrix);
        }
        break;
    case Simulation_Vector_Multiply:
        {
            szwVector vector1 = {}, vector2 = {};
            UpdateVectorData(1, &vector1, Input_Matrix1);
            UpdateVectorData(1, &vector2, Input_Matrix2);
            szwVector resultVector = {};
            err = ZwVectorCrossProduct(vector1, vector2, &resultVector);
            UpdateVectorData(0, &resultVector, Result_Matrix);
        }
        break;
    case Simulation_X_direction:
    case Simulation_Y_direction:
    case Simulation_Z_direction:
        {
            szwMatrix matrix = {};
            UpdateMatrixData(1, &matrix, Input_Matrix1);
            szwVector resultVector = {};
            err = ZwMatrixCoordinateSystemGet(matrix, nullptr,
                                              g_simulationSequence == Simulation_X_direction ? &resultVector : nullptr,
                                              g_simulationSequence == Simulation_Y_direction ? &resultVector : nullptr,
                                              g_simulationSequence == Simulation_Z_direction ? &resultVector : nullptr);
            UpdateVectorData(0, &resultVector, Result_Matrix);
        }
        break;
    default:
        break;
    }

    if (err != ZW_API_NO_ERROR)
    {
        if (err == ZW_API_INVALID_INPUT)
            cvxMsgDisp2("Matrix operation execution failed, error described as invalid input.", 5);
        else
            cvxMsgDisp2("Matrix operation execution failed.", 5);
    }
    return 0;
}