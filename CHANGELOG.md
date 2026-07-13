# Changelog / 更新日誌

本檔案記錄 `zeta987/AyuGramDesktop` fork 每個發布 tag 的詳細變更，固定以正體中文與 English 雙語呈現。
This file records the detailed changes of every release tag of the `zeta987/AyuGramDesktop` fork, always in Traditional Chinese and English.

## v6.9.4-beta.8 — 2026-07-14

### 正體中文

- 訊息截圖（Message Shot）彈窗現在會自適應主視窗寬度：彈窗寬度以視窗可用區域為上限，預覽圖按比例縮小（不放大、不拉伸、不裁切），「儲存」「複製」按鈕在最小視窗寬度下仍完整可見可按。
- 彈窗開啟期間縮放主視窗，彈窗寬度會即時跟隨調整；內容過高時沿用垂直捲動，底部按鈕列固定可見。
- 「儲存」與「複製」輸出維持原始解析度，不受預覽縮小影響。
- App 內「關於」、主選單與登入頁的版本文字改為顯示 beta 序號（例如 `6.9.4 beta.8 x64`）；Windows EXE 的 FileVersion／ProductVersion 維持 `6.9.4.0`。
- 為 Qt 6 加入埃及聖書體（Egyptian Hieroglyphs）字型 fallback，修正聖書體使用者名稱無法顯示的問題（`Telegram/lib_ui` 提交 `3775d69f32b1`）。
- Windows 建置文件改為可攜路徑模型，不再綁定特定磁碟路徑。

### English

- The Message Shot box now adapts to the main-window width: the box width is capped by the available window area, the preview scales down proportionally (never upscaled, stretched or cropped), and the Save / Copy buttons stay fully visible and clickable even at the minimum window width.
- Resizing the main window while the box is open adjusts the box width live; overly tall content keeps using vertical scrolling with the footer buttons always visible.
- Save and Copy still output the full-resolution image, unaffected by the preview downscaling.
- The in-app version text in About, the main menu and the intro screen now shows the beta serial (for example `6.9.4 beta.8 x64`); the Windows EXE FileVersion / ProductVersion stays `6.9.4.0`.
- Added a Qt 6 font fallback for Egyptian Hieroglyphs, fixing hieroglyph usernames that failed to render (`Telegram/lib_ui` commit `3775d69f32b1`).
- The Windows build documentation now uses a portable path model instead of a fixed drive layout.

## v6.9.4-beta.7 — 2026-07-14

### 正體中文

- 建立本機 Windows Release 封裝流程 `Telegram/build/package_windows_release.ps1`：驗證簽署 commit 與簽署 annotated tag、tag 與版本檔一致性、EXE 版本資訊、D3D 模組版本與 SHA-256、ZIP 內容清單，並產出 `.sha256` 與 `BUILD-INFO.txt`。
- GitHub Actions 的 Windows Release workflow 改為手動備援（`workflow_dispatch`），正常發布改由本機建置與封裝。
- 修正封裝器對 scalar Git 輸出的處理，並加入無版本資訊 Updater 的驗證。
- C++/WinRT 與 Windows SDK 版本對齊，修正建置相容性。
- AyuGram 設定的中文顯示改為跟隨 App 目前語言：中文 App 語言顯示中文，非中文維持原文。
- 本 fork 第一個公開發布的 GitHub Release。

### English

- Introduced the local Windows release packaging flow `Telegram/build/package_windows_release.ps1`: it validates the signed commit and signed annotated tag, tag/version-file consistency, EXE version info, the D3D module version and SHA-256, and the ZIP content list, and produces `.sha256` plus `BUILD-INFO.txt`.
- The GitHub Actions Windows release workflow became a manual fallback (`workflow_dispatch`); normal releases are built and packaged locally.
- Fixed scalar Git output handling in the packager and added validation for the versionless Updater binary.
- Aligned C++/WinRT with the Windows SDK to fix build compatibility.
- Chinese AyuGram settings now follow the active app language: Chinese app languages show Traditional Chinese, other languages keep the original strings.
- First published GitHub Release of this fork.

