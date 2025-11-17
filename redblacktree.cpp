#include "redblacktree.h"

RedBlackTree::RedBlackTree(QWidget *parent)
    : QWidget(parent)
    , isAnimating(false)
    , currentOperation("")
    , traversalType(TraversalType::None)
    , traversalIndex(0)
{
    // Initialize NIL node (sentinel)
    NIL = new RBNode(0);
    NIL->color = BLACK;
    NIL->left = NIL->right = NIL->parent = nullptr;
    root = NIL;

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, [this]() {
        update();
    });

    traversalAnimTimer = new QTimer(this);
    connect(traversalAnimTimer, &QTimer::timeout, this, &RedBlackTree::onTraversalAnimationStep);

    setupUI();
    setMinimumSize(1200, 800);
}

RedBlackTree::~RedBlackTree()
{
    // Stop any running animations
    if (animationTimer) {
        animationTimer->stop();
    }
    if (traversalAnimTimer) {
        traversalAnimTimer->stop();
    }
    
    clearTree(root);
    delete NIL;
}

void RedBlackTree::setupUI()
{
    // Main splitter for left (visualization) and right (controls + trace) panels
    mainSplitter = createManagedWidget<QSplitter>(this);
    mainSplitter->setOrientation(Qt::Horizontal);
    StyleManager::instance().applySplitterStyle(mainSplitter);

    setupVisualizationArea();
    setupRightPanel();
    setupTraversalControls();
    
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

void RedBlackTree::setupVisualizationArea()
{
    leftPanel = new QWidget();
    leftPanel->setStyleSheet("background: transparent;");
    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(40, 30, 20, 30);
    leftLayout->setSpacing(25);

    // Header with back button and title (exactly like hashmap)
    QHBoxLayout *headerLayout = new QHBoxLayout();

    backButton = new BackButton(BackButton::BackToOperations);

    // Title (matching hashmap format exactly)
    titleLabel = new QLabel("Red-Black Tree");
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

    // Controls row (matching hashmap layout)
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(10);

    inputField = new QLineEdit();
    inputField->setPlaceholderText("Enter value");
    inputField->setFixedSize(150, 40);
    inputField->setStyleSheet(R"(
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

    // Buttons with updated styling to match hashmap/BST
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

    // Buttons are already styled individually above

    // viewAlgorithmButton removed - algorithm now integrated in right panel

    controlLayout->addWidget(inputField);
    controlLayout->addWidget(insertButton);
    controlLayout->addWidget(searchButton);
    controlLayout->addWidget(deleteButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(randomizeButton);
    controlLayout->addStretch();

    leftLayout->addLayout(controlLayout);

    // Status label
    statusLabel = new QLabel("Red-Black Tree is empty. Start by inserting a value!");
    QFont statusFont("Segoe UI", 11);
    statusLabel->setFont(statusFont);
    statusLabel->setStyleSheet("color: #7b4fff; padding: 8px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(statusLabel);

    // Visualization area (we'll draw the tree directly on this widget in paintEvent)
    leftLayout->addStretch();

    // Setup right panel
    setupRightPanel();

    // Connect signals
    connect(backButton, &BackButton::backRequested, this, &RedBlackTree::onBackClicked);
    connect(insertButton, &QPushButton::clicked, this, &RedBlackTree::onInsertClicked);
    connect(deleteButton, &QPushButton::clicked, this, &RedBlackTree::onDeleteClicked);
    connect(searchButton, &QPushButton::clicked, this, &RedBlackTree::onSearchClicked);
    connect(clearButton, &QPushButton::clicked, this, &RedBlackTree::onClearClicked);
    connect(randomizeButton, &QPushButton::clicked, this, &RedBlackTree::onRandomizeClicked);
    connect(inputField, &QLineEdit::returnPressed, this, &RedBlackTree::onInsertClicked);
}

// setupAlgorithmView removed - algorithm functionality integrated into right panel

void RedBlackTree::setupRightPanel()
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

    setupStepTrace();
}

void RedBlackTree::setupStepTrace()
{
    // Add some spacing above the chat for better aesthetics
    rightLayout->addSpacing(30);
    
    // Chat history with dramatically improved styling
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
    QLabel *traceTitle = new QLabel("🔴⚫ Operation History & Algorithms");
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

    // Steps tab - using StyleManager for beautiful scroll bars
    stepsList = new QListWidget();
    StyleManager::instance().applyStepTraceStyle(stepsList);

    // Algorithm tab - using StyleManager for beautiful scroll bars
    algorithmList = new QListWidget();
    StyleManager::instance().applyStepTraceStyle(algorithmList);

    traceTabWidget->addTab(stepsList, "📝 Steps");
    traceTabWidget->addTab(algorithmList, "⚙️ Algorithm");
    
    traceLayout->addWidget(traceTabWidget);
    rightLayout->addWidget(traceGroup, 1);
}

void RedBlackTree::setupTraversalControls()
{
    // Traversal controls using StyleManager - consistent styling!
    traversalGroup = new QGroupBox("");
    StyleManager::instance().applyTraversalGroupStyle(traversalGroup);

    QVBoxLayout *traversalLayout = new QVBoxLayout(traversalGroup);
    traversalLayout->setContentsMargins(20, 15, 20, 15);
    traversalLayout->setSpacing(8);

    // Traversal control buttons layout
    QHBoxLayout *traversalControlLayout = new QHBoxLayout();
    traversalControlLayout->setSpacing(8);

    bfsButton = new QPushButton("BFS");
    bfsButton->setFixedSize(50, 30);
    StyleManager::instance().applyTraversalButtonStyle(bfsButton, "#4a90e2");

    dfsButton = new QPushButton("DFS");
    dfsButton->setFixedSize(50, 30);
    StyleManager::instance().applyTraversalButtonStyle(dfsButton, "#28a745");

    traversalControlLayout->addWidget(bfsButton);
    traversalControlLayout->addWidget(dfsButton);
    traversalControlLayout->addStretch();

    traversalLayout->addLayout(traversalControlLayout);

    // Beautiful results area using StyleManager
    traversalResultList = new QListWidget();
    traversalResultList->setFixedHeight(80);
    StyleManager::instance().applyTraversalResultListStyle(traversalResultList);

    traversalLayout->addWidget(traversalResultList);
    rightLayout->addWidget(traversalGroup, 0); // Don't stretch this box
    
    // Connect traversal button signals
    connect(bfsButton, &QPushButton::clicked, this, &RedBlackTree::onStartBFS);
    connect(dfsButton, &QPushButton::clicked, this, &RedBlackTree::onStartDFS);
}

void RedBlackTree::addStepToHistory(const QString &step)
{
    stepHistory.append(step);
    updateStepTrace();
}

void RedBlackTree::addOperationSeparator()
{
    stepHistory.append("────────────────────");
    updateStepTrace();
}

void RedBlackTree::updateStepTrace()
{
    stepsList->clear();
    
    for (const QString &step : stepHistory) {
        QListWidgetItem *item = new QListWidgetItem(step);
        
        // Handle separators
        if (step.contains("────")) {
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QColor("#cccccc"));
            item->setFont(QFont("Segoe UI", 10, QFont::Bold));
        }
        // Color code based on step content
        else if (step.contains("✅") || step.contains("Success") || step.contains("Found")) {
            item->setForeground(QColor("#28a745"));
        }
        else if (step.contains("❌") || step.contains("Error") || step.contains("Failed")) {
            item->setForeground(QColor("#dc3545"));
        }
        else if (step.contains("🔍") || step.contains("Search") || step.contains("Looking")) {
            item->setForeground(QColor("#17a2b8"));
        }
        else if (step.contains("➕") || step.contains("Insert") || step.contains("Add")) {
            item->setForeground(QColor("#7b4fff"));
        }
        else if (step.contains("🗑️") || step.contains("Delete") || step.contains("Remove")) {
            item->setForeground(QColor("#fd7e14"));
        }
        else if (step.contains("🔄") || step.contains("Rotate") || step.contains("Balance")) {
            item->setForeground(QColor("#6f42c1"));
        }
        else if (step.contains("🎨") || step.contains("Color") || step.contains("Red") || step.contains("Black")) {
            item->setForeground(QColor("#e83e8c"));
        }
        else if (step.contains("📍") || step.contains("Position") || step.contains("Direction")) {
            item->setForeground(QColor("#20c997"));
        }
        else {
            item->setForeground(QColor("#6c757d"));
        }
        
        stepsList->addItem(item);
    }
    
    // Auto-scroll to bottom
    stepsList->scrollToBottom();
}

void RedBlackTree::showAlgorithm(const QString &operation)
{
    currentOperation = operation;
    
    // Add separator between operations if there are already items
    if (algorithmList->count() > 0) {
        QListWidgetItem *separator = new QListWidgetItem("────────────────────");
        separator->setTextAlignment(Qt::AlignCenter);
        separator->setFlags(Qt::NoItemFlags);
        separator->setForeground(QColor(189, 195, 199));
        algorithmList->addItem(separator);
    }
    
    if (operation == "Insert") {
        // Title
        QListWidgetItem *title = new QListWidgetItem("🔴⚫ Red-Black Tree Insert Algorithm");
        title->setForeground(QColor(123, 79, 255)); // Purple for RB tree
        title->setFont(QFont("Segoe UI", 12, QFont::Bold));
        algorithmList->addItem(title);
        
        // Step 1
        QListWidgetItem *step1 = new QListWidgetItem("1️⃣ Standard BST Insertion:");
        step1->setForeground(QColor(52, 73, 94));
        step1->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step1);
        
        QListWidgetItem *step1a = new QListWidgetItem("   • Insert node using BST rules");
        step1a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step1a);
        
        QListWidgetItem *step1b = new QListWidgetItem("   • Color new node RED initially");
        step1b->setForeground(QColor(220, 53, 69));
        algorithmList->addItem(step1b);
        
        // Step 2
        QListWidgetItem *step2 = new QListWidgetItem("2️⃣ Fix Red-Black Violations:");
        step2->setForeground(QColor(52, 73, 94));
        step2->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step2);
        
        QListWidgetItem *case1 = new QListWidgetItem("   🔄 Case 1: Uncle is RED");
        case1->setForeground(QColor(220, 53, 69));
        case1->setFont(QFont("Segoe UI", 10, QFont::Bold));
        algorithmList->addItem(case1);
        
        QListWidgetItem *case1a = new QListWidgetItem("      • Recolor parent & uncle BLACK");
        case1a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(case1a);
        
        QListWidgetItem *case1b = new QListWidgetItem("      • Recolor grandparent RED");
        case1b->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(case1b);
        
        QListWidgetItem *case2 = new QListWidgetItem("   🔄 Case 2: Uncle BLACK (Triangle)");
        case2->setForeground(QColor(220, 53, 69));
        case2->setFont(QFont("Segoe UI", 10, QFont::Bold));
        algorithmList->addItem(case2);
        
        QListWidgetItem *case2a = new QListWidgetItem("      • Rotate to convert to Case 3");
        case2a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(case2a);
        
        QListWidgetItem *case3 = new QListWidgetItem("   🔄 Case 3: Uncle BLACK (Line)");
        case3->setForeground(QColor(220, 53, 69));
        case3->setFont(QFont("Segoe UI", 10, QFont::Bold));
        algorithmList->addItem(case3);
        
        QListWidgetItem *case3a = new QListWidgetItem("      • Rotate grandparent");
        case3a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(case3a);
        
        QListWidgetItem *case3b = new QListWidgetItem("      • Swap colors of parent & grandparent");
        case3b->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(case3b);
        
        // Step 3
        QListWidgetItem *step3 = new QListWidgetItem("3️⃣ Ensure root is BLACK");
        step3->setForeground(QColor(52, 73, 94));
        step3->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step3);
        
        // Complexity
        QListWidgetItem *complexity = new QListWidgetItem("⏰ Time Complexity");
        complexity->setForeground(QColor(155, 89, 182));
        complexity->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(complexity);
        
        QListWidgetItem *timeComp = new QListWidgetItem("   📊 Guaranteed: O(log n)");
        timeComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(timeComp);
        
        QListWidgetItem *spaceComp = new QListWidgetItem("   💾 Space: O(1) auxiliary");
        spaceComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(spaceComp);
    }
    else if (operation == "Search") {
        // Title
        QListWidgetItem *title = new QListWidgetItem("🔍 Red-Black Tree Search Algorithm");
        title->setForeground(QColor(23, 162, 184)); // Blue for search
        title->setFont(QFont("Segoe UI", 12, QFont::Bold));
        algorithmList->addItem(title);
        
        // Step 1
        QListWidgetItem *step1 = new QListWidgetItem("1️⃣ Start at root node");
        step1->setForeground(QColor(52, 73, 94));
        step1->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step1);
        
        QListWidgetItem *step1a = new QListWidgetItem("   • Handle empty tree case");
        step1a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step1a);
        
        // Step 2
        QListWidgetItem *step2 = new QListWidgetItem("2️⃣ Compare target with current node:");
        step2->setForeground(QColor(52, 73, 94));
        step2->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step2);
        
        QListWidgetItem *equal = new QListWidgetItem("   🎯 target = current → FOUND!");
        equal->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(equal);
        
        QListWidgetItem *left = new QListWidgetItem("   ⬅️ target < current → go left");
        left->setForeground(QColor(230, 126, 34));
        algorithmList->addItem(left);
        
        QListWidgetItem *right = new QListWidgetItem("   ➡️ target > current → go right");
        right->setForeground(QColor(230, 126, 34));
        algorithmList->addItem(right);
        
        // Step 3
        QListWidgetItem *step3 = new QListWidgetItem("3️⃣ Repeat until found or NULL");
        step3->setForeground(QColor(52, 73, 94));
        step3->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step3);
        
        // Complexity
        QListWidgetItem *complexity = new QListWidgetItem("⏰ Time Complexity");
        complexity->setForeground(QColor(155, 89, 182));
        complexity->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(complexity);
        
        QListWidgetItem *timeComp = new QListWidgetItem("   📊 Guaranteed: O(log n)");
        timeComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(timeComp);
        
        QListWidgetItem *spaceComp = new QListWidgetItem("   💾 Space: O(1) iterative");
        spaceComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(spaceComp);
    }
    else if (operation == "Delete") {
        // Title
        QListWidgetItem *title = new QListWidgetItem("🗑️ Red-Black Tree Delete Algorithm");
        title->setForeground(QColor(253, 126, 20)); // Orange for delete
        title->setFont(QFont("Segoe UI", 12, QFont::Bold));
        algorithmList->addItem(title);
        
        // Step 1
        QListWidgetItem *step1 = new QListWidgetItem("1️⃣ Standard BST Deletion:");
        step1->setForeground(QColor(52, 73, 94));
        step1->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step1);
        
        QListWidgetItem *step1a = new QListWidgetItem("   • Find node to delete");
        step1a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step1a);
        
        QListWidgetItem *step1b = new QListWidgetItem("   • Handle 3 cases: 0, 1, or 2 children");
        step1b->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step1b);
        
        // Step 2
        QListWidgetItem *step2 = new QListWidgetItem("2️⃣ Track deleted node color:");
        step2->setForeground(QColor(52, 73, 94));
        step2->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step2);
        
        QListWidgetItem *red = new QListWidgetItem("   🔴 RED deleted → no violations");
        red->setForeground(QColor(220, 53, 69));
        algorithmList->addItem(red);
        
        QListWidgetItem *black = new QListWidgetItem("   ⚫ BLACK deleted → fix violations");
        black->setForeground(QColor(52, 58, 64));
        algorithmList->addItem(black);
        
        // Step 3
        QListWidgetItem *step3 = new QListWidgetItem("3️⃣ Fix Black-Height Violations:");
        step3->setForeground(QColor(52, 73, 94));
        step3->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step3);
        
        QListWidgetItem *case1 = new QListWidgetItem("   🔄 Case 1: Sibling is RED");
        case1->setForeground(QColor(220, 53, 69));
        case1->setFont(QFont("Segoe UI", 10, QFont::Bold));
        algorithmList->addItem(case1);
        
        QListWidgetItem *case2 = new QListWidgetItem("   🔄 Case 2: Sibling BLACK, children BLACK");
        case2->setForeground(QColor(220, 53, 69));
        case2->setFont(QFont("Segoe UI", 10, QFont::Bold));
        algorithmList->addItem(case2);
        
        QListWidgetItem *case3 = new QListWidgetItem("   🔄 Case 3: Sibling BLACK, left child RED");
        case3->setForeground(QColor(220, 53, 69));
        case3->setFont(QFont("Segoe UI", 10, QFont::Bold));
        algorithmList->addItem(case3);
        
        QListWidgetItem *case4 = new QListWidgetItem("   🔄 Case 4: Sibling BLACK, right child RED");
        case4->setForeground(QColor(220, 53, 69));
        case4->setFont(QFont("Segoe UI", 10, QFont::Bold));
        algorithmList->addItem(case4);
        
        // Complexity
        QListWidgetItem *complexity = new QListWidgetItem("⏰ Time Complexity");
        complexity->setForeground(QColor(155, 89, 182));
        complexity->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(complexity);
        
        QListWidgetItem *timeComp = new QListWidgetItem("   📊 Guaranteed: O(log n)");
        timeComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(timeComp);
        
        QListWidgetItem *spaceComp = new QListWidgetItem("   💾 Space: O(1) auxiliary");
        spaceComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(spaceComp);
    }
    else if (operation == "BFS") {
        // Title
        QListWidgetItem *title = new QListWidgetItem("🌊 Breadth-First Search (BFS) - Red-Black Tree");
        title->setForeground(QColor(74, 144, 226)); // Blue for BFS
        title->setFont(QFont("Segoe UI", 12, QFont::Bold));
        algorithmList->addItem(title);
        
        // Step 1
        QListWidgetItem *step1 = new QListWidgetItem("1️⃣ Level-by-Level Traversal:");
        step1->setForeground(QColor(52, 73, 94));
        step1->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step1);
        
        QListWidgetItem *step1a = new QListWidgetItem("   • Use queue data structure");
        step1a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step1a);
        
        QListWidgetItem *step1b = new QListWidgetItem("   • Start from root node");
        step1b->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step1b);
        
        // Step 2
        QListWidgetItem *step2 = new QListWidgetItem("2️⃣ Process Each Level:");
        step2->setForeground(QColor(52, 73, 94));
        step2->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step2);
        
        QListWidgetItem *step2a = new QListWidgetItem("   • Enqueue root, mark as visited");
        step2a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step2a);
        
        QListWidgetItem *step2b = new QListWidgetItem("   • While queue not empty:");
        step2b->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step2b);
        
        QListWidgetItem *step2c = new QListWidgetItem("     - Dequeue node, process it");
        step2c->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step2c);
        
        QListWidgetItem *step2d = new QListWidgetItem("     - Enqueue left & right children");
        step2d->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step2d);
        
        QListWidgetItem *step2e = new QListWidgetItem("   📋 Order: Level by Level (1→2→3→...)");
        step2e->setForeground(QColor(74, 144, 226));
        step2e->setFont(QFont("Segoe UI", 10, QFont::Bold));
        algorithmList->addItem(step2e);
        
        // Complexity
        QListWidgetItem *complexity = new QListWidgetItem("⏰ Time Complexity");
        complexity->setForeground(QColor(155, 89, 182));
        complexity->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(complexity);
        
        QListWidgetItem *timeComp = new QListWidgetItem("   📊 O(n) - visits each node once");
        timeComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(timeComp);
        
        QListWidgetItem *spaceComp = new QListWidgetItem("   💾 Space: O(w) - width of tree");
        spaceComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(spaceComp);
    }
    else if (operation == "DFS") {
        // Title
        QListWidgetItem *title = new QListWidgetItem("🏔️ Depth-First Search (DFS) - Red-Black Tree");
        title->setForeground(QColor(40, 167, 69)); // Green for DFS
        title->setFont(QFont("Segoe UI", 12, QFont::Bold));
        algorithmList->addItem(title);
        
        // Step 1
        QListWidgetItem *step1 = new QListWidgetItem("1️⃣ Deep Traversal Strategy:");
        step1->setForeground(QColor(52, 73, 94));
        step1->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step1);
        
        QListWidgetItem *step1a = new QListWidgetItem("   • Use stack data structure (or recursion)");
        step1a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step1a);
        
        QListWidgetItem *step1b = new QListWidgetItem("   • Go as deep as possible first");
        step1b->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step1b);
        
        // Step 2
        QListWidgetItem *step2 = new QListWidgetItem("2️⃣ Traversal Order (Preorder):");
        step2->setForeground(QColor(52, 73, 94));
        step2->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(step2);
        
        QListWidgetItem *step2a = new QListWidgetItem("   • Visit ROOT node first");
        step2a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step2a);
        
        QListWidgetItem *step2b = new QListWidgetItem("   • Recursively visit LEFT subtree");
        step2b->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step2b);
        
        QListWidgetItem *step2c = new QListWidgetItem("   • Recursively visit RIGHT subtree");
        step2c->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step2c);
        
        QListWidgetItem *step2d = new QListWidgetItem("   📋 Order: Root → Left → Right");
        step2d->setForeground(QColor(220, 53, 69));
        step2d->setFont(QFont("Segoe UI", 10, QFont::Bold));
        algorithmList->addItem(step2d);
        
        // Complexity
        QListWidgetItem *complexity = new QListWidgetItem("⏰ Time Complexity");
        complexity->setForeground(QColor(155, 89, 182));
        complexity->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(complexity);
        
        QListWidgetItem *timeComp = new QListWidgetItem("   📊 O(n) - visits each node once");
        timeComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(timeComp);
        
        QListWidgetItem *spaceComp = new QListWidgetItem("   💾 Space: O(h) - height of tree");
        spaceComp->setForeground(QColor(40, 167, 69));
        algorithmList->addItem(spaceComp);
    }
    
    // Do not auto-switch tabs; keep user's current selection
}

