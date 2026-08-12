# ZWpyTool

围绕 **ZW3D（中望 3D）二次开发 API** 的调研、样板代码与原型工程。

当前主线课题：**对话式改模** —— 打开一个从 Creo 导进来的哑几何工装模型，用一句中文把模型改好。

---

## 目录

| 路径 | 内容 |
|---|---|
| [`ZW3D_对话式改模_Demo方案.md`](ZW3D_对话式改模_Demo方案.md) | Demo 的完整方案：为什么走「直接编辑」而不是「改参数重生成」、形态设计、工期拆解 |
| [`ZW3D_对话式改模_接口速查表.md`](ZW3D_对话式改模_接口速查表.md) | 从官方 API 文档整理的接口速查，按任务分组 |
| [`examples/`](examples/) | 从官方 `ApiHelpDoc_chs.chm` 解包提取的 20 个示例（4,918 行），附索引与优先级说明 |
| [`poc/`](poc/) | **PidTest** —— 持久标识存活性验证工程。验证「直接编辑改变拓扑后，实体标识还能不能查回来」这一地基假设 |

---

## 先看哪个

1. [`ZW3D_对话式改模_Demo方案.md`](ZW3D_对话式改模_Demo方案.md) —— 搞清楚要做什么、为什么这么做
2. [`poc/README.md`](poc/README.md) —— 最大的架构风险在这里出清，半天能跑完
3. [`examples/README.md`](examples/README.md) —— 官方样板代码索引，标了哪 5 个跟本课题直接相关

---

## 编译 PoC

需要 MSVC Build Tools + 已安装的 ZW3D（环境变量 `ZW3D_DIR` 由安装程序设置）。

```bat
cd poc
build.bat
```

产物 `build\PidTest.dll` 拷到 `%ZW3D_DIR%apilibs\`，启动 ZW3D 后命令行输入 `~PidMark`。
详见 [`poc/README.md`](poc/README.md)。

---

## 说明

- `examples/` 下的代码提取自中望官方帮助文档，**(C) ZWSOFT**，仅作学习与接口查阅之用。
- 官方帮助文档 `ApiHelpDoc_chs.chm` 本身**不入库**（见 `.gitignore`），请从 ZW3D 安装目录 `api\` 下自取。
