# AyuGram 本機開發與發布手冊

> 最後核對：2026-07-14。此文件描述 `D:\TBuild` 的 Windows 原生
> checkout，並作為後續 AI agent 的主要接手入口。

## 文件權限與適用範圍

本文件記錄此 fork 的實際目錄、分支、建置、驗證、tag 與 GitHub
Release 程序。歷史研究文件可以提供背景，但如果內容與本文件或
`AGENTS.md` 衝突，以目前 checkout、`AGENTS.md` 與本文件為準。

發布不是每次程式碼修改的自動後續。正常順序是先完成一個可測試段落，
產出 Debug EXE 交給開發者確認；只有在開發者確認後，才能整合到 `dev`、
本機建置 Release、建立簽署 tag 並上傳一般 GitHub Release。

## 不可違反的遠端規則

- `origin` 是 `zeta987/AyuGramDesktop`，是 superproject 唯一可寫遠端。
- `upstream` 的 `AyuGram/AyuGramDesktop` 與 `telegram` 的
  `telegramdesktop/tdesktop` 一律視為唯讀，即使 Git 設定中存在 push
  URL，也禁止 push。
- 永遠不可向 AyuGram、Telegram Desktop、desktop-app 或任何其他上游
  建立 Pull Request。
- 需要修改外部 repo 或 submodule 時，先建立或重用 `zeta987` fork，
  在 fork 的 feature branch 提交並推送，再更新 superproject 的
  `.gitmodules` 與 gitlink。
- Pull Request 只可在使用者自己的 fork 內、且在使用者明確要求時建立；
  PR 說明必須註明由 AI 產生。
- Issue 與 PRD 只放在 `zeta987/AyuGramDesktop`。完整規則見
  `docs/agents/issue-tracker.md`。

目前四個帶自訂提交的 submodule 已指向使用者自己的 fork：

| 路徑 | 可寫 fork |
| --- | --- |
| `Telegram/codegen` | `zeta987/codegen` |
| `Telegram/lib_tl` | `zeta987/lib_tl` |
| `Telegram/lib_ui` | `zeta987/lib_ui` |
| `cmake` | `zeta987/cmake_helpers` |

其他 submodule 預設保持上游唯讀。修改任何其他 submodule 前，必須先建立
對應的 `zeta987` fork，不可把只有本機才存在的 commit 寫入 superproject。

## D:\TBuild 目錄地圖

`D:\TBuild` 本身不是有效 Git repository；其中的空 `.git` 目錄不代表
可提交的 repo。真正承載 `dev`、tags 與文件的 superproject 是
`D:\TBuild\tdesktop`。

| 路徑 | 關係與用途 |
| --- | --- |
| `D:\TBuild\tdesktop` | `zeta987/AyuGramDesktop` superproject；主要整合分支是 `dev`。 |
| `D:\TBuild\tdesktop\Telegram\...` | Telegram/AyuGram 主程式與多個 Git submodule。 |
| `D:\TBuild\tdesktop\cmake` | CMake helper submodule；自訂提交來自 `zeta987/cmake_helpers`。 |
| `D:\TBuild\tdesktop\out` | 已 configure 的 Visual Studio multi-config 建置樹。 |
| `D:\TBuild\Libraries\win64` | Windows x64 依賴；目前 target Qt 是 6.11.1。 |
| `D:\TBuild\ThirdParty` | prepare 使用的 Python、NuGet、MSYS2、jom 與 cache keys。 |
| `D:\TBuild\QtHostTools` | prepare 留下的 Qt host-tool 資料；版本可能與 target Qt 不同，不可當成 target Qt 版本來源。 |
| `D:\TBuild\release` | 本機 Release 封裝輸出；不是 repo，現有資產不可由腳本自動覆蓋。 |
| `D:\TBuild\ayu-debug-smoke` | 以 `-workdir` 啟動 Debug EXE 的乾淨煙霧測試資料夾。 |
| `D:\TBuild\ayu-verify` | anti-recall 資料庫升級 fixture 與檢查腳本。 |
| `D:\TBuild\.agents`, `.claude`, `.codex` | 本機代理設定與暫存狀態，不屬於 `tdesktop` commit。 |

