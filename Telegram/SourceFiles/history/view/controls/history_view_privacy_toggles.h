/*
This file is part of Telegram Desktop,
the official desktop application for the Telegram messaging service.

For license and copyright information please follow this link:
https://github.com/telegramdesktop/tdesktop/blob/master/LEGAL
*/
#pragma once

#include "ui/rp_widget.h"

#include <optional>
#include <vector>

namespace Main {
class Session;
} // namespace Main

namespace Data {
class PeerData;
} // namespace Data

namespace Ui {
class Checkbox;
} // namespace Ui

namespace HistoryView {

class PrivacyTogglesRow final : public Ui::RpWidget {
public:
	enum class Toggle {
		SendReadReceipts,
	};

	PrivacyTogglesRow(not_null<Main::Session*> session, QWidget *parent);

	void addToggle(Toggle toggle, const QString &text);

	void setPeer(not_null<Data::PeerData*> peer);

	[[nodiscard]] bool isDisplayed() const;
	[[nodiscard]] int rowHeight() const;

protected:
	int resizeGetHeight(int newWidth) override;
	void updateControlsGeometry(QSize size) override;

private:
	[[nodiscard]] static bool isApplicable(
		Toggle toggle,
		not_null<Data::PeerData*> peer);
	[[nodiscard]] bool isChecked(Toggle toggle) const;
	void setSendReadReceipts(bool sendReadReceipts);

	struct Item {
		Toggle toggle;
		Ui::Checkbox *button = nullptr;
	};

	const not_null<Main::Session*> _session;
	std::optional<not_null<Data::PeerData*>> _peer;
	std::vector<Item> _items;
	bool _displayed = false;
	int _itemsLeft = 0;
};

} // namespace HistoryView