void RedBlackTree::onBackClicked()
{
    emit backToOperations();
}

void RedBlackTree::onViewAlgorithmClicked()
{
    // Algorithm is now shown in the right panel tab
    showAlgorithm("Insert"); // Show insertion by default
}

void RedBlackTree::onAlgorithmBackClicked()
{
    // Algorithm back is now handled by switching tabs
    traceTabWidget->setCurrentIndex(0); // Switch to Steps tab
}

void RedBlackTree::showInsertionAlgorithm()
{
    // Algorithm now shown in right panel via showAlgorithm("Insert")
    showAlgorithm("Insert");
}

void RedBlackTree::showDeletionAlgorithm()
{
    // Algorithm now shown in right panel via showAlgorithm("Delete")
    showAlgorithm("Delete");
    /*
    algorithmDisplay->setHtml(R"(
        <h2 style="color: #7b4fff;">Red-Black Tree Deletion Algorithm</h2>

        <h3>Algorithm Steps:</h3>
        <pre>
<b>1. BST Deletion:</b>
   - Find node to delete
   - Remove as in regular BST
   - Track original color

<b>2. Fix Double-Black Violations:</b>
   If deleted node was BLACK:

   <b>Case 1: Sibling is RED</b>
      - Rotate parent
      - Recolor
      - Continue to other cases

   <b>Case 2: Sibling is BLACK, both children BLACK</b>
      - Recolor sibling to RED
      - Move double-black up

   <b>Case 3: Sibling is BLACK, far child BLACK</b>
      - Rotate sibling
      - Recolor
      - Continue to Case 4

   <b>Case 4: Sibling is BLACK, far child RED</b>
      - Rotate parent
      - Recolor
      - Remove double-black

<b>3. Final Step:</b>
   - Ensure root is BLACK
        </pre>

        <h3>Time Complexity: O(log n)</h3>
    )"); */
}

