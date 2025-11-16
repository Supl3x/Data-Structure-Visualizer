#include "hashmapvisualization.h"


// Define static constants
const int HashMapVisualization::BUCKET_WIDTH = 80;
const int HashMapVisualization::BUCKET_HEIGHT = 60;
const int HashMapVisualization::BUCKET_SPACING = 10;
const int HashMapVisualization::CHAIN_ITEM_HEIGHT = 25;
const int HashMapVisualization::MAX_VISIBLE_BUCKETS = 12;

HashMapVisualization::HashMapVisualization(QWidget *parent)
    : QWidget(parent)
    , hashMap(new HashMap(8, 0.75f))  // 8 buckets, normal load factor (0.75) to enable rehashing
    , animationTimer(new QTimer(this))
    , highlightRect(nullptr)
    , previousBucketCount(8)
{
    setupUI();
    updateVisualization();
    updateStepTrace();

    // Connect timer for delayed updates
    animationTimer->setSingleShot(true);
    connect(animationTimer, &QTimer::timeout, this, &HashMapVisualization::updateVisualization);

    setMinimumSize(1200, 800);
}

HashMapVisualization::~HashMapVisualization()
{
    delete hashMap;
}

void HashMapVisualization::setupUI()
{
    // Main splitter for left (visualization) and right (controls + trace) panels
    mainSplitter = createManagedWidget<QSplitter>(this);
    mainSplitter->setOrientation(Qt::Horizontal);
    StyleManager::instance().applySplitterStyle(mainSplitter);

    setupVisualizationArea();
    setupRightPanel();
    // Set splitter proportions (65% visualization, 35% controls+trace)
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);
    mainSplitter->setSizes({780, 420});
    // Main layout
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mainSplitter);
    setLayout(mainLayout);
}

void HashMapVisualization::setupVisualizationArea()
{
    leftPanel = new QWidget();
    leftPanel->setStyleSheet("background: transparent;");
    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(40, 30, 20, 30);
    leftLayout->setSpacing(25);
    // Header with back button and title
    QHBoxLayout *headerLayout = new QHBoxLayout();

    backButton = new BackButton(BackButton::BackToOperations);
    connect(backButton, &BackButton::backRequested, this, &HashMapVisualization::onBackClicked);
    titleLabel = new QLabel("Hash Map");
    QFont titleFont;
    QStringList preferredFonts = {"Segoe UI", "Poppins", "SF Pro Display", "Arial"};
    for (const QString &fontName : preferredFonts) {
        if (QFontDatabase::families().contains(fontName)) {
            titleFont.setFamily(fontName);
            break;
        }
    }
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2c3e50; background: transparent;");
    titleLabel->setAlignment(Qt::AlignCenter);

    headerLayout->addWidget(backButton, 0, Qt::AlignLeft);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch();

    leftLayout->addLayout(headerLayout);

    // Stats and controls at top of visualization area
    setupStatsAndControls();

    // Visualization area with gradient background (fixed size, no scroll)
    scene = new QGraphicsScene(this);

    visualizationView = new QGraphicsView(scene);
    visualizationView->setRenderHint(QPainter::Antialiasing);
    visualizationView->setFixedHeight(450);
    visualizationView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    visualizationView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    visualizationView->setStyleSheet(R"(
        QGraphicsView {
            border: 2px solid rgba(74, 144, 226, 0.2);
            border-radius: 20px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(248, 251, 255, 0.98),
                stop:1 rgba(240, 247, 255, 0.98));
        }
    )");

    // Add drop shadow to visualization area
    QGraphicsDropShadowEffect *viewShadow = new QGraphicsDropShadowEffect();
    viewShadow->setBlurRadius(20);
    viewShadow->setXOffset(0);
    viewShadow->setYOffset(6);
    viewShadow->setColor(QColor(74, 144, 226, 25));
    visualizationView->setGraphicsEffect(viewShadow);

    leftLayout->addWidget(visualizationView, 1);

    // Bottom note about bucket limitation
    bucketNote = new QLabel(QString("* Buckets shown: %1").arg(hashMap->bucketCount()));
    bucketNote->setStyleSheet(R"(
        QLabel {
            color: #7f8c8d;
            font-size: 11px;
            font-style: italic;
            padding: 5px;
        }
    )");
    bucketNote->setAlignment(Qt::AlignLeft);
    leftLayout->addWidget(bucketNote);
}

