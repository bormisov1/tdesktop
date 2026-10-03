/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#include "history/view/controls/history_view_privacy_toggles.h"

#include "base/assertion.h"
#include "data/data_peer.h"
#include "main/main_session.h"
#include "main/main_session_settings.h"
#include "ui/widgets/checkbox.h"

#include <algorithm>

namespace HistoryView {
namespace {

constexpr auto kItemsSkip = 16;

} // namespace

PrivacyTogglesRow::PrivacyTogglesRow(
	not_null<Main::Session*> session,
	QWidget *parent)
: Ui::RpWidget(parent)
, _session(session) {
	setVisible(false);
}

void PrivacyTogglesRow::addToggle(Toggle toggle, const QString &text) {
	const auto button = new Ui::Checkbox(this, text, false);
	button->checkedChanges(
	) | rpl::on_next([=](bool checked) {
		switch (toggle) {
		case Toggle::SendReadReceipts:
			setSendReadReceipts(checked);
			break;
		}
	}, lifetime());
	_items.push_back({ toggle, button });
}

void PrivacyTogglesRow::setPeer(not_null<Data::PeerData*> peer) {
	_peer = peer;
	_displayed = false;
	for (const auto &item : _items) {
		const auto shown = isApplicable(item.toggle, peer);
		_displayed = _displayed || shown;
		item.button->setVisible(shown);
		item.button->setChecked(
			shown && isChecked(item.toggle),
			Ui::Checkbox::NotifyAboutChange::DontNotify);
	}
	setVisible(_displayed);
}

bool PrivacyTogglesRow::isDisplayed() const {
	return _displayed;
}

int PrivacyTogglesRow::rowHeight() const {
	return _displayed ? heightNoMargins() : 0;
}

int PrivacyTogglesRow::resizeGetHeight(int newWidth) {
	const auto shownItems = std::count_if(_items.begin(), _items.end(),
		[](const auto &item) { return !item.button->isHidden(); });
	if (!shownItems) {
		_itemsLeft = 0;
		return 0;
	}
	const auto width = std::max(
		0,
		(newWidth - kItemsSkip * (shownItems - 1)) / int(shownItems));
	auto height = 0;
	for (const auto &item : _items) {
		if (item.button->isHidden()) {
			continue;
		}
		item.button->resizeToWidth(width);
		height = std::max(height, item.button->heightNoMargins());
	}
	const auto totalWidth = width * shownItems + kItemsSkip * (shownItems - 1);
	_itemsLeft = std::max(0, (newWidth - totalWidth) / 2);
	return height;
}

void PrivacyTogglesRow::updateControlsGeometry(QSize size) {
	auto left = _itemsLeft;
	for (const auto &item : _items) {
		if (item.button->isHidden()) {
			continue;
		}
		item.button->moveToLeft(
			left,
			(size.height() - item.button->heightNoMargins()) / 2);
		left += item.button->widthNoMargins() + kItemsSkip;
	}
}

bool PrivacyTogglesRow::isApplicable(
		Toggle toggle,
		not_null<Data::PeerData*> peer) {
	switch (toggle) {
	case Toggle::SendReadReceipts:
		// Read receipts are never sent in broadcast channels.
		return (peer->isUser() || peer->isChat());
	}
	Unexpected("PrivacyTogglesRow::Toggle in isApplicable.");
}

bool PrivacyTogglesRow::isChecked(Toggle toggle) const {
	Expects(_peer.has_value());
	switch (toggle) {
	case Toggle::SendReadReceipts:
		return !_session->settings().noReadReceipts(_peer->id);
	}
	Unexpected("PrivacyTogglesRow::Toggle in isChecked.");
}

void PrivacyTogglesRow::setSendReadReceipts(bool sendReadReceipts) {
	if (!_peer) {
		return;
	}
	auto &settings = _session->settings();
	if (sendReadReceipts) {
		settings.removeNoReadReceipts(_peer->id);
	} else {
		settings.setNoReadReceipts(_peer->id);
	}
	_session->saveSettingsDelayed();
}

} // namespace HistoryView