`D:\TBuild` 根目錄的重要歷史文件如下：

| 文件 | 用途 |
| --- | --- |
| `260713_AyuGram_Rich_Messages_Vibe_Agent_Prompt.md` | Rich Messages 初始任務與目標。 |
| `260713_AyuGram_Rich_Messages_Migration_Report_zh-TW.md` | 上游遷移研究與差異盤點。 |
| `260713_AyuGram_Rich_Messages_Review_Findings_zh-TW.md` | 第一輪程式碼審查發現。 |
| `260713_AyuGram_Followup_Agent_Prompt_zh-TW.md` | submodule fork、版本與後續驗證的歷史接手稿；部分狀態已過時。 |
| `BUILD_GUIDE.md` | 舊建置環境紀錄；Visual Studio、Qt 與禁止 Release 等內容可能已過時。 |

repo 內與 Rich Messages 直接相關的持續性文件是
`docs/rich-messages-ayugram-conflicts.md` 與
`docs/rich-messages-ayugram-test-matrix.md`。

## 分支角色

- `dev` 是預設整合與發布來源分支。上游同步、自訂功能與已確認修正最後
  都進入 `dev`。
- `feat/*` 與 `fix/*` 用於隔離修改與產出 Debug 測試版。不要直接在 tag
  上開發。
- `feat/rich-messages-upstream-ed73b49` 保留 Telegram 6.9.4 Rich Messages
  遷移與後續修正的可追蹤歷史。
- `feat/ayu-settings-zh-tw` 保留 AyuGram 設定與實驗性設定翻譯的可重用歷史。
- `fix/egyptian-hieroglyph-fallback` 保留 Qt 6 字型 fallback 修正；該修正已
  fast-forward 到 `dev`。
- 未經使用者明確指示，不刪除上述保留分支。

## 已完成工作

| 項目 | 目前狀態與主要證據 |
| --- | --- |
| Telegram 6.9.4 Rich Messages | `feat/rich-messages-upstream-ed73b49` 已完成上游整合、AyuGram 衝突處理與兩輪修正，並由 `b303930a81b1` 整合進 `dev`。 |
| anti-recall 資料庫升級 | 已修正升級時清空舊資料的問題；`D:\TBuild\ayu-verify` 保留 v1 fixture 與檢查工具。 |
| Rich Messages 轉傳 fallback | 截斷後綴已改用 `ayu_ForwardTruncatedSuffix` 語言 key；rich-page 文字與 fallback 路徑位於 `Telegram/SourceFiles/ayu/features/forward`。 |
| IV/Markdown 預覽視窗 | `Iv::Markdown::Controller::createWindow()` 在 `show()` 後呼叫 `setNativeFrame(false)`，避免 Windows 原生標題列在 resize 前殘留。 |
| AyuGram 設定台灣正體中文 | `051471fb1bcc` 將翻譯分支整合到 `dev`，包含 AyuGram 設定與「實驗性設定」。 |
| 中文顯示條件 | `19c981c3fd6b` 改為尊重應用程式目前使用的語言；中文 App 語言顯示台灣正體中文，非中文 App 語言維持 AyuGram 原文。不可只依作業系統語言強制翻譯。 |
| 版本同步 | 程式版本固定為 6.9.4：`AppVersion=6009004`、`BetaChannel=1`、`AppVersionOriginal=6.9.4.beta`，Windows EXE FileVersion/ProductVersion 是 `6.9.4.0`。 |
| Windows 建置相容修正 | 已包含 NASM/libvpx、VS runner、C++/WinRT SDK 與 versionless Updater 驗證等修正。 |
| 本機 Release 封裝 | `7edcd683950c` 整合本機發布程序；後續 `18ad92955e74` 與 `34b714897937` 修正封裝器輸出處理。 |
| `v6.9.4-beta.7` | 已由 commit `34b714897937` 本機建置並發布；是一般 Release、不是 prerelease，而且是 Latest。 |
| 埃及聖書體使用者名稱 | `Telegram/lib_ui` 的簽署提交 `3775d69f32b1` 為 Qt 6 加入 Egyptian Hieroglyphs script fallback；superproject 簽署提交 `d0181d6d545a` 已進入本機 `dev`，Debug 實測通過。 |

