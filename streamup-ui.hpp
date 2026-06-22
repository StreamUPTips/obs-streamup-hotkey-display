#pragma once

#include <QAbstractButton>
#include <QApplication>
#include <QDialog>
#include <QEvent>
#include <QFont>
#include <QFontInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QRegularExpression>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWindow>
#include <algorithm>
#include <cmath>

// ── UI scale ──
// Qt already maps logical pixels through the per-monitor display scale
// (OBS runs with HighDpiScaleFactorRoundingPolicy::PassThrough), so widget
// geometry tracks the monitor's scale automatically. What it does NOT track
// is the OS text-size preference (Windows "Make text bigger", macOS larger
// system font): that only shows up in the default application font. OBS's
// own UI follows that font; our hardcoded px sizes don't. ui_scale() derives
// a factor from the default font so our dialogs grow with it too.
inline qreal ui_scale()
{
	static const qreal s = []() {
		// Baseline: Windows Segoe UI 9pt = 12 logical px at 100% text size.
#ifdef __APPLE__
		const qreal baseline = 13.0; // macOS system font 13pt
#else
		const qreal baseline = 12.0;
#endif
		const int px = QFontInfo(QApplication::font()).pixelSize();
		if (px <= 0)
			return 1.0;
		return std::clamp(px / baseline, 1.0, 3.0);
	}();
	return s;
}

// Scale a logical-pixel design size by the text-size factor.
inline int S(int px)
{
	return (int)std::lround(px * ui_scale());
}

// Scale every "<N>px" value inside a stylesheet by the text-size factor.
// Lets existing stylesheets stay readable (literal design px) while still
// honouring the OS text-size preference. No-op at 100%.
inline QString scale_qss(const QString &qss)
{
	if (ui_scale() <= 1.0)
		return qss;
	static const QRegularExpression rx(QStringLiteral("(-?\\d+)px"));
	QString out;
	out.reserve(qss.size() + 16);
	qsizetype last = 0;
	auto it = rx.globalMatch(qss);
	while (it.hasNext()) {
		const QRegularExpressionMatch m = it.next();
		out += qss.mid(last, m.capturedStart() - last);
		out += QString::number(S(m.captured(1).toInt()));
		out += QStringLiteral("px");
		last = m.capturedEnd();
	}
	out += qss.mid(last);
	return out;
}

#define C_BG "#1e1e2e"
#define C_CARD "#272738"
#define C_CODE "#1a1a2a"
#define C_PRI "#0076df"
#define C_PRI_HOVER "#0071e3"
#define C_TAG "#89b4fa"
#define C_EX "#a6e3a1"
#define C_TEXT "#cdd6f4"
#define C_TEXT2 "#bac2de"
#define C_DIM "#6c7086"
#define C_BORDER "rgba(255,255,255,0.06)"
#define C_BORDER_MED "rgba(255,255,255,0.15)"
#define C_DANGER "#ff453a"
#define C_HOVER "rgba(49,50,68,0.6)"

// --------------- SwitchButton (iOS-style toggle) ---------------

class SwitchButton : public QAbstractButton {
	float m_knobPos = 0;
	float m_targetPos = 0;
	QTimer *m_anim;

public:
	SwitchButton(QWidget *parent = nullptr) : QAbstractButton(parent)
	{
		setCheckable(true);
		setFixedSize(S(54), S(22));
		setCursor(Qt::PointingHandCursor);

		m_anim = new QTimer(this);
		m_anim->setInterval(16);
		connect(m_anim, &QTimer::timeout, this, [this]() {
			float diff = m_targetPos - m_knobPos;
			if (std::fabs(diff) < 0.01f) {
				m_knobPos = m_targetPos;
				m_anim->stop();
			} else {
				m_knobPos += diff * 0.25f;
			}
			update();
		});

		connect(this, &QAbstractButton::toggled, this, [this](bool checked) {
			m_targetPos = checked ? 1.0f : 0.0f;
			if (!m_anim->isActive())
				m_anim->start();
		});
	}

