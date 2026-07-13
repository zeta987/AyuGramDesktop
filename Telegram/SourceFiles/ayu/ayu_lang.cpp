// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#include "ayu/ayu_lang.h"

#include "qjsondocument.h"
#include "core/application.h"
#include "core/core_settings.h"
#include "lang/lang_file_parser.h"
#include "lang/lang_instance.h"
#include "storage/localstorage.h"

#include <QDir>
#include <QFile>

#include <optional>
#include <vector>

// hard-coded languages
std::map<QString, QString> langMapping = {
	{"pt-br", "pt"},
	{"zh-hans-beta", "zh-hans"},
	{"zh-hant-beta", "zh-hant"},
	{"zh-hans-raw", "zh-hans"},
	{"zh-hant-raw", "zh-hant"},
};

constexpr auto postfixes = {
	"zero",
	"one",
	"two",
	"few",
	"many",
	"other"
};

constexpr auto kBundledTaiwanChinese
	= ":/gui/langs/ayu_zh_tw.strings"_cs;

using BundledTaiwanChinese
	= std::vector<std::pair<QByteArray, QByteArray>>;

[[nodiscard]] std::optional<BundledTaiwanChinese>
ReadBundledTaiwanChinese() {
	QFile file(kBundledTaiwanChinese.utf16());
	if (!file.open(QIODevice::ReadOnly)) {
		LOG(("Could not open bundled AyuGram zh-TW language."));
		return std::nullopt;
	}
	auto result = BundledTaiwanChinese();
	Lang::FileParser parser(file.readAll(), [&](QLatin1String key, const QByteArray &value) {
		result.emplace_back(QByteArray(key.data(), key.size()), value);
	});
	if (!parser.errors().isEmpty()) {
		LOG(("Could not parse bundled AyuGram zh-TW language: %1"
			).arg(parser.errors()));
		return std::nullopt;
	}
	if (!parser.warnings().isEmpty()) {
		LOG(("Bundled AyuGram zh-TW language warnings: %1"
			).arg(parser.warnings()));
	}
	return result;
}

AyuLanguage *AyuLanguage::instance = nullptr;

AyuLanguage::AyuLanguage() {
	Lang::GetInstance().idChanges(
	) | rpl::on_next([=] {
		refresh();
	}, _lifetime);
	Lang::GetInstance().updated(
	) | rpl::on_next([this] {
		if (_applyingBundledTaiwanChinese) {
			return;
		}
		const auto &language = Lang::GetInstance();
		if (language.isChineseContext()) {
			applyBundledTaiwanChinese();
		} else {
			resetBundledTaiwanChinese();
		}
	}, _lifetime);
}

void AyuLanguage::init() {
	if (!instance) instance = new AyuLanguage;
	instance->refresh();
}

AyuLanguage *AyuLanguage::currentInstance() {
	return instance;
}

void AyuLanguage::refresh() {
	if (_chkReply) {
		QObject::disconnect(_chkReply, nullptr, this, nullptr);
		_chkReply->abort();
		_chkReply->deleteLater();
		_chkReply = nullptr;
	}
	needFallback = false;
	_currentLangId = QString();

	const auto &language = Lang::GetInstance();
	if (language.isChineseContext()) {
		applyBundledTaiwanChinese();
		return;
	}

	resetBundledTaiwanChinese();
	loadCachedLanguage();
	if (!language.id().isEmpty() && !language.isCustom()) {
		fetchLanguage(language.id(), language.baseId());
	}
}

void AyuLanguage::applyBundledTaiwanChinese() {
	if (_applyingBundledTaiwanChinese) {
		return;
	}
	_applyingBundledTaiwanChinese = true;
	const auto guard = gsl::finally([&] {
		_applyingBundledTaiwanChinese = false;
	});
	const auto values = ReadBundledTaiwanChinese();
	if (!values || values->empty()) {
		return;
	}
	for (const auto &[key, value] : *values) {
		Lang::GetInstance().resetValue(key);
		Lang::GetInstance().applyValue(key, value);
	}
	_bundledTaiwanChineseApplied = true;
	Lang::GetInstance().updatePluralRules();
	Lang::GetInstance().notifyUpdated();
}

void AyuLanguage::resetBundledTaiwanChinese() {
	if (!_bundledTaiwanChineseApplied) {
		return;
	}
	const auto values = ReadBundledTaiwanChinese();
	if (!values) {
		return;
	}
	for (const auto &entry : *values) {
		Lang::GetInstance().resetValue(entry.first);
	}
	_bundledTaiwanChineseApplied = false;
	Lang::GetInstance().updatePluralRules();
	Lang::GetInstance().notifyUpdated();
}

QString AyuLanguage::getCacheDir() const {
	return cWorkingDir() + u"tdata/ayu/languages/"_q;
}

QString AyuLanguage::getCachePath(const QString &langId) const {
	return getCacheDir() + langId + u".json"_q;
}

void AyuLanguage::loadCachedLanguage() {
	const auto langPackId = Lang::GetInstance().id();
	const auto langPackBaseId = Lang::GetInstance().baseId();
	auto finalLangPackId = langMapping.contains(langPackId) ? langMapping[langPackId] : langPackId;

	if (finalLangPackId.isEmpty()) {
		finalLangPackId = langPackBaseId;
	}
	if (finalLangPackId.isEmpty()) {
		return;
	}

	const auto cachePath = getCachePath(finalLangPackId);
	QFile file(cachePath);
	if (!file.exists()) {
		const auto basePath = getCachePath(langPackBaseId);
		if (!QFile::exists(basePath)) {
			return;
		}
		file.setFileName(basePath);
	}

	if (file.open(QIODevice::ReadOnly)) {
		const auto data = file.readAll();
		file.close();

		QJsonParseError error{};
		const auto doc = QJsonDocument::fromJson(data, &error);
		if (error.error == QJsonParseError::NoError) {
			LOG(("Loading cached AyuGram language: %1").arg(finalLangPackId));
			applyLanguageJson(doc);
		}
	}
}