`v6.9.4-beta.7` 的 ZIP 僅包含：

```text
AyuGram.exe
Updater.exe
modules/x64/d3d/d3dcompiler_47.dll
```

該版本另附 ZIP 的 `.sha256` 與 `BUILD-INFO.txt`。兩個 EXE 均未使用
Authenticode 憑證簽署，因此 SHA-256 與簽署 Git tag 是必要驗證資料。

2026-07-14 本階段的接手狀態：字型 fallback Debug EXE 的 SHA-256 是
`CE462E4F9701A6CCFDD10F3042C7E6D72511E032EA1EA5636CB5388E54C9DB42`，
FileVersion 與 ProductVersion 都是 `6.9.4.0`。`lib_ui` 提交
`3775d69f32b1` 與 superproject 的 `dev` 新提交都尚未推送，沒有建立新 tag
或 GitHub Release。使用者已要求先完成本機文件與 commit，發布階段保持暫停；
未來收到明確指示後，必須先推送 `lib_ui` feature branch 到
`zeta987/lib_ui`，再推送 `origin/dev`，才能繼續 Release 程序。

## 正常開發循環

1. 從最新 `dev` 建立 `feat/<name>` 或 `fix/<name>`。
2. 如果修改 submodule，在對應 `zeta987` fork 建立同名或可追蹤的 feature
   branch；先在 submodule 提交，再更新 superproject gitlink。
3. 完成一個可測試段落後，先執行必要的單元或靜態檢查。
4. 產出 Debug EXE 給開發者實機確認。這時 feature branch 可以保留尚未
   提交的試驗修改，但必須清楚記錄差異。
5. 開發者確認後，建立簽署 commit，執行 `git verify-commit HEAD`，再整合
   到 `dev`。
6. 推送 submodule feature branch 到使用者 fork，確認 superproject 記錄的
   每個自訂 gitlink 都能從對應 fork取得。
7. 推送已確認的 `dev` 到 `origin`。其他遠端保持唯讀。
8. 只有進入發布階段時，才從同一個乾淨且已提交的 `dev` HEAD 本機建置
   Release。
9. Release 實測通過後建立簽署 annotated tag，執行本機封裝器，推送 tag，
   再用 `gh release create` 上傳一般 Release。

不要在開發者尚未確認 Debug 版本時提前建置 Release、建立 tag 或發布資產。

## Git 操作前置檢查

每輪 Git 操作前先檢查 `.git/index.lock`。存在時先停止 Git 指令，確認沒有
其他 Git 程序正在使用 repo；鎖檔已無程序持有時才可刪除。確認結果必須是
`False`，再繼續 Git 操作。

在 superproject 與有修改的 submodule 分別執行：

```powershell
Test-Path -LiteralPath .git\index.lock
git status --short --branch
git diff --check
```

submodule 的實際 gitdir 通常位於 superproject 的 `.git\modules`，因此
`.git` 可能是一個指向 gitdir 的文字檔。需要移除鎖檔時先用
`git rev-parse --git-dir` 確認實際位置。

## 本機建置環境

2026-07-14 核對的既有 `out\CMakeCache.txt` 是：

| 設定 | 值 |
| --- | --- |
| Generator | Visual Studio 18 2026 |
| Platform | x64 |
| Qt | 6.11.1 |
| Auto-update | ON |
| Crash reports | ON |
| LTO | OFF |