	QSize sizeHint() const override { return QSize(S(54), S(22)); }

protected:
	void paintEvent(QPaintEvent *) override
	{
		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing);

		int trackR = height() / 2;
		QColor trackOff(0x3A, 0x3A, 0x3D);
		QColor trackOn(0x65, 0xC4, 0x66);

		float t = m_knobPos;
		QColor track;
		track.setRedF(trackOff.redF() + (trackOn.redF() - trackOff.redF()) * t);
		track.setGreenF(trackOff.greenF() + (trackOn.greenF() - trackOff.greenF()) * t);
		track.setBlueF(trackOff.blueF() + (trackOn.blueF() - trackOff.blueF()) * t);
		if (underMouse())
			track = track.lighter(110);

		p.setPen(Qt::NoPen);
		p.setBrush(track);
		p.drawRoundedRect(QRectF(0, 0, width(), height()), trackR, trackR);

		int margin = S(2);
		int knobH = height() - margin * 2;
		int knobW = S(32);
		float maxTravel = width() - knobW - margin * 2;
		float x = margin + t * maxTravel;

		QLinearGradient grad(QPointF(x, margin), QPointF(x, margin + knobH));
		grad.setColorAt(0, QColor(0xFF, 0xFF, 0xFF));
		grad.setColorAt(1, QColor(0xF8, 0xF9, 0xFA));
		p.setBrush(grad);
		p.drawRoundedRect(QRectF(x, margin, knobW, knobH), knobH / 2.0, knobH / 2.0);
	}
};

// --------------- SwitchWidget (toggle + label row) ---------------

class SwitchWidget : public QWidget {
public:
	SwitchButton *switchBtn;

	SwitchWidget(const QString &text, QWidget *parent = nullptr) : QWidget(parent)
	{
		QHBoxLayout *lay = new QHBoxLayout(this);
		lay->setContentsMargins(0, S(2), 0, S(2));
		lay->setSpacing(S(10));
		switchBtn = new SwitchButton(this);
		QLabel *lbl = new QLabel(text, this);
		lbl->setStyleSheet(scale_qss(QString("color: %1; font-size: 13px; font-weight: 500;").arg(C_TEXT)));
		lay->addWidget(switchBtn);
		lay->addWidget(lbl);
		lay->addStretch();
	}

	bool isChecked() const { return switchBtn->isChecked(); }
	void setChecked(bool checked) { switchBtn->setChecked(checked); }

	void setToolTip(const QString &tip)
	{
		QWidget::setToolTip(tip);
		switchBtn->setToolTip(tip);
	}
};

// --------------- RoundedContainer ---------------

class RoundedContainer : public QFrame {
	int m_radius;

public:
	RoundedContainer(int radius, QWidget *parent = nullptr) : QFrame(parent), m_radius(radius) {}

	void resizeEvent(QResizeEvent *event) override
	{
		QFrame::resizeEvent(event);
		QPainterPath path;
		path.addRoundedRect(QRectF(rect()), m_radius, m_radius);
		setMask(path.toFillPolygon().toPolygon());
	}

	void paintEvent(QPaintEvent *event) override
	{
		QPainter painter(this);
		painter.setRenderHint(QPainter::Antialiasing);
		QPainterPath path;
		path.addRoundedRect(QRectF(rect()), m_radius, m_radius);
		painter.fillPath(path, QColor(C_BG));
		// Fill only — no border stroke. The elevation shadow is the edge; a
		// painted hairline would wrap the header/footer corners like a frame.
		QFrame::paintEvent(event);
	}
};

// --------------- ShadowDialog ---------------
// Host for the elevation shadow: concentric rounded-rect rings painted into a
// transparent margin around the card (no QGraphicsDropShadowEffect — it fights
// setMask and re-rasterises on repaints). ApplyDialogChrome reserves
// kShadowMargin in the outer layout when the dialog is a ShadowDialog.