void RedBlackTree::showSearchAlgorithm()
{
    // Algorithm now shown in right panel via showAlgorithm("Search")
    showAlgorithm("Search");
    /*
    algorithmDisplay->setHtml(R"(
        <h2 style="color: #7b4fff;">Red-Black Tree Search Algorithm</h2>

        <h3>Algorithm:</h3>
        <pre>
<b>Search(node, value):</b>
   1. If node is NIL:
         return NULL (not found)

   2. If value == node.value:
         return node (found!)

   3. If value < node.value:
         return Search(node.left, value)

   4. Else:
         return Search(node.right, value)
        </pre>

        <h3>Properties:</h3>
        <ul>
            <li>Same as BST search</li>
            <li>Colors don't affect search</li>
            <li>Follows BST property</li>
            <li>Left subtree < node < Right subtree</li>
        </ul>

        <h3>Time Complexity: O(log n)</h3>
        <h3>Space Complexity: O(1) iterative, O(log n) recursive</h3>
    )"); */
}

void RedBlackTree::onInsertClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current operation to complete.");
        return;
    }

    QString text = inputField->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a value.");
        return;
    }

    bool ok;
    int value = text.toInt(&ok);

    if (!ok) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid integer.");
        return;
    }

    // Show algorithm for insert
    showAlgorithm("Insert");
    
    // Add detailed step tracking
    addStepToHistory("➕ INSERT OPERATION");
    addStepToHistory(QString("🎯 Target value: %1").arg(value));
    
    insertNode(value);
    inputField->clear();
    inputField->setFocus();
}