這是 Visual Studio multi-config build tree，所以 Debug 與 Release 共用同一個
`out`。不要因為只修改 `.cpp` 就重新 configure 或刪除 `out`；既有中介檔
能讓下一次建置保持增量。

如果真的需要重新 configure，API ID 與 API hash 必須由環境變數或本機私密
設定傳入，不可把 credential-like 值寫進 Markdown、commit 或終端紀錄。
Release 配置必須保持 `DESKTOP_APP_DISABLE_AUTOUPDATE=OFF` 與
`DESKTOP_APP_DISABLE_CRASH_REPORTS=OFF`。重現 `v6.9.4-beta.7` 時使用
`DESKTOP_APP_ENABLE_LTO=OFF`；GitHub Actions 的 LTO 設定不同，不可混稱為
相同產物。

## Debug 建置與開發者確認

只有在使用者要求建置，或工作已到達 Debug 驗證階段時才執行：

```powershell
cmake --build D:\TBuild\tdesktop\out `
  --config Debug `
  --target Telegram `
  --parallel 4
```

產物是：

```text
D:\TBuild\tdesktop\out\Debug\AyuGram.exe
D:\TBuild\tdesktop\out\Debug\Updater.exe
```

第一次缺少 Debug 中介檔時可能重新編譯大量檔案；同一 build tree 後續修改
單一 `.cpp` 通常可以增量建置。Visual Studio 18 對 `/DEBUG:FASTLINK` 顯示
`LNK4315` 是已知警告；只有 exit code 0 且最後顯示 `Telegram.vcxproj ->
...\AyuGram.exe` 才算成功。

如果出現 PDB、EXE、LNK1104、access denied 或 file in use 錯誤，停止重試並
請開發者關閉正在執行的 AyuGram 或 debugger。不可刪除被占用的產物繞過。

交付 Debug EXE 時至少記錄：

- 建置 commit 或未提交 diff。
- `AyuGram.exe` 的 FileVersion、ProductVersion 與 SHA-256。
- 實際測試步驟與開發者回覆。
- 是否為完整建置或增量建置。

## Release 建置

只有在 Debug 版已由開發者確認、所有程式碼已簽署提交到 `dev`、所有自訂
submodule commit 已推到使用者 fork，而且工作目錄乾淨時執行：

```powershell
cmake --build D:\TBuild\tdesktop\out `
  --config Release `
  --target Telegram `
  --parallel 4
```

主要產物位於：

```text
D:\TBuild\tdesktop\out\Release\AyuGram.exe
D:\TBuild\tdesktop\out\Release\Updater.exe
D:\TBuild\tdesktop\out\Release\modules\x64\d3d\d3dcompiler_47.dll
```

目前 6.9.4 系列的 `AyuGram.exe` FileVersion 與 ProductVersion 都必須是
`6.9.4.0`。`beta.N` 只由簽署 tag、Release 名稱與資產名稱識別，不應把
應用程式版本任意改成其他 Telegram 上游版本。

Release 建置完成後，先用乾淨 `-workdir` 執行煙霧測試，再檢查版本、檔案
大小、SHA-256、Updater 與 modules。不要把 PDB 放進發布 ZIP。

## 版本與簽署 tag

允許的 tag 格式只有：

```text
vX.Y.Z
vX.Y.Z-beta.N
```

建立 tag 前核對以下版本來源：

- `Telegram/build/version`
- `Telegram/SourceFiles/core/version.h`
- `Telegram/Resources/winrc/Telegram.rc`
- `Telegram/Resources/winrc/Updater.rc`
- `Telegram/build/setup.iss`
- `AyuGram.exe` 的 FileVersion 與 ProductVersion

本機封裝器目前只直接檢查 `Telegram/build/version` 與 EXE VersionInfo，
所以其餘版本檔仍需人工或額外腳本核對。

