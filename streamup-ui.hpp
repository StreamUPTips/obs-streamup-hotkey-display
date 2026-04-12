#pragma once

#include <QAbstractButton>
#include <QDialog>
#include <QEvent>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPushButton>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWindow>
#include <cmath>

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
		setFixedSize(54, 22);
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

	QSize sizeHint() const override { return QSize(54, 22); }

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

		int margin = 2;
		int knobH = height() - margin * 2;
		int knobW = 32;
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
		lay->setContentsMargins(0, 2, 0, 2);
		lay->setSpacing(10);
		switchBtn = new SwitchButton(this);
		QLabel *lbl = new QLabel(text, this);
		lbl->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: 500;").arg(C_TEXT));
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
		QPainterPath borderPath;
		borderPath.addRoundedRect(QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5), m_radius, m_radius);
		painter.setPen(QPen(QColor(255, 255, 255, 15), 1));
		painter.drawPath(borderPath);
		QFrame::paintEvent(event);
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

	QVBoxLayout *outerLay = new QVBoxLayout(dialog);
	outerLay->setContentsMargins(1, 1, 1, 1);
	outerLay->setSpacing(0);

	RoundedContainer *container = new RoundedContainer(14);
	QVBoxLayout *mainLay = new QVBoxLayout(container);
	mainLay->setContentsMargins(0, 0, 0, 0);
	mainLay->setSpacing(0);

	QWidget *header = new QWidget();
	header->setFixedHeight(44);
	header->setStyleSheet(QString("background: transparent; border-bottom: 1px solid %1;").arg(C_BORDER));
	QHBoxLayout *headerLay = new QHBoxLayout(header);
	headerLay->setContentsMargins(18, 0, 10, 0);

	QLabel *titleLabel = new QLabel(title);
	titleLabel->setStyleSheet(
		QString("color: %1; font-size: 14px; font-weight: bold; "
			"font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif;")
			.arg(C_TEXT));
	headerLay->addWidget(titleLabel);
	headerLay->addStretch();

	QToolButton *closeBtn = new QToolButton();
	closeBtn->setText(QString::fromUtf8("\xC3\x97"));
	closeBtn->setFixedSize(28, 28);
	closeBtn->setCursor(Qt::PointingHandCursor);
	closeBtn->setFont(QFont("Arial", 14));
	closeBtn->setAutoRaise(true);
	closeBtn->setStyleSheet("QToolButton { color: " C_TEXT "; background: rgba(255,255,255,0.06); border-radius: 6px; }"
				"QToolButton:hover { color: " C_DANGER "; background: rgba(255,69,58,0.3); }");
	headerLay->addWidget(closeBtn);
	QObject::connect(closeBtn, &QToolButton::clicked, dialog, &QDialog::reject);

	header->installEventFilter(new DragFilter(dialog));
	mainLay->addWidget(header);

	QWidget *content = new QWidget();
	content->setStyleSheet(QString("background: %1;").arg(C_BG));
	QVBoxLayout *contentLayout = new QVBoxLayout(content);
	contentLayout->setContentsMargins(20, 16, 20, 16);
	contentLayout->setSpacing(14);
	mainLay->addWidget(content, 1);

	QWidget *footer = new QWidget();
	footer->setStyleSheet(QString("background: %1; border-top: 1px solid %2;").arg(C_BG, C_BORDER));
	QVBoxLayout *footerLayout = new QVBoxLayout(footer);
	footerLayout->setContentsMargins(20, 12, 20, 12);
	footerLayout->setSpacing(8);
	mainLay->addWidget(footer, 0);

	outerLay->addWidget(container);

	return {contentLayout, footerLayout};
}

// --------------- Buttons (pill shape) ---------------

inline QPushButton *CreateStyledButton(const QString &text, const QString &type, int height = 28)
{
	QPushButton *btn = new QPushButton(text);
	btn->setCursor(Qt::PointingHandCursor);
	btn->setFixedHeight(height);

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

	btn->setStyleSheet(style);
	return btn;
}

// --------------- Group Boxes ---------------

inline QString GetGroupBoxStyle()
{
	return QString("QGroupBox {"
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
		.arg(C_TEXT);
}

// --------------- Input Fields (matching StreamUP) ---------------

inline QString GetInputStyle()
{
	return QString("QLineEdit { background-color: %1; border: none; border-radius: 8px; "
		       "padding: 2px 2px; color: %2; font-weight: 500; font-size: 13px; min-height: 20px; "
		       "selection-background-color: rgba(0,118,223,0.3); selection-color: %2; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif; }"
		       "QLineEdit:focus { border: 2px solid %3; outline: none; }")
		.arg(C_CARD)
		.arg(C_TEXT)
		.arg(C_PRI);
}

inline QString GetPlainTextEditStyle()
{
	return QString("QPlainTextEdit { background-color: %1; border: none; border-radius: 8px; "
		       "padding: 2px 2px; color: %2; font-weight: 500; font-size: 13px; "
		       "selection-background-color: rgba(0,118,223,0.3); selection-color: %2; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif; }"
		       "QPlainTextEdit:focus { border: 2px solid %3; outline: none; }")
		.arg(C_CARD)
		.arg(C_TEXT)
		.arg(C_PRI);
}

// --------------- ComboBox (matching StreamUP) ---------------

inline QString GetComboBoxStyle()
{
	return QString("QComboBox { background-color: %1; border: none; border-radius: 8px; "
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
		.arg(C_BORDER_MED);
}

// --------------- SpinBox (matching input style) ---------------

inline QString GetSpinBoxStyle()
{
	return QString("QSpinBox, QDoubleSpinBox { background-color: %1; border: none; border-radius: 8px; "
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
		.arg(C_PRI);
}

// --------------- List Widget ---------------

inline QString GetListWidgetStyle()
{
	return QString("QListWidget { background: #181825; border: none; outline: none; border-radius: 8px; }"
		       "QListWidget::item { color: %1; padding: 5px 10px; border: none; border-radius: 0px; }"
		       "QListWidget::item:hover { background: rgba(255,255,255,0.04); }"
		       "QListWidget::item:selected { background: rgba(0,118,223,0.08); border-left: 2px solid %2; }"
		       "QScrollBar:vertical { width: 6px; background: transparent; }"
		       "QScrollBar::handle:vertical { background: rgba(0,118,223,0.25); border-radius: 3px; min-height: 20px; }"
		       "QScrollBar::handle:vertical:hover { background: rgba(0,118,223,0.45); }"
		       "QScrollBar::add-line, QScrollBar::sub-line { height: 0; }"
		       "QScrollBar::add-page, QScrollBar::sub-page { background: transparent; }")
		.arg(C_TEXT)
		.arg(C_PRI);
}

// --------------- Labels ---------------

inline QString GetLabelStyle()
{
	return QString("color: %1; font-size: 13px; font-weight: 500; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif;")
		.arg(C_TEXT);
}

inline QString GetDimLabelStyle()
{
	return QString("color: %1; font-size: 11px; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif;")
		.arg(C_DIM);
}

inline QString GetFormLabelStyle()
{
	return QString("color: %1; font-size: 10px; font-weight: bold; text-transform: uppercase; "
		       "font-family: Roboto, 'Open Sans', '.AppleSystemUIFont', Helvetica, Arial, sans-serif;")
		.arg(C_DIM);
}
