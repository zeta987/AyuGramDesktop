/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "settings/settings_experimental.h"

#include "data/components/passkeys.h"
#include "main/main_session.h"
#include "ui/boxes/confirm_box.h"
#include "ui/search_field_controller.h"
#include "ui/text/text_entity.h"
#include "ui/toast/toast.h"
#include "ui/widgets/menu/menu_add_action_callback.h"
#include "ui/widgets/fields/input_field.h"
#include "ui/wrap/padding_wrap.h"
#include "ui/wrap/vertical_layout.h"
#include "ui/wrap/slide_wrap.h"
#include "ui/widgets/buttons.h"
#include "ui/widgets/labels.h"
#include "ui/widgets/popup_menu.h"
#include "ui/vertical_list.h"
#include "ui/gl/gl_detection.h"
#include "ui/chat/chat_style_radius.h"
#include "ui/controls/compose_ai_button_factory.h"
#include "base/options.h"
#include "boxes/moderate_messages_box.h"
#include "core/application.h"
#include "core/launcher.h"
#include "core/sandbox.h"
#include "chat_helpers/tabbed_panel.h"
#include "dialogs/dialogs_entry.h"
#include "dialogs/dialogs_widget.h"
#include "dialogs/ui/dialogs_layout.h"
#include "ffmpeg/ffmpeg_utility.h"
#include "history/history_item_components.h"
#include "history/view/controls/compose_controls_common.h"
#include "history/view/history_view_message.h"
#include "info/profile/info_profile_actions.h"
#include "info/profile/tabs/info_profile_tabs_host.h"
#include "lang/lang_keys.h"
#include "lang/lang_instance.h"
#include "mainwindow.h"
#include "mainwidget.h"
#include "media/player/media_player_instance.h"
#include "mtproto/session_private.h"
#include "webview/webview_embed.h"
#include "window/main_window.h"
#include "window/window_peer_menu.h"
#include "window/window_session_controller.h"
#include "window/window_controller.h"
#include "window/notifications_manager.h"
#include "info/info_flexible_scroll.h"
#include "chat_helpers/stickers_list_widget.h"
#include "styles/style_chat_helpers.h"
#include "styles/style_info.h"
#include "styles/style_settings.h"
#include "styles/style_layers.h"
#include "styles/style_menu_icons.h"

#include <QtCore/QJsonDocument>
#include <QtGui/QGuiApplication>
#include <map>

// AyuGram includes
#include "ayu/ui/settings/settings_main.h"
#include "settings/settings_builder.h"