class ShadowDialog : public QDialog {
public:
	static constexpr int kShadowMargin = 20; // transparent canvas for the shadow
	using QDialog::QDialog;

protected:
	void paintEvent(QPaintEvent *) override
	{
		QPainter p(this);
		p.setRenderHint(QPainter::Antialiasing);
		p.setPen(Qt::NoPen);
		const QRectF card = QRectF(rect()).adjusted(
			kShadowMargin, kShadowMargin, -kShadowMargin, -kShadowMargin);
		for (int i = kShadowMargin; i >= 1; --i) {
			const qreal t = (qreal)i / kShadowMargin;
			const int alpha = (int)(10.0 * (1.0 - t) * (1.0 - t) + 0.5);
			if (alpha <= 0)
				continue;
			QPainterPath ring;
			ring.addRoundedRect(card.adjusted(-i, -i + 2, i, i + 2), 14 + i, 14 + i);
			p.fillPath(ring, QColor(0, 0, 0, alpha));
		}
	}
};

// --------------- DragFilter ---------------

class DragFilter : public QObject {
	QPoint dragPos;
	bool dragging = false;

public:
	using QObject::QObject;
	bool eventFilter(QObject *obj, QEvent *event) override
	{
		Q_UNUSED(obj);
		QWidget *dlg = qobject_cast<QWidget *>(parent());
		if (!dlg)
			return false;
		if (event->type() == QEvent::MouseButtonPress) {
			QMouseEvent *me = static_cast<QMouseEvent *>(event);
			if (me->button() == Qt::LeftButton) {
				QWindow *wh = dlg->windowHandle();
				if (wh && wh->startSystemMove())
					return true;
				dragPos = me->globalPosition().toPoint() - dlg->frameGeometry().topLeft();
				dragging = true;
				return true;
			}
		} else if (event->type() == QEvent::MouseMove && dragging) {
			QMouseEvent *me = static_cast<QMouseEvent *>(event);
			dlg->move(me->globalPosition().toPoint() - dragPos);
			return true;
		} else if (event->type() == QEvent::MouseButtonRelease) {
			dragging = false;
		}
		return false;
	}
};

// --------------- Dialog Chrome ---------------

struct DialogChrome {
	QVBoxLayout *contentLayout;
	QVBoxLayout *footerLayout;
};

