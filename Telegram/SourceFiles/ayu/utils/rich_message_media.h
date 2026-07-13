// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include "base/basic_types.h"

#include <memory>
#include <vector>

class DocumentData;
class HistoryItem;
class PhotoData;

namespace Iv {
struct RichPage;
} // namespace Iv

namespace AyuUtils {

struct RichMessageVisualMedia {
	std::vector<not_null<PhotoData*>> photos;
	std::vector<not_null<DocumentData*>> documents;
	std::vector<not_null<DocumentData*>> thumbnailOnlyDocuments;
};

[[nodiscard]] std::shared_ptr<const Iv::RichPage> EffectiveRichPage(
	not_null<const HistoryItem*> item);
[[nodiscard]] RichMessageVisualMedia CollectRichMessageVisualMedia(
	const Iv::RichPage &page);
[[nodiscard]] RichMessageVisualMedia CollectRichMessageVisualMedia(
	not_null<const HistoryItem*> item);

} // namespace AyuUtils