namespace Settings {
namespace {

const auto kOptionsClipboardPrefix = u"tdesktop-flags:"_q;

enum class TaiwanExperimentalText {
	Title,
	About,
	Restore,
	Irrelevant,
	CopyDeepLink,
	DeepLinkCopied,
	Export,
	ExportCopied,
	Import,
	InvalidClipboard,
	UnsupportedFormat,
	Imported,
	NeedRestart,
	RestartNow,
	RestartLater,
};

struct LocalizedOptionText {
	QString name;
	QString description;
};

[[nodiscard]] bool UseTaiwanChinese() {
	return Lang::GetInstance().isChineseContext();
}

[[nodiscard]] QString TaiwanText(TaiwanExperimentalText text) {
	switch (text) {
	case TaiwanExperimentalText::Title:
		return u"實驗性設定"_q;
	case TaiwanExperimentalText::About:
		return u"警告！以下為實驗性設定。部分功能可能無法運作，其他功能可能造成應用程式異常。任何選項都可能在下一個版本中無預警移除，使用時請自行承擔風險。"_q;
	case TaiwanExperimentalText::Restore:
		return u"還原預設值"_q;
	case TaiwanExperimentalText::Irrelevant:
		return u"此選項不適用於你的系統。"_q;
	case TaiwanExperimentalText::CopyDeepLink:
		return u"複製深層連結"_q;
	case TaiwanExperimentalText::DeepLinkCopied:
		return u"深層連結已複製到剪貼簿。"_q;
	case TaiwanExperimentalText::Export:
		return u"匯出"_q;
	case TaiwanExperimentalText::ExportCopied:
		return u"實驗性設定代碼已複製到剪貼簿。"_q;
	case TaiwanExperimentalText::Import:
		return u"匯入"_q;
	case TaiwanExperimentalText::InvalidClipboard:
		return u"剪貼簿中沒有有效的實驗性設定代碼。"_q;
	case TaiwanExperimentalText::UnsupportedFormat:
		return u"實驗性設定代碼有效，但不支援其資料格式。"_q;
	case TaiwanExperimentalText::Imported:
		return u"已從剪貼簿中的代碼匯入實驗性設定。"_q;
	case TaiwanExperimentalText::NeedRestart:
		return u"必須重新啟動才能套用部分新設定。要現在重新啟動嗎？"_q;
	case TaiwanExperimentalText::RestartNow:
		return u"重新啟動"_q;
	case TaiwanExperimentalText::RestartLater:
		return u"稍後"_q;
	}
	Unexpected("TaiwanExperimentalText value.");
}

[[nodiscard]] QString LocalizedText(
		TaiwanExperimentalText text,
		QString fallback) {
	return UseTaiwanChinese() ? TaiwanText(text) : std::move(fallback);
}

[[nodiscard]] const std::map<QString, LocalizedOptionText> &TaiwanOptionTexts() {
	static const auto result
		= std::map<QString, LocalizedOptionText>{
		{
			u"tabbed-panel-show-on-click"_q,
			{
				u"點擊時顯示分頁面板"_q,
				u"僅在點擊後顯示表情符號、貼圖與 GIF 面板。"_q,
			},
		},
		{
			u"forum-hide-chats-list"_q,
			{
				u"在論壇中隱藏聊天列表"_q,
				u"不保留狹窄的聊天列表欄。"_q,
			},
		},
		{
			u"dialogs-unread-on-top"_q,
			{
				u"將未靜音的未讀聊天置頂"_q,
				u"有新的未靜音訊息時，將聊天排列在置頂聊天下方，並維持在該處直到讀取。"_q,
			},
		},
		{
			u"dialogs-mute-icon"_q,
			{
				u"在聊天列表顯示靜音圖示"_q,
				u"在已靜音聊天名稱旁顯示小型靜音圖示。"_q,
			},
		},
		{
			u"fractional-scaling-enabled"_q,
			{
				u"啟用精確的高 DPI 縮放"_q,
				u"完全依照系統介面縮放設定。"_q,
			},
		},
		{
			u"high-dpi-downscale"_q,
			{
				u"高 DPI 向下縮放"_q,
				u"完全依照系統介面縮放設定（另一種作法，影像品質可能較佳）。"_q,
			},
		},
		{
			u"use-qt-rhi"_q,
			{ u"使用 Qt RHI 繪圖器"_q, QString() },
		},
		{
			u"view-profile-in-chats-list-context-menu"_q,
			{
				u"新增「檢視個人檔案」"_q,
				u"在聊天列表的右鍵選單中新增「檢視個人檔案」。"_q,
			},
		},
		{
			u"show-peer-id-below-about"_q,
			{
				u"在個人檔案顯示 Peer ID"_q,
				u"在個人簡介或描述下方顯示 API 提供的 Peer ID，並將聯絡人 ID 加入匯出資料。"_q,
			},
		},
		{
			u"show-channel-joined-below-about"_q,
			{
				u"在個人檔案顯示加入頻道日期"_q,
				u"在頻道描述下方顯示你加入頻道的日期。"_q,
			},
		},
		{
			u"profile-media-tabs"_q,
			{
				u"在個人檔案中以分頁顯示共用媒體"_q,
				u"將個人檔案中的共用媒體按鈕改為內嵌媒體列表的分頁列。此功能仍在開發中。"_q,
			},
		},
		{
			u"use-small-msg-bubble-radius"_q,
			{
				u"使用較小的訊息氣泡圓角"_q,
				u"讓多數訊息氣泡呈現較方正的外觀。"_q,
			},
		},
		{
			u"disable-autoplay-next"_q,
			{
				u"停用自動播放下一個項目"_q,
				u"停用自動播放下一個音訊檔案、語音訊息或影片訊息。"_q,
			},
		},
		{
			u"webview-debug-enabled"_q,
			{
				u"啟用 WebView 檢查工具"_q,
				u"可在 WebView 視窗按右鍵選擇「檢查」。（macOS 請啟動 Safari，從「開發」選單開啟。）"_q,
			},
		},
		{
			u"webview-legacy-edge"_q,
			{
				u"強制使用舊版 Edge WebView"_q,
				u"略過現代 CoreWebView2 檢查，並在 Windows 強制使用舊版 Edge WebView。"_q,
			},
		},
		{
			u"auto-scroll-inactive-chat"_q,
			{
				u"非作用中聊天自動標記已讀"_q,
				u"即使視窗沒有焦點，也將新訊息標記為已讀並捲動聊天。"_q,
			},
		},
		{
			u"hide-reply-button"_q,
			{
				u"隱藏回覆按鈕"_q,
				u"隱藏通知中的回覆按鈕。"_q,
			},
		},
		{
			u"custom-notification"_q,
			{
				u"強制提供非原生通知"_q,
				u"即使此平台的自訂通知無法正常運作，仍允許停用原生通知。"_q,
			},
		},
		{
			u"gnotification"_q,
			{
				u"GNotification"_q,
				u"強制啟用 GLib 的 GNotification；關閉時使用自動偵測。"_q,
			},
		},
		{
			u"freetype"_q,
			{
				u"FreeType 字型引擎"_q,
				u"使用 Linux 的字型引擎取代系統字型引擎。"_q,
			},
		},
		{
			u"skip-url-scheme-register"_q,
			{
				u"略過 URL Scheme 註冊"_q,
				u"自動更新時不重新註冊 tg:// URL Scheme。"_q,
			},
		},
		{
			u"deadlock-detector"_q,
			{
				u"死結偵測器"_q,
				u"每 30 秒檢查主執行緒是否仍有回應。"_q,
			},
		},
		{
			u"external-media-viewer"_q,
			{
				u"外部媒體檢視器"_q,
				u"使用系統媒體檢視器取代內建檢視器。"_q,
			},
		},
		{
			u"new-windows-size-as-first"_q,
			{
				u"調整新聊天視窗大小"_q,
				u"使用主視窗大小開啟新視窗。"_q,
			},
		},
		{
			u"prefer-ipv6"_q,
			{
				u"優先使用 IPv6"_q,
				u"IPv6 可用時優先使用；必須先啟用「嘗試透過 IPv6 連線」。"_q,
			},
		},
		{
			u"fast-buttons-mode"_q,
			{
				u"快速按鈕模式"_q,
				u"使用鍵盤數字鍵 1–9 觸發行內鍵盤按鈕。"_q,
			},
		},
		{
			u"touchbar-disabled"_q,
			{ u"停用 Touch Bar（僅限 macOS）"_q, QString() },
		},
		{
			u"classic-profile-scroll"_q,
			{
				u"使用傳統個人檔案捲動處理"_q,
				u"將個人檔案封面捲動還原為先前以填充區塊實作的方式。"_q,
			},
		},
		{
			u"moderate-common-groups"_q,
			{ u"同時在多個群組封鎖使用者"_q, QString() },
		},
		{
			u"force-compose-search-one-column"_q,
			{
				u"強制在聊天中使用內嵌搜尋"_q,
				u"在單欄模式強制使用聊天內嵌搜尋。"_q,
			},
		},
		{
			u"unlimited-recent-stickers"_q,
			{
				u"不限制最近使用貼圖數量"_q,
				u"顯示伺服器提供的所有最近使用貼圖。"_q,
			},
		},
		{
			u"hide-ai-button"_q,
			{
				u"隱藏 AI 按鈕"_q,
				u"隱藏訊息輸入框中的 AI 工具按鈕。"_q,
			},
		},
		{
			u"unlimited-message-width"_q,
			{
				u"不限制訊息寬度"_q,
				u"允許純文字訊息氣泡超過預設最大寬度。"_q,
			},
		},
		{
			u"mac-cmd-reply-immediately"_q,
			{
				u"Mac：按 Command + 上／下鍵立即回覆"_q,
				u"按 Command + 上／下鍵時立即回覆上一則或下一則訊息，不先將文字游標移至輸入框開頭或結尾。按住 Shift 可維持原本的游標移動行為。"_q,
			},
		},
		{
			u"qscroller"_q,
			{
				u"使用 QScroller 處理觸控板捲動"_q,
				u"提供慣性捲動與越界捲動效果。"_q,
			},
		},
		{
			u"ffmpeg-multithread"_q,
			{
				u"多執行緒影片解碼"_q,
				u"允許 FFmpeg 使用執行緒池進行解碼，通常會為每個 CPU 執行緒配置一條解碼執行緒。"_q,
			},
		},
	};
	return result;
}

[[nodiscard]] LocalizedOptionText ResolveOptionText(
		const base::options::option<bool> &option) {
	auto result = LocalizedOptionText{
		.name = option.name().isEmpty() ? option.id() : option.name(),
		.description = option.description(),
	};
	if (!UseTaiwanChinese()) {
		return result;
	}
	const auto &translations = TaiwanOptionTexts();
	const auto i = translations.find(option.id());
	return (i != translations.end()) ? i->second : result;
}

struct DecodeOptionsResult {
	bool ok = false;
	QString json;
};

struct ResolvedReferrer {
	QString controlId;
	Type section = AyuMain::Id();
};

[[nodiscard]] QString EncodeOptionsToText(const QString &json) {
	const auto flags = QByteArray::Base64UrlEncoding
		| QByteArray::OmitTrailingEquals;
	return kOptionsClipboardPrefix
		+ qs(qCompress(json.toLatin1(), 9).toBase64(flags));
}

[[nodiscard]] DecodeOptionsResult DecodeOptionsFromText(const QString &text) {
	auto result = DecodeOptionsResult();
	if (!text.startsWith(kOptionsClipboardPrefix)) {
		return result;
	}
	auto encoded = QStringView(text).mid(
		kOptionsClipboardPrefix.size()).toLatin1();
	const auto compressed = QByteArray::fromBase64Encoding(
		std::move(encoded),
		QByteArray::Base64UrlEncoding
			| QByteArray::AbortOnBase64DecodingErrors);
	if (!compressed || (*compressed).isEmpty()) {
		return result;
	}
	const auto decoded = qUncompress(*compressed);
	if (decoded.isEmpty()) {
		return result;
	}

	auto error = QJsonParseError();
	const auto parsed = QJsonDocument::fromJson(decoded, &error);
	if ((error.error != QJsonParseError::NoError) || !parsed.isObject()) {
		return result;
	}
	result.ok = true;
	result.json = QString::fromUtf8(decoded);
	return result;
}

[[nodiscard]] ResolvedReferrer ResolveReferrer(
		const QString &controlId,
		not_null<Main::Session*> session) {
	const auto &registry = Builder::SearchRegistry::Instance();
	const auto entries = registry.collectAll(session);
	for (const auto &entry : entries) {
		if (!entry.section) {
			continue;
		}
		if (entry.id == controlId) {
			return {
				.controlId = entry.id,
				.section = entry.section,
			};
		}
		if (entry.altIds.contains(controlId)) {
			return {
				.controlId = entry.id,
				.section = entry.section,
			};
		}
	}
	return {
		.controlId = controlId,
	};
}

[[nodiscard]] QString OptionReferrer(const base::options::option<bool> &option) {
	const auto &id = option.id();
	if (id == u"tabbed-panel-show-on-click"_q) {
		return u"ayu/showEmojiPopup"_q;
	} else if (id == u"show-peer-id-below-about"_q) {
		return u"ayu/showPeerId"_q;
	} else if (id == u"use-small-msg-bubble-radius"_q) {
		return u"ayu/messageBubbleRadius"_q;
	} else if (id == u"unlimited-recent-stickers"_q) {
		return u"ayu/recentStickersCount"_q;
	} else if (id == u"hide-ai-button"_q) {
		return u"ayu/showAiEditorButtonInMessageField"_q;
	}
	return QString();
}

void AddOption(
		not_null<Window::Controller*> window,
		not_null<Window::SessionController*> controller,
		not_null<Ui::VerticalLayout*> container,
		base::options::option<bool> &option,
		rpl::producer<> resetClicks,
		rpl::producer<> reloadOptionsRequests,
		rpl::producer<QString> query,
		Fn<void(const QString&, not_null<QWidget*>)> registerHighlight) {
	const auto text = ResolveOptionText(option);
	const auto &name = text.name;
	const auto &description = text.description;

	const auto wrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
			container,
			object_ptr<Ui::VerticalLayout>(container)));
	const auto inner = wrap->entity();