void HashMapVisualization::setupStatsAndControls()
{
    // Combined stats and controls layout
    QVBoxLayout *topLayout = new QVBoxLayout();
    topLayout->setSpacing(15);

    // Stats row
    QHBoxLayout *statsLayout = new QHBoxLayout();
    statsLayout->setSpacing(15);

    sizeLabel = new QLabel("Size: 0");
    bucketCountLabel = new QLabel(QString("Buckets: %1").arg(hashMap->bucketCount()));
    loadFactorLabel = new QLabel("Load Factor: 0.00");
    loadFactorWarning = new QLabel("");
    loadFactorWarning->setVisible(false);
    
    QString statsStyle = R"(
        QLabel {
            color: #34495e;
            font-weight: bold;
            font-family: 'Segoe UI';
            font-size: 12px;
            padding: 8px 12px;
            background-color: rgba(74, 144, 226, 0.1);
            border-radius: 12px;
            border: 1px solid rgba(74, 144, 226, 0.2);
        }
    )";

    sizeLabel->setStyleSheet(statsStyle);
    bucketCountLabel->setStyleSheet(statsStyle);
    loadFactorLabel->setStyleSheet(statsStyle);

    statsLayout->addWidget(sizeLabel);
    statsLayout->addWidget(bucketCountLabel);
    statsLayout->addWidget(loadFactorLabel);
    statsLayout->addWidget(loadFactorWarning);
    statsLayout->addStretch();

    // Controls row (similar to Red Black Tree format)
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(10);

    keyInput = new QLineEdit();
    keyInput->setPlaceholderText("Enter key");
    keyInput->setFixedSize(150, 40);
    keyInput->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            border: 2px solid #d0c5e8;
            border-radius: 20px;
            padding: 8px 16px;
            color: #2d1b69;
            font-size: 12px;
        }
        QLineEdit:focus { border-color: #7b4fff; }
    )");

    valueInput = new QLineEdit();
    valueInput->setPlaceholderText("Enter value");
    valueInput->setFixedSize(150, 40);
    valueInput->setStyleSheet(R"(
        QLineEdit {
            background-color: white;
            border: 2px solid #d0c5e8;
            border-radius: 20px;
            padding: 8px 16px;
            color: #2d1b69;
            font-size: 12px;
        }
        QLineEdit:focus { border-color: #7b4fff; }
    )");

    insertButton = new QPushButton("Insert");
    insertButton->setFixedSize(75, 35);
    insertButton->setCursor(Qt::PointingHandCursor);
    insertButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7b4fff, stop:1 #9b6fff);
            color: white;
            border: none;
            border-radius: 17px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #6c3cff, stop:1 #8b5fff);
        }
        QPushButton:disabled { background: #cccccc; }
    )");

    searchButton = new QPushButton("Search");
    searchButton->setFixedSize(75, 35);
    searchButton->setCursor(Qt::PointingHandCursor);
    searchButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #4a90e2, stop:1 #6bb6ff);
            color: white;
            border: none;
            border-radius: 17px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #357abd, stop:1 #5ba0e6);
        }
        QPushButton:disabled { background: #cccccc; }
    )");

    deleteButton = new QPushButton("Delete");
    deleteButton->setFixedSize(75, 35);
    deleteButton->setCursor(Qt::PointingHandCursor);
    deleteButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ff6b6b, stop:1 #ff8e8e);
            color: white;
            border: none;
            border-radius: 17px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ff5252, stop:1 #ff7575);
        }
        QPushButton:disabled { background: #cccccc; }
    )");

    clearButton = new QPushButton("Clear");
    clearButton->setFixedSize(75, 35);
    clearButton->setCursor(Qt::PointingHandCursor);
    clearButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #95a5a6, stop:1 #bdc3c7);
            color: white;
            border: none;
            border-radius: 17px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #7f8c8d, stop:1 #95a5a6);
        }
        QPushButton:disabled { background: #cccccc; }
    )");

    randomizeButton = new QPushButton("Random");
    randomizeButton->setFixedSize(75, 35);
    randomizeButton->setCursor(Qt::PointingHandCursor);
    randomizeButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #28a745, stop:1 #34ce57);
            color: white;
            border: none;
            border-radius: 17px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #218838, stop:1 #28a745);
        }
        QPushButton:disabled { background: #cccccc; }
    )");

    controlLayout->addWidget(keyInput);
    controlLayout->addWidget(valueInput);
    controlLayout->addWidget(insertButton);
    controlLayout->addWidget(deleteButton);
    controlLayout->addWidget(searchButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(randomizeButton);
    controlLayout->addStretch();

    // Connect signals
    connect(insertButton, &QPushButton::clicked, this, &HashMapVisualization::onInsertClicked);
    connect(searchButton, &QPushButton::clicked, this, &HashMapVisualization::onSearchClicked);
    connect(deleteButton, &QPushButton::clicked, this, &HashMapVisualization::onDeleteClicked);
    connect(clearButton, &QPushButton::clicked, this, &HashMapVisualization::onClearClicked);
    connect(randomizeButton, &QPushButton::clicked, this, &HashMapVisualization::onRandomizeClicked);

    topLayout->addLayout(statsLayout);
    topLayout->addLayout(controlLayout);
    leftLayout->addLayout(topLayout);
}

void HashMapVisualization::setupRightPanel()
{
    rightPanel = new QWidget();
    rightPanel->setMinimumWidth(400);
    rightPanel->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(250, 252, 255, 0.9),
                stop:1 rgba(245, 249, 255, 0.95));
            border-left: 1px solid rgba(123, 79, 255, 0.1);
        }
    )");

    rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(20, 20, 20, 20);
    rightLayout->setSpacing(15);

    // Split right panel: chat history with spacing, type selection bottom
    setupStepTraceTop();
    setupTypeSelection();
}

void HashMapVisualization::setupStepTraceTop()
{
    // Add some spacing above the chat for better aesthetics
    rightLayout->addSpacing(30);
    
    // Chat history with improved styling (copying binary tree design)
    traceGroup = new QGroupBox("");
    traceGroup->setStyleSheet(R"(
        QGroupBox {
            border: 3px solid qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(138, 43, 226, 0.6),
                stop:0.5 rgba(30, 144, 255, 0.6),
                stop:1 rgba(0, 191, 255, 0.6));
            border-radius: 20px;
            margin-top: 15px;
            padding-top: 15px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(240, 248, 255, 0.98),
                stop:0.3 rgba(230, 245, 255, 0.98),
                stop:0.7 rgba(245, 240, 255, 0.98),
                stop:1 rgba(250, 245, 255, 0.98));
            box-shadow: 0px 8px 25px rgba(138, 43, 226, 0.15);
        }
    )");

    QVBoxLayout *traceLayout = new QVBoxLayout(traceGroup);
    traceLayout->setContentsMargins(20, 20, 20, 20);
    traceLayout->setSpacing(15);

    // Add heading inside the box
    QLabel *traceTitle = new QLabel("🗂️ Hash Map Operations & Algorithms");
    traceTitle->setStyleSheet(R"(
        QLabel {
            font-weight: bold;
            font-size: 16px;
            color: white;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(138, 43, 226, 0.9),
                stop:0.5 rgba(30, 144, 255, 0.9),
                stop:1 rgba(0, 191, 255, 0.9));
            border-radius: 15px;
            padding: 10px 25px;
            margin: 5px;
        }
    )");
    traceTitle->setAlignment(Qt::AlignCenter);
    traceLayout->addWidget(traceTitle);

    // Create tab widget for steps and algorithms
    traceTabWidget = new QTabWidget();
    traceTabWidget->setStyleSheet(R"(
        QTabWidget::pane {
            border: 2px solid rgba(123, 79, 255, 0.2);
            border-radius: 10px;
            background: white;
            margin-top: 5px;
        }
        QTabWidget::tab-bar {
            alignment: center;
        }
        QTabBar::tab {
            background: rgba(123, 79, 255, 0.1);
            color: #2d1b69;
            padding: 8px 16px;
            margin: 2px;
            border-radius: 8px;
            font-weight: bold;
            font-size: 12px;
        }
        QTabBar::tab:selected {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 rgba(123, 79, 255, 0.8),
                stop:1 rgba(155, 89, 182, 0.8));
            color: white;
        }
        QTabBar::tab:hover:!selected {
            background: rgba(123, 79, 255, 0.2);
        }
    )");

    // Steps tab - using StyleManager for beautiful scroll bars (like BST)
    stepsList = new QListWidget();
    StyleManager::instance().applyStepTraceStyle(stepsList);

    // Algorithm tab - using StyleManager for beautiful scroll bars (like BST)
    algorithmList = new QListWidget();
    StyleManager::instance().applyStepTraceStyle(algorithmList);

    // Add both widgets to tabs with enhanced names and icons
    traceTabWidget->addTab(stepsList, "📝 Steps");
    traceTabWidget->addTab(algorithmList, "⚙️ Algorithm");

    traceLayout->addWidget(traceTabWidget);
    rightLayout->addWidget(traceGroup, 2);  // Give it more space (2/3 of right panel)
}