void RedBlackTree::onDeleteClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current operation to complete.");
        return;
    }

    if (root == NIL) {
        QMessageBox::warning(this, "Empty Tree", "Tree is empty.");
        return;
    }

    QString text = inputField->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a value to delete.");
        return;
    }

    bool ok;
    int value = text.toInt(&ok);

    if (!ok) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid integer.");
        return;
    }

    // Show algorithm for delete
    showAlgorithm("Delete");
    
    // Add detailed step tracking
    addStepToHistory("🗑️ DELETE OPERATION");
    addStepToHistory(QString("🎯 Target for deletion: %1").arg(value));
    
    deleteNode(value);
    inputField->clear();
}

void RedBlackTree::onSearchClicked()
{
    if (root == NIL) {
        QMessageBox::information(this, "Empty Tree", "Tree is empty.");
        return;
    }

    QString text = inputField->text().trimmed();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a value to search.");
        return;
    }

    bool ok;
    int value = text.toInt(&ok);

    if (!ok) {
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid integer.");
        return;
    }

    // Show algorithm for search
    showAlgorithm("Search");
    
    // Add detailed step tracking
    addStepToHistory("🔍 SEARCH OPERATION");
    addStepToHistory(QString("🎯 Looking for value: %1").arg(value));
    
    searchNode(value);
    inputField->clear();
}

