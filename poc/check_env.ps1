# =====================================================================
#  check_env.ps1 —— ZW3D 二次开发环境自检 + 代码疑点核对
#
#  在【装了 ZW3D 的那台机器】上运行：
#      右键本文件 -> 使用 PowerShell 运行
#      或者：powershell -ExecutionPolicy Bypass -File check_env.ps1
#
#  它不改动任何东西，只读。跑完在同目录生成 env_report.txt，
#  把那个文件内容发回来即可。
# =====================================================================

$ErrorActionPreference = 'Continue'
$report = Join-Path $PSScriptRoot 'env_report.txt'
$lines  = New-Object System.Collections.Generic.List[string]

function Say($t) { $lines.Add($t); Write-Host $t }
function Section($t) { Say ""; Say ("=" * 70); Say ("  " + $t); Say ("=" * 70) }

Section "0. 机器信息"
Say ("OS        : " + (Get-CimInstance Win32_OperatingSystem).Caption)
Say ("PowerShell: " + $PSVersionTable.PSVersion)
Say ("时间      : " + (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'))

# ---------------------------------------------------------------------
Section "1. 定位 ZW3D 安装目录"

$zwRoot = $null

if ($env:ZW3D_DIR -and (Test-Path $env:ZW3D_DIR)) {
    $zwRoot = $env:ZW3D_DIR.TrimEnd('\')
    Say "[OK] 环境变量 ZW3D_DIR = $env:ZW3D_DIR"
} else {
    Say "[--] 环境变量 ZW3D_DIR 未设置或路径无效，开始搜索..."

    $cands = @()
    foreach ($drv in (Get-PSDrive -PSProvider FileSystem).Root) {
        foreach ($sub in @('Program Files\ZWSOFT', 'Program Files (x86)\ZWSOFT', 'ZWSOFT')) {
            $p = Join-Path $drv $sub
            if (Test-Path $p) {
                $cands += (Get-ChildItem $p -Directory -ErrorAction SilentlyContinue).FullName
            }
        }
    }
    # 注册表兜底
    $ukeys = @('HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\*',
               'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\*')
    foreach ($it in (Get-ItemProperty $ukeys -ErrorAction SilentlyContinue)) {
        if ($it.DisplayName -match 'ZW3D|ZWCAD|中望') {
            Say ("     注册表: " + $it.DisplayName + "  ->  " + $it.InstallLocation)
            if ($it.InstallLocation -and (Test-Path $it.InstallLocation)) { $cands += $it.InstallLocation }
        }
    }

    $cands = $cands | Sort-Object -Unique
    foreach ($c in $cands) { Say "     候选: $c" }

    # 优先挑含 api 子目录的（那才是 ZW3D，ZWCAD 2D 版没有）
    foreach ($c in $cands) {
        if (Test-Path (Join-Path $c 'api')) { $zwRoot = $c.TrimEnd('\'); break }
    }
    if (-not $zwRoot -and $cands.Count -gt 0) { $zwRoot = $cands[0].TrimEnd('\') }
}

if (-not $zwRoot) {
    Say "[!!] 没找到 ZW3D 安装目录。请手工把路径填到本脚本顶部，或直接告诉我路径。"
} else {
    Say ""
    Say ">>> 采用的 ZW3D 根目录: $zwRoot"
    Say ("    主程序 exe: " + ((Get-ChildItem $zwRoot -Filter '*.exe' -ErrorAction SilentlyContinue | Select-Object -First 5).Name -join ', '))
}

# ---------------------------------------------------------------------
Section "2. SDK 结构（api 目录）"

$apiDir = $null
$incDir = $null
$libDir = $null

if ($zwRoot) {
    $apiDir = Join-Path $zwRoot 'api'
    if (Test-Path $apiDir) {
        Say "[OK] api 目录存在: $apiDir"
        Say ""
        Say "--- api 下的子目录 ---"
        Get-ChildItem $apiDir -Directory -ErrorAction SilentlyContinue | ForEach-Object {
            $n = (Get-ChildItem $_.FullName -File -Recurse -ErrorAction SilentlyContinue).Count
            Say ("  " + $_.Name.PadRight(28) + " ($n 个文件)")
        }
        Say ""
        Say "--- api 根下的文件 ---"
        Get-ChildItem $apiDir -File -ErrorAction SilentlyContinue | ForEach-Object {
            Say ("  " + $_.Name.PadRight(40) + ("{0,10:N0} B" -f $_.Length))
        }

        # 头文件目录：2024 之后多一层 inc
        foreach ($cand in @((Join-Path $apiDir 'inc'), $apiDir)) {
            if (Test-Path $cand) {
                $h = Get-ChildItem $cand -Filter '*.h' -Recurse -ErrorAction SilentlyContinue
                if ($h.Count -gt 0) { $incDir = $cand; break }
            }
        }
        Say ""
        if ($incDir) {
            $allH = Get-ChildItem $incDir -Filter '*.h' -Recurse -ErrorAction SilentlyContinue
            Say ("[OK] 头文件目录: $incDir   (共 " + $allH.Count + " 个 .h)")
            $vx = $allH | Where-Object { $_.Name -eq 'VXApi.h' }
            if ($vx) { Say ("     VXApi.h 存在: " + $vx[0].FullName) }
            else     { Say "     [!!] 没找到 VXApi.h —— PidTest.h 引用了它" }
        } else {
            Say "[!!] 没找到任何 .h 文件"
        }

        # lib
        $libs = Get-ChildItem $apiDir -Filter '*.lib' -Recurse -ErrorAction SilentlyContinue
        Say ""
        if ($libs.Count -gt 0) {
            Say "[OK] 找到 .lib："
            foreach ($l in $libs) { Say ("     " + $l.FullName) }
            $libDir = $libs[0].DirectoryName
        } else {
            Say "[!!] api 下没找到 .lib —— build.bat 的 ZWLIB/ZWLIBNAME 需要你手工确认"
        }

        # VSIX 向导
        $vsix = Get-ChildItem $apiDir -Filter '*.vsix' -Recurse -ErrorAction SilentlyContinue
        if ($vsix.Count -gt 0) {
            Say ""
            Say "[OK] VS 工程向导（双击安装后可在 VS 里新建 ZW3D 项目）："
            foreach ($v in $vsix) { Say ("     " + $v.FullName) }
        }
    } else {
        Say "[!!] $apiDir 不存在。"
        Say "     如果你装的是 ZWCAD(2D) 而不是 ZW3D，那 SDK 完全是另一套，本 PoC 不适用。"
    }
}

# apilibs：插件 DLL 的投放目录
if ($zwRoot) {
    $apilibs = Join-Path $zwRoot 'apilibs'
    Say ""
    if (Test-Path $apilibs) {
        Say "[OK] 插件投放目录已存在: $apilibs"
        $ex = Get-ChildItem $apilibs -ErrorAction SilentlyContinue
        Say ("     现有内容: " + $(if ($ex) { ($ex.Name -join ', ') } else { '(空)' }))
    } else {
        Say "[--] $apilibs 不存在 —— 编译出 DLL 后需要自己建这个目录"
    }
}

# ---------------------------------------------------------------------
Section "3. MSVC 编译环境"

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vcvars  = $null
if (Test-Path $vswhere) {
    $insts = & $vswhere -products * -property installationPath 2>$null
    foreach ($i in $insts) {
        Say "[OK] VS 安装: $i"
        $v = Join-Path $i 'VC\Auxiliary\Build\vcvars64.bat'
        if (Test-Path $v) { Say "     vcvars64: $v"; if (-not $vcvars) { $vcvars = $v } }
    }
    $disp = & $vswhere -products * -property displayName 2>$null
    Say ("     版本: " + ($disp -join ' | '))
} else {
    Say "[!!] 没找到 vswhere.exe，VS 可能没装或装在非常规位置"
}

# ---------------------------------------------------------------------
Section "4. 核对 [TODO-VERIFY] #1 —— ezwEntityType 中『面』的枚举名"
Say "（PidTest.cpp 假设是 ZW_ENTITY_FACE，这里从真头文件查实）"
Say ""

if ($incDir) {
    $hit = Select-String -Path (Join-Path $incDir '*.h') -Pattern 'ezwEntityType' -List -ErrorAction SilentlyContinue
    if ($hit) {
        foreach ($f in ($hit | Select-Object -ExpandProperty Path -Unique)) {
            Say "--- 出现于: $f ---"
        }
        # 把枚举体整段打出来
        foreach ($f in ($hit | Select-Object -ExpandProperty Path -Unique)) {
            $txt = Get-Content $f -Raw -ErrorAction SilentlyContinue
            $m = [regex]::Match($txt, 'typedef\s+enum\s+ezwEntityType[\s\S]*?\}\s*ezwEntityType\s*;')
            if ($m.Success) {
                Say ""
                Say ">>> 枚举定义全文（来自 $([System.IO.Path]::GetFileName($f))）:"
                foreach ($l in ($m.Value -split "`r?`n")) { Say ("    " + $l) }
            }
        }
    } else {
        Say "[!!] 头文件里没搜到 ezwEntityType"
    }

    Say ""
    Say "--- 所有以 ZW_ENTITY_ 开头的常量（去重）---"
    $consts = Select-String -Path (Join-Path $incDir '*.h') -Pattern 'ZW_ENTITY_[A-Z0-9_]+' -AllMatches -ErrorAction SilentlyContinue |
              ForEach-Object { $_.Matches } | ForEach-Object { $_.Value } | Sort-Object -Unique
    foreach ($c in $consts) { Say ("    " + $c) }
    if ($consts -contains 'ZW_ENTITY_FACE') { Say ""; Say "  >>> ZW_ENTITY_FACE 存在，代码里的假设成立。" }
    else { Say ""; Say "  >>> [!!] 没有 ZW_ENTITY_FACE，需要从上面列表里挑正确的名字替换。" }
}

# ---------------------------------------------------------------------
Section "5. 核对 [TODO-VERIFY] #2 —— szwEntityIdentifier 是否需要手工释放"
Say ""

if ($incDir) {
    $hit = Select-String -Path (Join-Path $incDir '*.h') -Pattern 'szwEntityIdentifier' -List -ErrorAction SilentlyContinue
    foreach ($f in ($hit | Select-Object -ExpandProperty Path -Unique)) {
        $txt = Get-Content $f -Raw -ErrorAction SilentlyContinue
        $m = [regex]::Match($txt, 'typedef\s+struct\s+szwEntityIdentifier[\s\S]*?\}\s*szwEntityIdentifier\s*;')
        if ($m.Success) {
            Say ">>> 结构体定义（来自 $([System.IO.Path]::GetFileName($f))）:"
            foreach ($l in ($m.Value -split "`r?`n")) { Say ("    " + $l) }
        }
    }
    Say ""
    Say "--- 名字里同时含 Identifier/UniqueId 和 Free 的函数（若有，说明要手工释放）---"
    $frees = Select-String -Path (Join-Path $incDir '*.h') `
                -Pattern '\w*(Identifier|UniqueId)\w*Free\w*|\w*Free\w*(Identifier|UniqueId)\w*' `
                -AllMatches -ErrorAction SilentlyContinue |
             ForEach-Object { $_.Matches } | ForEach-Object { $_.Value } | Sort-Object -Unique
    if ($frees) { foreach ($x in $frees) { Say ("    " + $x) } }
    else { Say "    (没有 —— 支持『不需要手工释放』，代码里深拷贝的写法安全)" }
}

# ---------------------------------------------------------------------
Section "6. 核对 [TODO-VERIFY] #3 —— 每个 API 函数由哪个头文件声明"
Say "（左边是函数名，右边是真正声明它的 .h。与 PidTest.cpp 顶部的 include 对照）"
Say ""

$funcs = @(
    'ZwShapeListGet','ZwShapeFaceListGet',
    'ZwEntityNameSet','ZwEntityUniqueIdGet','ZwEntityGetByNameAndType','ZwEntityGetByUniqueId',
    'ZwEntityExistenceCheck','ZwEntityIdGet','ZwEntityHandleFree','ZwEntityHandleListFree',
    'ZwFaceCylinderialCheck','ZwFaceCylinderGeometricInformationGet',
    'cvxPartDERad','cvxPartDERadInit','cvxPartInqHoles','cvxPartFreeHoles',
    'cvxMsgDisp','cvxCmdFunc','cvxCmdFuncUnload','ZwMemoryFree',
    'szwEntityHandle','szwPoint','svxDERad','svxHoleData','ZW_API_NO_ERROR','VX_CODE_GENERAL'
)

if ($incDir) {
    foreach ($fn in $funcs) {
        $r = Select-String -Path (Join-Path $incDir '*.h') -Pattern ("\b" + [regex]::Escape($fn) + "\b") -List -ErrorAction SilentlyContinue
        if ($r) {
            $hdrs = ($r | ForEach-Object { [System.IO.Path]::GetFileName($_.Path) } | Sort-Object -Unique) -join ', '
            Say ("  " + $fn.PadRight(40) + " -> " + $hdrs)
        } else {
            Say ("  " + $fn.PadRight(40) + " -> [!! 没找到 !!]")
        }
    }
}

# ---------------------------------------------------------------------
Section "7. 给 build.bat 用的真实路径"
Say ""
Say "把下面几行抄进 poc\build.bat 顶部（或确认已经一致）："
Say ""
if ($incDir) { Say ("    set `"ZWINC=" + $incDir + "`"") }
if ($libDir) { Say ("    set `"ZWLIB=" + $libDir + "`"") }
if ($libDir) {
    $ln = (Get-ChildItem $libDir -Filter '*.lib' -ErrorAction SilentlyContinue | Select-Object -First 1).Name
    if ($ln) { Say ("    set `"ZWLIBNAME=" + $ln + "`"") }
}
if ($vcvars) { Say ("    set `"VCVARS=" + $vcvars + "`"") }
Say ""
if ($zwRoot -and -not $env:ZW3D_DIR) {
    Say "另外 ZW3D_DIR 没设置，建议设一下（管理员 PowerShell，永久生效）："
    Say ("    [Environment]::SetEnvironmentVariable('ZW3D_DIR','" + $zwRoot + "\','Machine')")
}

# ---------------------------------------------------------------------
Section "完成"
$lines | Out-File -FilePath $report -Encoding utf8
Write-Host ""
Write-Host "报告已写入: $report" -ForegroundColor Green
Write-Host "把这个文件的内容发回来即可。" -ForegroundColor Green