	auto &lifetime = inner->lifetime();
	const auto toggles = lifetime.make_state<rpl::event_stream<bool>>();
	std::move(
		resetClicks
	) | rpl::map_to(
		option.defaultValue()
	) | rpl::start_to_stream(*toggles, lifetime);
	std::move(reloadOptionsRequests) | rpl::on_next([=, &option] {
		toggles->fire_copy(option.value());
	}, lifetime);

	const auto referrer = OptionReferrer(option);
	const auto button = inner->add(object_ptr<Button>(
		inner,
		rpl::single(name),
		(!referrer.isEmpty() || option.relevant())
			? st::settingsButtonNoIcon
			: st::settingsOptionDisabled));
	if (!referrer.isEmpty()) {
		button->addClickHandler([=] {
			const auto resolved = ResolveReferrer(
				referrer,
				&controller->session());
			controller->setHighlightControlId(resolved.controlId);
			controller->showSettings(resolved.section);
			window->activate();
		});
	} else {
		button->toggleOn(toggles->events_starting_with(option.value()));
	}

	if (registerHighlight) {
		registerHighlight(u"experimental/"_q + option.id(), button);
	}

	const auto link = u"tg://settings/experimental/"_q + option.id();
	const auto menu
		= button->lifetime().make_state<base::unique_qptr<Ui::PopupMenu>>();
	button->events(
	) | rpl::filter([](not_null<QEvent*> e) {
		return e->type() == QEvent::ContextMenu;
	}) | rpl::on_next([=](not_null<QEvent*> e) {
		*menu = base::make_unique_q<Ui::PopupMenu>(
			button,
			st::popupMenuWithIcons);
		(*menu)->addAction(LocalizedText(
			TaiwanExperimentalText::CopyDeepLink,
			u"Copy deep link"_q), [=] {
			TextUtilities::SetClipboardText({ link });
			window->showToast({
				.text = { LocalizedText(
					TaiwanExperimentalText::DeepLinkCopied,
					u"Deep link copied to clipboard."_q) },
				.iconLottie = u"toast/voip_invite"_q,
				.iconLottieSize = st::toastLottieIconSize,
			});
		}, &st::menuIconCopy);
		(*menu)->popup(QCursor::pos());
		e->accept();
	}, button->lifetime());