void HashMapVisualization::setupTypeSelection()
{
    typeGroup = new QGroupBox("");
    typeGroup->setStyleSheet(R"(
        QGroupBox {
            border: 3px solid qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(138, 43, 226, 0.6),
                stop:0.5 rgba(30, 144, 255, 0.6),
                stop:1 rgba(0, 191, 255, 0.6));
            border-radius: 20px;
            margin-top: 15px;
            padding-top: 15px;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(240, 248, 255, 0.98),
                stop:0.3 rgba(230, 245, 255, 0.98),
                stop:0.7 rgba(245, 240, 255, 0.98),
                stop:1 rgba(250, 245, 255, 0.98));
            box-shadow: 0px 8px 25px rgba(138, 43, 226, 0.15);
        }
    )");

    QVBoxLayout *typeMainLayout = new QVBoxLayout(typeGroup);
    typeMainLayout->setContentsMargins(20, 20, 20, 20);
    typeMainLayout->setSpacing(15);

    // Add heading inside the box
    QLabel *typeTitle = new QLabel("🏷️ Data Types");
    typeTitle->setStyleSheet(R"(
        QLabel {
            font-weight: bold;
            font-size: 16px;
            color: white;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(138, 43, 226, 0.9),
                stop:0.5 rgba(30, 144, 255, 0.9),
                stop:1 rgba(0, 191, 255, 0.9));
            border-radius: 15px;
            padding: 10px 25px;
            margin: 5px;
        }
    )");
    typeTitle->setAlignment(Qt::AlignCenter);
    typeMainLayout->addWidget(typeTitle);

    // Create horizontal layout for the controls
    QHBoxLayout *typeLayout = new QHBoxLayout();
    typeLayout->setContentsMargins(15, 10, 15, 10);
    typeLayout->setSpacing(20);

    QLabel *keyLabel = new QLabel("Key:");
    keyLabel->setStyleSheet("color: #1a1a2e; font-weight: 700; font-size: 13px; background: transparent;");
    keyTypeCombo = new QComboBox();
    keyTypeCombo->addItems({"String", "Integer", "Double", "Float", "Char"});

    QLabel *valueLabel = new QLabel("Value:");
    valueLabel->setStyleSheet("color: #1a1a2e; font-weight: 700; font-size: 13px; background: transparent;");
    valueTypeCombo = new QComboBox();
    valueTypeCombo->addItems({"String", "Integer", "Double", "Float", "Char"});

    QString comboStyle = R"(
        QComboBox {
            border: 2px solid rgba(123, 79, 255, 0.3);
            border-radius: 6px;
            padding: 6px 10px;
            font-size: 10px;
            font-weight: 500;
            background: white;
            color: #2d1b69;
            min-width: 70px;
            selection-background-color: rgba(123, 79, 255, 0.2);
        }
        QComboBox:hover {
            border-color: rgba(123, 79, 255, 0.6);
            background: rgba(248, 251, 255, 1.0);
        }
        QComboBox:focus {
            border-color: #7b4fff;
            background: white;
        }
        QComboBox:on {
            border-color: #7b4fff;
            background: rgba(248, 251, 255, 1.0);
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left: 1px solid rgba(74, 144, 226, 0.3);
            border-top-right-radius: 6px;
            border-bottom-right-radius: 6px;
            background: rgba(74, 144, 226, 0.05);
        }
        QComboBox::drop-down:hover {
            background: rgba(74, 144, 226, 0.1);
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 6px solid #4a90e2;
            margin: 0px;
        }
        QComboBox::down-arrow:hover {
            border-top-color: #2980b9;
        }
        QComboBox QAbstractItemView {
            border: 2px solid rgba(74, 144, 226, 0.3);
            border-radius: 8px;
            background: white;
            color: #2c3e50;
            selection-background-color: rgba(74, 144, 226, 0.15);
            selection-color: #2c3e50;
            outline: none;
        }
        QComboBox QAbstractItemView::item {
            padding: 8px 12px;
            border: none;
            min-height: 20px;
        }
        QComboBox QAbstractItemView::item:hover {
            background-color: rgba(74, 144, 226, 0.1);
        }
        QComboBox QAbstractItemView::item:selected {
            background-color: rgba(74, 144, 226, 0.2);
            color: #2c3e50;
        }
    )";

    keyTypeCombo->setStyleSheet(comboStyle);
    valueTypeCombo->setStyleSheet(comboStyle);

    // Horizontal layout: Key [dropdown] Value [dropdown]
    typeLayout->addWidget(keyLabel);
    typeLayout->addWidget(keyTypeCombo);
    typeLayout->addWidget(valueLabel);
    typeLayout->addWidget(valueTypeCombo);
    typeLayout->addStretch();

    // Add the horizontal layout to the main vertical layout
    typeMainLayout->addLayout(typeLayout);

    connect(keyTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HashMapVisualization::onTypeChanged);
    connect(valueTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &HashMapVisualization::onTypeChanged);

    rightLayout->addWidget(typeGroup);
}



void HashMapVisualization::setupStepTrace()
{
    traceGroup = new QGroupBox("Operation History");
    traceGroup->setStyleSheet(R"(
        QGroupBox {
            font-weight: bold;
            font-size: 14px;
            color: #2d1b69;
            border: 2px solid rgba(123, 79, 255, 0.2);
            border-radius: 10px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
        }
    )");

    QVBoxLayout *traceLayout = new QVBoxLayout(traceGroup);

    stepsList = new QListWidget();
    stepsList->setStyleSheet(R"(
        QListWidget {
            background: white;
            border: 1px solid rgba(123, 79, 255, 0.2);
            border-radius: 8px;
            padding: 5px;
            font-family: 'Segoe UI';
            font-size: 12px;
        }
        QListWidget::item {
            padding: 6px;
            border-bottom: 1px solid #f0f0f0;
        }
        QListWidget::item:selected {
            background: rgba(123, 79, 255, 0.1);
        }
    )");

    traceLayout->addWidget(stepsList);
    rightLayout->addWidget(traceGroup, 1);  // Give it more space
}


