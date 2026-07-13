// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include <memory>

namespace tl {
template <typename bare>
class boxed;
} // namespace tl

class MTPrichMessage;
using MTPRichMessage = tl::boxed<MTPrichMessage>;

namespace AyuMapper {

std::pair<std::string, std::vector<char>> serializeTextWithEntities(not_null<HistoryItem*> item);
[[nodiscard]] MTPVector<MTPMessageEntity> deserializeTextWithEntities(std::vector<char> serialized);
[[nodiscard]] std::vector<char> serializeRichMessage(
	const MTPRichMessage &message);
[[nodiscard]] auto deserializeRichMessage(
	const std::vector<char> &serialized)
-> std::shared_ptr<const MTPRichMessage>;
int mapItemFlagsToMTPFlags(not_null<HistoryItem*> item);

} // namespace AyuMapper