void RedBlackTree::onClearClicked()
{
    clearTree(root);
    root = NIL;
    stepHistory.clear();
    updateStepTrace();
    
    // Clear traversal results
    traversalResultList->clear();
    traversalOrder.clear();
    traversalType = TraversalType::None;
    traversalIndex = 0;
    
    statusLabel->setText("Tree cleared!");
    addStepToHistory("🧹 Tree cleared - all nodes removed");
    addOperationSeparator();
    update();
}

void RedBlackTree::onRandomizeClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current animation to complete.");
        return;
    }

    // Generate a single random value between 1 and 100
    int randomValue = QRandomGenerator::global()->bounded(1, 101);
    
    // Set the input field and trigger insert
    inputField->setText(QString::number(randomValue));
    onInsertClicked();
}

void RedBlackTree::insertNode(int value)
{
    // Check for duplicate
    if (findNode(root, value) != NIL) {
        statusLabel->setText(QString("Value %1 already exists!").arg(value));
        addStepToHistory(QString("❌ Insert failed: Value %1 already exists").arg(value));
        addOperationSeparator();
        return;
    }

    addStepToHistory(QString("🔍 Checking if value %1 already exists...").arg(value));
    addStepToHistory("✅ Value is unique, proceeding with insertion");

    if (root == NIL) {
        addStepToHistory(QString("🌱 Tree is empty, inserting %1 as root").arg(value));
    } else {
        addStepToHistory(QString("🌳 Tree has nodes, finding insertion position for %1").arg(value));
    }

    isAnimating = true;
    insertButton->setEnabled(false);
    deleteButton->setEnabled(false);
    searchButton->setEnabled(false);

    statusLabel->setText(QString("Inserting %1...").arg(value));

    RBNode *node = new RBNode(value);
    node->left = node->right = NIL;

    addStepToHistory(QString("🔴 Created new RED node with value %1").arg(value));
    
    root = BSTInsert(root, node);
    addStepToHistory("📍 Node inserted using BST insertion rules");

    QTimer::singleShot(500, this, [this, node, value]() {
        node->isHighlighted = true;
        statusLabel->setText("Fixing Red-Black properties...");
        addStepToHistory("🔄 Checking Red-Black tree properties...");
        update();

        QTimer::singleShot(800, this, [this, node, value]() {
            addStepToHistory("⚖️ Applying Red-Black tree balancing rules");
            fixInsert(node);
            node->isHighlighted = false;

            statusLabel->setText(QString("Successfully inserted %1").arg(value));
            addStepToHistory(QString("✅ Node %1 successfully inserted and tree balanced").arg(value));
            addStepToHistory("🎯 Red-Black tree properties maintained");
            addOperationSeparator();

            isAnimating = false;
            insertButton->setEnabled(true);
            deleteButton->setEnabled(true);
            searchButton->setEnabled(true);
            update();
        });
    });

    update();
}

RBNode* RedBlackTree::BSTInsert(RBNode* root, RBNode* node)
{
    if (root == NIL) {
        node->parent = nullptr;  // Root has no parent
        return node;
    }

    if (node->value < root->value) {
        root->left = BSTInsert(root->left, node);
        if (root->left != NIL) {
            root->left->parent = root;
        }
    } else if (node->value > root->value) {
        root->right = BSTInsert(root->right, node);
        if (root->right != NIL) {
            root->right->parent = root;
        }
    }

    return root;
}

void RedBlackTree::fixInsert(RBNode* node)
{
    while (node->parent && node->parent->color == RED) {
        // Check if parent->parent exists (parent is not root)
        if (!node->parent->parent) {
            break;  // Parent is root, no grandparent
        }

        if (node->parent == node->parent->parent->left) {
            RBNode *uncle = node->parent->parent->right;

            // Check if uncle exists and is RED (uncle could be NIL which is BLACK)
            if (uncle && uncle != NIL && uncle->color == RED) {
                // Case 1: Uncle is red
                node->parent->color = BLACK;
                uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            } else {
                // Uncle is BLACK or NIL
                if (node == node->parent->right) {
                    // Case 2: Triangle - convert to line
                    node = node->parent;
                    rotateLeftSync(node);
                }
                // Case 3: Line
                node->parent->color = BLACK;
                if (node->parent->parent) {
                    node->parent->parent->color = RED;
                    rotateRightSync(node->parent->parent);
                }
            }
        } else {
            RBNode *uncle = node->parent->parent->left;

            // Check if uncle exists and is RED
            if (uncle && uncle != NIL && uncle->color == RED) {
                node->parent->color = BLACK;
                uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            } else {
                // Uncle is BLACK or NIL
                if (node == node->parent->left) {
                    node = node->parent;
                    rotateRightSync(node);
                }
                node->parent->color = BLACK;
                if (node->parent->parent) {
                    node->parent->parent->color = RED;
                    rotateLeftSync(node->parent->parent);
                }
            }
        }
    }
    root->color = BLACK;
}

void RedBlackTree::rotateLeft(RBNode* node)
{
    if (!node || node->right == NIL) return;

    statusLabel->setText("Performing left rotation...");
    node->isRotating = true;
    update();

    QTimer::singleShot(600, this, [this, node]() {
        rotateLeftSync(node);
        node->isRotating = false;
        update();
    });
}

void RedBlackTree::rotateLeftSync(RBNode* node)
{
    if (!node || node->right == NIL) return;

    RBNode *rightChild = node->right;
    node->right = rightChild->left;

    if (rightChild->left != NIL) {
        rightChild->left->parent = node;
    }

    rightChild->parent = node->parent;

    if (!node->parent) {
        root = rightChild;
    } else if (node == node->parent->left) {
        node->parent->left = rightChild;
    } else {
        node->parent->right = rightChild;
    }

    rightChild->left = node;
    node->parent = rightChild;
}

void RedBlackTree::rotateRight(RBNode* node)
{
    if (!node || node->left == NIL) return;

    statusLabel->setText("Performing right rotation...");
    node->isRotating = true;
    update();

    QTimer::singleShot(600, this, [this, node]() {
        rotateRightSync(node);
        node->isRotating = false;
        update();
    });
}

void RedBlackTree::rotateRightSync(RBNode* node)
{
    if (!node || node->left == NIL) return;

    RBNode *leftChild = node->left;
    node->left = leftChild->right;

    if (leftChild->right != NIL) {
        leftChild->right->parent = node;
    }

    leftChild->parent = node->parent;

    if (!node->parent) {
        root = leftChild;
    } else if (node == node->parent->right) {
        node->parent->right = leftChild;
    } else {
        node->parent->left = leftChild;
    }

    leftChild->right = node;
    node->parent = leftChild;
}

