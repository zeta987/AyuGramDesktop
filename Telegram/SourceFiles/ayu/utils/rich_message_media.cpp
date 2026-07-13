// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/utils/rich_message_media.h"

#include "base/flat_set.h"
#include "data/data_document.h"
#include "history/history_item.h"
#include "iv/iv_rich_page.h"

namespace AyuUtils {
namespace {

using Block = Iv::RichPage::Block;
using BlockKind = Iv::RichPage::BlockKind;

struct CollectionState {
	RichMessageVisualMedia result;
	base::flat_set<not_null<PhotoData*>> photos;
	base::flat_set<not_null<DocumentData*>> documents;
};

void AddPhoto(CollectionState *state, PhotoData *photo) {
	if (photo && state->photos.emplace(photo).second) {
		state->result.photos.emplace_back(photo);
	}
}

void AddDocument(
		CollectionState *state,
		DocumentData *document,
		bool thumbnailOnly = false) {
	if (!document || !state->documents.emplace(document).second) {
		return;
	}
	if (thumbnailOnly
		|| document->isAudioFile()
		|| document->isVoiceMessage()) {
		state->result.thumbnailOnlyDocuments.emplace_back(document);
	} else {
		state->result.documents.emplace_back(document);
	}
}

void CollectBlocks(
		CollectionState *state,
		const std::vector<Block> &blocks) {
	for (const auto &block : blocks) {
		switch (block.kind) {
		case BlockKind::Photo:
		case BlockKind::EmbedPost:
			AddPhoto(state, block.photo);
			break;
		case BlockKind::Video:
			AddDocument(state, block.document);
			break;
		case BlockKind::Audio:
			AddDocument(state, block.document, true);
			break;
		default:
			break;
		}
		for (const auto &item : block.mediaItems) {
			switch (item.kind) {
			case BlockKind::Photo:
				AddPhoto(state, item.photo);
				break;
			case BlockKind::Video:
				AddDocument(state, item.document);
				break;
			case BlockKind::Audio:
				AddDocument(state, item.document, true);
				break;
			default:
				break;
			}
		}
		for (const auto &article : block.relatedArticles) {
			AddPhoto(state, article.photo);
		}
		CollectBlocks(state, block.blocks);
		for (const auto &item : block.listItems) {
			CollectBlocks(state, item.blocks);
		}
	}
}

} // namespace

std::shared_ptr<const Iv::RichPage> EffectiveRichPage(
		not_null<const HistoryItem*> item) {
	if (const auto full = item->fullRichPage()) {
		return full;
	}
	return item->richPage();
}

RichMessageVisualMedia CollectRichMessageVisualMedia(
		const Iv::RichPage &page) {
	auto state = CollectionState();
	CollectBlocks(&state, page.blocks);
	return std::move(state.result);
}

RichMessageVisualMedia CollectRichMessageVisualMedia(
		not_null<const HistoryItem*> item) {
	const auto page = EffectiveRichPage(item);
	return page
		? CollectRichMessageVisualMedia(*page)
		: RichMessageVisualMedia();
}

} // namespace AyuUtils
