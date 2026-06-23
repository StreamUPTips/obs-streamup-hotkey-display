#include "streamup-hotkey-display-settings.hpp"
#include "streamup-hotkey-display.hpp"
#include <obs-module.h>
#include <streamup/ui/mac-inputs.hpp> // MacComboBox, MacSpinBox
#include <streamup/ui/pill-button.hpp> // PillButton
#include <streamup/ui/labels.hpp>      // labelStyle, sectionHeader
#include <streamup/ui/glass.hpp>       // makeComboAnimated (combo slide-down animation)
#include <streamup/ui/ui-scrollbar.hpp> // useScrollBars (capsule scrollbars)
#include "version.h"                    // PROJECT_VERSION

using namespace StreamUP::UIStyles;

// Labeled switch row — replaces the old SwitchWidget. The canonical SwitchButton
// renders its own inline label, so this is just construct + setText.
static SwitchButton *makeSwitch(const QString &text, QWidget *parent)
{
	auto *s = new SwitchButton(parent);
	s->setText(text);
	return s;
}

StreamupHotkeyDisplaySettings::StreamupHotkeyDisplaySettings(HotkeyDisplayDock *dock, QWidget *parent)
	: ShadowDialog(parent),
	  hotkeyDisplayDock(dock),
	  sceneLayout(new QHBoxLayout()),
	  sourceLayout(new QHBoxLayout()),
	  prefixLayout(new QHBoxLayout()),
	  suffixLayout(new QHBoxLayout()),
	  sceneLabel(new QLabel(obs_module_text("Settings.Label.Scene"), this)),
	  sourceLabel(new QLabel(obs_module_text("Settings.Label.TextSource"), this)),
	  timeLabel(new QLabel(obs_module_text("Settings.Label.OnScreenTime"), this)),
	  prefixLabel(new QLabel(obs_module_text("Settings.Label.Prefix"), this)),
	  suffixLabel(new QLabel(obs_module_text("Settings.Label.Suffix"), this)),
	  prefixLineEdit(new QLineEdit(this)),
	  suffixLineEdit(new QLineEdit(this)),
	  sceneComboBox(new MacComboBox(this)),
	  sourceComboBox(new MacComboBox(this)),
	  timeSpinBox(new MacSpinBox(this)),
	  displayInTextSourceCheckBox(makeSwitch(obs_module_text("Settings.Checkbox.DisplayInTextSource"), this)),
	  textSourceGroupBox(new QWidget(this)),
	  singleKeyGroupBox(new QWidget(this)),
	  captureNumpadCheckBox(makeSwitch(obs_module_text("Settings.Checkbox.CaptureNumpad"), this)),
	  captureNumbersCheckBox(makeSwitch(obs_module_text("Settings.Checkbox.CaptureNumbers"), this)),
	  captureLettersCheckBox(makeSwitch(obs_module_text("Settings.Checkbox.CaptureLetters"), this)),
	  capturePunctuationCheckBox(makeSwitch(obs_module_text("Settings.Checkbox.CapturePunctuation"), this)),
	  captureStandaloneMouseCheckBox(makeSwitch(obs_module_text("Settings.Checkbox.CaptureStandaloneMouse"), this)),
	  whitelistLabel(new QLabel(obs_module_text("Settings.Label.Whitelist"), this)),
	  separatorLabel(new QLabel(obs_module_text("Settings.Label.Separator"), this)),
	  separatorLineEdit(new QLineEdit(this)),
	  maxHistoryLabel(new QLabel(obs_module_text("Settings.Label.MaxHistory"), this)),
	  maxHistorySpinBox(new MacSpinBox(this)),
	  enableLoggingCheckBox(makeSwitch(obs_module_text("Settings.Checkbox.EnableLogging"), this))
{
	// Apply StreamUP dialog chrome (frameless window with custom title bar)
	// brandFooter=true → SoT brand line + taskbar button/thumbnail (WS_EX_APPWINDOW).
	WindowShell chrome = applyChrome(this, obs_module_text("Settings.Title"), "v" PROJECT_VERSION,
					/*brandFooter=*/true, "StreamUP Hotkey Display");
	mainLayout = chrome.content;

	// Create styled buttons
	applyButton = new PillButton(obs_module_text("Settings.Button.Apply"), "primary");
	closeButton = new PillButton(obs_module_text("Settings.Button.Close"), "outline");

	setAccessibleName(obs_module_text("Settings.Title"));
	setAccessibleDescription(obs_module_text("Settings.Description"));

	// Content min-width + both scaled shadow margins (the chrome's outer layout
	// reserves S(kShadowMargin) each side — keep this consistent so it doesn't
	// mix scaled + unscaled).
	setMinimumWidth(S(550) + 2 * S(ShadowDialog::kShadowMargin));

	// Create QPlainTextEdit for whitelist (replaces QLineEdit)
	whitelistTextEdit = new QPlainTextEdit(this);
	whitelistTextEdit->setMinimumHeight(S(80));

	// Create Display Settings group box
	displayGroupBox = new QWidget(this);

	// Sections are plain widgets now (titled by a sectionHeader in their layout);
	// combos/spins are the canonical custom-painted Mac widgets — popup styled for
	// the combos, spins self-paint.
	// Combos: opaque popup + slide-down animation + capsule scrollbar (SoT combo).
	sceneComboBox->setStyleSheet(comboStyle(false));
	sourceComboBox->setStyleSheet(comboStyle(false));
	for (QComboBox *cb : {sceneComboBox, sourceComboBox}) {
		makeComboAnimated(cb);
		useScrollBars(cb->view());
	}

	prefixLineEdit->setStyleSheet(lineEditStyle());
	suffixLineEdit->setStyleSheet(lineEditStyle());
	separatorLineEdit->setStyleSheet(lineEditStyle());

	whitelistTextEdit->setStyleSheet(plainTextStyle());

	sceneLabel->setStyleSheet(labelStyle());
	sourceLabel->setStyleSheet(labelStyle());
	timeLabel->setStyleSheet(labelStyle());
	prefixLabel->setStyleSheet(labelStyle());
	suffixLabel->setStyleSheet(labelStyle());
	whitelistLabel->setStyleSheet(labelStyle());
	separatorLabel->setStyleSheet(labelStyle());
	maxHistoryLabel->setStyleSheet(labelStyle());

	// Configure tooltips and accessibility
	sceneComboBox->setToolTip(obs_module_text("Settings.Tooltip.Scene"));
	sceneComboBox->setAccessibleName(obs_module_text("Settings.Label.Scene"));
	sceneComboBox->setAccessibleDescription(obs_module_text("Settings.Tooltip.Scene"));

	sourceComboBox->setToolTip(obs_module_text("Settings.Tooltip.TextSource"));
	sourceComboBox->setAccessibleName(obs_module_text("Settings.Label.TextSource"));
	sourceComboBox->setAccessibleDescription(obs_module_text("Settings.Tooltip.TextSource"));

	timeSpinBox->setToolTip(obs_module_text("Settings.Tooltip.OnScreenTime"));
	timeSpinBox->setAccessibleName(obs_module_text("Settings.Label.OnScreenTime"));
	timeSpinBox->setAccessibleDescription(obs_module_text("Settings.Tooltip.OnScreenTime"));

	prefixLineEdit->setToolTip(obs_module_text("Settings.Tooltip.Prefix"));
	prefixLineEdit->setAccessibleName(obs_module_text("Settings.Label.Prefix"));
	prefixLineEdit->setAccessibleDescription(obs_module_text("Settings.Tooltip.Prefix"));
	prefixLineEdit->setPlaceholderText(obs_module_text("Settings.Placeholder.Prefix"));

	suffixLineEdit->setToolTip(obs_module_text("Settings.Tooltip.Suffix"));
	suffixLineEdit->setAccessibleName(obs_module_text("Settings.Label.Suffix"));
	suffixLineEdit->setAccessibleDescription(obs_module_text("Settings.Tooltip.Suffix"));
	suffixLineEdit->setPlaceholderText(obs_module_text("Settings.Placeholder.Suffix"));

	applyButton->setToolTip(obs_module_text("Settings.Tooltip.Apply"));
	applyButton->setAccessibleName(obs_module_text("Settings.Button.Apply"));
	applyButton->setAccessibleDescription(obs_module_text("Settings.Tooltip.Apply"));
	applyButton->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));

	closeButton->setToolTip(obs_module_text("Settings.Tooltip.Close"));
	closeButton->setAccessibleName(obs_module_text("Settings.Button.Close"));
	closeButton->setAccessibleDescription(obs_module_text("Settings.Tooltip.Close"));
	closeButton->setShortcut(QKeySequence(Qt::Key_Escape));

	displayInTextSourceCheckBox->setToolTip(obs_module_text("Settings.Tooltip.DisplayInTextSource"));
	displayInTextSourceCheckBox->setAccessibleName(obs_module_text("Settings.Checkbox.DisplayInTextSource"));
	displayInTextSourceCheckBox->setAccessibleDescription(obs_module_text("Settings.Tooltip.DisplayInTextSource"));

	textSourceGroupBox->setAccessibleName(obs_module_text("Settings.Group.TextSource"));
	displayGroupBox->setAccessibleName(obs_module_text("Settings.Group.Display"));

	// Set accessible properties for labels
	sceneLabel->setAccessibleName(obs_module_text("Settings.Label.Scene"));
	sourceLabel->setAccessibleName(obs_module_text("Settings.Label.TextSource"));
	timeLabel->setAccessibleName(obs_module_text("Settings.Label.OnScreenTime"));
	prefixLabel->setAccessibleName(obs_module_text("Settings.Label.Prefix"));
	suffixLabel->setAccessibleName(obs_module_text("Settings.Label.Suffix"));

	// Configure timeSpinBox
	timeSpinBox->setRange(100, 10000);
	timeSpinBox->setSingleStep(1);

	// Populate sceneComboBox
	PopulateSceneComboBox();

	// Add widgets to layouts
	sceneLayout->addWidget(sceneLabel);
	sceneLayout->addWidget(sceneComboBox);

	sourceLayout->addWidget(sourceLabel);
	sourceLayout->addWidget(sourceComboBox);

	prefixLayout->addWidget(prefixLabel);
	prefixLayout->addWidget(prefixLineEdit);

	suffixLayout->addWidget(suffixLabel);
	suffixLayout->addWidget(suffixLineEdit);

	// Create and configure textSourceGroupBox layout
	QVBoxLayout *textSourceLayout = new QVBoxLayout();
	textSourceLayout->addWidget(sectionHeader(obs_module_text("Settings.Group.TextSource")));
	textSourceLayout->addLayout(sceneLayout);
	textSourceLayout->addLayout(sourceLayout);
	textSourceLayout->addLayout(prefixLayout);
	textSourceLayout->addLayout(suffixLayout);
	textSourceGroupBox->setLayout(textSourceLayout);

	QHBoxLayout *timeLayout = new QHBoxLayout();
	timeLayout->addWidget(timeLabel);
	timeLayout->addWidget(timeSpinBox);

	// Create and configure singleKeyGroupBox layout
	QVBoxLayout *singleKeyLayout = new QVBoxLayout();
	singleKeyLayout->addWidget(sectionHeader(obs_module_text("Settings.Group.SingleKeyCapture")));
	singleKeyLayout->addWidget(captureNumpadCheckBox);
	singleKeyLayout->addWidget(captureNumbersCheckBox);
	singleKeyLayout->addWidget(captureLettersCheckBox);
	singleKeyLayout->addWidget(capturePunctuationCheckBox);
	singleKeyLayout->addWidget(captureStandaloneMouseCheckBox);
	singleKeyLayout->addWidget(whitelistLabel);
	singleKeyLayout->addWidget(whitelistTextEdit);
	singleKeyGroupBox->setLayout(singleKeyLayout);

	// Set tooltips for single key capture options
	captureNumpadCheckBox->setToolTip(obs_module_text("Settings.Tooltip.CaptureNumpad"));
	captureNumbersCheckBox->setToolTip(obs_module_text("Settings.Tooltip.CaptureNumbers"));
	captureLettersCheckBox->setToolTip(obs_module_text("Settings.Tooltip.CaptureLetters"));
	capturePunctuationCheckBox->setToolTip(obs_module_text("Settings.Tooltip.CapturePunctuation"));
	captureStandaloneMouseCheckBox->setToolTip(obs_module_text("Settings.Tooltip.CaptureStandaloneMouse"));
	whitelistTextEdit->setToolTip(obs_module_text("Settings.Tooltip.Whitelist"));
	whitelistTextEdit->setPlaceholderText(obs_module_text("Settings.Placeholder.Whitelist"));

	// Set tooltip for logging checkbox
	enableLoggingCheckBox->setToolTip(obs_module_text("Settings.Tooltip.EnableLogging"));

	// Configure separator line edit
	separatorLineEdit->setToolTip(obs_module_text("Settings.Tooltip.Separator"));
	separatorLineEdit->setPlaceholderText(" + ");
	separatorLineEdit->setMaximumWidth(S(80));

	// Configure max history spin box
	maxHistorySpinBox->setToolTip(obs_module_text("Settings.Tooltip.MaxHistory"));
	maxHistorySpinBox->setRange(0, 100);
	maxHistorySpinBox->setSingleStep(1);

	QHBoxLayout *separatorLayout = new QHBoxLayout();
	separatorLayout->addWidget(separatorLabel);
	separatorLayout->addWidget(separatorLineEdit);

	QHBoxLayout *maxHistoryLayout = new QHBoxLayout();
	maxHistoryLayout->addWidget(maxHistoryLabel);
	maxHistoryLayout->addWidget(maxHistorySpinBox);

	// Create and configure displayGroupBox layout
	QVBoxLayout *displayLayout = new QVBoxLayout();
	displayLayout->addWidget(sectionHeader(obs_module_text("Settings.Group.Display")));
	displayLayout->addLayout(separatorLayout);
	displayLayout->addLayout(maxHistoryLayout);
	displayLayout->addLayout(timeLayout);
	displayLayout->addWidget(enableLoggingCheckBox);
	displayGroupBox->setLayout(displayLayout);

	// Two-column layout. applyChrome()'s content area has zero padding (unlike the
	// old ApplyDialogChrome), so the columns set their own content margins.
	QHBoxLayout *columnsLayout = new QHBoxLayout();
	columnsLayout->setContentsMargins(S(20), S(16), S(20), S(16));
	columnsLayout->setSpacing(S(20));

	QVBoxLayout *leftCol = new QVBoxLayout();
	leftCol->setSpacing(S(14));
	QVBoxLayout *rightCol = new QVBoxLayout();
	rightCol->setSpacing(S(14));

	leftCol->addWidget(singleKeyGroupBox);
	leftCol->addStretch();

	rightCol->addWidget(displayGroupBox);
	rightCol->addSpacing(S(8));
	rightCol->addWidget(displayInTextSourceCheckBox);
	rightCol->addWidget(textSourceGroupBox);
	rightCol->addStretch();

	columnsLayout->addLayout(leftCol, 1);
	columnsLayout->addLayout(rightCol, 1);
	mainLayout->addLayout(columnsLayout);

	// Add buttons to footer layout
	// Action buttons go in the footer's right-anchored slot, inline with the
	// brand line (Cancel/Close = outline on the left, primary on the right).
	chrome.footerButtons->addWidget(closeButton);
	chrome.footerButtons->addWidget(applyButton);

	// Set up proper tab order for keyboard navigation
	setTabOrder(captureNumpadCheckBox, captureNumbersCheckBox);
	setTabOrder(captureNumbersCheckBox, captureLettersCheckBox);
	setTabOrder(captureLettersCheckBox, capturePunctuationCheckBox);
	setTabOrder(capturePunctuationCheckBox, whitelistTextEdit);
	setTabOrder(whitelistTextEdit, separatorLineEdit);
	setTabOrder(separatorLineEdit, maxHistorySpinBox);
	setTabOrder(maxHistorySpinBox, timeSpinBox);
	setTabOrder(timeSpinBox, enableLoggingCheckBox);
	setTabOrder(enableLoggingCheckBox, displayInTextSourceCheckBox);
	setTabOrder(displayInTextSourceCheckBox, sceneComboBox);
	setTabOrder(sceneComboBox, sourceComboBox);
	setTabOrder(sourceComboBox, prefixLineEdit);
	setTabOrder(prefixLineEdit, suffixLineEdit);
	setTabOrder(suffixLineEdit, applyButton);
	setTabOrder(applyButton, closeButton);

	// Connect signals to slots
	connect(applyButton, &QPushButton::clicked, this, &StreamupHotkeyDisplaySettings::applySettings);
	connect(closeButton, &QPushButton::clicked, this, &StreamupHotkeyDisplaySettings::close);
	connect(sceneComboBox, &QComboBox::currentTextChanged, this, &StreamupHotkeyDisplaySettings::onSceneChanged);
	connect(displayInTextSourceCheckBox, &SwitchButton::toggled, this,
		&StreamupHotkeyDisplaySettings::onDisplayInTextSourceToggled);

	// Load current settings
	obs_data_t *settings = SaveLoadSettingsCallback(nullptr, false);
	if (settings) {
		LoadSettings(settings);
		obs_data_release(settings);
	}
}