void HashMapVisualization::drawBuckets()
{
    // Clear existing visualization
    scene->clear();
    bucketRects.clear();
    bucketTexts.clear();
    chainTexts.clear();

    int actualBucketCount = hashMap->bucketCount();
    // Limit display to 16 buckets for visualization
    const int bucketCount = qMin(actualBucketCount, 16);
    const QVector<int> bucketSizes = hashMap->bucketSizes();
    const QVector<QVector<QPair<QVariant, QVariant>>> bucketContents = hashMap->getBucketContents();

    // Calculate layout for all buckets in a single row
    const int totalWidth = bucketCount * (BUCKET_WIDTH + BUCKET_SPACING) - BUCKET_SPACING;
    const int startX = -totalWidth / 2;

    bucketRects.resize(bucketCount);
    bucketTexts.resize(bucketCount);
    chainTexts.resize(bucketCount);

    for (int i = 0; i < bucketCount; ++i) {
        const int x = startX + i * (BUCKET_WIDTH + BUCKET_SPACING);
        const int y = 0;

        // Calculate dynamic bucket height based on content
        int bucketHeight = BUCKET_HEIGHT;
        if (i < bucketContents.size() && !bucketContents[i].isEmpty()) {
            bucketHeight = BUCKET_HEIGHT + (bucketContents[i].size() * 30); // 30px per item
        }

        // Create bucket with dynamic height
        QGraphicsPathItem *bucketPath = new QGraphicsPathItem();
        QPainterPath path;
        path.addRoundedRect(QRectF(x, y, BUCKET_WIDTH, bucketHeight), 12, 12);
        bucketPath->setPath(path);

        // Set gradient brush for bucket
        QLinearGradient bucketGradient(x, y, x, y + bucketHeight);
        if (bucketSizes[i] > 0) {
            // Filled bucket - purple gradient
            bucketGradient.setColorAt(0.0, QColor(123, 79, 255, 15));
            bucketGradient.setColorAt(1.0, QColor(123, 79, 255, 25));
        } else {
            // Empty bucket - light gradient
            bucketGradient.setColorAt(0.0, QColor(255, 255, 255, 200));
            bucketGradient.setColorAt(1.0, QColor(250, 248, 255, 200));
        }
        bucketPath->setBrush(QBrush(bucketGradient));
        bucketPath->setPen(QPen(QColor(123, 79, 255, 120), 2.5));
        scene->addItem(bucketPath);

        // Bucket index label
        QGraphicsTextItem *indexText = scene->addText(QString::number(i));
        indexText->setPos(x + BUCKET_WIDTH/2 - 8, y - 35);
        indexText->setDefaultTextColor(QColor(45, 27, 105));
        QFont indexFont("Segoe UI", 14);
        indexFont.setBold(true);
        indexText->setFont(indexFont);

        // Show data directly inside the bucket
        QVector<QGraphicsTextItem*> chainItems;
        if (i < bucketContents.size()) {
            const auto &items = bucketContents[i];
            for (int j = 0; j < items.size(); ++j) {
                const int itemY = y + 10 + j * 30; // Items stacked vertically inside bucket

                // Chain item background inside bucket
                QGraphicsPathItem *itemBgPath = new QGraphicsPathItem();
                QPainterPath itemPath;
                itemPath.addRoundedRect(QRectF(x + 4, itemY, BUCKET_WIDTH - 8, 25), 6, 6);
                itemBgPath->setPath(itemPath);
                itemBgPath->setBrush(QBrush(QColor(255, 255, 255, 180)));
                itemBgPath->setPen(QPen(QColor(123, 79, 255, 100), 1.5));
                itemBgPath->setZValue(1);
                scene->addItem(itemBgPath);

                // Chain item text with actual key-value pair
                const QVariant &key = items[j].first;
                const QVariant &value = items[j].second;
                QString keyStr = HashMap::variantToDisplayString(key);
                QString valueStr = HashMap::variantToDisplayString(value);
                QString displayText = QString("%1→%2").arg(keyStr.left(4), valueStr.left(4));

                QGraphicsTextItem *chainItem = scene->addText(displayText);
                chainItem->setPos(x + 6, itemY + 2);
                chainItem->setDefaultTextColor(QColor(45, 27, 105));
                QFont chainFont("Segoe UI", 8);
                chainFont.setBold(true);
                chainItem->setFont(chainFont);
                chainItem->setZValue(2);
                chainItems.append(chainItem);

                // Add chain link arrow for multiple items
                if (j > 0) {
                    QGraphicsTextItem *arrow = scene->addText("↓");
                    arrow->setPos(x + BUCKET_WIDTH/2 - 5, itemY - 15);
                    arrow->setDefaultTextColor(QColor(123, 79, 255, 150));
                    QFont arrowFont("Segoe UI", 10);
                    arrowFont.setBold(true);
                    arrow->setFont(arrowFont);
                    arrow->setZValue(2);
                }
            }
        }
        chainTexts[i] = chainItems;

        // Empty bucket label
        if (bucketSizes[i] == 0) {
            QGraphicsTextItem *emptyText = scene->addText("empty");
            emptyText->setPos(x + BUCKET_WIDTH/2 - 15, y + BUCKET_HEIGHT/2 - 10);
            emptyText->setDefaultTextColor(QColor(150, 150, 150));
            QFont emptyFont("Segoe UI", 9);
            emptyFont.setItalic(true);
            emptyText->setFont(emptyFont);
            emptyText->setZValue(2);
        }
    }

    // Add title higher up
    QGraphicsTextItem *vizTitle = scene->addText("Hash Map (Open Chaining)");
    QFont titleFont("Segoe UI", 16);
    titleFont.setBold(true);
    vizTitle->setFont(titleFont);
    vizTitle->setDefaultTextColor(QColor(44, 62, 80));
    vizTitle->setPos(-totalWidth/2, -120);


    // Adjust scene rect with padding
    scene->setSceneRect(scene->itemsBoundingRect().adjusted(-60, -100, 60, 80));
}

void HashMapVisualization::updateVisualization()
{
    int currentBucketCount = hashMap->bucketCount();
    
    drawBuckets();
    showStats();
    
    // Zoom out if bucket count increased (rehashing occurred)
    if (currentBucketCount > previousBucketCount) {
        QTimer::singleShot(100, this, &HashMapVisualization::zoomToFit);
    }
    
    previousBucketCount = currentBucketCount;
}