## v6.9.4-beta.6 — 2026-07-13

### 正體中文

- 修正 CI 中 NASM 執行檔路徑的處理：改用絕對路徑並正確加上引號，解決 libvpx 組譯失敗。

### English

- Fixed NASM executable path handling in CI: switched to an absolute, properly quoted path so the libvpx assembly step succeeds.

## v6.9.4-beta.5 — 2026-07-13

### 正體中文

- 將 libvpx 使用的 NASM 固定為 3.01 版，避免工具版本漂移造成的建置失敗。

### English

- Pinned NASM 3.01 for libvpx to avoid build breakage from tool-version drift.

## v6.9.4-beta.4 — 2026-07-13

### 正體中文

- libvpx 改用 NASM 組譯，修正 Windows Release 建置失敗。

### English

- Built libvpx with NASM, fixing the Windows release build failure.

## v6.9.4-beta.3 — 2026-07-13

### 正體中文

- CI 改用 VS 2022 runner 以取得 v143 工具組，修正 Windows 建置環境不相容。

### English

- Switched CI to the VS 2022 runner to obtain the v143 toolset, fixing the incompatible Windows build environment.

## v6.9.4-beta.2 — 2026-07-13

### 正體中文

- 將 6.9.4 各整合分支合併進 `dev`，並強化發布 CI 驗證：tag 版本中繼資料檢查、只從 tag 觸發的 dev 建置、release tag 格式驗證。

### English

- Merged the 6.9.4 integration branches into `dev` and hardened release CI validation: tag version metadata checks, tag-only dev builds, and release tag format validation.

## v6.9.4-beta.1 — 2026-07-13

### 正體中文

- 將上游 Telegram Desktop 6.9.4「Rich Messages」（富文本文章訊息，基於上游提交 [telegramdesktop/tdesktop@ed73b49d0110](https://github.com/telegramdesktop/tdesktop/commit/ed73b49d0110a8e949e7fe06a8a90bd7e7b421a8)）整合進 AyuGram，並完成 AyuGram 功能的相容調整：
  - anti-recall 資料庫保存 rich messages，並修正資料庫升級時清空舊資料的問題。
  - 訊息截圖（Message Shot）預載 rich 訊息媒體與頻道頭像。
  - rich 訊息轉傳前抓取完整頁面、轉傳截斷後綴在地化、rich 編輯以頁面內容等值去重。
- AyuGram 設定與介面字串完成中文化，內建字串為權威來源。
- Submodule（codegen、lib_tl、lib_ui、cmake）改指向 `zeta987` fork。
- 版本同步為 6.9.4 beta（`AppVersion=6009004`、`BetaChannel=1`），Windows EXE FileVersion／ProductVersion 為 `6.9.4.0`。
- 建立只由 tag 觸發的 Windows Release／Debug CI。

### English

- Merged upstream Telegram Desktop 6.9.4 "Rich Messages" (upstream commit [telegramdesktop/tdesktop@ed73b49d0110](https://github.com/telegramdesktop/tdesktop/commit/ed73b49d0110a8e949e7fe06a8a90bd7e7b421a8)) into AyuGram and adapted the AyuGram feature set:
  - Anti-recall now persists rich messages, with a fix that preserves existing data during the database upgrade.
  - Message Shot preloads rich-message media and channel avatars.
  - Rich forwards fetch the full page first, the forward truncation suffix is localized, and rich edits are deduplicated by page-content equality.
- AyuGram settings and UI strings are localized into Traditional Chinese, with the built-in strings as the authoritative source.
- Submodules (codegen, lib_tl, lib_ui, cmake) now point at the `zeta987` forks.
- Version synced to 6.9.4 beta (`AppVersion=6009004`, `BetaChannel=1`); the Windows EXE FileVersion / ProductVersion is `6.9.4.0`.
- Added tag-only Windows Release / Debug CI.