tag 必須是簽署 annotated tag、指向目前乾淨的 `dev` HEAD，而且 HEAD 必須
已存在於 `origin/dev`。下列是 6.9.4 下一個 beta 的範例；每次執行 Git 前
仍須先做鎖檔檢查：

```powershell
git verify-commit HEAD
git fetch origin dev --no-tags
git merge-base --is-ancestor HEAD origin/dev
git tag -s v6.9.4-beta.8 -m "AyuGram v6.9.4-beta.8"
git cat-file -t v6.9.4-beta.8
git verify-tag v6.9.4-beta.8
git rev-list -n 1 v6.9.4-beta.8
git rev-parse HEAD
```

`git cat-file` 必須輸出 `tag`，最後兩個 SHA 必須完全一致。tag 建立後不可
移動；需要更換二進位檔或內容時使用下一個 beta 序號。

## 本機封裝

正式封裝入口是：

```text
Telegram/build/package_windows_release.ps1
```

執行範例：

```powershell
pwsh -NoProfile -File `
  D:\TBuild\tdesktop\Telegram\build\package_windows_release.ps1 `
  -Tag v6.9.4-beta.8
```

腳本預設輸出至 `D:\TBuild\release`，並驗證：

- repo 乾淨。
- HEAD commit 與 annotated tag 的簽章有效。
- tag 指向 HEAD。
- tag 版本與 `Telegram/build/version` 一致。
- `AyuGram.exe` 的 FileVersion/ProductVersion 一致。
- `Updater.exe` 存在且非空。
- D3D 模組版本與固定 SHA-256 一致。
- ZIP 小於 2 GiB，且只含三個預期檔案。

腳本會產生：

```text
AyuGram-Windows-x64-v6.9.4-beta.8.zip
AyuGram-Windows-x64-v6.9.4-beta.8.zip.sha256
AyuGram-Windows-x64-v6.9.4-beta.8-BUILD-INFO.txt
```

封裝器會拒絕覆蓋同名資產。不要自動刪除既有檔案，也不要在 GitHub 使用
`--clobber`；先由人工確認既有資產的來源，必要時改用下一個 tag。

## 推送與建立 GitHub Release

先確認 `gh auth status` 登入 `zeta987`。自訂 submodule branch 必須先推送到
對應 `zeta987` fork，再推 `origin/dev`。禁止對 `upstream` 或 `telegram`
執行 push。

Release binary 驗證、簽署 tag 與封裝完成後，只推送該 tag 到 `origin`：

```powershell
git push origin refs/tags/v6.9.4-beta.8
```

為了符合目前政策，帶 `-beta.N` 的版本也建立「一般 Release」，不要加入
`--prerelease`。`v6.9.4-beta.7` 同時是一般 Release 與 Latest，因此後續
同型發布使用 `--latest`。先準備 UTF-8 release notes，再把下列多行內容存成
暫存 `.ps1`，以 `pwsh -File` 執行：

```powershell
gh release create v6.9.4-beta.8 `
  D:\TBuild\release\AyuGram-Windows-x64-v6.9.4-beta.8.zip `
  D:\TBuild\release\AyuGram-Windows-x64-v6.9.4-beta.8.zip.sha256 `
  D:\TBuild\release\AyuGram-Windows-x64-v6.9.4-beta.8-BUILD-INFO.txt `
  --repo zeta987/AyuGramDesktop `
  --verify-tag `
  --title "AyuGram v6.9.4-beta.8" `
  --notes-file D:\TBuild\release\v6.9.4-beta.8-notes.md `
  --latest
```

發布說明至少列出功能變更、已知限制、建置 commit、6.9.4.0 版本資訊、ZIP
內容、未使用 Authenticode，以及應以 SHA-256 驗證資產。

發布後執行：

```powershell
gh release view v6.9.4-beta.8 `
  --repo zeta987/AyuGramDesktop `
  --json tagName,name,isDraft,isPrerelease,targetCommitish,assets,url
gh api repos/zeta987/AyuGramDesktop/releases/latest --jq .tag_name
```

