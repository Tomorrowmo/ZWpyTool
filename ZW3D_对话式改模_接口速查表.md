# ZW3D 对话式改模 · 接口速查表

> 数据来源：`ApiHelpDoc_chs.chm`（生成日期 2026-05-22，项目名 ZWAPIOPEN）
> 全库规模：324 个模块 / 4,296 个函数（`Zw*` 新接口 1,538 + `cvx*` 旧接口 1,907）
> 用途：Creo 历史模型导入中望后，用自然语言驱动局部改型（改尺寸 / 加孔 / 去孔）

---

## ⚠️ 阅读前必读

1. **本表的字段说明由脚本从 chm 批量提取，个别字段的中文描述存在错位**（Doxygen 的 `memitem`/`memdesc` 行配对问题，例如 `idShapes` 与 `NoProjPntLoc` 的说明疑似串行）。**动手写代码前，务必回 chm 原页面核对该字段**。函数签名和字段名本身是准确的。
2. 直接编辑（DE）与孔操作 **全部只有 `cvx*` 旧接口**，新 `Zw*` 接口尚未覆盖。本方案必须新旧混用。
3. 所有 `cvx*` 接口返回 `evxErrors`，`Zw*` 返回 `ezwErrors`，成功均为 `ZW_API_NO_ERROR`。**每次调用都要判返回值。**

---

## 一、总链路

```
Creo 文件
  └─ ZwFileProeImport ─────────────► 中望哑几何
                                        │
        ┌───────────────────────────────┴──────────────────┐
        │  语义化提取（AI 的"眼睛"）                        │
        │   cvxPartInqHoles      → 全部孔 + 完整属性        │
        │   ZwFaceThreadDataGet  → 面上螺纹数据             │
        │   ZwFaceCylinderialCheck / ...GeometricInfo...    │
        │   cvxPartInqFaceBox / cvxPartInqFaceSrfPrim       │
        │   ZwFaceConnectedFaceGet → 相邻面（相关面组）      │
        └───────────────────────────────┬──────────────────┘
                                        ▼
                            "模型语义清单" JSON
                                        ▼
                         AI：自然语言 → 结构化编辑指令
                                        ▼
                            确定性规则校验层
                                        ▼
        ┌───────────────────────────────┴──────────────────┐
        │  执行（全部 cvx 旧接口）                          │
        │   cvxPartModifyHole  改孔                         │
        │   cvxPartHole        加孔                         │
        │   cvxPartSimplify    去孔 / 删面                  │
        │   cvxPartDEDimMove   按尺寸移动面                 │
        │   cvxPartDEOffset    偏移面                       │
        │   cvxPartDERad       改圆柱/球半径                │
        │   cvxPartDEFllt      改圆角                       │
        └───────────────────────────────┬──────────────────┘
                                        ▼
              验证：ZwFileShapeCompare / ZwComponentInterferenceCheck
              回退：ZwHistoryOperationDelete（每步编辑都生成独立特征）
```

---

## 二、导入：`ZwFileProeImport`

```c
ZW_API_C ezwErrors ZwFileProeImport(
    const zwPath           filePath,   // Proe/Creo 文件完整路径
    ezwFileImportTo        importTo,   // 导入目标
    szwProeImportOptions  *options);   // 传 NULL 则使用平台当前配置
```
> chm 原文："导入 Proe/Creo 文件到 ZW3D。功能索引：文件 → 输入"
> 初始化用 `ZwFileProeImportInit()`

### `szwProeImportOptions`

| 字段 | 说明 |
|---|---|
| `int repairGap` | 是否修复缝隙 |
| `int autoActivatePart` | 是否自动激活零件 |
| `int createSubPart` | 是否自动创建子零件 |
| **`int associativeImport`** | **是否关联导入** ← 值得重点验证其语义 |
| `ezwFileImportMode importMode` | 输入模式 |
| `int freeCurve` / `freePoint` / `sheetBody` | 自由曲线 / 自由点 / 片体 |
| `int suppressedComponent` | 是否导入抑制的组件 |
| `int workplane` | 是否导入参考平面 |
| `int hiddenEntity` / `hiddenComponent` | 隐藏实体 / 隐藏组件 |
| `int connectorPort` | 是否当作线束连接器读取 |
| `ezwFileImportPMI PMI` | 是否导入 PMI 对象 |
| `int count` + `zwPath *searchPaths` | 文件搜索路径列表（**装配体找子零件靠这个**） |