inline DialogChrome ApplyDialogChrome(QDialog *dialog, const QString &title)
{
	dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
	dialog->setAttribute(Qt::WA_TranslucentBackground);

	// ShadowDialogs reserve a transparent margin that the elevation shadow
	// paints into; a plain QDialog keeps the old 1px margin so nothing renders
	// a dead transparent band.
	const int margin = dynamic_cast<ShadowDialog *>(dialog) ? ShadowDialog::kShadowMargin : 1;
	QVBoxLayout *outerLay = new QVBoxLayout(dialog);
	outerLay->setContentsMargins(margin, margin, margin, margin);
	outerLay->setSpacing(0);

	RoundedContainer *container = new RoundedContainer(S(14));
	QVBoxLayout *mainLay = new QVBoxLayout(container);
	mainLay->setContentsMargins(0, 0, 0, 0);
	mainLay->setSpacing(0);

	// Divider scoped by object name so the border doesn't propagate to the
	// title label / close button.
	QWidget *header = new QWidget();
	header->setObjectName("dlgHeader");
	header->setFixedHeight(S(44));
	header->setStyleSheet(scale_qss(QString("QWidget#dlgHeader { background: transparent; border-bottom: 1px solid %1; }").arg(C_BORDER)));
	QHBoxLayout *headerLay = new QHBoxLayout(header);
	headerLay->setContentsMargins(S(18), 0, S(10), 0);

	QLabel *titleLabel = new QLabel(title);
	titleLabel->setStyleSheet(scale_qss(
		QString("color: %1; font-size: 14px; font-weight: bold; "
			"font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif;")
			.arg(C_TEXT)));
	headerLay->addWidget(titleLabel);
	headerLay->addStretch();

	QToolButton *closeBtn = new QToolButton();
	closeBtn->setText(QString::fromUtf8("\xC3\x97"));
	closeBtn->setFixedSize(S(28), S(28));
	closeBtn->setCursor(Qt::PointingHandCursor);
	closeBtn->setFont(QFont("Arial", 14));
	closeBtn->setAutoRaise(true);
	closeBtn->setStyleSheet(scale_qss(QString("QToolButton { color: " C_TEXT "; background: rgba(255,255,255,0.06); border-radius: 6px; }"
				"QToolButton:hover { color: " C_DANGER "; background: rgba(255,69,58,0.3); }")));
	headerLay->addWidget(closeBtn);
	QObject::connect(closeBtn, &QToolButton::clicked, dialog, &QDialog::reject);

	header->installEventFilter(new DragFilter(dialog));
	mainLay->addWidget(header);

	QWidget *content = new QWidget();
	content->setStyleSheet(QString("background: %1;").arg(C_BG));
	QVBoxLayout *contentLayout = new QVBoxLayout(content);
	contentLayout->setContentsMargins(S(20), S(16), S(20), S(16));
	contentLayout->setSpacing(S(14));
	mainLay->addWidget(content, 1);

	QWidget *footer = new QWidget();
	footer->setObjectName("dlgFooter");
	footer->setStyleSheet(scale_qss(QString("QWidget#dlgFooter { background: %1; border-top: 1px solid %2; }").arg(C_BG, C_BORDER)));
	QVBoxLayout *footerLayout = new QVBoxLayout(footer);
	footerLayout->setContentsMargins(S(20), S(12), S(20), S(12));
	footerLayout->setSpacing(S(8));
	mainLay->addWidget(footer, 0);

	outerLay->addWidget(container);

	return {contentLayout, footerLayout};
}

// --------------- Buttons (pill shape) ---------------

inline QPushButton *CreateStyledButton(const QString &text, const QString &type, int height = 28)
{
	QPushButton *btn = new QPushButton(text);
	btn->setCursor(Qt::PointingHandCursor);
	btn->setFixedHeight(S(height));

	int radius = (height + 2) / 2;

	QString base = QString("min-height: %1px; max-height: %1px; border-radius: %2px; "
			       "padding: 0px 14px; font-weight: bold; outline: none; "
			       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif; "
			       "font-size: 11px; min-width: 80px; ")
			       .arg(height)
			       .arg(radius);

	QString style;
	if (type == "primary") {
		style = QString("QPushButton { %1 background: %2; border: 1px solid %2; color: white; }"
				"QPushButton:hover { background: %3; border: 1px solid %3; }"
				"QPushButton:pressed { background: #005abb; border: 1px solid #005abb; }"
				"QPushButton:disabled { background: transparent; border: 1px solid %4; color: rgba(205,214,244,0.4); }")
				.arg(base, C_PRI, C_PRI_HOVER, C_BORDER);
	} else if (type == "danger") {
		style = QString("QPushButton { %1 background: transparent; border: 1px solid %2; color: %2; }"
				"QPushButton:hover { background: %2; color: white; }"
				"QPushButton:pressed { background: rgba(255,69,58,0.8); }"
				"QPushButton:disabled { background: transparent; border: 1px solid %3; color: rgba(205,214,244,0.4); }")
				.arg(base, C_DANGER, C_BORDER);
	} else if (type == "primary-outline") {
		style = QString("QPushButton { %1 background: transparent; border: 1px solid %2; color: %2; }"
				"QPushButton:hover { background: %2; color: white; }"
				"QPushButton:pressed { background: %3; }"
				"QPushButton:disabled { background: transparent; border: 1px solid %4; color: rgba(205,214,244,0.4); }")
				.arg(base, C_PRI, C_PRI_HOVER, C_BORDER);
	} else if (type == "success") {
		style = QString("QPushButton { %1 background: transparent; border: 1px solid %2; color: %2; }"
				"QPushButton:hover { background: %2; color: white; }"
				"QPushButton:pressed { background: rgba(166,227,161,0.8); }"
				"QPushButton:disabled { background: transparent; border: 1px solid %3; color: rgba(205,214,244,0.4); }")
				.arg(base, C_EX, C_BORDER);
	} else {
		style = QString("QPushButton { %1 background: transparent; border: 1px solid %2; color: %3; }"
				"QPushButton:hover { background: rgba(255,255,255,0.04); }"
				"QPushButton:pressed { background: rgba(255,255,255,0.02); }"
				"QPushButton:disabled { background: transparent; border: 1px solid %4; color: rgba(205,214,244,0.4); }")
				.arg(base, C_DIM, C_TEXT2, C_BORDER);
	}

	btn->setStyleSheet(scale_qss(style));
	return btn;
}

