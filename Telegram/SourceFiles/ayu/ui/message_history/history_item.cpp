// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/ui/message_history/history_item.h"

#include "history/history_item.h"
#include "api/api_text_entities.h"
#include "ayu/data/entities.h"
#include "ayu/ui/message_history/history_inner.h"
#include "ayu/utils/ayu_mapper.h"
#include "base/unixtime.h"
#include "core/application.h"
#include "core/click_handler_types.h"
#include "data/data_channel.h"
#include "data/data_file_origin.h"
#include "data/data_forum_topic.h"
#include "data/data_session.h"
#include "data/data_user.h"
#include "history/history.h"
#include "history/view/history_view_element.h"
#include "iv/iv_rich_page.h"
#include "ui/basic_click_handlers.h"
#include "ui/text/text_utilities.h"

namespace MessageHistory {

OwnedItem::OwnedItem(std::nullptr_t) {
}

OwnedItem::OwnedItem(
	not_null<HistoryView::ElementDelegate*> delegate,
	not_null<HistoryItem*> data)
	: _data(data), _view(_data->createView(delegate)) {
}

OwnedItem::OwnedItem(OwnedItem &&other)
	: _data(base::take(other._data)), _view(base::take(other._view)) {
}

OwnedItem &OwnedItem::operator=(OwnedItem &&other) {
	_data = base::take(other._data);
	_view = base::take(other._view);
	return *this;
}

OwnedItem::~OwnedItem() {
	clearView();
	if (_data) {
		_data->destroy();
	}
}

void OwnedItem::refreshView(
	not_null<HistoryView::ElementDelegate*> delegate) {
	_view = _data->createView(delegate);
}

void OwnedItem::clearView() {
	_view = nullptr;
}

void GenerateItems(
	not_null<HistoryView::ElementDelegate*> delegate,
	not_null<History*> history,
	AyuMessageBase message,
	Fn<void(OwnedItem item, TimeId sentDate, MsgId)> callback) {
	PeerData *from = history->owner().userLoaded(message.fromId);
	if (!from) {
		from = history->owner().channelLoaded(message.fromId);
	}
	if (!from) {
		from = reinterpret_cast<PeerData*>(history->owner().chatLoaded(message.fromId));
	}
	const auto date = message.entityCreateDate;
	const auto addPart = [&](
		not_null<HistoryItem*> item,
		TimeId sentDate = 0,
		MsgId realId = MsgId())
	{
		return callback(OwnedItem(delegate, item), sentDate, realId);
	};

	const auto makeSimpleTextMessage = [&](TextWithEntities &&text)
	{
		base::flags<MessageFlag> flags = MessageFlag::AdminLogEntry;
		if (from) {
			flags |= MessageFlag::HasFromId;
		} else {
			flags |= MessageFlag::HasPostAuthor;
		}
		if (!message.postAuthor.empty()) {
			flags |= MessageFlag::HasPostAuthor;
		}

		return history->makeMessage({
										.id = history->nextNonHistoryEntryId(),
										.flags = flags,
										.from = from ? from->id : 0,
										.date = date,
										.postAuthor = !message.postAuthor.empty()
														  ? QString::fromStdString(message.postAuthor)
														  : from
																? QString()
																: QString("unknown user: %1").arg(message.fromId),
									},
									std::move(text),
									MTP_messageMediaEmpty());
	};

	auto textAndEntities = Ui::Text::WithEntities(
		QString::fromStdString(message.text));
	if (!message.textEntities.empty()) {
		const auto entities = AyuMapper::deserializeTextWithEntities(
			message.textEntities);
		textAndEntities.entities = Api::EntitiesFromMTP(
			&history->session(),
			entities.v);
	}
	auto richMessage = AyuMapper::deserializeRichMessage(
		message.richMessageSerialized);
	auto richPage = richMessage
		? Iv::ParseRichPage(&history->session(), *richMessage)
		: nullptr;
	if (richPage && richPage->part) {
		auto archivedPage = std::make_shared<Iv::RichPage>(*richPage);
		archivedPage->part = false;
		richPage = std::move(archivedPage);
	}
	if (richPage) {
		textAndEntities = Iv::FlattenRichPageSummary(richPage);
	} else if (!message.richMessageSummary.empty()) {
		textAndEntities = Ui::Text::WithEntities(
			QString::fromUtf8(
				message.richMessageSummary.data(),
				int(message.richMessageSummary.size())));
	}
	const auto item = makeSimpleTextMessage(std::move(textAndEntities));
	if (richPage) {
		item->setRichPage(std::move(richPage), std::move(richMessage));
	}
	addPart(item);
}

} // namespace MessageHistory