	const auto restarter = (referrer.isEmpty()
		&& option.relevant()
		&& option.restartRequired())
		? button->lifetime().make_state<base::Timer>()
		: nullptr;
	if (restarter) {
		restarter->setCallback([=] {
			const auto text = LocalizedText(
				TaiwanExperimentalText::NeedRestart,
				tr::lng_settings_need_restart(tr::now));
			const auto confirmText = LocalizedText(
				TaiwanExperimentalText::RestartNow,
				tr::lng_settings_restart_now(tr::now));
			const auto cancelText = LocalizedText(
				TaiwanExperimentalText::RestartLater,
				tr::lng_settings_restart_later(tr::now));
			window->show(Ui::MakeConfirmBox({
				.text = text,
				.confirmed = [] { Core::Restart(); },
				.confirmText = confirmText,
				.cancelText = cancelText,
			}));
		});
	}
	if (referrer.isEmpty()) {
		button->toggledChanges(
		) | rpl::on_next([=, &option](bool toggled) {
			if (!option.relevant() && toggled != option.defaultValue()) {
				toggles->fire_copy(option.defaultValue());
				window->showToast(LocalizedText(
					TaiwanExperimentalText::Irrelevant,
					tr::lng_settings_experimental_irrelevant(tr::now)));
				return;
			}
			option.set(toggled);
			if (restarter) {
				restarter->callOnce(st::settingsButtonNoIcon.toggle.duration);
			}
		}, inner->lifetime());
	}

