# ZW3D 官方示例代码索引

从 `ApiHelpDoc_chs.chm` 解包提取，共 **20 个示例 / 4,918 行**，(C) 2024 ZWSOFT。
这是**唯一的官方可编译样板代码**，比文字文档有用得多。

> 提取自 chm 的 HTML 渲染页，**缩进和空行已尽量还原，但未经编译验证**。
> 建议对照 ZW3D 安装目录下 `api\` 的原始工程使用（若有）。

---

## 🔴 与"对话式改模"直接相关（优先看这 5 个）

| 文件 | 行数 | 为什么重要 |
|---|---|---|
| **`17_ex_trudewith_preview…`** | 325 | ⭐ **预览机制的官方范例**。`cvxEchoStart()` / `cvxEchoEnd()` 包住几何操作，输出参数传 `NULL` → 只预览不落地。**"AI 先给你看，你点确认才改"就靠这个**。同时演示了模板命令三回调（Func / Init / Echo）的完整注册与 `cvxDataGetAll` 读表单 |
| **`16_entity_name…`** | 125 | ⭐ **持久标识的官方范例**。`ZwEntityNameSet` + `ZwEntityNameTagSet` 给实体打名字。配合 `ZwEntityGetByNameAndType` 反查，**解决"编辑后面 id 失效"这个核心难题**。也演示了 `ZwEntityPathTransfer` 批量 path→handle |
| **`20_chamfer_add_and_delete…`** | 391 | 特征的**增 + 删**完整闭环。去孔（`cvxPartSimplify`）的直接参考 |
| **`2_topo_inquiry…`** | 362 | **拓扑查询**：面/边/环的遍历。语义提取层的基础，"找出所有圆柱面"这类操作看它 |
| **`1_base_inquiry…`** | 215 | 基础查询入门。先看这个再看 `2_topo_inquiry` |

## 🟡 次相关

| 文件 | 行数 | 内容 |
|---|---|---|
| `15_edit_variable…` | 280 | 变量读写。阶段 4（参数化模板）会用到 |
| `12_dll_register…` | 187 | **DLL 注册的完整范例**，搭工程时第一个看 |
| `14_colored_box…` | 226 | 面属性设置（颜色）。**高亮预览**可参考 |
| `19_matrix_operations…` | 668 | 矩阵变换全集。定位、阵列、坐标系换算 |
| `3_view_tool…` | 570 | 视图操作。截图给 AI 看时会用到 |
| `6_pattern…` | 126 | 阵列。"四个角的孔"这类成组操作 |
| `18_layer_entity_manager…` | 144 | 图层管理 + 按图层批量取实体 |

## 🟢 其他

| 文件 | 行数 | 内容 |
|---|---|---|
| `4_fillet_box…` | 148 | 圆角（含 Custom 特征 `CustomOp` 用法）|
| `5_file_operation…` | 184 | 文件打开/保存/关闭 |
| `9_file_export…` | 194 | 导出 |
| `10_igs_operation…` | 131 | IGS 导入导出。**批量格式转换可参考** |
| `13_table_set…` | 202 | 表格控件 |
| `11_opt_auxframe…` | 148 | 辅助坐标系 |
| `7_curve_create…` | 91 | NURBS 曲线 |
| `8_nurbs_surface…` | 201 | NURBS 曲面 |

---

## 从 `17_ExTrudewithPreview` 提炼的骨架

```c
/* ---- 注册（在插件 Init 里调用）---- */
ZwCommandFunctionLoad("MyCmd",     (void *)MyCmd,     ZW_LICENSE_CODE_GENERAL);
ZwCommandCallbackLoad("MyCmdInit", (void *)MyCmdInit);   /* 表单创建前：填默认值 */
ZwCommandCallbackLoad("MyCmdEcho", (void *)MyCmdEcho);   /* 参数变化：实时预览   */

/* ---- 卸载（在插件 Exit 里）---- */
ZwCommandFunctionUnload("MyCmd");
ZwCommandFunctionUnload("MyCmdInit");
ZwCommandFunctionUnload("MyCmdEcho");

/* ---- 预览回调：与执行函数逻辑相同，但不落地 ---- */
void MyCmdEcho(int idData, void *ohEcho)
{
    cvxEchoStart();
    /* …几何操作，输出参数一律传 NULL… */
    cvxEchoEnd();
}

/* ---- 执行函数：点 OK/APPLY 时调用 ---- */
int MyCmd(int idData)
{
    svxFldData *fldData = nullptr;
    int numField = 0;
    if (cvxDataGetAll(idData, &numField, &fldData)) return 1;
    /* fldData[i].count / fldData[i].fld_data[j].idEntity / .Num / .Dir */
    /* …真正执行，输出参数正常接收… */
    return 0;
}

/* ---- 初始化回调：注意此时表单尚未创建，不要碰表单 ---- */
void MyCmdInit(int idData)
{
    svxData d{};
    d.isNumber = 1; d.Num = 15; d.NumType = VX_DST;
    cvxDataSet(idData, FIELD_ID, &d);
}
```

**新旧接口搭桥的实际用法**（示例 17 里反复出现）：
```c
int idEnt = fldData[0].fld_data[i].idEntity;   /* 表单给的是旧的 id */
szwEntityHandle h{};
ZwEntityIdTransfer(1, &idEnt, &h);             /* → 转成新的 handle */
ZwEntityTypeNumberGet(h, &type);               /* 用新接口查类型   */
ZwEntityCurveCheck(h, &isCurve);
/* …用完必须释放… */
ZwEntityHandleFree(&h);
```

---

## 提取方式

```
hh.exe -decompile <输出目录> ApiHelpDoc_chs.chm
```
示例页为 `*-example.html`（GBK 编码），代码在 `<div class="fragment">` / `<div class="line">` 里。