**POC 要验的**：`createSubPart` + `searchPaths` 能否完整还原装配层级；`associativeImport` 到底关联什么；批量导入时会不会弹窗。

---

## 三、语义层核心：`cvxPartInqHoles`

```c
ZW_API_C evxErrors cvxPartInqHoles(
    const vxLongPath  File,    // 文件名（激活文件传 NULL，未加载文件传完整路径）
    const vxRootName  Part,    // 根对象名（激活零件/装配传 NULL）
    int              *Count,   // [out] 孔的数量
    svxHoleData     **Holes);  // [out] 孔数据列表
```
> **内存：调用者负责用 `cvxPartFreeHoles()` 释放 `Holes`**

### 🚨 chm 明确标注的查询限制

> "目前结构体中的变量 `idInsFace` 和 `idUntilFace` 不被支持固定输出 0，变量 `ThreadType` 固定输出 `VX_THREAD_CUSTOM`"

**含义**：`cvxPartInqHoles` **读不出螺纹类型**，只能拿到 `ThreadDiameter`。
**补救**：用 `ZwFaceThreadDataGet` / `ZwFaceExternThreadDataGet` 从面上单独取螺纹数据。**这是语义层必须打的一个补丁。**

### `svxHoleData` 字段全表

这个结构体既是 `cvxPartInqHoles` 的输出，也是 `cvxPartHole` / `cvxPartModifyHole` 的输入——**读和写用同一个结构体，这是整个方案能成立的关键**。

**类型与定位**
| 字段 | 说明 |
|---|---|
| `evxHoleType Type` | 打孔方式 |
| `evxHoleOpt HoleOpt` | 孔类型 |
| `evxClearHoleType ClearHoleType` | 间隙孔类型（仅 `HoleOpt=VX_CLEARANCE_HOLE` 时生效）|
| `int idInsFace` | 打孔面对象 id（传 0 忽略）· ⚠️ 查询时固定返回 0 |
| `int Count` + `svxPoint *Points` | 定位点数量 + 坐标列表 |
| `int UseDirection` + `svxVector Direction` | 孔中心线方向（默认 0 = 不使用）|
| `evxEndCondition End` + `int idUntilFace` | 结束端类型 · ⚠️ `idUntilFace` 查询时固定返回 0 |

**几何尺寸**
| 字段 | 说明 |
|---|---|
| `double Diameter1` / `Depth1` | 孔直径 / 深度 (mm) |
| `double Diameter2` / `Depth2` | 沉孔直径 / 深度 (mm) |
| `double TaperAngle` | 锥形孔角度 (deg) |
| `double TipAngle` | 孔尖角度 (deg)（0 转为 180，**默认 118**）|
| `double slotLength` / `slotAngle` | 槽长度 (mm) / 槽角度 (deg) |

**螺纹**
| 字段 | 说明 |
|---|---|
| `evxThreadType ThreadType` | 螺纹孔类型 · ⚠️ **查询时固定返回 `VX_THREAD_CUSTOM`** |
| `vxLongName ThreadSize` | 螺纹规格 · 取值参考 `zw3d/supp/Thread.xml` 的 `SizeValue` |
| `double ThreadDiameter` | 螺纹直径 (mm)（仅 `ThreadType=VX_THREAD_CUSTOM` 生效）|
| `double ThreadLength` | 螺纹深度（仅 `threadDepthType=VX_FLAG_THREAD_DEPTH_CUSTOM` 生效）|
| `double ThreadsPerUnit` / `ThreadPitch` | 螺纹/单位 · 螺距 (mm)（传 0 忽略）|
| `evxFlagThreadDepthType threadDepthType` | 螺纹深度类型 |
| `int fThrdClass` / `int thrdClass` | 标记为螺纹孔 / 螺纹类别（**仅用于"标记孔"特征**，参考 `evxThreadHoleClass`）|

**标准件关联** ← 语义层的金矿
| 字段 | 说明 |
|---|---|
| `vxLongName Standard` | 执行标准 · 取值参考 `zw3d/supp/Simple.xml` 的 `Standard` |
| `vxLongName Screw` | 螺旋类型 · 参考 `Simple.xml` 的 `ScrewType` |
| `vxLongName Size` | 尺寸 · 参考 `Simple.xml` 的 `ThreadSize` |
| `vxLongName Fit` | 配合方式：`Close` / `Normal` / `Loose` / `Custom` |