	if (!description.isEmpty()) {
		Ui::AddSkip(inner, st::settingsCheckboxesSkip);
		Ui::AddDividerText(inner, rpl::single(description));
		Ui::AddSkip(inner, st::settingsCheckboxesSkip);
	}

	std::move(
		query
	) | rpl::on_next([=](const QString &text) {
		const auto trimmed = text.trimmed();
		const auto matches = trimmed.isEmpty()
			|| name.contains(trimmed, Qt::CaseInsensitive)
			|| description.contains(trimmed, Qt::CaseInsensitive);
		wrap->toggle(matches, anim::type::instant);
	}, wrap->lifetime());
}

void SetupExperimental(
		not_null<Window::Controller*> window,
		not_null<Window::SessionController*> controller,
		not_null<Ui::VerticalLayout*> container,
		rpl::producer<> reloadOptionsRequests,
		rpl::producer<QString> query,
		Fn<void(const QString&, not_null<QWidget*>)> registerHighlight) {
	const auto headerWrap = container->add(
		object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
			container,
			object_ptr<Ui::VerticalLayout>(container)));
	const auto header = headerWrap->entity();

	Ui::AddSkip(header, st::settingsCheckboxesSkip);

	header->add(
		object_ptr<Ui::FlatLabel>(
			header,
			rpl::single(LocalizedText(
				TaiwanExperimentalText::About,
				tr::lng_settings_experimental_about(tr::now))),
			st::boxLabel),
		st::defaultBoxDividerLabelPadding);

	auto reset = (Button*)nullptr;
	if (base::options::changed()) {
		const auto wrap = header->add(
			object_ptr<Ui::SlideWrap<Ui::VerticalLayout>>(
				header,
				object_ptr<Ui::VerticalLayout>(header)));
		const auto inner = wrap->entity();
		Ui::AddDivider(inner);
		Ui::AddSkip(inner, st::settingsCheckboxesSkip);
		reset = inner->add(object_ptr<Button>(
			inner,
			rpl::single(LocalizedText(
				TaiwanExperimentalText::Restore,
				tr::lng_settings_experimental_restore(tr::now))),
			st::settingsButtonNoIcon));
		reset->addClickHandler([=] {
			base::options::reset();
			wrap->hide(anim::type::normal);
		});
		Ui::AddSkip(inner, st::settingsCheckboxesSkip);
	}

	Ui::AddDivider(header);
	Ui::AddSkip(header, st::settingsCheckboxesSkip);

	rpl::duplicate(
		query
	) | rpl::on_next([=](const QString &text) {
		headerWrap->toggle(text.trimmed().isEmpty(), anim::type::instant);
	}, headerWrap->lifetime());

	const auto addToggle = [&](const char name[]) {
		AddOption(
			window,
			controller,
			container,
			base::options::lookup<bool>(name),
			(reset
				? (reset->clicks() | rpl::to_empty)
				: rpl::producer<>()),
			rpl::duplicate(reloadOptionsRequests),
			rpl::duplicate(query),
			registerHighlight);
	};

	addToggle(ChatHelpers::kOptionTabbedPanelShowOnClick);
	addToggle(Dialogs::kOptionForumHideChatsList);
	addToggle(Dialogs::kOptionDialogsUnreadOnTop);
	addToggle(Dialogs::Ui::kOptionDialogsMuteIcon);
	addToggle(Core::kOptionFractionalScalingEnabled);
	addToggle(Core::kOptionHighDpiDownscale);
	addToggle(Ui::GL::kOptionUseQtRhi);
	addToggle(Window::kOptionViewProfileInChatsListContextMenu);
	addToggle(Info::Profile::kOptionShowPeerIdBelowAbout);
	addToggle(Info::Profile::kOptionShowChannelJoinedBelowAbout);
	addToggle(Info::Profile::kOptionProfileMediaTabs);
	addToggle(Ui::kOptionUseSmallMsgBubbleRadius);
	addToggle(Media::Player::kOptionDisableAutoplayNext);
	addToggle(Webview::kOptionWebviewDebugEnabled);
	addToggle(Webview::kOptionWebviewLegacyEdge);
	addToggle(kOptionAutoScrollInactiveChat);
	addToggle(Window::Notifications::kOptionHideReplyButton);
	addToggle(Window::Notifications::kOptionCustomNotification);
	addToggle(Window::Notifications::kOptionGNotification);
	addToggle(Core::kOptionFreeType);
	addToggle(Core::kOptionSkipUrlSchemeRegister);
	addToggle(Core::kOptionDeadlockDetector);
	addToggle(Window::kOptionExternalMediaViewer);
	addToggle(Window::kOptionNewWindowsSizeAsFirst);
	addToggle(MTP::details::kOptionPreferIPv6);
	if (base::options::lookup<bool>(kOptionFastButtonsMode).value()) {
		addToggle(kOptionFastButtonsMode);
	}
	addToggle(Window::kOptionDisableTouchbar);
	addToggle(Info::kClassicProfileScroll);
	addToggle(kModerateCommonGroups);
	addToggle(kForceComposeSearchOneColumn);
	addToggle(ChatHelpers::kOptionUnlimitedRecentStickers);
	addToggle(Ui::kOptionHideAiButton);
	addToggle(HistoryView::kOptionUnlimitedMessageWidth);
	addToggle(HistoryView::Controls::kOptionMacCmdReplyImmediately);
	addToggle(Ui::kOptionQScroller);
	addToggle(FFmpeg::kOptionFFmpegMultiThread);
}

} // namespace