// --------------- Group Boxes ---------------

inline QString GetGroupBoxStyle()
{
	return scale_qss(QString("QGroupBox {"
		       "    padding: 0px; padding-top: 22px; margin: 0px;"
		       "    border: none; border-radius: 0px;"
		       "    background: transparent;"
		       "}"
		       "QGroupBox::title {"
		       "    color: %1;"
		       "    padding: 0px 0px 4px 0px;"
		       "    font-size: 13px; font-weight: 700;"
		       "    subcontrol-origin: margin;"
		       "    subcontrol-position: top left;"
		       "}")
		.arg(C_TEXT));
}

// --------------- Input Fields (matching StreamUP) ---------------

inline QString GetInputStyle()
{
	return scale_qss(QString("QLineEdit { background-color: %1; border: none; border-radius: 8px; "
		       "padding: 2px 2px; color: %2; font-weight: 500; font-size: 13px; min-height: 20px; "
		       "selection-background-color: rgba(0,118,223,0.3); selection-color: %2; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif; }"
		       "QLineEdit:focus { border: 2px solid %3; outline: none; }")
		.arg(C_CARD)
		.arg(C_TEXT)
		.arg(C_PRI));
}

inline QString GetPlainTextEditStyle()
{
	return scale_qss(QString("QPlainTextEdit { background-color: %1; border: none; border-radius: 8px; "
		       "padding: 2px 2px; color: %2; font-weight: 500; font-size: 13px; "
		       "selection-background-color: rgba(0,118,223,0.3); selection-color: %2; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif; }"
		       "QPlainTextEdit:focus { border: 2px solid %3; outline: none; }")
		.arg(C_CARD)
		.arg(C_TEXT)
		.arg(C_PRI));
}

// --------------- ComboBox (matching StreamUP) ---------------

inline QString GetComboBoxStyle()
{
	return scale_qss(QString("QComboBox { background-color: %1; border: none; border-radius: 8px; "
		       "padding: 2px 28px 2px 12px; margin: 2px; color: %2; font-weight: 500; font-size: 13px; "
		       "min-height: 20px; max-height: 20px; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif; }"
		       "QComboBox:hover, QComboBox:focus, QComboBox:on { background-color: %3; border-radius: 8px; outline: none; }"
		       "QComboBox::drop-down { subcontrol-origin: padding; subcontrol-position: top right; "
		       "border: none; width: 20px; background-color: %4; "
		       "border-top-right-radius: 8px; border-bottom-right-radius: 8px; }"
		       "QComboBox::down-arrow { image: url(:/images/icons/ui/dropdown-arrow-light.svg); width: 12px; height: 14px; }"
		       "QComboBox::down-arrow:on { image: url(:/images/icons/ui/dropdown-arrow-light.svg); }"
		       "QComboBox QAbstractItemView { background-color: rgba(30,30,46,0.95); "
		       "border: 1px solid %5; border-radius: 14px; "
		       "selection-background-color: rgba(0,118,223,0.3); color: %2; padding: 8px; "
		       "font-weight: 600; outline: none; show-decoration-selected: 0; }"
		       "QComboBox QAbstractItemView QAbstractScrollArea::corner { background: transparent; border: none; }"
		       "QComboBox QAbstractScrollArea::corner { background: transparent; border: none; }"
		       "QComboBox QAbstractItemView::item { padding: 2px 10px; border-radius: 6px; "
		       "margin: 1px 2px; border: none; outline: none; }"
		       "QComboBox QAbstractItemView::item:selected { background-color: %4; color: %2; outline: none; }"
		       "QScrollBar:vertical { width: 6px; background: transparent; border: none; margin: 0px; }"
		       "QScrollBar::handle:vertical { background: rgba(0,118,223,0.25); border: none; border-radius: 3px; min-height: 20px; }"
		       "QScrollBar::handle:vertical:hover { background: rgba(0,118,223,0.45); }"
		       "QScrollBar::add-line, QScrollBar::sub-line { height: 0; }"
		       "QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }")
		.arg(C_BG)
		.arg(C_TEXT)
		.arg(C_HOVER)
		.arg(C_PRI)
		.arg(C_BORDER_MED));
}

