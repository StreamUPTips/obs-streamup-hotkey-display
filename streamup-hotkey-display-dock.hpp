#pragma once

#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QAction>
#include <QToolBar>
#include <QTimer>
#include <QListWidget>
#include <obs.h>

// Default value constants
namespace StyleConstants {
constexpr const char *DEFAULT_SCENE_NAME = "Default Scene";
constexpr const char *DEFAULT_TEXT_SOURCE = "Default Text Source";
constexpr const char *NO_TEXT_SOURCE = "No text source available";
constexpr int DEFAULT_ONSCREEN_TIME = 3000;
constexpr int DEFAULT_MAX_HISTORY = 10;
} // namespace StyleConstants

class HotkeyDisplayDock : public QFrame {
	Q_OBJECT

public:
	HotkeyDisplayDock(QWidget *parent = nullptr);
	~HotkeyDisplayDock();

	// setLog is Q_INVOKABLE so it can be called via QMetaObject::invokeMethod
	Q_INVOKABLE void setLog(const QString &log);

	// Hook management
	bool enableHooks();
	void disableHooks();
	void updateUIState(bool enabled);

	// Accessors
	bool isHookEnabled() const { return hookEnabled; }
	void setHookEnabled(bool enabled) { hookEnabled = enabled; }
	QAction *getToggleAction() const { return toggleAction; }
	QLabel *getLabel() const { return label; }

	QString getSceneName() const { return sceneName; }
	void setSceneName(const QString &name) { sceneName = name; }

	QString getTextSource() const { return textSource; }
	void setTextSource(const QString &source) { textSource = source; }

	int getOnScreenTime() const { return onScreenTime; }
	void setOnScreenTime(int ms) { onScreenTime = ms; }

	QString getPrefix() const { return prefix; }
	void setPrefix(const QString &p) { prefix = p; }

	QString getSuffix() const { return suffix; }
	void setSuffix(const QString &s) { suffix = s; }

	bool getDisplayInTextSource() const { return displayInTextSource; }
	void setDisplayInTextSource(bool enabled) { displayInTextSource = enabled; }

	int getMaxHistory() const { return maxHistory; }
	void setMaxHistory(int max) { maxHistory = max; }

public slots:
	void toggleKeyboardHook();
	void openSettings();
	void clearDisplay();

private:
	void setSourceVisibility(bool visible);
	void stopAllActivities();
	void addToHistory(const QString &combination);

	QVBoxLayout *layout;
	QToolBar *toolbar;
	QLabel *label;
	QListWidget *historyList;
	QAction *toggleAction;
	QAction *settingsAction;
	bool hookEnabled;
	QString sceneName;
	QString textSource;
	int onScreenTime;
	QString prefix;
	QString suffix;
	QTimer *clearTimer;
	bool displayInTextSource;
	int maxHistory;
};
