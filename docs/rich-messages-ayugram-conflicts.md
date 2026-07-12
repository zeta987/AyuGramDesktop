# AyuGram Rich Messages 合併衝突紀錄

## 固定快照

- AyuGram 基底：`ba8c1a6b0f456c6a5601d675e765cd5fba10d54c`
- Telegram Desktop 上游：`ed73b49d0110a8e949e7fe06a8a90bd7e7b421a8`
- 共同祖先：`6e90f6876e2a2daf04d97ea97345cc9ad8ade378`
- Superproject merge commit：`f5050abf6dea0804a5bb084a52b435b81356a3bd`
- Anti-recall adaptation：`862ccc4d4f374de1605e1b85dea0ab68ba15a888`
- Translator、filters 與 forward adaptation：`438268aa15041a9ef3c6e90e4d21adf5b094ac86`
- Message Shot media adaptation：`857b080078734c41dd47894a2667c9bfecf2818c`
- 本機分支：`feat/rich-messages-upstream-ed73b49`
- 基底備份標籤：`backup/ayu-pre-rich-ba8c1a6`

合併保留 Telegram Desktop 上游歷史與完整 Rich Message protocol、native renderer、partial/full fetch、streaming draft、editor、selection、media、translation 與 export 管線。AyuGram 的品牌、設定、anti-recall、filters、translator、Message Shot、AyuForward 與 custom bubble 行為留在同一棵歷史中。

## Submodule 合併

三個 Ayu fork submodule 以各自的本機 `feat/rich-messages-ed73b49` 分支保留 Ayu 與上游父系。

| Submodule | Ayu parent | Telegram parent | Merge commit | 處理 |
|---|---|---|---|---|
| `Telegram/codegen` | `1c919609` | `63ec3de7` | `ae7386e7` | 自動合併，保留 Ayu wide-message style codegen 與新版 child-field access |
| `Telegram/lib_tl` | `1bd29399` | `aa779132` | `f6c56694` | 自動合併，提供目標 TL layer 所需 generator/runtime API |
| `Telegram/lib_ui` | `864dd38f` | `e1f50111` | `50a668d9` | 保留 Ayu slide easing，加入新版 finished callback 與 Rich renderer 所需 UI API |

三個 submodule merge commits 與 superproject merge commit 均使用本機 SSH signing key 建立，並以 `git verify-commit` 驗證。

三個 Ayu adaptation commits 亦使用同一把本機 SSH signing key 建立並完成 `git verify-commit`；forward adaptation 的 API failure path 不會把 protected-forward 來源內容建立成目標對話 cloud draft。

## Repository 與品牌

| 檔案 | 保留的 Ayu 行為 | 接入的上游行為 |
|---|---|---|
| `.gitmodules` | `codegen`、`lib_tl`、`lib_ui`、`lib_icu` 使用 Ayu fork URL | 新增 `cmark-gfm`、`MicroTeX`、`TooManyCooks` |
| `AGENTS.md` | 避免自行 build、PR 需標 AI generated 的既有規則 | 新版開發與序列化注意事項；本專案為自用，不建立 PR |
| `README.md` | Ayu 品牌、下載方式、credits | 沒有引入 Telegram 專案 badges 或品牌文字 |
| `Telegram/CMakeLists.txt` | 全部 `ayu/**` sources、Ayu output name、bundle id、icons、ICU 設定 | Rich Message sources、editor、styles、MicroTeX 與新資源 |
| `Telegram/SourceFiles/core/version.h` | Ayu AppId、AppName、AppFile 與 `6.7.8` 專案版本 | 新版 core API；版本號仍交由 Ayu 維護者決定 |
| `Telegram/Resources/winrc/Telegram.rc` | Ayu company、product、file description 與版本資源 | 新版 resource layout |
| `Telegram/Resources/winrc/Updater.rc` | Ayu updater 品牌與版本資源 | 新版 updater resource layout |

Ayu 原本刪除的 `.github/workflows/*.yml` 與 `snap/snapcraft.yaml` 維持刪除，避免帶入使用 Telegram Desktop 發佈憑證、名稱與 CI 假設的工作檔。

## API、data 與 HistoryItem