void HashMapVisualization::zoomToFit()
{
    if (!scene || !visualizationView) return;
    
    // Get the bounding rect of all items
    QRectF itemsRect = scene->itemsBoundingRect();
    if (itemsRect.isEmpty()) return;
    
    // Add padding for better visibility
    itemsRect.adjust(-80, -120, 80, 100);
    
    // Fit the view to show all buckets, allowing expansion if needed
    visualizationView->fitInView(itemsRect, Qt::KeepAspectRatioByExpanding);
    
    // If the view is zoomed in too much, reset to show everything
    QRectF viewRect = visualizationView->mapToScene(visualizationView->viewport()->rect()).boundingRect();
    if (viewRect.width() < itemsRect.width() * 0.9 || viewRect.height() < itemsRect.height() * 0.9) {
        visualizationView->fitInView(itemsRect, Qt::KeepAspectRatio);
    }
}

void HashMapVisualization::updateStepTrace()
{
    stepsList->clear();
    const QVector<QString> &steps = hashMap->lastSteps();

    for (int i = 0; i < steps.size(); ++i) {
        const QString &step = steps[i];

        // Handle separator lines
        if (step == "────────────────────") {
            QListWidgetItem *separator = new QListWidgetItem(step);
            separator->setTextAlignment(Qt::AlignCenter);
            separator->setFlags(Qt::NoItemFlags); // Make it non-selectable
            separator->setForeground(QColor(189, 195, 199));
            stepsList->addItem(separator);
            continue;
        }

        // Add step with better formatting
        QListWidgetItem *item = new QListWidgetItem(step);

        // Style different types of steps
        if (step.contains("✅")) {
            item->setForeground(QColor(39, 174, 96)); // Green for success
        } else if (step.contains("❌")) {
            item->setForeground(QColor(231, 76, 60)); // Red for failure
        } else if (step.contains("🔍")) {
            item->setForeground(QColor(52, 152, 219)); // Blue for search
        } else if (step.contains("➕")) {
            item->setForeground(QColor(46, 125, 50)); // Green for insert/put
        } else if (step.contains("🗑️")) {
            item->setForeground(QColor(211, 47, 47)); // Red for delete
        } else if (step.contains("📊") || step.contains("🎯")) {
            item->setForeground(QColor(155, 89, 182)); // Purple for calculation
        } else {
            item->setForeground(QColor(44, 62, 80)); // Default dark color
        }

        stepsList->addItem(item);
    }

    // Auto-scroll to bottom to show latest steps
    if (stepsList->count() > 0) {
        stepsList->scrollToBottom();
    }
}

void HashMapVisualization::showStats()
{
    // Update main area stats only
    sizeLabel->setText(QString("Size: %1").arg(hashMap->size()));
    bucketCountLabel->setText(QString("Buckets: %1").arg(hashMap->bucketCount()));
    
    // Update bucket note at bottom with display limitation message
    if (bucketNote) {
        int bucketCount = hashMap->bucketCount();
        if (bucketCount > 16) {
            bucketNote->setText(QString("* Maximum buckets shown: 16 (Due to screen size limitation. Actual buckets: %1)").arg(bucketCount));
            bucketNote->setStyleSheet(R"(
                QLabel {
                    color: #e67e22;
                    font-size: 11px;
                    font-style: italic;
                    font-weight: bold;
                    padding: 5px;
                    background-color: rgba(230, 126, 34, 0.1);
                    border-radius: 5px;
                }
            )");
        } else {
            bucketNote->setText(QString("* Maximum buckets shown: 16 (Due to screen size limitation)"));
            bucketNote->setStyleSheet(R"(
                QLabel {
                    color: #7f8c8d;
                    font-size: 11px;
                    font-style: italic;
                    padding: 5px;
                }
            )");
        }
    }
    
    // Update load factor with color coding
    float loadFactor = hashMap->loadFactor();
    QString loadFactorText = QString("Load Factor: %1").arg(loadFactor, 0, 'f', 2);
    
    // Color coding: yellow when approaching threshold (>= 0.6), red when at/above threshold (>= 0.75)
    QString colorStyle;
    if (loadFactor >= 0.75f) {
        // Red when at or above threshold
        colorStyle = R"(
            QLabel {
                color: #ffffff;
                font-weight: bold;
                font-family: 'Segoe UI';
                font-size: 12px;
                padding: 8px 12px;
                background-color: rgba(231, 76, 60, 0.9);
                border-radius: 12px;
                border: 1px solid rgba(192, 57, 43, 0.5);
            }
        )";
    } else if (loadFactor >= 0.6f) {
        // Yellow when approaching threshold
        colorStyle = R"(
            QLabel {
                color: #856404;
                font-weight: bold;
                font-family: 'Segoe UI';
                font-size: 12px;
                padding: 8px 12px;
                background-color: rgba(255, 193, 7, 0.9);
                border-radius: 12px;
                border: 1px solid rgba(255, 152, 0, 0.5);
            }
        )";
    } else {
        // Normal color when below threshold
        colorStyle = R"(
            QLabel {
                color: #34495e;
                font-weight: bold;
                font-family: 'Segoe UI';
                font-size: 12px;
                padding: 8px 12px;
                background-color: rgba(74, 144, 226, 0.1);
                border-radius: 12px;
                border: 1px solid rgba(74, 144, 226, 0.2);
            }
        )";
    }
    
    loadFactorLabel->setText(loadFactorText);
    loadFactorLabel->setStyleSheet(colorStyle);
    
    // Update warning label
    if (loadFactor >= 0.75f) {
        loadFactorWarning->setText("⚠️ FULL");
        loadFactorWarning->setStyleSheet(R"(
            QLabel {
                color: #ffffff;
                font-weight: bold;
                font-family: 'Segoe UI';
                font-size: 11px;
                padding: 6px 10px;
                background-color: rgba(231, 76, 60, 0.9);
                border-radius: 10px;
                border: 1px solid rgba(192, 57, 43, 0.5);
            }
        )");
        loadFactorWarning->setVisible(true);
    } else if (loadFactor >= 0.6f) {
        loadFactorWarning->setText("⚠️ NEARLY FULL");
        loadFactorWarning->setStyleSheet(R"(
            QLabel {
                color: #856404;
                font-weight: bold;
                font-family: 'Segoe UI';
                font-size: 11px;
                padding: 6px 10px;
                background-color: rgba(255, 193, 7, 0.9);
                border-radius: 10px;
                border: 1px solid rgba(255, 152, 0, 0.5);
            }
        )");
        loadFactorWarning->setVisible(true);
    } else {
        loadFactorWarning->setVisible(false);
    }
}