> **`Standard` + `Screw` + `Size` + `Fit` 四个字段合起来，就是一句人能读懂的话："GB 内六角螺钉 M8 Normal 配合"。语义标签几乎是白送的。**

**倒角**（`svxHoleChams` × 3：起始边 / 中间段 / 末端）
| `svxHoleChams` 字段 | 说明 |
|---|---|
| `int fCham` | 1 启用孔倒角，0 不启用（默认 0）|
| `int fChamSym` | 1 启用对称倒角（默认 0）|
| `double chamDst` / `chamAngle` | 倒角距离 / 角度 |

> ⚠️ `sCham` / `mCham` / `eCham` 三者与"起始边/中间段/末端"的对应关系，**在我的提取中疑似错位一行，务必回 chm 核对**。

**公差 / 布尔 / 其他**
| 字段 | 说明 |
|---|---|
| `int UseTol` + `double TolIn` / `TolOut` | 孔直径公差下限 / 上限 |
| `int UseD2Tol` + `double D2InTol` / `D2OutTol` | 沉孔直径公差（默认 0 = 不使用）|
| `int OpNoRemove` | 布尔操作（默认 0）|
| `int cntShps` + `int *idShapes` | 布尔基体数量 + id 列表（`OpNoRemove=0` 且 `cntShps=0` 时默认取所有与孔接触的实体）|
| `char Callout[128]` | **编号标签** ← 可直接用作人类可读标识 |
| `int DoNotMachine` | 1 不加工，0 加工（默认 0）|
| `int idSketch` | 轮廓孔的草图 id（传 0 忽略）|
| `int AddBase` | 是否添加底孔（默认 1，`Type=VX_HOLE_TAPERED` 时生效）|

---

## 四、执行层

### 4.1 改孔 — `cvxPartModifyHole`

```c
ZW_API_C evxErrors cvxPartModifyHole(
    int                 cntFace,  // 孔面数量
    int                *pFace,    // 孔面 id 列表
    const svxHoleData  *data,     // 新的孔数据
    int                *idFtr);   // [out] 新特征 id（传 NULL 忽略）
```
> "创建一个**修改孔**特征"，初始化用 `cvxPartModifyHoleInit()`

**用法**：`cvxPartInqHoles` 读出来 → 改 `Diameter1` → 传回去。读写同构，最省事。

### 4.2 加孔 — `cvxPartHole`
```c
cvxPartHoleInit(&data);   // 先初始化，默认值不全为 0
cvxPartHole(...);
```

### 4.3 去孔 / 删面 — `cvxPartSimplify`

```c
ZW_API_C evxErrors cvxPartSimplify(
    int   numEnts,      // 面对象或特征对象的数量
    int  *idEnts,       // 面对象或特征对象的 id 列表
    int  *idSimplify);  // [out] 新特征 id（传 NULL 忽略）
```
> "创建**简化特征**。将指定的面或者特征参数的面**移除掉**"

**这就是"删面 + 愈合"。** 去掉一个孔 = 把孔的圆柱面（+ 沉孔面）传进去。

**兜底方案**：万一某些孔简化失败（开口在曲面上、与其他特征交叠），改用 `ZwFeatureAddShapeCreate` 布尔加一个圆柱把孔填上——一定能成，只是历史树上多一步。

### 4.4 改局部尺寸 — `cvxPartDEDimMove`（最常用）

```c
struct svxDEDimMove {
    evxDEDimMoveType type;        // 对齐移动类型（LINEAR / ANGULAR）
    int              idMotion;    // 移动面 id
    int              idStation;   // 固定参考对象 id
    int              isSwitch;    // 1=随动面上的点，0=随动面实体（ANGULAR 时忽略）
    svxPoint         stationPnt;  // LINEAR 且 isSwitch=1 时用，点必须在固定参考对象上
    int              numEnt;      // 随动面数量
    int             *idEnts;      // 随动面 id 列表   ← 解决"相关面跟不跟着走"
    double           dist;        // 移动距离（默认 0，LINEAR 时由用户确定）
    svxPntOnEnt      anchorPnt;   // 通过点（LINEAR 时用）
    svxPntOnEnt      dir;         // 测量方向（LINEAR 时用）
    double           angle;       // 角度（ANGULAR 时用，默认 0）
    evxDEFaceOverflow overflow;   // 面溢出类型（默认 VX_DE_OVERFLOW_AUTO）
};
```