確認 `isDraft=false`、`isPrerelease=false`、Latest tag 正確，而且三個遠端資產
digest 與本機 SHA-256 完全一致。

## GitHub Actions 的角色

`.github/workflows/windows-release.yml` 目前只有 `workflow_dispatch`，沒有 tag
push 自動觸發。它是耗時的手動備援，不是正常發布入口。

- 從 `dev` dispatch 時，`GITHUB_REF_NAME=dev` 不符合 tag 格式，驗證會失敗。
- 在 Actions 介面必須選擇簽署 tag 才可能通過前段驗證。
- 現有 workflow 會把 beta tag 發成 prerelease，與目前「beta 也發一般
  Release」政策衝突。
- Actions 使用的編譯器與 LTO 設定也和 `v6.9.4-beta.7` 本機產物不同。

因此未經使用者明確要求與重新核對 workflow，不要執行這個 Action。正常
Release 由本機建置、封裝，再以 `gh release create` 上傳。

## 未來 TODO

### 訊息截圖彈窗自適應

狀態：已確認問題，尚未修改程式碼。

主要位置是
`Telegram/SourceFiles/ayu/ui/boxes/message_shot_box.cpp`。目前約在
`boxWidth` 與 `setDimensionsToContent()` 附近，寬度直接來自預覽圖片寬度加
padding，內容高度也在一次性尺寸計算後固定。主視窗或可用螢幕區域較窄時，
底部「儲存」「複製」等操作按鈕可能落在可視區域外。

後續修正必須：

- 將彈窗寬高限制在目前視窗或可用螢幕區域內。
- 預覽與設定內容可以捲動，但主要操作按鈕始終可見且可按。
- 尺寸、padding 與 breakpoint 使用 `.style` 值，避免直接硬編碼未縮放像素。
- 保留大型預覽在寬視窗中的現有體驗。
- 驗證窄視窗、長訊息、短訊息、中文與英文 UI、100%/125%/150%/200%
  縮放，以及所有操作按鈕都能鍵盤與滑鼠操作。
- 先產出 Debug EXE 給開發者確認，再考慮進入 `dev` 與下一個 Release。

### 仍需重新驗證的歷史待辦

以下項目來自先前遷移審查，尚未納入本次修正；處理前先以目前 `dev` 重新
確認，不能直接假設舊描述仍完整成立：

- `AyuSync` 的非同步 rich-message callback 仍捕捉 session/item 相關指標，
  需要檢查 session teardown 期間的生命週期。
- `UseTelegramRichTranslation()` 與 `CreateTranslateProvider()` 對 Native
  provider 的最終 MTProto fallback 判斷需要共用同一個 resolver。
- `Telegram/SourceFiles/ayu/data/messages_storage.cpp` 的 anti-recall 媒體 mapping
  仍有 `todo`，媒體資料保存範圍需要單獨設計與測試。
- Message Shot 對 Rich Messages 的 Channel block 頭像預載仍需測試矩陣確認。
- `cmake/external/zlib/CMakeLists.txt` 的 `ZLIB_WINAPI` 對未來 Win32/x86 建置
  仍需獨立驗證；目前發布目標只有 Windows x64。
- `ayu_PeekOnlineSuccess` 與 `ayu_PeekOnlineMenuText` 仍存在於語言檔，移除前
  需再次確認沒有動態或產生式使用點。

## 階段收尾檢查

每個功能段落完成時，至少記錄：

- feature/fix branch 與 commit SHA。
- submodule commit、fork branch 與遠端可取得性。
- 執行過的測試、Debug EXE 版本與 SHA-256。
- 開發者實測結果。
- 是否已整合到 `dev`。
- 是否需要 Release；如果不需要，明確保持 tag 與 GitHub Release 不變。
- 未完成 TODO 與下一個可操作的程式位置。

只有所有發布檢查都完成時，才建立新 tag 與 GitHub Release。