void HashMapVisualization::animateOperation(const QString &operation)
{
    Q_UNUSED(operation);
    // TODO: Add specific animations for different operations
    updateVisualization();
    updateStepTrace();
}

void HashMapVisualization::onBackClicked()
{
    emit backToOperations();
}

void HashMapVisualization::onTypeChanged()
{
    // Update HashMap data types based on combo box selections
    HashMap::DataType keyType = static_cast<HashMap::DataType>(keyTypeCombo->currentIndex());
    HashMap::DataType valueType = static_cast<HashMap::DataType>(valueTypeCombo->currentIndex());

    hashMap->setKeyType(keyType);
    hashMap->setValueType(valueType);

    // Update input placeholders based on selected types
    QString keyPlaceholder = QString("Enter %1 key").arg(HashMap::dataTypeToString(keyType).toLower());
    QString valuePlaceholder = QString("Enter %1 value").arg(HashMap::dataTypeToString(valueType).toLower());

    keyInput->setPlaceholderText(keyPlaceholder);
    valueInput->setPlaceholderText(valuePlaceholder);

    // Clear existing data when types change
    hashMap->clear();
    updateVisualization();
    updateStepTrace();
}

QVariant HashMapVisualization::convertStringToVariant(const QString &str, HashMap::DataType type)
{
    bool ok;
    switch (type) {
    case HashMap::STRING:
        return QVariant(str);
    case HashMap::INTEGER: {
        int intVal = str.toInt(&ok);
        return ok ? QVariant(intVal) : QVariant();
    }
    case HashMap::DOUBLE: {
        double doubleVal = str.toDouble(&ok);
        return ok ? QVariant(doubleVal) : QVariant();
    }
    case HashMap::FLOAT: {
        float floatVal = str.toFloat(&ok);
        return ok ? QVariant(floatVal) : QVariant();
    }
    case HashMap::CHAR: {
        if (str.isEmpty()) return QVariant();
        return QVariant(QChar(str.at(0)));
    }
    default:
        return QVariant();
    }
}