> **`idEnts` 随动面列表是这个方案的关键。** 哑几何没有约束联动，"把侧压板从 12 加到 15" 时，相邻的圆角、倒角、螺纹孔需要一起进随动面列表，否则会撕裂。**语义层必须能自动算出"相关面组"** → 用 `ZwFaceConnectedFaceGet`。

### 4.5 其他直接编辑

```c
// 改圆柱面/球面半径（改孔径、改环槽内径）
struct svxDERad { int numEnt; int *idEnts; double R; };  // R 默认 1
cvxPartDERad(&data, &idRad);

// 偏移面
struct svxDEOffset {
    int numEnt; int *idEnts; double dist;
    evxDESideFaceType  side;          // 默认 VX_DE_SIDE_FACE_CREATE
    evxDEIsectType     intersection;  // 默认 VX_DE_ISECT_NO_REMOVE
    evxDEFaceOverflow  overflow;      // 默认 VX_DE_OVERFLOW_AUTO
};

// 改圆角
struct svxDEFllt {
    int numEnt; int *idEnts;
    double R;        // 默认 5
    int chain;       // 是否链选相同半径（默认 0）← 批量改圆角靠它
    double relief; int arcType; double ratio; int hold; int strategy;
    int numEdge; int *idEgdes;
};

// 移动面（含 union svxDEMoveData：点/方向/坐标系/路径/旋转/动态 六种方式）
struct svxDEMove { evxDEMoveCopyType type; int numEnt; int *idEnts;
                   evxDEFaceOverflow overflow; union svxDEMoveData move; };
```

**直接编辑模块全量（20 个函数，均配 `*Init`）**：
`cvxPartDEMove` · `cvxPartDEOffset` · `cvxPartDEDimMove` · `cvxPartDEAlignMove` · `cvxPartDERad` · `cvxPartDEFllt` · `cvxPartDEDraft` · `cvxPartDEPtn` · `cvxPartDECopy` · `cvxPartDEMirror`

---

## 五、面查询（语义层工具箱）

`ZwFace*` 51 个 + `cvx*Face*` 41 个。挑出对本方案有用的：

| 需求 | 接口 |
|---|---|
| **是不是圆柱面**（→ 是不是孔） | `ZwFaceCylinderialCheck` |
| 圆柱面几何信息（半径、轴线） | `ZwFaceCylinderGeometricInformationGet` |
| **面上的螺纹数据** ← 补 InqHoles 的缺口 | `ZwFaceThreadDataGet` / `ZwFaceExternThreadDataGet` |
| 曲面基元类型（平面/圆柱/圆锥/球） | `cvxPartInqFaceSrfPrim` · `ezwFaceGeometryType` |
| 面的包围盒（**定位、判断"左边那个"**） | `cvxPartInqFaceBox` |
| 面积 | `ZwFaceAreaGet` · `cvxFaceGetArea` |
| **相邻面 / 相邻边**（算相关面组） | `ZwFaceConnectedFaceGet` · `ZwFaceConnectedEdgeGet` |
| 凹凸判断 | `ZwFaceConcaveCheck` · `cvxFaceIsConcave` |
| 是否平面 | `cvxFaceIsPlanar` |
| 回转面轴线 | `ZwRotationalFaceAxisPointGet` |
| 圆角面识别 / 半径 | `cvxPartInqFaceFillet` · `cvxFaceRadius` |
| 面的显示属性（**高亮预览用**） | `ZwFaceDisplayAttributeSet` + `...SetInit` |

> **`ZwFaceDisplayAttributeSet` 是做"AI 高亮它认为要改的面"那个交互的关键接口。**

---

## 五·五、⭐ 持久标识 —— 解决"面 id 失效"的正解

> 来源：官方示例 `16_EntityName` + chm 接口表。**这组接口把架构从"每轮重建索引"升级为"一次打标、按名索引"，是本方案最重要的一个发现。**

### 5.5.1 实体命名（持久，随文件保存）