Experimental::Experimental(
	QWidget *parent,
	not_null<Window::SessionController*> controller)
: Section(parent, controller) {
	setupContent();
}

Experimental::~Experimental() = default;

rpl::producer<QString> Experimental::title() {
	return rpl::single(LocalizedText(
		TaiwanExperimentalText::Title,
		tr::lng_settings_experimental(tr::now)));
}

void Experimental::fillTopBarMenu(const Ui::Menu::MenuCallback &addAction) {
	const auto window = &controller()->window();
	addAction(
		LocalizedText(TaiwanExperimentalText::Export, u"Export"_q),
		[=] {
			TextUtilities::SetClipboardText(
				{ EncodeOptionsToText(base::options::serialize()) });
			window->showToast({
				.text = { LocalizedText(
					TaiwanExperimentalText::ExportCopied,
					u"Experimental settings code copied to clipboard."_q) },
				.iconLottie = u"toast/copy"_q,
				.iconLottieSize = st::toastLottieIconSize,
			});
		},
		&st::menuIconCopy);
	if (!DecodeOptionsFromText(QGuiApplication::clipboard()->text()).ok) {
		return;
	}
	addAction(
		LocalizedText(TaiwanExperimentalText::Import, u"Import"_q),
		[=] {
			const auto decoded = DecodeOptionsFromText(
				QGuiApplication::clipboard()->text());
			if (!decoded.ok) {
				window->showToast(LocalizedText(
					TaiwanExperimentalText::InvalidClipboard,
					u"Clipboard does not contain a valid experimental "
					"settings code."_q));
				return;
			}
			if (!base::options::deserialize(decoded.json)) {
				window->showToast(LocalizedText(
					TaiwanExperimentalText::UnsupportedFormat,
					u"Experimental settings code is valid, but data format "
					"is not supported."_q));
				return;
			}
			_reloadOptionsRequests.fire({});
			window->showToast(LocalizedText(
				TaiwanExperimentalText::Imported,
				u"Experimental settings imported from code in clipboard."_q));
		},
		&st::menuIconImportTheme);
}