void AyuLanguage::saveCachedLanguage(const QByteArray &json, const QString &langId) {
	const auto cacheDir = getCacheDir();
	QDir().mkpath(cacheDir);

	const auto cachePath = getCachePath(langId);
	QFile file(cachePath);
	if (file.open(QIODevice::WriteOnly)) {
		file.write(json);
		file.close();
		LOG(("Cached AyuGram language: %1").arg(langId));
	}
}

void AyuLanguage::fetchLanguage(const QString &id, const QString &baseId) {
	auto finalLangPackId = langMapping.contains(id) ? langMapping[id] : id;
	_currentLangId = finalLangPackId.isEmpty() ? baseId : finalLangPackId;

	if (Core::App().settings().proxy().isEnabled()) {
		const auto proxy = Core::App().settings().proxy().selected();
		if (proxy.type == MTP::ProxyData::Type::Socks5 || proxy.type == MTP::ProxyData::Type::Http) {
			const auto networkProxy = ToNetworkProxy(ToDirectIpProxy(Core::App().settings().proxy().selected()));
			networkManager.setProxy(networkProxy);
		}
	}

	// using `jsdelivr` since China (...and maybe other?) users have some problems with GitHub
	// https://crowdin.com/project/ayugram/discussions/6
	QUrl url;
	if (!finalLangPackId.isEmpty() && !baseId.isEmpty() && !needFallback) {
		url.setUrl(qsl("https://cdn.jsdelivr.net/gh/AyuGram/Languages@l10n_main/values/langs/%1/Shared.json").arg(
			finalLangPackId));
	} else {
		url.setUrl(qsl("https://cdn.jsdelivr.net/gh/AyuGram/Languages@l10n_main/values/langs/%1/Shared.json").arg(
			needFallback ? baseId : finalLangPackId));
	}
	_chkReply = networkManager.get(QNetworkRequest(url));
	connect(_chkReply, &QNetworkReply::errorOccurred,
		this, &AyuLanguage::fetchError);
	connect(_chkReply, &QNetworkReply::finished,
		this, &AyuLanguage::fetchFinished);
}

void AyuLanguage::fetchFinished() {
	if (!_chkReply) return;
	const auto reply = base::take(_chkReply);
	const auto cleanup = gsl::finally([reply] {
		reply->deleteLater();
	});

	QString langPackBaseId = Lang::GetInstance().baseId();
	QString langPackId = Lang::GetInstance().id();
	auto statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	if (statusCode == 404 && !langPackId.isEmpty() && !langPackBaseId.isEmpty() && !needFallback) {
		LOG(("AyuGram Language not found! Fallback to main language: %1...").arg(langPackBaseId));
		needFallback = true;
		fetchLanguage("", langPackBaseId);
		return;
	}
	if (reply->error() != QNetworkReply::NoError) {
		return;
	}

	const auto result = reply->readAll().trimmed();
	QJsonParseError error{};
	const auto doc = QJsonDocument::fromJson(result, &error);
	if (error.error == QJsonParseError::NoError) {
		saveCachedLanguage(result, _currentLangId);
		applyLanguageJson(doc);
	} else {
		LOG(("Incorrect language JSON File."));
	}
}

void AyuLanguage::fetchError(QNetworkReply::NetworkError e) {
	LOG(("Network error: %1").arg(e));
}

void AyuLanguage::applyLanguageJson(QJsonDocument doc) {
	const auto json = doc.object();
	for (const QString &brokenKey : json.keys()) {
		auto key = qsl("ayu_") + brokenKey;
		auto val = json.value(brokenKey).toString().replace(qsl("&amp;"), qsl("&"));

		if (key.endsWith("_Android")) {
			continue;
		}

		for (const auto &postfix : postfixes) {
			if (key.endsWith(qsl("_") + postfix)) {
				key = key.replace(qsl("_") + postfix, qsl("#") + postfix);
				break;
			}
		}

		if (key.endsWith("_PC")) {
			key = key.replace("_PC", "");
		}

		if (val.contains(qsl("%1$d")) && !val.contains(qsl("%2$d"))) {
			val = val.replace(qsl("%1$d"), qsl("{count}"));
		} else if (val.contains(qsl("%1$d")) && val.contains(qsl("%2$d"))) {
			val = val.replace(qsl("%1$d"), qsl("{count1}")).replace(qsl("%2$d"), qsl("{count2}"));
		} else if (val.contains(qsl("%1$s")) && !val.contains(qsl("%2$s"))) {
			val = val.replace(qsl("%1$s"), qsl("{item}"));
		} else if (val.contains(qsl("%1$s")) && val.contains(qsl("%2$s"))) {
			val = val.replace(qsl("%1$s"), qsl("{item1}")).replace(qsl("%2$s"), qsl("{item2}"));
		}

		Lang::GetInstance().resetValue(key.toUtf8());
		Lang::GetInstance().applyValue(key.toUtf8(), val.toUtf8());
	}
	Lang::GetInstance().updatePluralRules();
	Lang::GetInstance().notifyUpdated();
}