void HashMapVisualization::onInsertClicked()
{
    const QString keyStr = keyInput->text().trimmed();
    const QString valueStr = valueInput->text().trimmed();

    if (keyStr.isEmpty() || valueStr.isEmpty()) {
        // Visual feedback instead of popup
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        valueInput->setStyleSheet(valueInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
            valueInput->setStyleSheet(valueInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }

    // Convert strings to appropriate QVariant types
    QVariant key = convertStringToVariant(keyStr, hashMap->getKeyType());
    QVariant value = convertStringToVariant(valueStr, hashMap->getValueType());

    if (!key.isValid() || !value.isValid()) {
        // Type conversion failed - show error
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        valueInput->setStyleSheet(valueInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
            valueInput->setStyleSheet(valueInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }

    hashMap->put(key, value);
    animateOperation("Insert");
    showAlgorithm("Insert");

    // Clear inputs
    keyInput->clear();
    valueInput->clear();
}

void HashMapVisualization::onSearchClicked()
{
    const QString keyStr = keyInput->text().trimmed();

    if (keyStr.isEmpty()) {
        // Show error if key field is empty
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }

    // Search by key only
    QVariant key = convertStringToVariant(keyStr, hashMap->getKeyType());

    if (!key.isValid()) {
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }

    // Perform search by key
    auto result = hashMap->get(key);
    animateOperation("Search");
    showAlgorithm("Search");

    // If found, show the value in the history
    if (result.has_value()) {
        QString foundValue = HashMap::variantToDisplayString(result.value());
        hashMap->addStepToHistory(QString("✅ Found! Key '%1' → Value '%2'").arg(keyStr, foundValue));
        updateStepTrace();
    }

    // Animate the search by highlighting the target bucket
    animateSearchResult(keyStr, result.has_value());

    // Clear input
    keyInput->clear();
}


void HashMapVisualization::animateSearchResult(const QString &key, bool found)
{
    // Convert key to QVariant for proper hashing
    QVariant keyVariant = convertStringToVariant(key, hashMap->getKeyType());
    if (!keyVariant.isValid()) return;

    // Calculate which bucket the key would be in using HashMap's indexFor method
    const int bucketIndex = hashMap->indexFor(keyVariant, hashMap->bucketCount());

    // Step 1: Show hash calculation (like Binary Search Tree's step-by-step approach)
    hashMap->addStepToHistory(QString("🔍 Searching for key: %1").arg(key));
    hashMap->addStepToHistory(QString("📊 Calculating hash for key..."));
    updateStepTrace();

    QTimer::singleShot(800, [this, key, bucketIndex, found]() {
        // Step 2: Show which bucket to check
        hashMap->addStepToHistory(QString("🎯 Hash points to bucket %1").arg(bucketIndex));
        updateStepTrace();

        // Clear any existing highlights
        if (highlightRect) {
            scene->removeItem(highlightRect);
            delete highlightRect;
            highlightRect = nullptr;
        }

        // Create highlight rectangle for the target bucket
        const int bucketCount = hashMap->bucketCount();
        const int totalWidth = bucketCount * (BUCKET_WIDTH + BUCKET_SPACING) - BUCKET_SPACING;
        const int startX = -totalWidth / 2;
        const int x = startX + bucketIndex * (BUCKET_WIDTH + BUCKET_SPACING);
        const int y = 0;

        // Calculate bucket height (same logic as drawBuckets)
        const QVector<QVector<QPair<QVariant, QVariant>>> bucketContents = hashMap->getBucketContents();
        int bucketHeight = BUCKET_HEIGHT;
        if (bucketIndex < bucketContents.size() && !bucketContents[bucketIndex].isEmpty()) {
            bucketHeight = BUCKET_HEIGHT + (bucketContents[bucketIndex].size() * 30);
        }

        // Create highlight effect (like Binary Search Tree node highlighting)
        highlightRect = scene->addRect(x - 3, y - 3, BUCKET_WIDTH + 6, bucketHeight + 6,
                                       QPen(found ? QColor(40, 167, 69, 200) : QColor(220, 53, 69, 200), 4),
                                       QBrush(Qt::transparent));
        highlightRect->setZValue(10);

        QTimer::singleShot(800, [this, key, found]() {
            // Step 3: Show search result
            if (found) {
                hashMap->addStepToHistory(QString("✅ Key '%1' found in bucket!").arg(key));
            } else {
                hashMap->addStepToHistory(QString("❌ Key '%1' not found in bucket").arg(key));
            }
            updateStepTrace();

            // Step 4: Fade out highlight after showing result (like Binary Search Tree)
            QTimer::singleShot(1200, [this]() {
                if (highlightRect) {
                    scene->removeItem(highlightRect);
                    delete highlightRect;
                    highlightRect = nullptr;
                }
            });
        });
    });
}

void HashMapVisualization::onDeleteClicked()
{
    const QString keyStr = keyInput->text().trimmed();

    if (keyStr.isEmpty()) {
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }

    // Convert string to appropriate QVariant type
    QVariant key = convertStringToVariant(keyStr, hashMap->getKeyType());

    if (!key.isValid()) {
        keyInput->setStyleSheet(keyInput->styleSheet() + "border-color: #dc3545 !important;");
        QTimer::singleShot(2000, [this]() {
            keyInput->setStyleSheet(keyInput->styleSheet().replace("border-color: #dc3545 !important;", ""));
        });
        return;
    }

    bool removed = hashMap->erase(key);
    animateOperation("Delete");
    showAlgorithm("Delete");

    // Animate the deletion result
    animateSearchResult(keyStr, removed);

    keyInput->clear();
}

void HashMapVisualization::onClearClicked()
{
    hashMap->clear();
    animateOperation("Clear");
    showAlgorithm("Clear");
    // Visual feedback - flash the entire visualization
    QPropertyAnimation *flashAnimation = new QPropertyAnimation(visualizationView, "opacity");
    flashAnimation->setDuration(300);
    flashAnimation->setStartValue(1.0);
    flashAnimation->setEndValue(0.3);
    flashAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    connect(flashAnimation, &QPropertyAnimation::finished, [this, flashAnimation]() {
        QPropertyAnimation *restoreAnimation = new QPropertyAnimation(visualizationView, "opacity");
        restoreAnimation->setDuration(300);
        restoreAnimation->setStartValue(0.3);
        restoreAnimation->setEndValue(1.0);
        restoreAnimation->setEasingCurve(QEasingCurve::InOutQuad);
        restoreAnimation->start();
        flashAnimation->deleteLater();
    });

    flashAnimation->start();
}

void HashMapVisualization::onRandomizeClicked()
{
    // Generate a single random value based on selected types
    HashMap::DataType keyType = hashMap->getKeyType();
    HashMap::DataType valueType = hashMap->getValueType();

    // Generate only one random value
    {
        QVariant key, value;

        // Generate random key based on type
        switch (keyType) {
        case HashMap::STRING: {
            QStringList keys = {"apple", "banana", "cherry", "date", "fig", "grape", "kiwi", "lemon"};
            key = keys[QRandomGenerator::global()->bounded(keys.size())];
            break;
        }
        case HashMap::INTEGER:
            key = QRandomGenerator::global()->bounded(1, 100);
            break;
        case HashMap::DOUBLE:
            key = QRandomGenerator::global()->generateDouble() * 100.0;
            break;
        case HashMap::FLOAT:
            key = static_cast<float>(QRandomGenerator::global()->generateDouble() * 100.0f);
            break;
        case HashMap::CHAR: {
            const QString alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
            int idx = QRandomGenerator::global()->bounded(alphabet.size());
            key = QVariant(QChar(alphabet.at(idx)));
            break;
        }
        }

        // Generate random value based on type
        switch (valueType) {
        case HashMap::STRING: {
            QStringList values = {"red", "blue", "green", "yellow", "purple", "orange", "pink", "brown"};
            value = values[QRandomGenerator::global()->bounded(values.size())];
            break;
        }
        case HashMap::INTEGER:
            value = QRandomGenerator::global()->bounded(1, 1000);
            break;
        case HashMap::DOUBLE:
            value = QRandomGenerator::global()->generateDouble() * 1000.0;
            break;
        case HashMap::FLOAT:
            value = static_cast<float>(QRandomGenerator::global()->generateDouble() * 1000.0f);
            break;
        case HashMap::CHAR: {
            const QString alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
            int idx = QRandomGenerator::global()->bounded(alphabet.size());
            value = QVariant(QChar(alphabet.at(idx)));
            break;
        }
        }

        hashMap->put(key, value);
    }

    animateOperation("Randomize");
    showAlgorithm("Randomize");
}

void HashMapVisualization::showAlgorithm(const QString &operation)
{
    // Keep history in algorithm page similar to Steps: append entries, don't clear
    // Add a visual separator between operations if there are already items
    if (algorithmList->count() > 0) {
        QListWidgetItem *separator = new QListWidgetItem("────────────────────");
        separator->setTextAlignment(Qt::AlignCenter);
        separator->setFlags(Qt::NoItemFlags); // Make it non-selectable
        separator->setForeground(QColor("#7b4fff"));
        QFont separatorFont = separator->font();
        separatorFont.setBold(true);
        separator->setFont(separatorFont);
        algorithmList->addItem(separator);
    }

    if (operation == "Insert" || operation == "Put") {
        QStringList lines = {
            "🔧 HashMap Insert Algorithm",
            "",
            "⏰ Time Complexity: O(1) average, O(n) worst",
            "💾 Space Complexity: O(1)",
            "",
            "🔄 Steps:",
            "1. Calculate hash value: hash(key)",
            "2. Find bucket index: hash % bucket_count",
            "3. Navigate to the bucket",
            "4. Search through the chain:",
            "   • If key exists: update value",
            "   • If key not found: add new node",
            "5. Increment size if new key added",
            "",
            "🔗 Collision Resolution: Open Chaining",
            "Multiple keys in same bucket form a linked list"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("🔧")) {
                QFont titleFont = item->font();
                titleFont.setBold(true);
                titleFont.setPointSize(14);
                item->setFont(titleFont);
                item->setForeground(QColor("#7b4fff"));
            } else if (line.startsWith("⏰") || line.startsWith("💾")) {
                QFont complexityFont = item->font();
                complexityFont.setBold(true);
                item->setFont(complexityFont);
                item->setForeground(QColor("#28a745"));
            } else if (line.startsWith("🔄") || line.startsWith("🔗")) {
                QFont stepsFont = item->font();
                stepsFont.setBold(true);
                item->setFont(stepsFont);
                item->setForeground(QColor("#007bff"));
            } else if (line.contains(". ") || line.contains("• ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }

    } else if (operation == "Search" || operation == "Get") {
        QStringList lines = {
            "🔍 HashMap Search Algorithm",
            "",
            "⏰ Time Complexity: O(1) average, O(n) worst",
            "💾 Space Complexity: O(1)",
            "",
            "🔄 Steps:",
            "1. Calculate hash value: hash(key)",
            "2. Find bucket index: hash % bucket_count",
            "3. Navigate to the bucket",
            "4. Traverse the chain:",
            "   • Compare each key with target",
            "   • If match found: return value",
            "   • If end reached: key not found"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("🔍")) {
                QFont titleFont = item->font();
                titleFont.setBold(true);
                titleFont.setPointSize(14);
                item->setFont(titleFont);
                item->setForeground(QColor("#7b4fff"));
            } else if (line.startsWith("⏰") || line.startsWith("💾")) {
                QFont complexityFont = item->font();
                complexityFont.setBold(true);
                item->setFont(complexityFont);
                item->setForeground(QColor("#28a745"));
            } else if (line.startsWith("🔄")) {
                QFont stepsFont = item->font();
                stepsFont.setBold(true);
                item->setFont(stepsFont);
                item->setForeground(QColor("#007bff"));
            } else if (line.contains(". ") || line.contains("• ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }

    } else if (operation == "Delete" || operation == "Remove") {
        QStringList lines = {
            "🗑️ HashMap Delete Algorithm",
            "",
            "⏰ Time Complexity: O(1) average, O(n) worst",
            "💾 Space Complexity: O(1)",
            "",
            "🔄 Steps:",
            "1. Calculate hash value: hash(key)",
            "2. Find bucket index: hash % bucket_count",
            "3. Navigate to the bucket",
            "4. Search through the chain:",
            "   • Compare each key with target",
            "   • If match found: remove node from chain",
            "   • If not found: return false",
            "5. Decrement size if key was removed"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("🗑️")) {
                QFont titleFont = item->font();
                titleFont.setBold(true);
                titleFont.setPointSize(14);
                item->setFont(titleFont);
                item->setForeground(QColor("#7b4fff"));
            } else if (line.startsWith("⏰") || line.startsWith("💾")) {
                QFont complexityFont = item->font();
                complexityFont.setBold(true);
                item->setFont(complexityFont);
                item->setForeground(QColor("#28a745"));
            } else if (line.startsWith("🔄")) {
                QFont stepsFont = item->font();
                stepsFont.setBold(true);
                item->setFont(stepsFont);
                item->setForeground(QColor("#007bff"));
            } else if (line.contains(". ") || line.contains("• ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }

    } else if (operation == "Clear") {
        QStringList lines = {
            "🧹 HashMap Clear Algorithm",
            "",
            "⏰ Time Complexity: O(n)",
            "💾 Space Complexity: O(1)",
            "",
            "🔄 Steps:",
            "1. Iterate through all buckets",
            "2. For each bucket:",
            "   • Clear the entire chain",
            "   • Reset bucket to empty state",
            "3. Reset size to 0"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("🧹")) {
                QFont titleFont = item->font();
                titleFont.setBold(true);
                titleFont.setPointSize(14);
                item->setFont(titleFont);
                item->setForeground(QColor("#7b4fff"));
            } else if (line.startsWith("⏰") || line.startsWith("💾")) {
                QFont complexityFont = item->font();
                complexityFont.setBold(true);
                item->setFont(complexityFont);
                item->setForeground(QColor("#28a745"));
            } else if (line.startsWith("🔄")) {
                QFont stepsFont = item->font();
                stepsFont.setBold(true);
                item->setFont(stepsFont);
                item->setForeground(QColor("#007bff"));
            } else if (line.contains(". ") || line.contains("• ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }

    } else if (operation == "Randomize") {
        QStringList lines = {
            "🎲 HashMap Randomize Algorithm",
            "",
            "⏰ Time Complexity: O(k) where k = number of items",
            "💾 Space Complexity: O(k)",
            "",
            "🔄 Steps:",
            "1. Generate random key-value pairs",
            "2. For each pair:",
            "   • Create key based on selected type",
            "   • Create value based on selected type",
            "   • Insert using standard insert algorithm",
            "",
            "📊 Sample Data Types:",
            "   • Strings: fruit names, colors",
            "   • Integers: random numbers 1-100",
            "   • Doubles/Floats: random decimals"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("🎲")) {
                QFont titleFont = item->font();
                titleFont.setBold(true);
                titleFont.setPointSize(14);
                item->setFont(titleFont);
                item->setForeground(QColor("#7b4fff"));
            } else if (line.startsWith("⏰") || line.startsWith("💾")) {
                QFont complexityFont = item->font();
                complexityFont.setBold(true);
                item->setFont(complexityFont);
                item->setForeground(QColor("#28a745"));
            } else if (line.startsWith("🔄") || line.startsWith("📊")) {
                QFont stepsFont = item->font();
                stepsFont.setBold(true);
                item->setFont(stepsFont);
                item->setForeground(QColor("#007bff"));
            } else if (line.contains(". ") || line.contains("• ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }
    }

    // Do not auto-switch tabs; keep user's current selection
}

void HashMapVisualization::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Create beautiful gradient background matching the Binary Search Tree style
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, QColor(250, 247, 255));    // Very light lavender
    gradient.setColorAt(0.5, QColor(242, 235, 255));    // Soft lavender
    gradient.setColorAt(1.0, QColor(237, 228, 255));    // Lavender

    painter.fillRect(rect(), gradient);

    // Add subtle circular gradients for depth (matching Binary Search Tree)
    QRadialGradient topCircle(width() * 0.2, height() * 0.15, width() * 0.4);
    topCircle.setColorAt(0.0, QColor(200, 180, 255, 30));
    topCircle.setColorAt(1.0, QColor(200, 180, 255, 0));
    painter.fillRect(rect(), topCircle);

    QRadialGradient bottomCircle(width() * 0.8, height() * 0.85, width() * 0.5);
    bottomCircle.setColorAt(0.0, QColor(180, 150, 255, 25));
    bottomCircle.setColorAt(1.0, QColor(180, 150, 255, 0));
    painter.fillRect(rect(), bottomCircle);
}