```c
ZW_API_C ezwErrors ZwEntityNameSet(szwEntityHandle entityHandle, const zwString32 name);
ZW_API_C ezwErrors ZwEntityNameGet(...);
ZW_API_C ezwErrors ZwEntityNameTagSet(szwEntityHandle entityHandle, const zwString32 name);
ZW_API_C ezwErrors ZwEntityNameTagGet(...);
ZW_API_C ezwErrors ZwEntityNameTagGetAll(int *count, zwString256 **nameTags);

/* ★ 反向查找：按名字直接拿实体句柄 */
ZW_API_C ezwErrors ZwEntityGetByNameAndType(const zwString32 name,
                                            ezwEntityType    type,
                                            szwEntityHandle *entityHandle);
```
> 旧接口对应：`cvxEntByName` / `cvxEntByName2` / `cvxEntSetNameTag`

**用法**：首次语义提取时给每个孔面/关键面打上稳定名字（`H001`、`BASE_TOP` …），之后**永远按名字查，不缓存 id**。编辑改变拓扑后，id 作废但名字仍在。

### 5.5.2 拾取集 —— 把"一组面"存成命名集合

```c
ZW_API_C ezwErrors ZwPicksetCreate(const zwString32       pickSetName,
                                   int                    entityCount,
                                   const szwEntityHandle *entityList,
                                   szwEntityHandle       *picksetHandle);
ZW_API_C ezwErrors ZwPicksetEntityAdd(szwEntityHandle picksetHandle,
                                      int entityCount, const szwEntityHandle *entityList);
ZW_API_C ezwErrors ZwPicksetListGet(int *pickSetCount, szwEntityHandle **picksetHandles);
ZW_API_C ezwErrors ZwPicksetInformationGet(...);
ZW_API_C ezwErrors ZwPicksetRename(...);
ZW_API_C ezwErrors ZwPicksetDelete(...);
ZW_API_C ezwErrors ZwPicksetEntityDelete(...);
```

**用法**：把"底板-四角安装孔组"直接建成一个 pickset 存进模型。
AI 说"四角安装孔组" → `ZwPicksetListGet` → 拿到那 4 个孔的全部面。**指代消解从"每次重算"变成"一次建立、永久可查"。**

### 5.5.3 实体自定义属性 —— 给面挂任意键值对

```c
ZW_API_C ezwErrors ZwEntityUserAttributeSet(szwEntityHandle          entityHandle,
                                            int                      count,
                                            const szwUserAttribute  *attributeList);
ZW_API_C ezwErrors ZwEntityUserAttributeGet(szwEntityHandle    entityHandle,
                                            const zwString64   attributeName,
                                            int               *count,
                                            szwUserAttribute **attributeList);
ZW_API_C ezwErrors ZwEntityUserAttributeDelete(...);
```

**用法**：把 AI 语义层的全部元数据挂在面上，随模型走：
```
semantic_label = "底板-四角安装孔"
group_id       = "MOUNTING_HOLES"
std_spec       = "GB/T 70.1 M10"
last_edit      = "2026-08-06 Φ10→Φ12"
```

### 5.5.4 由此得到的架构

```
首次导入
  └─ 语义提取（cvxPartInqHoles + ZwFace*）
      └─ 给每个孔/关键面：ZwEntityNameSet + ZwEntityUserAttributeSet
      └─ 给每个语义组：   ZwPicksetCreate
          ↓  ★ 语义层从此持久化在模型里，不在内存里
每轮编辑
  └─ AI 输出语义标签 → ZwEntityGetByNameAndType / ZwPicksetListGet → 拿到当前 handle
  └─ ZwEntityIdGet 转成 id → 调 cvx* 执行
  └─ 编辑后受影响的面重新打标（增量，不是全量重建）
```

> **原风险 #1（面 id 失效）由"每轮全量重建"降级为"增量重新打标"。**
> **副产品**：语义层随 `.Z3` 文件保存，换台电脑打开还在；也让"这个模型上次 AI 改过什么"变得可追溯。

---

## 五·六、预览机制 —— `cvxEchoStart` / `cvxEchoEnd`

> 来源：官方示例 `17_ExTrudewithPreview`

模板命令的三个回调，用 `ZwCommandCallbackLoad` 注册：

```c
ZwCommandFunctionLoad("MyCmd",     (void*)MyCmd,     ZW_LICENSE_CODE_GENERAL);
ZwCommandCallbackLoad("MyCmdInit", (void*)MyCmdInit);  /* 表单创建前，填默认值 */
ZwCommandCallbackLoad("MyCmdEcho", (void*)MyCmdEcho);  /* ★ 参数变化时实时预览 */
```