void Experimental::setInnerFocus() {
	if (_searchField) {
		_searchField->setFocus();
	} else {
		setFocus();
	}
}

void Experimental::showFinished() {
	AbstractSection::showFinished();
	for (const auto &[id, widget] : _highlights) {
		if (widget) {
			controller()->checkHighlightControl(id, widget);
		}
	}
}

base::weak_qptr<Ui::RpWidget> Experimental::createPinnedToTop(
		not_null<QWidget*> parent) {
	_searchController = std::make_unique<Ui::SearchFieldController>(
		_query.current());
	auto rowView = _searchController->createRowView(
		parent,
		st::infoLayerMediaSearch);
	_searchField = rowView.field;

	const auto searchContainer = Ui::CreateChild<Ui::FixedHeightWidget>(
		parent.get(),
		st::infoLayerMediaSearch.height);
	const auto wrap = rowView.wrap.release();
	wrap->setParent(searchContainer);
	wrap->show();

	searchContainer->widthValue(
	) | rpl::on_next([=](int width) {
		wrap->resizeToWidth(width);
		wrap->moveToLeft(0, 0);
	}, searchContainer->lifetime());

	_searchController->queryValue(
	) | rpl::on_next([=](QString text) {
		_query = std::move(text);
	}, searchContainer->lifetime());

	return base::make_weak(not_null<Ui::RpWidget*>{ searchContainer });
}

void Experimental::setupContent() {
	const auto content = Ui::CreateChild<Ui::VerticalLayout>(this);

	SetupExperimental(
		&controller()->window(),
		controller(),
		content,
		_reloadOptionsRequests.events(),
		_query.value(),
		[this](const QString &id, not_null<QWidget*> widget) {
			_highlights.push_back({ id, widget.get() });
		});

	Ui::ResizeFitChild(this, content);
}

} // namespace Settings
