#pragma once

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <obs-frontend-api.h>
#include "streamup-hotkey-display-dock.hpp"
#include <streamup/ui/window-chrome.hpp> // ShadowDialog, applyChrome, WindowShell
#include <streamup/ui/switch-button.hpp> // SwitchButton, CreateStyledSwitch

class StreamupHotkeyDisplaySettings : public StreamUP::UIStyles::ShadowDialog {
	Q_OBJECT

public:
	StreamupHotkeyDisplaySettings(HotkeyDisplayDock *dock, QWidget *parent);
	void LoadSettings(obs_data_t *settings);
	void SaveSettings();

	void PopulateSceneComboBox();
	void PopulateSourceComboBox(const QString &sceneName);

	QString sceneName;
	QString textSource;
	int onScreenTime;
	bool displayInTextSource;

	// Single key capture settings
	bool captureNumpad;
	bool captureNumbers;
	bool captureLetters;
	bool capturePunctuation;
	bool captureStandaloneMouse;
	QString whitelistedKeys;

	// Logging settings
	bool enableLogging;

private:
	HotkeyDisplayDock *hotkeyDisplayDock;
	QVBoxLayout *mainLayout;
	QHBoxLayout *sceneLayout;
	QHBoxLayout *sourceLayout;
	QHBoxLayout *prefixLayout;
	QHBoxLayout *suffixLayout;
	QLabel *sceneLabel;
	QLabel *sourceLabel;
	QLabel *timeLabel;
	QLabel *prefixLabel;
	QLabel *suffixLabel;
	QLineEdit *prefixLineEdit;
	QLineEdit *suffixLineEdit;
	QComboBox *sceneComboBox;
	QComboBox *sourceComboBox;
	QSpinBox *timeSpinBox;
	QPushButton *applyButton;
	QPushButton *closeButton;
	StreamUP::UIStyles::SwitchButton *displayInTextSourceCheckBox;
	QWidget *textSourceGroupBox;

	// Single key capture UI elements
	QWidget *singleKeyGroupBox;
	StreamUP::UIStyles::SwitchButton *captureNumpadCheckBox;
	StreamUP::UIStyles::SwitchButton *captureNumbersCheckBox;
	StreamUP::UIStyles::SwitchButton *captureLettersCheckBox;
	StreamUP::UIStyles::SwitchButton *capturePunctuationCheckBox;
	StreamUP::UIStyles::SwitchButton *captureStandaloneMouseCheckBox;
	QLabel *whitelistLabel;
	QPlainTextEdit *whitelistTextEdit;

	// Display settings group
	QWidget *displayGroupBox;

	// Display format UI elements
	QLabel *separatorLabel;
	QLineEdit *separatorLineEdit;
	QLabel *maxHistoryLabel;
	QSpinBox *maxHistorySpinBox;

	// Logging UI elements
	StreamUP::UIStyles::SwitchButton *enableLoggingCheckBox;

private slots:
	void applySettings();
	void onSceneChanged(const QString &sceneName);
	void onDisplayInTextSourceToggled(bool checked);
};