void StreamupHotkeyDisplaySettings::LoadSettings(obs_data_t *settings)
{
	// Existing settings
	sceneName = QString::fromUtf8(obs_data_get_string(settings, "sceneName"));
	sceneComboBox->setCurrentText(sceneName);
	PopulateSourceComboBox(sceneName);
	textSource = QString::fromUtf8(obs_data_get_string(settings, "textSource"));
	sourceComboBox->setCurrentText(textSource);
	onScreenTime = obs_data_get_int(settings, "onScreenTime");
	timeSpinBox->setValue(onScreenTime);
	displayInTextSource = obs_data_get_bool(settings, "displayInTextSource");
	displayInTextSourceCheckBox->setChecked(displayInTextSource);

	// New settings
	QString prefix = QString::fromUtf8(obs_data_get_string(settings, "prefix"));
	prefixLineEdit->setText(prefix);
	QString suffix = QString::fromUtf8(obs_data_get_string(settings, "suffix"));
	suffixLineEdit->setText(suffix);

	// Single key capture settings
	captureNumpad = obs_data_get_bool(settings, "captureNumpad");
	captureNumpadCheckBox->setChecked(captureNumpad);
	captureNumbers = obs_data_get_bool(settings, "captureNumbers");
	captureNumbersCheckBox->setChecked(captureNumbers);
	captureLetters = obs_data_get_bool(settings, "captureLetters");
	captureLettersCheckBox->setChecked(captureLetters);
	capturePunctuation = obs_data_get_bool(settings, "capturePunctuation");
	capturePunctuationCheckBox->setChecked(capturePunctuation);
	captureStandaloneMouse = obs_data_get_bool(settings, "captureStandaloneMouse");
	captureStandaloneMouseCheckBox->setChecked(captureStandaloneMouse);
	whitelistedKeys = QString::fromUtf8(obs_data_get_string(settings, "whitelistedKeys"));
	whitelistTextEdit->setPlainText(whitelistedKeys);

	// Logging settings
	enableLogging = obs_data_get_bool(settings, "enableLogging");
	enableLoggingCheckBox->setChecked(enableLogging);

	// Display format settings
	QString sep = QString::fromUtf8(obs_data_get_string(settings, "keySeparator"));
	separatorLineEdit->setText(sep.isEmpty() ? " + " : sep);
	int maxHist = obs_data_get_int(settings, "maxHistory");
	maxHistorySpinBox->setValue(maxHist > 0 ? maxHist : StyleConstants::DEFAULT_MAX_HISTORY);

	onDisplayInTextSourceToggled(displayInTextSource);
}