| 檔案 | 手工保留與整合內容 |
|---|---|
| `Telegram/SourceFiles/api/api_polls.cpp` | Ayu poll 行為與新版 request signatures |
| `Telegram/SourceFiles/apiwrap.cpp` | Ayu pseudo-reply、AyuForward 權限例外、SendDice 分流與 upstream Rich send/draft/edit/ephemeral 路徑 |
| `Telegram/SourceFiles/data/data_channel.h` | Ayu no-forward state 與新版 community flags |
| `Telegram/SourceFiles/data/data_document_resolver.cpp` | Ayu document resolve hooks 與新版 resolver API |
| `Telegram/SourceFiles/data/data_session.cpp` | Ayu deleted/edited snapshot、TTL delete hooks 與 upstream batched destroy notifications；rich-only edit 不被空文字判斷略過 |
| `Telegram/SourceFiles/data/data_types.h` | Ayu protected-forward state與 upstream guest-chat/ephemeral flags |
| `Telegram/SourceFiles/history/history_item.cpp` | Ayu TTL、deleted state、filterZalgo 與 upstream RichPage receive/edit/echo/full-page lifecycle |
| `Telegram/SourceFiles/history/history_item_text.cpp` | Ayu summaryEntry copy precedence 與 upstream rich/plain clipboard fallback |

`Telegram/SourceFiles/mtproto/scheme/api.tl` 使用目標上游 layer 的完整 schema，沒有手工混入舊 layer constructors。

## History view 與互動

| 檔案群組 | 手工保留與整合內容 |
|---|---|
| `history/history_inner_widget.*` | Ayu filter action、force-click、scroll-date 行為與 upstream album-part editing、day-crossing timeout、Rich selection |
| `history/history_widget.cpp` | Ayu gift、TTL、attach、commands、emoji visibility 與 upstream hide-extra state、Rich editor/draft preview |
| `history/view/history_view_bottom_info.*` | Ayu deleted/burnt/zalgo/time state 與 upstream Silent、EditedPrimary、Ephemeral flags，使用互不重疊 bits |
| `history/view/history_view_element.cpp` | Ayu deleted-opacity animation、Message Shot state 與 upstream EphemeralBadge、Rich element geometry |
| `history/view/history_view_message.cpp` | Ayu bubble geometry與 upstream Rich Message measure/paint/hit-test |
| `history/view/controls/history_view_compose_controls.cpp` | Ayu compose buttons與 upstream Rich draft controls |
| `history/view/history_view_translate_tracker.h` | 可重建的 Ayu provider 與 upstream Rich translation sender |
| `history/view/history_view_view_button.cpp` | 採用新版單次 paint path，避免舊 ripple/rounded rect 重複繪製 |
| `history/view/media/history_view_document.cpp` | Ayu Message Shot/transcribe 行為與 upstream effective-media TTL |
| `history/view/media/history_view_gif.cpp` | Message Shot 隱藏 transcribe 與 upstream effective-media TTL |

## 其他 UI 衝突

以下檔案逐段保留 Ayu 功能並改接新版 API，未採整檔 ours 或 theirs。

- `boxes/delete_messages_box.cpp`、`boxes/language_box.cpp`
- `chat_helpers/gifs_list_widget.cpp`、`chat_helpers/stickers_list_widget.cpp`
- `core/click_handler_types.cpp`
- `dialogs/dialogs_inner_widget.cpp`、`dialogs/dialogs_row.cpp`、`dialogs/dialogs_widget.cpp`
- `info/profile/info_profile_actions.cpp`、`info/profile/info_profile_inner_widget.cpp`
- `settings/sections/settings_information.cpp`、`settings/sections/settings_main.cpp`、`settings/settings_experimental.cpp`
- `ui/chat/attach/attach_bot_webview.cpp`、`ui/chat/chat_style.h`
- `ui/controls/compose_ai_button_factory.cpp`、`ui/unread_badge.cpp`、`ui/userpic_view.cpp`
- `window/main_window.cpp`、`window/themes/window_themes_cloud_list.cpp`
- `window/window_controller.cpp`、`window/window_filters_menu.cpp`、`window/window_peer_menu.cpp`

這些解析同時保留 Ayu GIF confirmation、sticker filter、link-warning 設定、plugin info、ID/registration date、AI button、custom badges、Message Shot theme、folder visibility、AyuForward callback，以及 upstream IV Markdown、communities、bot verification、ephemeral 與 normalized forward options。

## 未執行的 generated outputs 與 CI

Repository 規則要求避免自行 build，因此本次不執行完整 `Telegram` target，也不把舊的 `out/Release/AyuGram.exe` 當成驗證結果。仍需由明確允許的建置環境執行 TL/style/lang codegen、啟用 `DESKTOP_APP_TEST_APPS` 的測試 targets、Windows Release build 與 `AyuGram.exe` smoke test。