```c
void MyCmdEcho(int idData, void *ohEcho)
{
    cvxEchoStart();
    /* …执行与正式函数完全相同的几何操作，
       但输出参数传 NULL —— 结果只作为预览显示，不落地进模型… */
    ZwFeatureExtrudeCreate(extrudeData, NULL);   /* ← 注意最后一个参数 NULL */
    cvxEchoEnd();
}
```

**这就是"AI 说要改什么 → 先预览 → 用户确认 → 才真改"的官方机制。**
读表单数据用 `cvxDataGetAll(idData, &numField, &fldData)`（拿全部字段）或 `cvxDataGetNum` / `cvxDataGet`（拿单个字段）。

---

## 六、验证与回退

| 用途 | 接口 |
|---|---|
| **改前/改后几何比对** | `ZwFileShapeCompare(baseFile, baseRoot, cmpFile, cmpRoot, &data)` — 输出相同/新增/删除三类面列表。⚠️ 不支持 `.Z3ASM` / `.Z3DRW`，两个文件都必须已在 ZW3D 中打开 |
| 干涉检查 | `ZwComponentInterferenceCheck` · `ZwShapeInterferenceCheck` |
| 质量 / 体积 | `cvxPartMassProp` · `cvxPartVolumAndMassGet` · `cvxPartInqShapeMass` |
| **单步撤销** | `ZwHistoryOperationDelete` |
| 历史树读取 / 重排 / 重放 | `ZwHistoryListGet` · `ZwHistoryListReorder` · `ZwHistoryReplay` |

> **每一次直接编辑都会生成一个独立特征**（`idSimplify` / `idFtr` / `idRad` 都是新特征 id），所以逐步回退是可行的。这是敢让 AI 动手改模型的技术前提。

---

## 七、内存管理速查（新旧两套不同，最容易出 bug）

| 场景 | 释放方式 |
|---|---|
| `cvxPartInqHoles` 输出的 `Holes` | `cvxPartFreeHoles()` |
| 单个 `szwEntityHandle` | `ZwEntityHandleFree(&handle)` |
| `szwEntityHandle` 数组（已知长度） | 循环 `ZwEntityHandleFree(&handles[i])` |
| `szwEntityHandle` 数组（未知长度） | `ZwEntityHandleListFree(count, &list)` |
| 有专用释放接口的 | 按 chm 注释调专用的（如 `ZwCurveNURBSDataFree`）|
| 自行分配 | `ZwMemoryAlloc` ↔ `ZwMemoryFree`；`cvxMemFree` |

**新旧对象标识互转**：
```c
ZwEntityIdTransfer(count, indexes, entityHandles);    // id   → handle
ZwEntityPathTransfer(count, entityPaths, handles);    // path → handle
ZwEntityIdGet(count, handles, indexes);               // handle → id
ZwEntityPathGet(count, handles, entityPaths);         // handle → path
```
> **本方案会频繁用到**：语义层用 `Zw*`（handle），执行层用 `cvx*`（id），中间靠这四个函数搭桥。

---

## 八、已知风险清单

| # | 风险 | 应对 |
|---|---|---|
| 1 | ~~**面 id 在每次编辑后失效**（拓扑变了）~~ **已降级** | ✅ 用 §5.5 的持久标识：`ZwEntityNameSet` 打标 + `ZwEntityGetByNameAndType` 按名查 + `ZwPicksetCreate` 存语义组。从"每轮全量重建"降为"增量重新打标"。**绝不缓存裸 id** |
| 2 | `cvxPartInqHoles` 读不出 `ThreadType` | 用 `ZwFaceThreadDataGet` 补 |
| 3 | 哑几何无约束联动，改一个面会撕裂 | `svxDEDimMove.idEnts` 随动面列表 + `ZwFaceConnectedFaceGet` 自动算相关面组 |
| 4 | `cvxPartSimplify` 对复杂孔可能失败 | 兜底：`ZwFeatureAddShapeCreate` 布尔加圆柱填孔 |
| 5 | DE 与孔操作全是旧 `cvx*` 接口 | 接受混用；封装一层适配层隔离，将来新接口补齐时只改适配层 |
| 6 | **headless 未知** | chm 中 `Batch` / `Silent` 零命中。**必须向中望确认**——决定产品是桌面插件还是服务端 |
| 7 | 参数默认值不全为 0 | **所有 `*Init` 函数都必须先调**，否则行为不可预期 |