void StreamupHotkeyDisplaySettings::SaveSettings()
{
	obs_data_t *settings = obs_data_create();

	// Existing settings
	obs_data_set_string(settings, "sceneName", sceneComboBox->currentText().toUtf8().constData());
	obs_data_set_string(settings, "textSource", sourceComboBox->currentText().toUtf8().constData());
	obs_data_set_int(settings, "onScreenTime", timeSpinBox->value());
	obs_data_set_bool(settings, "displayInTextSource", displayInTextSourceCheckBox->isChecked());

	// New settings
	obs_data_set_string(settings, "prefix", prefixLineEdit->text().toUtf8().constData());
	obs_data_set_string(settings, "suffix", suffixLineEdit->text().toUtf8().constData());

	// Single key capture settings
	obs_data_set_bool(settings, "captureNumpad", captureNumpadCheckBox->isChecked());
	obs_data_set_bool(settings, "captureNumbers", captureNumbersCheckBox->isChecked());
	obs_data_set_bool(settings, "captureLetters", captureLettersCheckBox->isChecked());
	obs_data_set_bool(settings, "capturePunctuation", capturePunctuationCheckBox->isChecked());
	obs_data_set_bool(settings, "captureStandaloneMouse", captureStandaloneMouseCheckBox->isChecked());
	obs_data_set_string(settings, "whitelistedKeys", whitelistTextEdit->toPlainText().toUtf8().constData());

	// Logging settings
	obs_data_set_bool(settings, "enableLogging", enableLoggingCheckBox->isChecked());

	// Display format settings
	obs_data_set_string(settings, "keySeparator", separatorLineEdit->text().toUtf8().constData());
	obs_data_set_int(settings, "maxHistory", maxHistorySpinBox->value());

	// Preserve hookEnabled in saved settings
	if (hotkeyDisplayDock) {
		obs_data_set_bool(settings, "hookEnabled", hotkeyDisplayDock->isHookEnabled());
	}

	SaveLoadSettingsCallback(settings, true);
	obs_data_release(settings);
}