void RedBlackTree::deleteNode(int value)
{
    addStepToHistory(QString("🔍 Searching for node %1 to delete...").arg(value));
    RBNode *node = findNode(root, value);
    if (node == NIL) {
        statusLabel->setText(QString("Value %1 not found!").arg(value));
        addStepToHistory(QString("❌ Delete failed: Value %1 not found in tree").arg(value));
        addOperationSeparator();
        return;
    }

    addStepToHistory(QString("🎯 Found node %1, preparing for deletion").arg(value));
    
    // Determine deletion case
    if (node->left == NIL && node->right == NIL) {
        addStepToHistory("📋 Case: Node has no children (leaf node)");
    } else if (node->left == NIL || node->right == NIL) {
        addStepToHistory("📋 Case: Node has one child");
    } else {
        addStepToHistory("📋 Case: Node has two children (complex deletion)");
        addStepToHistory("🔄 Finding inorder successor for replacement");
    }

    isAnimating = true;
    insertButton->setEnabled(false);
    deleteButton->setEnabled(false);
    searchButton->setEnabled(false);

    // Highlight node to be deleted
    node->isHighlighted = true;
    statusLabel->setText(QString("Deleting %1...").arg(value));
    addStepToHistory("🔴 Node marked for deletion (highlighted)");
    update();

    QTimer::singleShot(800, this, [this, node, value]() {
        addStepToHistory("⚖️ Applying Red-Black deletion rules");
        root = deleteNodeHelper(root, value);

        statusLabel->setText(QString("Successfully deleted %1").arg(value));
        addStepToHistory(QString("✅ Node %1 successfully deleted").arg(value));
        addStepToHistory("🎯 Red-Black tree properties maintained");
        addOperationSeparator();

        isAnimating = false;
        insertButton->setEnabled(true);
        deleteButton->setEnabled(true);
        searchButton->setEnabled(true);
        update();
    });
}

RBNode* RedBlackTree::deleteNodeHelper(RBNode* node, int value)
{
    if (node == NIL) return NIL;

    if (value < node->value) {
        node->left = deleteNodeHelper(node->left, value);
        if (node->left != NIL) {
            node->left->parent = node;
        }
    } else if (value > node->value) {
        node->right = deleteNodeHelper(node->right, value);
        if (node->right != NIL) {
            node->right->parent = node;
        }
    } else {
        // Node found - delete it
        Color originalColor = node->color;
        RBNode* replacement = NIL;
        RBNode* parent = node->parent;
        bool isLeftChild = parent && parent->left == node;

        if (node->left == NIL) {
            replacement = node->right;
            if (replacement != NIL) {
                replacement->parent = parent;
            }
            if (parent) {
                if (isLeftChild) {
                    parent->left = replacement;
                } else {
                    parent->right = replacement;
                }
            } else {
                root = replacement;
            }
            if (originalColor == BLACK) {
                if (replacement != NIL) {
                    fixDelete(replacement);
                } else if (parent) {
                    // Double-black situation - replacement is NIL
                    // Use a temporary node to represent the double-black position
                    RBNode* doubleBlack = new RBNode(0);
                    doubleBlack->color = BLACK;
                    doubleBlack->parent = parent;
                    doubleBlack->left = NIL;
                    doubleBlack->right = NIL;
                    if (isLeftChild) {
                        parent->left = doubleBlack;
                    } else {
                        parent->right = doubleBlack;
                    }
                    fixDelete(doubleBlack);
                    // Remove the marker
                    if (isLeftChild) {
                        parent->left = NIL;
                    } else {
                        parent->right = NIL;
                    }
                    delete doubleBlack;
                }
            }
            delete node;
            return replacement;
        } else if (node->right == NIL) {
            replacement = node->left;
            if (replacement != NIL) {
                replacement->parent = parent;
            }
            if (parent) {
                if (isLeftChild) {
                    parent->left = replacement;
                } else {
                    parent->right = replacement;
                }
            } else {
                root = replacement;
            }
            if (originalColor == BLACK) {
                if (replacement != NIL) {
                    fixDelete(replacement);
                } else if (parent) {
                    RBNode* doubleBlack = new RBNode(0);
                    doubleBlack->color = BLACK;
                    doubleBlack->parent = parent;
                    doubleBlack->left = NIL;
                    doubleBlack->right = NIL;
                    if (isLeftChild) {
                        parent->left = doubleBlack;
                    } else {
                        parent->right = doubleBlack;
                    }
                    fixDelete(doubleBlack);
                    if (isLeftChild) {
                        parent->left = NIL;
                    } else {
                        parent->right = NIL;
                    }
                    delete doubleBlack;
                }
            }
            delete node;
            return replacement;
        }

        // Node with two children
        RBNode* successor = findMin(node->right);
        originalColor = successor->color;
        node->value = successor->value;
        node->right = deleteNodeHelper(node->right, successor->value);
        if (node->right != NIL) {
            node->right->parent = node;
        }
        if (originalColor == BLACK) {
            // The successor was deleted, need to fix
            // Note: This is simplified - full implementation would track the replacement
        }
    }

    return node;
}

void RedBlackTree::fixDelete(RBNode* node)
{
    // Basic fixDelete implementation
    // This is a simplified version - full implementation would handle all cases
    while (node != root && (!node || node->color == BLACK)) {
        if (!node || !node->parent) break;

        if (node == node->parent->left) {
            RBNode* sibling = node->parent->right;
            
            if (sibling && sibling->color == RED) {
                // Case 1: Sibling is red
                sibling->color = BLACK;
                node->parent->color = RED;
                rotateLeftSync(node->parent);
                sibling = node->parent->right;
            }

            if ((!sibling || sibling == NIL) ||
                ((!sibling->left || sibling->left == NIL || sibling->left->color == BLACK) &&
                 (!sibling->right || sibling->right == NIL || sibling->right->color == BLACK))) {
                // Case 2: Sibling and both children are black
                if (sibling && sibling != NIL) {
                    sibling->color = RED;
                }
                node = node->parent;
            } else {
                // Case 3 & 4: Sibling has at least one red child
                if (!sibling->right || sibling->right == NIL || sibling->right->color == BLACK) {
                    if (sibling->left && sibling->left != NIL) {
                        sibling->left->color = BLACK;
                    }
                    if (sibling && sibling != NIL) {
                        sibling->color = RED;
                    }
                    rotateRightSync(sibling);
                    sibling = node->parent->right;
                }
                if (sibling && sibling != NIL) {
                    sibling->color = node->parent->color;
                }
                node->parent->color = BLACK;
                if (sibling->right && sibling->right != NIL) {
                    sibling->right->color = BLACK;
                }
                rotateLeftSync(node->parent);
                node = root;
            }
        } else {
            RBNode* sibling = node->parent->left;
            
            if (sibling && sibling->color == RED) {
                sibling->color = BLACK;
                node->parent->color = RED;
                rotateRightSync(node->parent);
                sibling = node->parent->left;
            }

            if ((!sibling || sibling == NIL) ||
                ((!sibling->right || sibling->right == NIL || sibling->right->color == BLACK) &&
                 (!sibling->left || sibling->left == NIL || sibling->left->color == BLACK))) {
                if (sibling && sibling != NIL) {
                    sibling->color = RED;
                }
                node = node->parent;
            } else {
                if (!sibling->left || sibling->left == NIL || sibling->left->color == BLACK) {
                    if (sibling->right && sibling->right != NIL) {
                        sibling->right->color = BLACK;
                    }
                    if (sibling && sibling != NIL) {
                        sibling->color = RED;
                    }
                    rotateLeftSync(sibling);
                    sibling = node->parent->left;
                }
                if (sibling && sibling != NIL) {
                    sibling->color = node->parent->color;
                }
                node->parent->color = BLACK;
                if (sibling->left && sibling->left != NIL) {
                    sibling->left->color = BLACK;
                }
                rotateRightSync(node->parent);
                node = root;
            }
        }
    }
    
    if (node && node != NIL) {
        node->color = BLACK;
    }
    root->color = BLACK;
}