// --------------- SpinBox (matching input style) ---------------

inline QString GetSpinBoxStyle()
{
	return scale_qss(QString("QSpinBox, QDoubleSpinBox { background-color: %1; border: none; border-radius: 8px; "
		       "padding: 2px 28px 2px 12px; margin: 2px; color: %2; font-weight: 500; font-size: 13px; "
		       "min-height: 20px; max-height: 20px; "
		       "selection-background-color: rgba(0,118,223,0.3); selection-color: %2; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif; }"
		       "QSpinBox:hover, QDoubleSpinBox:hover, QSpinBox:focus, QDoubleSpinBox:focus { "
		       "background-color: %3; outline: none; }"
		       "QSpinBox::up-button, QSpinBox::down-button, QDoubleSpinBox::up-button, QDoubleSpinBox::down-button { "
		       "border: none; background-color: %4; width: 20px; }"
		       "QSpinBox::up-button, QDoubleSpinBox::up-button { border-top-right-radius: 8px; }"
		       "QSpinBox::down-button, QDoubleSpinBox::down-button { border-bottom-right-radius: 8px; }")
		.arg(C_CARD)
		.arg(C_TEXT)
		.arg(C_HOVER)
		.arg(C_PRI));
}

// --------------- List Widget ---------------

inline QString GetListWidgetStyle()
{
	return scale_qss(QString("QListWidget { background: #181825; border: none; outline: none; border-radius: 8px; }"
		       "QListWidget::item { color: %1; padding: 5px 10px; border: none; border-radius: 0px; }"
		       "QListWidget::item:hover { background: rgba(255,255,255,0.04); }"
		       "QListWidget::item:selected { background: rgba(0,118,223,0.08); border-left: 2px solid %2; }"
		       "QScrollBar:vertical { width: 6px; background: transparent; }"
		       "QScrollBar::handle:vertical { background: rgba(0,118,223,0.25); border-radius: 3px; min-height: 20px; }"
		       "QScrollBar::handle:vertical:hover { background: rgba(0,118,223,0.45); }"
		       "QScrollBar::add-line, QScrollBar::sub-line { height: 0; }"
		       "QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }")
		.arg(C_TEXT)
		.arg(C_PRI));
}

// --------------- Labels ---------------

inline QString GetLabelStyle()
{
	return scale_qss(QString("color: %1; font-size: 13px; font-weight: 500; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif;")
		.arg(C_TEXT));
}

inline QString GetDimLabelStyle()
{
	return scale_qss(QString("color: %1; font-size: 11px; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif;")
		.arg(C_DIM));
}

inline QString GetFormLabelStyle()
{
	return scale_qss(QString("color: %1; font-size: 10px; font-weight: bold; text-transform: uppercase; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif;")
		.arg(C_DIM));
}