void StreamupHotkeyDisplaySettings::applySettings()
{
	sceneName = sceneComboBox->currentText();
	textSource = sourceComboBox->currentText();
	onScreenTime = timeSpinBox->value();
	displayInTextSource = displayInTextSourceCheckBox->isChecked();
	QString newPrefix = prefixLineEdit->text();
	QString newSuffix = suffixLineEdit->text();

	// Single key capture settings
	captureNumpad = captureNumpadCheckBox->isChecked();
	captureNumbers = captureNumbersCheckBox->isChecked();
	captureLetters = captureLettersCheckBox->isChecked();
	capturePunctuation = capturePunctuationCheckBox->isChecked();
	captureStandaloneMouse = captureStandaloneMouseCheckBox->isChecked();
	whitelistedKeys = whitelistTextEdit->toPlainText();

	// Logging settings
	enableLogging = enableLoggingCheckBox->isChecked();

	SaveSettings();

	if (hotkeyDisplayDock) {
		hotkeyDisplayDock->setSceneName(sceneName);
		hotkeyDisplayDock->setTextSource(textSource);
		hotkeyDisplayDock->setOnScreenTime(onScreenTime);
		hotkeyDisplayDock->setPrefix(newPrefix);
		hotkeyDisplayDock->setSuffix(newSuffix);
		hotkeyDisplayDock->setDisplayInTextSource(displayInTextSource);
		hotkeyDisplayDock->setMaxHistory(maxHistorySpinBox->value());
	}

	// Reload settings to update global single key capture variables
	obs_data_t *reloadedSettings = SaveLoadSettingsCallback(nullptr, false);
	if (reloadedSettings) {
		loadSingleKeyCaptureSettings(reloadedSettings);
		obs_data_release(reloadedSettings);
	}

	accept(); // Close the dialog
}

