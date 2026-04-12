#include "streamup-hotkey-display-settings.hpp"
#include "streamup-hotkey-display.hpp"
#include <obs-module.h>

StreamupHotkeyDisplaySettings::StreamupHotkeyDisplaySettings(HotkeyDisplayDock *dock, QWidget *parent)
	: QDialog(parent),
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
	  sceneComboBox(new QComboBox(this)),
	  sourceComboBox(new QComboBox(this)),
	  timeSpinBox(new QSpinBox(this)),
	  displayInTextSourceCheckBox(new SwitchWidget(obs_module_text("Settings.Checkbox.DisplayInTextSource"), this)),
	  textSourceGroupBox(new QGroupBox(obs_module_text("Settings.Group.TextSource"), this)),
	  singleKeyGroupBox(new QGroupBox(obs_module_text("Settings.Group.SingleKeyCapture"), this)),
	  captureNumpadCheckBox(new SwitchWidget(obs_module_text("Settings.Checkbox.CaptureNumpad"), this)),
	  captureNumbersCheckBox(new SwitchWidget(obs_module_text("Settings.Checkbox.CaptureNumbers"), this)),
	  captureLettersCheckBox(new SwitchWidget(obs_module_text("Settings.Checkbox.CaptureLetters"), this)),
	  capturePunctuationCheckBox(new SwitchWidget(obs_module_text("Settings.Checkbox.CapturePunctuation"), this)),
	  whitelistLabel(new QLabel(obs_module_text("Settings.Label.Whitelist"), this)),
	  separatorLabel(new QLabel(obs_module_text("Settings.Label.Separator"), this)),
	  separatorLineEdit(new QLineEdit(this)),
	  maxHistoryLabel(new QLabel(obs_module_text("Settings.Label.MaxHistory"), this)),
	  maxHistorySpinBox(new QSpinBox(this)),
	  enableLoggingCheckBox(new SwitchWidget(obs_module_text("Settings.Checkbox.EnableLogging"), this))
{
	// Apply StreamUP dialog chrome (frameless window with custom title bar)
	DialogChrome chrome = ApplyDialogChrome(this, obs_module_text("Settings.Title"));
	mainLayout = chrome.contentLayout;
	buttonLayout = new QHBoxLayout();

	// Create styled buttons
	applyButton = CreateStyledButton(obs_module_text("Settings.Button.Apply"), "primary");
	closeButton = CreateStyledButton(obs_module_text("Settings.Button.Close"), "default");

	setAccessibleName(obs_module_text("Settings.Title"));
	setAccessibleDescription(obs_module_text("Settings.Description"));

	setMinimumWidth(550);

	// Create QPlainTextEdit for whitelist (replaces QLineEdit)
	whitelistTextEdit = new QPlainTextEdit(this);
	whitelistTextEdit->setMinimumHeight(80);

	// Create Display Settings group box
	displayGroupBox = new QGroupBox(obs_module_text("Settings.Group.Display"), this);

	// Apply Catppuccin Mocha styles
	textSourceGroupBox->setStyleSheet(GetGroupBoxStyle());
	singleKeyGroupBox->setStyleSheet(GetGroupBoxStyle());
	displayGroupBox->setStyleSheet(GetGroupBoxStyle());

	sceneComboBox->setStyleSheet(GetComboBoxStyle());
	sourceComboBox->setStyleSheet(GetComboBoxStyle());

	timeSpinBox->setStyleSheet(GetSpinBoxStyle());
	maxHistorySpinBox->setStyleSheet(GetSpinBoxStyle());

	prefixLineEdit->setStyleSheet(GetInputStyle());
	suffixLineEdit->setStyleSheet(GetInputStyle());
	separatorLineEdit->setStyleSheet(GetInputStyle());

	whitelistTextEdit->setStyleSheet(GetPlainTextEditStyle());

	sceneLabel->setStyleSheet(GetLabelStyle());
	sourceLabel->setStyleSheet(GetLabelStyle());
	timeLabel->setStyleSheet(GetLabelStyle());
	prefixLabel->setStyleSheet(GetLabelStyle());
	suffixLabel->setStyleSheet(GetLabelStyle());
	whitelistLabel->setStyleSheet(GetLabelStyle());
	separatorLabel->setStyleSheet(GetLabelStyle());
	maxHistoryLabel->setStyleSheet(GetLabelStyle());

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
	singleKeyLayout->addWidget(captureNumpadCheckBox);
	singleKeyLayout->addWidget(captureNumbersCheckBox);
	singleKeyLayout->addWidget(captureLettersCheckBox);
	singleKeyLayout->addWidget(capturePunctuationCheckBox);
	singleKeyLayout->addWidget(whitelistLabel);
	singleKeyLayout->addWidget(whitelistTextEdit);
	singleKeyGroupBox->setLayout(singleKeyLayout);

	// Set tooltips for single key capture options
	captureNumpadCheckBox->setToolTip(obs_module_text("Settings.Tooltip.CaptureNumpad"));
	captureNumbersCheckBox->setToolTip(obs_module_text("Settings.Tooltip.CaptureNumbers"));
	captureLettersCheckBox->setToolTip(obs_module_text("Settings.Tooltip.CaptureLetters"));
	capturePunctuationCheckBox->setToolTip(obs_module_text("Settings.Tooltip.CapturePunctuation"));
	whitelistTextEdit->setToolTip(obs_module_text("Settings.Tooltip.Whitelist"));
	whitelistTextEdit->setPlaceholderText(obs_module_text("Settings.Placeholder.Whitelist"));

	// Set tooltip for logging checkbox
	enableLoggingCheckBox->setToolTip(obs_module_text("Settings.Tooltip.EnableLogging"));

	// Configure separator line edit
	separatorLineEdit->setToolTip(obs_module_text("Settings.Tooltip.Separator"));
	separatorLineEdit->setPlaceholderText(" + ");
	separatorLineEdit->setMaximumWidth(80);

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
	displayLayout->addLayout(separatorLayout);
	displayLayout->addLayout(maxHistoryLayout);
	displayLayout->addLayout(timeLayout);
	displayLayout->addWidget(enableLoggingCheckBox);
	displayGroupBox->setLayout(displayLayout);

	// Two-column layout
	QHBoxLayout *columnsLayout = new QHBoxLayout();
	columnsLayout->setSpacing(20);

	QVBoxLayout *leftCol = new QVBoxLayout();
	leftCol->setSpacing(14);
	QVBoxLayout *rightCol = new QVBoxLayout();
	rightCol->setSpacing(14);

	leftCol->addWidget(singleKeyGroupBox);
	leftCol->addStretch();

	rightCol->addWidget(displayGroupBox);
	rightCol->addSpacing(8);
	rightCol->addWidget(displayInTextSourceCheckBox);
	rightCol->addWidget(textSourceGroupBox);
	rightCol->addStretch();

	columnsLayout->addLayout(leftCol, 1);
	columnsLayout->addLayout(rightCol, 1);
	mainLayout->addLayout(columnsLayout);

	// Add buttons to footer layout
	buttonLayout->addWidget(applyButton);
	buttonLayout->addWidget(closeButton);
	chrome.footerLayout->addLayout(buttonLayout);

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
	connect(displayInTextSourceCheckBox->switchBtn, &SwitchButton::toggled, this,
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
