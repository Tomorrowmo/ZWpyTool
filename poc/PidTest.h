/*
 *  PidTest —— 持久标识存活性验证
 *
 *  目的：验证「直接编辑改变拓扑后，实体标识是否还能查回来」
 *        这是"对话式改模"整个架构成立与否的关键假设。
 *
 *  同时对比两条路：
 *    A) ZwEntityNameSet      + ZwEntityGetByNameAndType   （用户可见的命名）
 *    B) ZwEntityUniqueIdGet  + ZwEntityGetByUniqueId      （内核级唯一 ID）
 *
 *  DLL 名必须是 PidTest.dll —— 初始化函数名 = DLL 名 + "Init"
 */
#ifndef PIDTEST_H
#define PIDTEST_H

#include "VXApi.h"

/* 插件生命周期（由 ZW3D 调用，名字必须与 DLL 同名前缀） */
int PidTestInit(int format, void *data);
int PidTestExit(void);

/* 三个命令 —— 均为非模板命令，用 "~" 前缀调用，不需要 .ui / .tcmd */
int PidMark(void);   /* ~PidMark   给当前零件所有面打标，落盘 CSV */
int PidEdit(void);   /* ~PidEdit   做一次直接编辑，制造拓扑变化   */
int PidCheck(void);  /* ~PidCheck  按标识查回，统计存活率         */

#endif /* PIDTEST_H */