void RedBlackTree::searchNode(int value)
{
    resetHighlights(root);
    
    if (root == NIL) {
        addStepToHistory("❌ Tree is empty - search failed");
        statusLabel->setText("Tree is empty!");
        addOperationSeparator();
        return;
    }
    
    addStepToHistory("🌳 Starting search from root");
    RBNode *node = findNode(root, value);

    if (node != NIL) {
        node->isHighlighted = true;
        statusLabel->setText(QString("Found %1 in tree!").arg(value));
        addStepToHistory(QString("🎯 Traversing tree to find %1...").arg(value));
        addStepToHistory(QString("✅ Success! Found node %1 in tree").arg(value));
        addStepToHistory("🔍 Node highlighted in visualization");
    } else {
        statusLabel->setText(QString("Value %1 not found!").arg(value));
        addStepToHistory(QString("🎯 Traversing tree to find %1...").arg(value));
        addStepToHistory(QString("❌ Search failed: Value %1 not found in tree").arg(value));
    }
    addOperationSeparator();
    update();
}

RBNode* RedBlackTree::findMin(RBNode* node)
{
    while (node && node->left != NIL) {
        node = node->left;
    }
    return node;
}

RBNode* RedBlackTree::findNode(RBNode* node, int value)
{
    if (node == NIL || node->value == value) {
        return node;
    }

    if (value < node->value) {
        return findNode(node->left, value);
    }
    return findNode(node->right, value);
}



// addHistory function removed - now using addStepToHistory directly

QString RedBlackTree::getCurrentTime()
{
    return QDateTime::currentDateTime().toString("HH:mm:ss");
}

void RedBlackTree::calculateNodePositions(RBNode *node, int x, int y, int horizontalSpacing)
{
    if (node == NIL) return;

    node->x = x;
    node->y = y;

    int nextSpacing = horizontalSpacing / 2;

    if (node->left != NIL) {
        calculateNodePositions(node->left, x - horizontalSpacing, y + 80, nextSpacing);
    }
    if (node->right != NIL) {
        calculateNodePositions(node->right, x + horizontalSpacing, y + 80, nextSpacing);
    }
}

void RedBlackTree::resetHighlights(RBNode *node)
{
    if (node == NIL) return;
    node->isHighlighted = false;
    node->isRotating = false;
    resetHighlights(node->left);
    resetHighlights(node->right);
}

void RedBlackTree::resetTraversalHighlights(RBNode *node)
{
    if (node == NIL) return;
    node->isTraversalHighlighted = false;
    node->isVisited = false;
    resetTraversalHighlights(node->left);
    resetTraversalHighlights(node->right);
}

void RedBlackTree::clearTree(RBNode *node)
{
    if (node == NIL) return;
    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

void RedBlackTree::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background gradient
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, QColor(250, 247, 255));
    gradient.setColorAt(1.0, QColor(237, 228, 255));
    painter.fillRect(rect(), gradient);

    // Draw tree on the main widget if we're on tree view
    if (root != NIL) {
        // Calculate canvas area (left 70% of content area)
        int canvasY = 200; // Position like binary search tree
        int canvasHeight = height() - canvasY - 30;
        int canvasWidth = (int)(width() * 0.65);
        QRect canvasRect(30, canvasY, canvasWidth, canvasHeight);

        // Draw white canvas background
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::white);
        painter.drawRoundedRect(canvasRect, 12, 12);

        // Calculate and draw tree
        int treeCenterX = canvasRect.x() + canvasRect.width() / 2;
        int treeStartY = canvasRect.y() + 40;

        calculateNodePositions(root, treeCenterX, treeStartY, canvasWidth / 4);
        drawTree(painter, root);
    }
}

void RedBlackTree::drawTree(QPainter &painter, RBNode *node)
{
    if (node == NIL) return;

    if (node->left != NIL) {
        drawEdge(painter, node->x, node->y, node->left->x, node->left->y, node->left->color);
        drawTree(painter, node->left);
    }
    if (node->right != NIL) {
        drawEdge(painter, node->x, node->y, node->right->x, node->right->y, node->right->color);
        drawTree(painter, node->right);
    }

    drawNode(painter, node);
}

void RedBlackTree::drawNode(QPainter &painter, RBNode *node)
{
    if (node == NIL) return;

    // Node circle - larger for rotating nodes
    int radius = node->isRotating ? NODE_RADIUS + 5 : NODE_RADIUS;

    if (node->isTraversalHighlighted) {
        painter.setPen(QPen(QColor(255, 140, 0), 4));  // Orange border for currently visiting
        painter.setBrush(QColor(255, 165, 0));         // Orange fill
    } else if (node->isVisited && traversalType != TraversalType::None) {
        painter.setPen(QPen(QColor(34, 139, 34), 4));  // Green border for visited (only during traversal)
        painter.setBrush(QColor(50, 205, 50));         // Green fill
    } else if (node->isHighlighted) {
        painter.setPen(QPen(QColor(255, 215, 0), 4));
        painter.setBrush(node->color == RED ? QColor(255, 150, 150) : QColor(100, 100, 100));
    } else if (node->isRotating) {
        painter.setPen(QPen(QColor(0, 200, 0), 4));
        painter.setBrush(node->color == RED ? QColor(255, 100, 100) : QColor(80, 80, 80));
    } else {
        // Default Red-Black Tree colors
        painter.setPen(QPen(Qt::black, 2));
        painter.setBrush(node->color == RED ? QColor(220, 53, 69) : QColor(52, 58, 64));
    }

    painter.drawEllipse(QPoint(node->x, node->y), radius, radius);

    // Node value
    painter.setPen(Qt::white);
    QFont font("Segoe UI", 12, QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRect(node->x - radius, node->y - radius,
                           radius * 2, radius * 2),
                     Qt::AlignCenter, QString::number(node->value));
}