void StreamupHotkeyDisplaySettings::onSceneChanged(const QString &sceneName)
{
	QString previousSource = sourceComboBox->currentText();
	PopulateSourceComboBox(sceneName);
	if (!previousSource.isEmpty()) {
		sourceComboBox->setCurrentText(previousSource);
	}
}

void StreamupHotkeyDisplaySettings::PopulateSceneComboBox()
{
	sceneComboBox->clear();

	struct obs_frontend_source_list scenes = {{{0}}};
	obs_frontend_get_scenes(&scenes);

	for (size_t i = 0; i < scenes.sources.num; i++) {
		obs_source_t *source = scenes.sources.array[i];
		const char *name = obs_source_get_name(source);
		sceneComboBox->addItem(QString::fromUtf8(name));
	}

	obs_frontend_source_list_free(&scenes);
}

void StreamupHotkeyDisplaySettings::PopulateSourceComboBox(const QString &sceneName)
{
	sourceComboBox->clear();

	obs_source_t *scene = obs_get_source_by_name(sceneName.toUtf8().constData());
	if (!scene) {
		sourceComboBox->addItem(StyleConstants::NO_TEXT_SOURCE);
		return;
	}

	obs_scene_t *sceneAsScene = obs_scene_from_source(scene);
	obs_scene_enum_items(
		sceneAsScene,
		[](obs_scene_t * /*scene*/, obs_sceneitem_t *item, void *param) {
			StreamupHotkeyDisplaySettings *settingsDialog = static_cast<StreamupHotkeyDisplaySettings *>(param);
			obs_source_t *source = obs_sceneitem_get_source(item);
			const char *sourceId = obs_source_get_id(source);

			// Support all text source types across platforms
			// Windows: text_gdiplus, text_gdiplus_v3
			// Cross-platform: text_ft2_source_v2, text_ft2_source
			// Linux: text_pango_source
			if (strcmp(sourceId, "text_gdiplus") == 0 ||
			    strcmp(sourceId, "text_gdiplus_v3") == 0 ||
			    strcmp(sourceId, "text_ft2_source_v2") == 0 ||
			    strcmp(sourceId, "text_ft2_source") == 0 ||
			    strcmp(sourceId, "text_pango_source") == 0) {
				const char *sourceName = obs_source_get_name(source);
				settingsDialog->sourceComboBox->addItem(QString::fromUtf8(sourceName));
			}
			return true;
		},
		this);

	obs_source_release(scene);

	if (sourceComboBox->count() == 0) {
		sourceComboBox->addItem(StyleConstants::NO_TEXT_SOURCE);
	}
}

void StreamupHotkeyDisplaySettings::onDisplayInTextSourceToggled(bool checked)
{
	textSourceGroupBox->setVisible(checked);
	adjustSize();
}