---

## 九、最小代码骨架

```c
#include "zwapi_file_general.h"
#include "zwapi_part_hole.h"
#include "zwapi_cmd_shape.h"
#include "zwapi_cmd_direct_edit.h"
#include "zwapi_brep_face.h"

/* ---------- 1. 导入 Creo ---------- */
szwProeImportOptions opt;
ZwFileProeImportInit(&opt);
opt.createSubPart = 1;
/* opt.searchPaths = ...; opt.count = ...; 装配体要给搜索路径 */
if (ZwFileProeImport(creoPath, importTo, &opt) != ZW_API_NO_ERROR) return -1;

/* ---------- 2. 语义提取：枚举全部孔 ---------- */
int nHole = 0;
svxHoleData *holes = NULL;
if (cvxPartInqHoles(NULL, NULL, &nHole, &holes) == ZW_API_NO_ERROR)
{
    for (int i = 0; i < nHole; i++)
    {
        /* 导出为 JSON 供 AI 消费：
           Diameter1 / Depth1 / Diameter2 / Depth2
           Standard / Screw / Size / Fit   ← 人类可读标签的来源
           Points[0..Count-1]              ← 位置，用于"左边那个"的指代消解
           Direction / Callout
           注意：ThreadType 此处不可信，需用 ZwFaceThreadDataGet 单独取 */
    }
    cvxPartFreeHoles(nHole, &holes);   /* 必须释放 */
}

/* ---------- 3a. 改孔径：Φ10 → Φ12 ---------- */
svxHoleData hd;
cvxPartModifyHoleInit(&hd);           /* 默认值不全为 0，必须先 Init */
hd.Diameter1 = 12.0;
int idFtr = 0;
cvxPartModifyHole(nTargetFaces, targetFaceIds, &hd, &idFtr);

/* ---------- 3b. 去孔 ---------- */
int idSimplify = 0;
cvxPartSimplify(nHoleFaces, holeFaceIds, &idSimplify);

/* ---------- 3c. 改圆柱面半径（环槽内径 +2） ---------- */
svxDERad rad;
cvxPartDERadInit(&rad);
rad.numEnt = 1;
rad.idEnts = &idCylFace;
rad.R      = oldR + 1.0;              /* 半径 +1 = 直径 +2 */
int idRad = 0;
cvxPartDERad(&rad, &idRad);

/* ---------- 3d. 改板厚：12 → 15 ---------- */
svxDEDimMove dm;
cvxPartDEDimMoveInit(&dm);
dm.type     = VX_DE_DIM_LINEAR;
dm.idMotion = idTopFace;              /* 要移动的面 */
dm.idStation= idBottomFace;           /* 固定参考 */
dm.dist     = 3.0;
dm.numEnt   = nFollow;                /* 随动面：相邻圆角、倒角、孔壁 */
dm.idEnts   = followFaceIds;          /* ← 靠 ZwFaceConnectedFaceGet 算出来 */
int idDim = 0;
cvxPartDEDimMove(&dm, &idDim);

/* ---------- 4. 撤销上一步 ---------- */
/* ZwHistoryOperationDelete(...)  — 每步编辑都是独立特征 */
```

---

## 十、还没查的 / 建议下一步补

- `evxHoleType` / `evxHoleOpt` / `evxDEDimMoveType` / `evxDEFaceOverflow` / `ezwFileImportMode` 的**枚举取值全表**（枚举页在 chm 里散落在各 group 页，需单独定位）
- `zw3d/supp/Thread.xml` 与 `zw3d/supp/Simple.xml` 的实际内容（**装了 ZW3D 后直接打开看**——这两个文件定义了全部标准孔/螺纹规格，是语义层的字典）
- `svxDEMoveData` union 六个分支的完整字段
- `ZwFaceThreadDataGet` 的输出结构体
- chm 自带的 **20 个示例工程**（解包目录下 `*-example.html`），其中 `20_ChamferAddAndDelete`、`15_EditVariable`、`16_EntityName`、`17_ExTrudewithPreview` 与本方案直接相关