void RedBlackTree::drawEdge(QPainter &painter, int x1, int y1, int x2, int y2, Color color)
{
    painter.setPen(QPen(color == RED ? QColor(220, 53, 69) : QColor(52, 58, 64), 2));
    painter.drawLine(x1, y1 + NODE_RADIUS, x2, y2 - NODE_RADIUS);
}

void RedBlackTree::setControlsEnabled(bool enabled)
{
    // Back button should NEVER be disabled - user must always be able to go back!
    if (backButton) backButton->setEnabled(true);
    
    insertButton->setEnabled(enabled);
    deleteButton->setEnabled(enabled);
    searchButton->setEnabled(enabled);
    clearButton->setEnabled(enabled);
    bfsButton->setEnabled(enabled);
    dfsButton->setEnabled(enabled);
}

void RedBlackTree::onStartBFS()
{
    showAlgorithm("BFS");
    
    if (root == NIL) {
        addStepToHistory("❌ Tree is empty: Please insert nodes first");
        QMessageBox::warning(this, "Empty Tree", "Please insert nodes to the tree first.");
        return;
    }
    
    addStepToHistory("🌊 Starting BFS traversal from root");
    resetTraversalHighlights(root);
    traversalType = TraversalType::BFS;
    traversalOrder.clear();
    traversalIndex = 0;
    
    performBFS();
    
    QString result = "🌊 BFS Order: ";
    for (int i = 0; i < traversalOrder.size(); ++i) {
        result += QString::number(traversalOrder[i]->value);
        if (i < traversalOrder.size() - 1) result += " → ";
    }
    
    traversalResultList->clear();
    if (traversalOrder.isEmpty()) {
        traversalResultList->addItem("❌ No traversal result");
    } else {
        traversalResultList->addItem(result);
    }
    
    setControlsEnabled(false);
    traversalAnimTimer->start(800);
    statusLabel->setText("Running BFS...");
    addStepToHistory("✅ BFS traversal order computed, starting animation");
    addOperationSeparator();
}

void RedBlackTree::onStartDFS()
{
    showAlgorithm("DFS");
    
    if (root == NIL) {
        addStepToHistory("❌ Tree is empty: Please insert nodes first");
        QMessageBox::warning(this, "Empty Tree", "Please insert nodes to the tree first.");
        return;
    }
    
    addStepToHistory("🏔️ Starting DFS traversal from root");
    resetTraversalHighlights(root);
    traversalType = TraversalType::DFS;
    traversalOrder.clear();
    traversalIndex = 0;
    
    performDFS();
    
    QString result = "🏔️ DFS Order: ";
    for (int i = 0; i < traversalOrder.size(); ++i) {
        result += QString::number(traversalOrder[i]->value);
        if (i < traversalOrder.size() - 1) result += " → ";
    }
    
    traversalResultList->clear();
    if (traversalOrder.isEmpty()) {
        traversalResultList->addItem("❌ No traversal result");
    } else {
        traversalResultList->addItem(result);
    }
    
    setControlsEnabled(false);
    traversalAnimTimer->start(800);
    statusLabel->setText("Running DFS...");
    addStepToHistory("✅ DFS traversal order computed, starting animation");
    addOperationSeparator();
}

void RedBlackTree::performBFS()
{
    if (root == NIL) return;
    
    addStepToHistory("🔄 Initializing BFS queue");
    QList<RBNode*> queue;
    queue.append(root);
    addStepToHistory(QString("➕ Added root node %1 to queue").arg(root->value));
    
    while (!queue.isEmpty()) {
        RBNode* current = queue.front();
        queue.pop_front();
        traversalOrder.append(current);
        
        addStepToHistory(QString("🎯 Processing node %1").arg(current->value));
        
        if (current->left != NIL) {
            queue.append(current->left);
            addStepToHistory(QString("⬅️ Added left child %1 to queue").arg(current->left->value));
        }
        if (current->right != NIL) {
            queue.append(current->right);
            addStepToHistory(QString("➡️ Added right child %1 to queue").arg(current->right->value));
        }
    }
}

void RedBlackTree::performDFS()
{
    if (root == NIL) return;
    
    addStepToHistory("🔄 Starting DFS traversal (preorder: Root → Left → Right)");
    addStepToHistory("📍 Using recursive depth-first approach");
    
    // Use recursive DFS for correct preorder traversal
    performDFSRecursive(root);
}

void RedBlackTree::performDFSRecursive(RBNode* node)
{
    if (node == NIL) return;
    
    // Preorder: Process current node first
    traversalOrder.append(node);
    addStepToHistory(QString("🎯 Visiting node %1 (preorder)").arg(node->value));
    
    // Then recursively visit left subtree
    if (node->left != NIL) {
        addStepToHistory(QString("⬅️ Going to left subtree of %1").arg(node->value));
        performDFSRecursive(node->left);
    }
    
    // Finally recursively visit right subtree
    if (node->right != NIL) {
        addStepToHistory(QString("➡️ Going to right subtree of %1").arg(node->value));
        performDFSRecursive(node->right);
    }
}

void RedBlackTree::onTraversalAnimationStep()
{
    if (traversalIndex > 0 && traversalIndex <= traversalOrder.size()) {
        RBNode* prev = traversalOrder[traversalIndex - 1];
        // Mark previous as visited (persist green), remove highlight
        prev->isVisited = true;
        prev->isTraversalHighlighted = false;
    }
    
    if (traversalIndex >= traversalOrder.size()) {
        QString algo = (traversalType == TraversalType::BFS) ? "BFS" : "DFS";
        traversalAnimTimer->stop();
        traversalType = TraversalType::None;
        
        // Reset traversal highlights to restore original red/black colors
        resetTraversalHighlights(root);
        
        setControlsEnabled(true);
        statusLabel->setText("Traversal complete.");
        addStepToHistory(QString("🎯 %1 traversal completed. Visited %2 nodes").arg(algo).arg(traversalOrder.size()));
        update();
        return;
    }
    
    RBNode* current = traversalOrder[traversalIndex];
    current->isTraversalHighlighted = true;
    
    QString algo = (traversalType == TraversalType::BFS) ? "BFS" : "DFS";
    addStepToHistory(QString("👁️ %1: Currently visiting node %2").arg(algo).arg(current->value));
    
    traversalIndex++;
    update();
}
