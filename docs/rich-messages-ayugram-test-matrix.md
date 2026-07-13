# AyuGram Rich Messages 驗收矩陣

## 驗證狀態

| 狀態 | 意義 |
|---|---|
| `PASS` | 已由本次本機靜態或 Git 檢查驗證 |
| `PENDING-RUNTIME` | 需要允許 build 後的本機程式或測試 target |
| `PENDING-SERVER` | 需要可發送 Bot API 10.1 Rich Message 的測試 bot/account |
| `PENDING-CI` | 需要專案既有 generated-output 或跨平台 CI |

本次依 `AGENTS.md` 不執行完整 build。任何 runtime case 在實際執行前都保持 pending，不以原有 `out/` 產物代替。

## Protocol 與 serialization

| Case | 期待結果 | 狀態 |
|---|---|---|
| 完整目標 `api.tl` | `Message.rich_message`、`RichMessage`、`InputRichMessage`、send/edit/save-draft/fetch constructors 同層 | `PASS` source inspection |
| TL generated outputs | codegen 能產生所有 `MTPRichMessage` types，無 unknown constructor | `PENDING-CI` |
| raw receive round-trip | received `MTPRichMessage` serialization/deserialization 保留 blocks、media 與 flags | `PENDING-RUNTIME` |
| malformed block | bubble 保留非空 summary，history 不 crash | `PENDING-RUNTIME` |
| limits | 32768 UTF-8 chars、500 blocks、16 nesting、50 media、20 table columns | `PENDING-RUNTIME` |
| future unknown constructor | message list不 crash並採 fallback | `PENDING-SERVER` |

## Receive、cache 與 lifecycle

| Case | 期待結果 | 狀態 |
|---|---|---|
| `Message.message == ""` | Rich Message bubble、preview、copy、search 不被視為空白 | `PENDING-SERVER` |
| cold receive | native `Iv::RichPage` renderer 顯示 | `PENDING-SERVER` |
| restart/cache restore | 重啟後內容、media、details state 與 summary 一致 | `PENDING-RUNTIME` |
| plain → rich edit | 保存舊 plain revision，建立 Rich component | `PENDING-SERVER` |
| rich → rich edit | 保存舊 raw blob，更新 layout/cache | `PENDING-SERVER` |
| rich → plain edit | 保存 rich revision並移除目前 Rich component | `PENDING-SERVER` |
| local rich send → server echo | local RichPage 不被 echo 降成空文字 | `PENDING-SERVER` |
| partial → full | Show more 僅一個 request，失敗保留 partial，成功原地更新 | `PENDING-SERVER` |
| streaming draft → final | 同 draft id 原地更新，final message 取代 preview | `PENDING-SERVER` |
| 30 秒 ephemeral draft | 不寫入 Ayu anti-recall | `PENDING-SERVER` |
| 同 sender/topic 的一般訊息 | streamed draft identity 只排除 draft item，不略過一般訊息 | `PASS` source inspection |
| streamed draft → final adoption | adoption 更新期間不建立 edited snapshot | `PASS` source inspection |
| non-premium receive | 任意帳號可 parse/render，只有 authoring UI 受 capability gate | `PENDING-SERVER` |

## Rich block renderer

| 群組 | Cases | 狀態 |
|---|---|---|
| Text | heading 1..6、paragraph、footer、bold、italic、underline、strike、spoiler、marked、code | `PENDING-RUNTIME` |
| Baseline | subscript、superscript、mixed CJK/emoji/RTL | `PENDING-RUNTIME` |
| Math/code | inline LaTeX、block LaTeX、preformatted code、very long word | `PENDING-RUNTIME` |
| Structure | divider、bullet/ordered/task list、nested blockquote、pull quote | `PENDING-RUNTIME` |
| Table | align、rowspan、colspan、bordered、striped、caption、20 columns、wide overflow | `PENDING-RUNTIME` |
| Interactive | details open/closed、local anchor、anchor in collapsed details、references、footnotes | `PENDING-RUNTIME` |
| Special | map、zero-size map、thinking block、partial Show more | `PENDING-RUNTIME` |
| Selection | collapsed details 不產生隱形 selection，copy 只含可見內容 | `PENDING-RUNTIME` |
| Links | `#anchor` 留在 message-local navigation，external URL 維持確認 | `PENDING-RUNTIME` |

## Media lifecycle

| Case | 期待結果 | 狀態 |
|---|---|---|
| photo/video/animation | render、download、open、save、copy、file-reference refresh | `PENDING-RUNTIME` |
| collage/slideshow | group navigation、gallery index 與 active item 正確 | `PENDING-RUNTIME` |
| audio/voice | playback、playlist與 file refresh 正確，不進 visual gallery | `PENDING-RUNTIME` |
| media-only partial page | ordinary text 空白仍可顯示與 Show more | `PENDING-SERVER` |
| delete cleanup | Rich media release/cache cleanup 不遺漏 | `PENDING-RUNTIME` |
| Message Shot preload | Rich photos/documents（含 EmbedPost 作者頭像）ready 後才 final render | `PENDING-RUNTIME` |

## Ayu anti-recall

| Case | 期待結果 | 狀態 |
|---|---|---|
| schema v1 → v2 | 新 rich 欄位改 nullable，升級走 `ALTER TABLE ADD COLUMN` 保留既有 DeletedMessage/EditedMessage rows（SQL 層 ALTER 保留已以 v1 測試 db 驗證） | `PENDING-RUNTIME` |
| deleted rich | row 同時保存 raw blob、visible summary，可重建 native bubble | `PENDING-SERVER` |
| edited revisions | 每版 raw blob/summary 獨立保存 | `PENDING-SERVER` |
| ordinary text empty | rich blob 存在時寫入 row；media-only（無文字、無 rich blob）不再寫入空 row，media 序列化仍為 todo | `PENDING-RUNTIME` |
| deleted-history search | searchable summary 可命中 visible text | `PENDING-RUNTIME` |
| malformed stored blob | 使用 summary fallback，不 crash archive | `PENDING-RUNTIME` |
| archived partial page | 顯示已保存 blocks 與 summary，不以 fake/歷史 id 請求 full page | `PASS` source inspection |

## Translation、filters 與 forward

| Case | 期待結果 | 狀態 |
|---|---|---|
| Telegram translation | upstream Rich translation 保留 block structure | `PENDING-SERVER` |
| Google/Yandex translation | 使用 `FlattenRichPageToSimpleText`，不覆寫 original RichPage | `PENDING-RUNTIME` |
| translation cache edit | source hash/version 改變後不重用舊結果 | `PENDING-RUNTIME` |
| translation provider switch | cache key 含 provider，延遲回傳不會污染另一 provider 的命中 | `PASS` source inspection |
| RegexFilter | 比對 visible summary，不讀 serialized TL bytes | `PENDING-RUNTIME` |
| dialog/notification fallback | rich-only message 顯示非空 summary | `PENDING-RUNTIME` |
| normal forward | upstream path保留 Rich structure | `PENDING-SERVER` |
| Ayu protected forward | 主執行緒送出完整 RichPage；partial page 先 `resolveRichMessage` 取全文，可送 rich 時送 rich、只能 plain 時（含 non-premium、NoNamesAndCaptions）flatten 全文，皆不靜默截斷；取回失敗才 flatten 現有 full page 或附加 `[message truncated]` 標記 | `PENDING-SERVER` |
| protected forward file reference | refresh 使用來源 message origin，失敗 fallback 不建立 cloud draft | `PASS` source inspection |
| copy/export | native rich/plain fallback 不為空且不遺失可見內容 | `PENDING-RUNTIME` |

## Message Shot 與 Ayu UI

| Case | 期待結果 | 狀態 |
|---|---|---|
| table/math/details | 使用同一個 `HistoryView::Element` renderer | `PENDING-RUNTIME` |
| spoiler | 遵守 Message Shot reveal 設定 | `PENDING-RUNTIME` |
| collage/slideshow | 所有需要的 Rich media 已 preload | `PENDING-RUNTIME` |
| audio cover | 只預載 Rich audio thumbnail，不下載音訊本體 | `PASS` source inspection |
| partial/full | 截斷與完整頁面的 dynamic height 正確 | `PENDING-RUNTIME` |
| 2x/3x DPR | 沒有模糊、裁切或錯誤 pixel ratio | `PENDING-RUNTIME` |
| custom bubble rounding/tails | full-width media、code、table 不超出 Ayu bubble | `PENDING-RUNTIME` |
| deleted opacity/badges | Ayu deleted/burnt 與 upstream ephemeral state 不共用 bit | `PASS` source inspection |

## 允許 build 後的執行順序

1. 執行目標 layer 的 TL generator 與 style/lang/resource generators。
2. 啟用 `DESKTOP_APP_TEST_APPS`，執行 Rich parsing/serialization/migration/translation/media collector tests。
3. 執行 `test_text` 與 Rich native visual harness。
4. 執行 Windows Release `Telegram` target，確認輸出仍為 `AyuGram.exe`。
5. 使用 Bot API 10.1 fixture bot 執行 protocol、partial、streaming、edit、media 與 non-premium receive cases。
6. 以 2x/3x DPR 比對 Message Shot 與 chat bubble screenshots。
