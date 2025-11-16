#include "treeinsertion.h"

TreeInsertion::TreeInsertion(QWidget *parent)
    : QWidget(parent)
    , root(nullptr)
    , currentTraversalStep(0)
    , isAnimating(false)
    , currentOperation("")
    , traversalType(TraversalType::None)
    , traversalIndex(0)
{
    setupUI();
    setMinimumSize(1200, 800);

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, [this]() {
        update();
    });

    traversalAnimTimer = new QTimer(this);
    connect(traversalAnimTimer, &QTimer::timeout, this, &TreeInsertion::onTraversalAnimationStep);
}

TreeInsertion::~TreeInsertion()
{
    // Stop any running animations
    if (traversalAnimTimer) {
        traversalAnimTimer->stop();
    }
    
    clearTree(root);
}

void TreeInsertion::setupUI()
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

void TreeInsertion::setupVisualizationArea()
{
    leftPanel = new QWidget();
    leftPanel->setStyleSheet("background: transparent;");
    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(40, 30, 20, 30);
    leftLayout->setSpacing(25);

    // Header with back button and title - using UIUtils
    QHBoxLayout *headerLayout = new QHBoxLayout();

    backButton = UIUtils::createBackButton(BackButton::BackToOperations);
    titleLabel = UIUtils::createTitleLabel("Binary Search Tree");

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

    // Buttons with hashmap-style sizing
    // Buttons using StyleManager - consistent and efficient!
    insertButton = new QPushButton("Insert");
    insertButton->setFixedSize(80, 40);
    StyleManager::instance().applyOperationButtonStyle(insertButton, "#7b4fff");
    
    searchButton = new QPushButton("Search");
    searchButton->setFixedSize(80, 40);
    StyleManager::instance().applyOperationButtonStyle(searchButton, "#4a90e2");
    
    deleteButton = new QPushButton("Delete");
    deleteButton->setFixedSize(80, 40);
    StyleManager::instance().applyOperationButtonStyle(deleteButton, "#ff6b6b");
    
    clearButton = new QPushButton("Clear");
    clearButton->setFixedSize(80, 40);
    StyleManager::instance().applyOperationButtonStyle(clearButton, "#95a5a6");
    
    randomizeButton = new QPushButton("Random");
    randomizeButton->setFixedSize(80, 40);
    StyleManager::instance().applyOperationButtonStyle(randomizeButton, "#28a745");

    controlLayout->addWidget(inputField);
    controlLayout->addWidget(insertButton);
    controlLayout->addWidget(searchButton);
    controlLayout->addWidget(deleteButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(randomizeButton);
    controlLayout->addStretch();

    leftLayout->addLayout(controlLayout);

    // Status label
    statusLabel = new QLabel("Tree is empty. Start by inserting a value!");
    QFont statusFont("Segoe UI", 11);
    statusLabel->setFont(statusFont);
    statusLabel->setStyleSheet("color: #7b4fff; padding: 8px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(statusLabel);

    // Visualization area (we'll draw the tree directly on this widget in paintEvent)
    leftLayout->addStretch();

    // Connect signals
    connect(backButton, &BackButton::backRequested, this, &TreeInsertion::onBackClicked);
    connect(insertButton, &QPushButton::clicked, this, &TreeInsertion::onInsertClicked);
    connect(searchButton, &QPushButton::clicked, this, &TreeInsertion::onSearchClicked);
    connect(deleteButton, &QPushButton::clicked, this, &TreeInsertion::onDeleteClicked);
    connect(clearButton, &QPushButton::clicked, this, &TreeInsertion::onClearClicked);
    connect(randomizeButton, &QPushButton::clicked, this, &TreeInsertion::onRandomizeClicked);
    connect(inputField, &QLineEdit::returnPressed, this, &TreeInsertion::onInsertClicked);
}


void TreeInsertion::setupRightPanel()
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

void TreeInsertion::setupStepTrace()
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
    QLabel *traceTitle = new QLabel("🌳 Operation History & Algorithms");
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

void TreeInsertion::setupTraversalControls()
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
    connect(bfsButton, &QPushButton::clicked, this, &TreeInsertion::onStartBFS);
    connect(dfsButton, &QPushButton::clicked, this, &TreeInsertion::onStartDFS);
}

void TreeInsertion::onBackClicked()
{
    emit backToOperations();
}

void TreeInsertion::onInsertClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current animation to complete.");
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

    currentOperation = "Insert";
    addStepToHistory("➕ INSERT OPERATION");
    addStepToHistory(QString("🔢 Adding value: %1").arg(value));
    showAlgorithm("Insert");
    animateInsertion(value);
    inputField->clear();
    inputField->setFocus();
}

void TreeInsertion::onSearchClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current animation to complete.");
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

    currentOperation = "Search";
    addStepToHistory("🔍 SEARCH OPERATION");
    showAlgorithm("Search");
    animateSearch(value);
    inputField->clear();
    inputField->setFocus();
}

void TreeInsertion::onDeleteClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current animation to complete.");
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

    currentOperation = "Delete";
    addStepToHistory("🗑️ DELETE OPERATION");
    showAlgorithm("Delete");
    animateDelete(value);
    inputField->clear();
    inputField->setFocus();
}

void TreeInsertion::onClearClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current animation to complete.");
        return;
    }

    clearTree(root);
    root = nullptr;
    stepHistory.clear();
    
    // Clear traversal results
    traversalResultList->clear();
    traversalOrder.clear();
    traversalType = TraversalType::None;
    traversalIndex = 0;
    
    statusLabel->setText("Tree cleared! Start by inserting a value.");
    addStepToHistory("🧹 CLEAR OPERATION");
    addStepToHistory("Tree cleared - all nodes removed");
    addOperationSeparator();
    updateStepTrace();
    update();
}

void TreeInsertion::onRandomizeClicked()
{
    if (isAnimating) {
        QMessageBox::warning(this, "Animation in Progress",
                             "Please wait for the current animation to complete.");
        return;
    }

    // Generate a single random value between 1 and 100
    int randomValue = QRandomGenerator::global()->bounded(1, 101);
    
    // Display the value being inserted in the status label and step history
    statusLabel->setText(QString("Inserting value: %1...").arg(randomValue));
    currentOperation = "Insert";
    addStepToHistory("➕ INSERT OPERATION");
    addStepToHistory(QString("🔢 Adding value: %1").arg(randomValue));
    updateStepTrace(); // Update immediately to show the message
    
    // Set the input field and trigger insert
    inputField->setText(QString::number(randomValue));
    
    // Now call the insertion logic directly (bypassing onInsertClicked to avoid duplicate messages)
    bool ok;
    int value = randomValue;
    
    showAlgorithm("Insert");
    animateInsertion(value);
    inputField->clear();
    inputField->setFocus();
}

void TreeInsertion::insertNode(int value)
{
    TreeNode *newNode = new TreeNode(value);
    newNode->isNewNode = true;

    if (!root) {
        root = newNode;
        statusLabel->setText(QString("Inserted %1 as root node").arg(value));
        addStepToHistory(QString("✅ Inserted %1 as root node").arg(value));
        return;
    }

    TreeNode *current = root;
    TreeNode *parent = nullptr;

    while (current) {
        parent = current;
        if (value < current->value) {
            current = current->left;
        } else if (value > current->value) {
            current = current->right;
        } else {
            delete newNode;
            statusLabel->setText(QString("Value %1 already exists in tree!").arg(value));
            addStepToHistory(QString("❌ Value %1 already exists in tree!").arg(value));
            return;
        }
    }

    if (value < parent->value) {
        parent->left = newNode;
        statusLabel->setText(QString("Inserted %1 as left child of %2").arg(value).arg(parent->value));
        addStepToHistory(QString("✅ Inserted %1 as left child of %2").arg(value).arg(parent->value));
    } else {
        parent->right = newNode;
        statusLabel->setText(QString("Inserted %1 as right child of %2").arg(value).arg(parent->value));
        addStepToHistory(QString("✅ Inserted %1 as right child of %2").arg(value).arg(parent->value));
    }
}

TreeNode* TreeInsertion::searchNode(int value)
{
    TreeNode *current = root;
    
    while (current) {
        if (value == current->value) {
            return current;
        } else if (value < current->value) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    
    return nullptr;
}

TreeNode* TreeInsertion::deleteNode(TreeNode* node, int value)
{
    if (!node) {
        return nullptr;
    }

    if (value < node->value) {
        node->left = deleteNode(node->left, value);
    } else if (value > node->value) {
        node->right = deleteNode(node->right, value);
    } else {
        // Node to be deleted found
        if (!node->left) {
            TreeNode* temp = node->right;
            delete node;
            return temp;
        } else if (!node->right) {
            TreeNode* temp = node->left;
            delete node;
            return temp;
        }

        // Node with two children
        TreeNode* temp = findMin(node->right);
        node->value = temp->value;
        node->right = deleteNode(node->right, temp->value);
    }
    
    return node;
}

TreeNode* TreeInsertion::findMin(TreeNode* node)
{
    while (node && node->left) {
        node = node->left;
    }
    return node;
}

void TreeInsertion::animateInsertion(int value)
{
    isAnimating = true;
    insertButton->setEnabled(false);
    searchButton->setEnabled(false);
    deleteButton->setEnabled(false);
    clearButton->setEnabled(false);

    addStepToHistory(QString("➕ Starting insertion of value: %1").arg(value));

    // Build traversal path
    traversalPath.clear();

    if (!root) {
        insertNode(value);
        addOperationSeparator();
        isAnimating = false;
        insertButton->setEnabled(true);
        searchButton->setEnabled(true);
        deleteButton->setEnabled(true);
        clearButton->setEnabled(true);
        updateStepTrace();
        update();
        return;
    }

    TreeNode *current = root;
    while (current) {
        traversalPath.append(current);
        if (value < current->value) {
            if (!current->left) {
                break;
            }
            current = current->left;
        } else if (value > current->value) {
            if (!current->right) {
                break;
            }
            current = current->right;
        } else {
            // Duplicate value
            QMessageBox::warning(this, "Duplicate Value",
                                 QString("Value %1 already exists in tree!").arg(value));
            addStepToHistory(QString("❌ Value %1 already exists in tree!").arg(value));
            addOperationSeparator();
            isAnimating = false;
            insertButton->setEnabled(true);
            searchButton->setEnabled(true);
            deleteButton->setEnabled(true);
            clearButton->setEnabled(true);
            updateStepTrace();
            return;
        }
    }

    currentTraversalStep = 0;

    QTimer::singleShot(0, this, [this, value]() {
        animateTraversal(root, value, 0);
    });
}

void TreeInsertion::animateSearch(int value)
{
    isAnimating = true;
    insertButton->setEnabled(false);
    searchButton->setEnabled(false);
    deleteButton->setEnabled(false);
    clearButton->setEnabled(false);

    addStepToHistory(QString("🔍 Starting search for value: %1").arg(value));

    if (!root) {
        statusLabel->setText("Tree is empty - value not found!");
        addStepToHistory("❌ Tree is empty - value not found!");
        addOperationSeparator();
        isAnimating = false;
        insertButton->setEnabled(true);
        searchButton->setEnabled(true);
        deleteButton->setEnabled(true);
        clearButton->setEnabled(true);
        updateStepTrace();
        return;
    }

    // Build traversal path for search
    traversalPath.clear();
    TreeNode *current = root;
    bool found = false;

    while (current) {
        traversalPath.append(current);
        if (value == current->value) {
            found = true;
            break;
        } else if (value < current->value) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    currentTraversalStep = 0;

    QTimer::singleShot(0, this, [this, value, found]() {
        animateSearchTraversal(root, value, 0);
    });
}

void TreeInsertion::animateDelete(int value)
{
    isAnimating = true;
    insertButton->setEnabled(false);
    searchButton->setEnabled(false);
    deleteButton->setEnabled(false);
    clearButton->setEnabled(false);

    addStepToHistory(QString("🗑️ Starting deletion of value: %1").arg(value));

    if (!root) {
        statusLabel->setText("Tree is empty - nothing to delete!");
        addStepToHistory("❌ Tree is empty - nothing to delete!");
        addOperationSeparator();
        isAnimating = false;
        insertButton->setEnabled(true);
        searchButton->setEnabled(true);
        deleteButton->setEnabled(true);
        clearButton->setEnabled(true);
        updateStepTrace();
        return;
    }

    // First, search for the node to see if it exists
    TreeNode *nodeToDelete = searchNode(value);
    if (!nodeToDelete) {
        statusLabel->setText(QString("Value %1 not found in tree!").arg(value));
        addStepToHistory(QString("❌ Value %1 not found in tree!").arg(value));
        addOperationSeparator();
        isAnimating = false;
        insertButton->setEnabled(true);
        searchButton->setEnabled(true);
        deleteButton->setEnabled(true);
        clearButton->setEnabled(true);
        updateStepTrace();
        return;
    }

    // Build traversal path to the node
    traversalPath.clear();
    TreeNode *current = root;

    while (current) {
        traversalPath.append(current);
        if (value == current->value) {
            break;
        } else if (value < current->value) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    currentTraversalStep = 0;

    QTimer::singleShot(0, this, [this, value]() {
        animateDeleteTraversal(root, value, 0);
    });
}

void TreeInsertion::animateTraversal(TreeNode *node, int value, int step)
{
    if (step >= traversalPath.size()) {
        // Animation complete, insert the node
        insertNode(value);
        addOperationSeparator();
        resetHighlights(root);
        isAnimating = false;
        insertButton->setEnabled(true);
        searchButton->setEnabled(true);
        deleteButton->setEnabled(true);
        clearButton->setEnabled(true);
        updateStepTrace();
        update();
        return;
    }

    resetHighlights(root);
    traversalPath[step]->isHighlighted = true;

    if (step == 0) {
        statusLabel->setText(QString("Comparing %1 with root %2").arg(value).arg(traversalPath[step]->value));
        addStepToHistory(QString("📊 Comparing %1 with root %2").arg(value).arg(traversalPath[step]->value));
    } else {
        statusLabel->setText(QString("Comparing %1 with %2").arg(value).arg(traversalPath[step]->value));
        addStepToHistory(QString("📊 Comparing %1 with %2").arg(value).arg(traversalPath[step]->value));
    }

    if (value < traversalPath[step]->value) {
        addStepToHistory(QString("⬅️ %1 < %2, go left").arg(value).arg(traversalPath[step]->value));
    } else if (value > traversalPath[step]->value) {
        addStepToHistory(QString("➡️ %1 > %2, go right").arg(value).arg(traversalPath[step]->value));
    }

    updateStepTrace();
    update();

    QTimer::singleShot(800, this, [this, node, value, step]() {
        animateTraversal(node, value, step + 1);
    });
}

void TreeInsertion::animateSearchTraversal(TreeNode *node, int value, int step)
{
    if (step >= traversalPath.size()) {
        // Animation complete
        resetHighlights(root);
        
        TreeNode *found = searchNode(value);
        if (found) {
            found->isSearchHighlighted = true;
            statusLabel->setText(QString("Found value %1 in the tree!").arg(value));
            addStepToHistory(QString("✅ Found value %1 in the tree!").arg(value));
        } else {
            statusLabel->setText(QString("Value %1 not found in tree!").arg(value));
            addStepToHistory(QString("❌ Value %1 not found in tree!").arg(value));
        }
        
        addOperationSeparator();
        isAnimating = false;
        insertButton->setEnabled(true);
        searchButton->setEnabled(true);
        deleteButton->setEnabled(true);
        clearButton->setEnabled(true);
        updateStepTrace();
        update();
        
        // Clear search highlight after a delay
        QTimer::singleShot(2000, this, [this]() {
            resetHighlights(root);
            update();
        });
        return;
    }

    resetHighlights(root);
    traversalPath[step]->isHighlighted = true;

    if (step == 0) {
        statusLabel->setText(QString("Searching: comparing %1 with root %2").arg(value).arg(traversalPath[step]->value));
        addStepToHistory(QString("🔍 Comparing %1 with root %2").arg(value).arg(traversalPath[step]->value));
    } else {
        statusLabel->setText(QString("Searching: comparing %1 with %2").arg(value).arg(traversalPath[step]->value));
        addStepToHistory(QString("🔍 Comparing %1 with %2").arg(value).arg(traversalPath[step]->value));
    }

    if (value == traversalPath[step]->value) {
        addStepToHistory(QString("🎯 Match found! %1 == %2").arg(value).arg(traversalPath[step]->value));
    } else if (value < traversalPath[step]->value) {
        addStepToHistory(QString("⬅️ %1 < %2, search left").arg(value).arg(traversalPath[step]->value));
    } else {
        addStepToHistory(QString("➡️ %1 > %2, search right").arg(value).arg(traversalPath[step]->value));
    }

    updateStepTrace();
    update();

    QTimer::singleShot(800, this, [this, node, value, step]() {
        animateSearchTraversal(node, value, step + 1);
    });
}

void TreeInsertion::animateDeleteTraversal(TreeNode *node, int value, int step)
{
    if (step >= traversalPath.size()) {
        // Animation complete, now delete the node
        resetHighlights(root);
        
        // Highlight the node to be deleted
        TreeNode *nodeToDelete = searchNode(value);
        if (nodeToDelete) {
            nodeToDelete->isDeleteHighlighted = true;
            addStepToHistory(QString("🎯 Found node %1 to delete").arg(value));
            updateStepTrace();
            update();
            
            // Wait a moment, then perform the deletion
            QTimer::singleShot(1000, this, [this, value]() {
                root = deleteNode(root, value);
                statusLabel->setText(QString("Deleted value %1 from tree").arg(value));
                addStepToHistory(QString("✅ Deleted value %1 from tree").arg(value));
                addOperationSeparator();
                
                isAnimating = false;
                insertButton->setEnabled(true);
                searchButton->setEnabled(true);
                deleteButton->setEnabled(true);
                clearButton->setEnabled(true);
                updateStepTrace();
                update();
            });
        }
        return;
    }

    resetHighlights(root);
    traversalPath[step]->isHighlighted = true;

    if (step == 0) {
        statusLabel->setText(QString("Deleting: comparing %1 with root %2").arg(value).arg(traversalPath[step]->value));
        addStepToHistory(QString("🗑️ Comparing %1 with root %2").arg(value).arg(traversalPath[step]->value));
    } else {
        statusLabel->setText(QString("Deleting: comparing %1 with %2").arg(value).arg(traversalPath[step]->value));
        addStepToHistory(QString("🗑️ Comparing %1 with %2").arg(value).arg(traversalPath[step]->value));
    }

    if (value == traversalPath[step]->value) {
        addStepToHistory(QString("🎯 Target found! %1 == %2").arg(value).arg(traversalPath[step]->value));
    } else if (value < traversalPath[step]->value) {
        addStepToHistory(QString("⬅️ %1 < %2, search left").arg(value).arg(traversalPath[step]->value));
    } else {
        addStepToHistory(QString("➡️ %1 > %2, search right").arg(value).arg(traversalPath[step]->value));
    }

    updateStepTrace();
    update();

    QTimer::singleShot(800, this, [this, node, value, step]() {
        animateDeleteTraversal(node, value, step + 1);
    });
}

void TreeInsertion::calculateNodePositions(TreeNode *node, int x, int y, int horizontalSpacing)
{
    if (!node) return;

    node->x = x;
    node->y = y;

    int nextSpacing = horizontalSpacing / 2;

    if (node->left) {
        calculateNodePositions(node->left, x - horizontalSpacing, y + LEVEL_HEIGHT, nextSpacing);
    }
    if (node->right) {
        calculateNodePositions(node->right, x + horizontalSpacing, y + LEVEL_HEIGHT, nextSpacing);
    }
}

void TreeInsertion::calculateNodePositionsWithSpacing(TreeNode *node, int x, int y, int horizontalSpacing, int verticalSpacing)
{
    if (!node) return;

    node->x = x;
    node->y = y;

    int nextSpacing = horizontalSpacing / 2;

    if (node->left) {
        calculateNodePositionsWithSpacing(node->left, x - horizontalSpacing, y + verticalSpacing, nextSpacing, verticalSpacing);
    }
    if (node->right) {
        calculateNodePositionsWithSpacing(node->right, x + horizontalSpacing, y + verticalSpacing, nextSpacing, verticalSpacing);
    }
}

void TreeInsertion::resetHighlights(TreeNode *node)
{
    if (!node) return;
    node->isHighlighted = false;
    node->isNewNode = false;
    node->isSearchHighlighted = false;
    node->isDeleteHighlighted = false;
    resetHighlights(node->left);
    resetHighlights(node->right);
}

void TreeInsertion::clearTree(TreeNode *node)
{
    if (!node) return;
    clearTree(node->left);
    clearTree(node->right);
    delete node;
}

int TreeInsertion::getTreeHeight(TreeNode *node)
{
    if (!node) return 0;
    return 1 + std::max(getTreeHeight(node->left), getTreeHeight(node->right));
}

void TreeInsertion::addStepToHistory(const QString &step)
{
    stepHistory.append(step);
}

void TreeInsertion::addOperationSeparator()
{
    // Add separator like hashmap does
    stepHistory.append("────────────────────");
}

void TreeInsertion::updateStepTrace()
{
    stepsList->clear();
    
    for (int i = 0; i < stepHistory.size(); ++i) {
        const QString &step = stepHistory[i];
        
        // Handle separator lines like hashmap
        if (step == "────────────────────") {
            QListWidgetItem *separator = new QListWidgetItem(step);
            separator->setTextAlignment(Qt::AlignCenter);
            separator->setFlags(Qt::NoItemFlags); // Make it non-selectable
            separator->setForeground(QColor(189, 195, 199));
            stepsList->addItem(separator);
            continue;
        }
        
        QListWidgetItem *item = new QListWidgetItem(step);
        
        // Style different types of steps
        if (step.contains("✅")) {
            item->setForeground(QColor(39, 174, 96)); // Green for success
        } else if (step.contains("❌")) {
            item->setForeground(QColor(231, 76, 60)); // Red for failure
        } else if (step.contains("🔍")) {
            item->setForeground(QColor(52, 152, 219)); // Blue for search
        } else if (step.contains("➕")) {
            item->setForeground(QColor(46, 125, 50)); // Green for insert
        } else if (step.contains("🗑️")) {
            item->setForeground(QColor(211, 47, 47)); // Red for delete
        } else if (step.contains("📊") || step.contains("🎯")) {
            item->setForeground(QColor(155, 89, 182)); // Purple for calculation
        } else if (step.contains("⬅️") || step.contains("➡️")) {
            item->setForeground(QColor(255, 152, 0)); // Orange for direction
        } else if (step.contains("🧹")) {
            item->setForeground(QColor(128, 128, 128)); // Gray for clear
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

void TreeInsertion::showAlgorithm(const QString &operation)
{
    // Add separator between operations if there are already items (like hashmap)
    if (algorithmList->count() > 0) {
        QListWidgetItem *separator = new QListWidgetItem("────────────────────");
        separator->setTextAlignment(Qt::AlignCenter);
        separator->setFlags(Qt::NoItemFlags); // Make it non-selectable
        separator->setForeground(QColor(189, 195, 199));
        algorithmList->addItem(separator);
    }
    
    if (operation == "Insert") {
        // Add items with proper colors and better formatting
        QListWidgetItem *title = new QListWidgetItem("🌳 BST Insert Algorithm");
        title->setForeground(QColor(46, 125, 50)); // Green for insert
        title->setFont(QFont("Segoe UI", 12, QFont::Bold));
        algorithmList->addItem(title);
        
        QListWidgetItem *step1 = new QListWidgetItem("1️⃣ Start at root node");
        step1->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step1);
        
        QListWidgetItem *step2 = new QListWidgetItem("2️⃣ Compare new value with current:");
        step2->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step2);
        
        QListWidgetItem *left = new QListWidgetItem("   ⬅️ value < current → go left");
        left->setForeground(QColor(230, 126, 34));
        algorithmList->addItem(left);
        
        QListWidgetItem *right = new QListWidgetItem("   ➡️ value > current → go right");
        right->setForeground(QColor(230, 126, 34));
        algorithmList->addItem(right);
        
        QListWidgetItem *equal = new QListWidgetItem("   ❌ value = current → duplicate");
        equal->setForeground(QColor(231, 76, 60));
        algorithmList->addItem(equal);
        
        QListWidgetItem *step3 = new QListWidgetItem("3️⃣ Repeat until null position found");
        step3->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step3);
        
        QListWidgetItem *step4 = new QListWidgetItem("4️⃣ Insert new node at null position");
        step4->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step4);
        
        QListWidgetItem *complexity = new QListWidgetItem("⏰ Time Complexity");
        complexity->setForeground(QColor(155, 89, 182));
        complexity->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(complexity);
        
        QListWidgetItem *avg = new QListWidgetItem("   📊 Average: O(log n) - balanced tree");
        avg->setForeground(QColor(39, 174, 96));
        algorithmList->addItem(avg);
        
        QListWidgetItem *worst = new QListWidgetItem("   📈 Worst: O(n) - skewed tree");
        worst->setForeground(QColor(231, 76, 60));
        algorithmList->addItem(worst);
        
    } else if (operation == "Search") {
        QListWidgetItem *title = new QListWidgetItem("🔍 BST Search Algorithm");
        title->setForeground(QColor(52, 152, 219));
        title->setFont(QFont("Segoe UI", 12, QFont::Bold));
        algorithmList->addItem(title);
        
        QListWidgetItem *step1 = new QListWidgetItem("1️⃣ Start at root node");
        step1->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step1);
        
        QListWidgetItem *step2 = new QListWidgetItem("2️⃣ Compare target with current:");
        step2->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step2);
        
        QListWidgetItem *found = new QListWidgetItem("   ✅ target = current → found!");
        found->setForeground(QColor(39, 174, 96));
        algorithmList->addItem(found);
        
        QListWidgetItem *left = new QListWidgetItem("   ⬅️ target < current → go left");
        left->setForeground(QColor(230, 126, 34));
        algorithmList->addItem(left);
        
        QListWidgetItem *right = new QListWidgetItem("   ➡️ target > current → go right");
        right->setForeground(QColor(230, 126, 34));
        algorithmList->addItem(right);
        
        QListWidgetItem *step3 = new QListWidgetItem("3️⃣ Repeat until found or null reached");
        step3->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step3);
        
        QListWidgetItem *step4 = new QListWidgetItem("4️⃣ Return result (found/not found)");
        step4->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step4);
        
        QListWidgetItem *complexity = new QListWidgetItem("⏰ Time Complexity");
        complexity->setForeground(QColor(155, 89, 182));
        complexity->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(complexity);
        
        QListWidgetItem *avg = new QListWidgetItem("   📊 Average: O(log n) - balanced tree");
        avg->setForeground(QColor(39, 174, 96));
        algorithmList->addItem(avg);
        
        QListWidgetItem *worst = new QListWidgetItem("   📈 Worst: O(n) - skewed tree");
        worst->setForeground(QColor(231, 76, 60));
        algorithmList->addItem(worst);
        
    } else if (operation == "Delete") {
        QListWidgetItem *title = new QListWidgetItem("🗑️ BST Delete Algorithm");
        title->setForeground(QColor(211, 47, 47));
        title->setFont(QFont("Segoe UI", 12, QFont::Bold));
        algorithmList->addItem(title);
        
        QListWidgetItem *step1 = new QListWidgetItem("1️⃣ Search for node to delete");
        step1->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step1);
        
        QListWidgetItem *step2 = new QListWidgetItem("2️⃣ Handle three cases:");
        step2->setForeground(QColor(52, 73, 94));
        algorithmList->addItem(step2);
        
        QListWidgetItem *case1 = new QListWidgetItem("   🍃 Case 1: No children");
        case1->setForeground(QColor(39, 174, 96));
        algorithmList->addItem(case1);
        
        QListWidgetItem *case1_action = new QListWidgetItem("      → Simply remove node");
        case1_action->setForeground(QColor(127, 140, 141));
        algorithmList->addItem(case1_action);
        
        QListWidgetItem *case2 = new QListWidgetItem("   🌿 Case 2: One child");
        case2->setForeground(QColor(230, 126, 34));
        algorithmList->addItem(case2);
        
        QListWidgetItem *case2_action = new QListWidgetItem("      → Replace with child");
        case2_action->setForeground(QColor(127, 140, 141));
        algorithmList->addItem(case2_action);
        
        QListWidgetItem *case3 = new QListWidgetItem("   🌳 Case 3: Two children");
        case3->setForeground(QColor(231, 76, 60));
        algorithmList->addItem(case3);
        
        QListWidgetItem *case3_action1 = new QListWidgetItem("      → Find inorder successor");
        case3_action1->setForeground(QColor(127, 140, 141));
        algorithmList->addItem(case3_action1);
        
        QListWidgetItem *case3_action2 = new QListWidgetItem("      → Replace & delete successor");
        case3_action2->setForeground(QColor(127, 140, 141));
        algorithmList->addItem(case3_action2);
        
        QListWidgetItem *complexity = new QListWidgetItem("⏰ Time Complexity");
        complexity->setForeground(QColor(155, 89, 182));
        complexity->setFont(QFont("Segoe UI", 11, QFont::Bold));
        algorithmList->addItem(complexity);
        
        QListWidgetItem *avg = new QListWidgetItem("   📊 Average: O(log n) - balanced tree");
        avg->setForeground(QColor(39, 174, 96));
        algorithmList->addItem(avg);
        
        QListWidgetItem *worst = new QListWidgetItem("   📈 Worst: O(n) - skewed tree");
        worst->setForeground(QColor(231, 76, 60));
        algorithmList->addItem(worst);
    }
    else if (operation == "BFS") {
        // Title
        QListWidgetItem *title = new QListWidgetItem("🌊 Breadth-First Search (BFS) - Binary Search Tree");
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
        QListWidgetItem *title = new QListWidgetItem("🏔️ Depth-First Search (DFS) - Binary Search Tree");
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
        
        QListWidgetItem *step2a = new QListWidgetItem("   • Visit current node");
        step2a->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step2a);
        
        QListWidgetItem *step2b = new QListWidgetItem("   • Recursively visit left subtree");
        step2b->setForeground(QColor(108, 117, 125));
        algorithmList->addItem(step2b);
        
        QListWidgetItem *step2c = new QListWidgetItem("   • Recursively visit right subtree");
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
    
    // Auto-scroll to bottom to show latest algorithm
    if (algorithmList->count() > 0) {
        algorithmList->scrollToBottom();
    }
}

void TreeInsertion::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Background gradient for left panel only
    QRect leftRect = leftPanel->geometry();
    QLinearGradient gradient(0, 0, leftRect.width(), leftRect.height());
    gradient.setColorAt(0.0, QColor(250, 247, 255));
    gradient.setColorAt(1.0, QColor(237, 228, 255));
    painter.fillRect(leftRect, gradient);

    // Draw canvas area for tree (matching hashmap style)
    int canvasY = 200; // Position like hashmap
    int canvasHeight = leftRect.height() - canvasY + leftRect.y() - 40;
    QRect canvasRect(leftRect.x() + 30, canvasY, leftRect.width() - 60, canvasHeight);

    // Draw canvas background (like hashmap)
    painter.setPen(QPen(QColor(74, 144, 226, 51), 2)); // rgba(74, 144, 226, 0.2)
    painter.setBrush(QBrush(QLinearGradient(canvasRect.topLeft(), canvasRect.bottomRight())));
    QLinearGradient canvasGradient(canvasRect.topLeft(), canvasRect.bottomRight());
    canvasGradient.setColorAt(0.0, QColor(248, 251, 255, 250)); // rgba(248, 251, 255, 0.98)
    canvasGradient.setColorAt(1.0, QColor(240, 247, 255, 250)); // rgba(240, 247, 255, 0.98)
    painter.setBrush(canvasGradient);
    painter.drawRoundedRect(canvasRect, 20, 20);

    // Draw tree if exists
    if (root) {
        // Calculate tree dimensions for auto-zoom
        int treeHeight = getTreeHeight(root);
        int canvasWidth = canvasRect.width();
        int canvasHeight_available = canvasRect.height() - 100; // Leave some margin
        
        // Auto-zoom: adjust horizontal spacing based on tree size and canvas width
        int baseSpacing = canvasWidth / 4;
        if (treeHeight > 3) {
            // Reduce spacing for larger trees
            baseSpacing = std::max(80, canvasWidth / (treeHeight + 1));
        }
        
        // Adjust vertical spacing if tree is too tall
        int verticalSpacing = LEVEL_HEIGHT;
        if (treeHeight * LEVEL_HEIGHT > canvasHeight_available) {
            verticalSpacing = std::max(60, canvasHeight_available / (treeHeight + 1));
        }

        // Draw tree - position relative to canvas
        int canvasCenterX = canvasRect.x() + canvasWidth / 2;
        int treeStartY = canvasRect.y() + 50;

        // Use adjusted spacing for positioning
        calculateNodePositionsWithSpacing(root, canvasCenterX, treeStartY, baseSpacing, verticalSpacing);
        drawTree(painter, root);
    }
}

void TreeInsertion::drawTree(QPainter &painter, TreeNode *node)
{
    if (!node) return;

    // Draw edges first
    if (node->left) {
        drawEdge(painter, node->x, node->y, node->left->x, node->left->y);
        drawTree(painter, node->left);
    }
    if (node->right) {
        drawEdge(painter, node->x, node->y, node->right->x, node->right->y);
        drawTree(painter, node->right);
    }

    // Draw node on top
    drawNode(painter, node);
}

void TreeInsertion::drawNode(QPainter &painter, TreeNode *node)
{
    if (!node) return;

    // Node circle with different colors based on state
    if (node->isTraversalHighlighted) {
        painter.setPen(QPen(QColor(255, 140, 0), 4));  // Orange border for currently visiting
        painter.setBrush(QColor(255, 165, 0));         // Orange fill
    } else if (node->isVisited && traversalType != TraversalType::None) {
        painter.setPen(QPen(QColor(34, 139, 34), 4));  // Green border for visited (only during traversal)
        painter.setBrush(QColor(50, 205, 50));         // Green fill
    } else if (node->isSearchHighlighted) {
        painter.setPen(QPen(QColor(52, 152, 219), 4)); // Blue for found
        painter.setBrush(QColor(174, 214, 241));
    } else if (node->isDeleteHighlighted) {
        painter.setPen(QPen(QColor(231, 76, 60), 4)); // Red for delete
        painter.setBrush(QColor(248, 148, 148));
    } else if (node->isHighlighted) {
        painter.setPen(QPen(QColor(255, 165, 0), 4)); // Orange for traversal
        painter.setBrush(QColor(255, 200, 100));
    } else if (node->isNewNode) {
        painter.setPen(QPen(QColor(50, 205, 50), 4)); // Green for new
        painter.setBrush(QColor(144, 238, 144));
    } else {
        painter.setPen(QPen(QColor(123, 79, 255), 3)); // Default purple
        painter.setBrush(QColor(200, 180, 255));
    }

    painter.drawEllipse(QPoint(node->x, node->y), NODE_RADIUS, NODE_RADIUS);

    // Node value
    painter.setPen(Qt::black);
    QFont font("Segoe UI", 14, QFont::Bold);
    painter.setFont(font);
    painter.drawText(QRect(node->x - NODE_RADIUS, node->y - NODE_RADIUS,
                           NODE_RADIUS * 2, NODE_RADIUS * 2),
                     Qt::AlignCenter, QString::number(node->value));
}

void TreeInsertion::drawEdge(QPainter &painter, int x1, int y1, int x2, int y2)
{
    painter.setPen(QPen(QColor(123, 79, 255), 2));
    painter.drawLine(x1, y1 + NODE_RADIUS, x2, y2 - NODE_RADIUS);
}

void TreeInsertion::resetTraversalHighlights(TreeNode *node)
{
    if (!node) return;
    node->isTraversalHighlighted = false;
    node->isVisited = false;
    resetTraversalHighlights(node->left);
    resetTraversalHighlights(node->right);
}

void TreeInsertion::setControlsEnabled(bool enabled)
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

void TreeInsertion::onStartBFS()
{
    showAlgorithm("BFS");
    
    if (!root) {
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

void TreeInsertion::onStartDFS()
{
    showAlgorithm("DFS");
    
    if (!root) {
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

void TreeInsertion::performBFS()
{
    if (!root) return;
    
    addStepToHistory("🔄 Initializing BFS queue");
    QList<TreeNode*> queue;
    queue.append(root);
    addStepToHistory(QString("➕ Added root node %1 to queue").arg(root->value));
    
    while (!queue.isEmpty()) {
        TreeNode* current = queue.front();
        queue.pop_front();
        traversalOrder.append(current);
        
        addStepToHistory(QString("🎯 Processing node %1").arg(current->value));
        
        if (current->left) {
            queue.append(current->left);
            addStepToHistory(QString("⬅️ Added left child %1 to queue").arg(current->left->value));
        }
        if (current->right) {
            queue.append(current->right);
            addStepToHistory(QString("➡️ Added right child %1 to queue").arg(current->right->value));
        }
    }
}

void TreeInsertion::performDFS()
{
    if (!root) return;
    
    addStepToHistory("🔄 Starting DFS traversal (preorder: Root → Left → Right)");
    addStepToHistory("📍 Using recursive depth-first approach");
    
    // Use recursive DFS for correct preorder traversal
    performDFSRecursive(root);
}

void TreeInsertion::performDFSRecursive(TreeNode* node)
{
    if (!node) return;
    
    // Preorder: Process current node first
    traversalOrder.append(node);
    addStepToHistory(QString("🎯 Visiting node %1 (preorder)").arg(node->value));
    
    // Then recursively visit left subtree
    if (node->left) {
        addStepToHistory(QString("⬅️ Going to left subtree of %1").arg(node->value));
        performDFSRecursive(node->left);
    }
    
    // Finally recursively visit right subtree
    if (node->right) {
        addStepToHistory(QString("➡️ Going to right subtree of %1").arg(node->value));
        performDFSRecursive(node->right);
    }
}

void TreeInsertion::onTraversalAnimationStep()
{
    if (traversalIndex > 0 && traversalIndex <= traversalOrder.size()) {
        TreeNode* prev = traversalOrder[traversalIndex - 1];
        // Mark previous as visited (persist green), remove highlight
        prev->isVisited = true;
        prev->isTraversalHighlighted = false;
    }
    
    if (traversalIndex >= traversalOrder.size()) {
        QString algo = (traversalType == TraversalType::BFS) ? "BFS" : "DFS";
        traversalAnimTimer->stop();
        traversalType = TraversalType::None;
        
        // Reset traversal highlights to restore original colors
        resetTraversalHighlights(root);
        
        setControlsEnabled(true);
        statusLabel->setText("Traversal complete.");
        addStepToHistory(QString("🎯 %1 traversal completed. Visited %2 nodes").arg(algo).arg(traversalOrder.size()));
        update();
        return;
    }
    
    TreeNode* current = traversalOrder[traversalIndex];
    current->isTraversalHighlighted = true;
    
    QString algo = (traversalType == TraversalType::BFS) ? "BFS" : "DFS";
    addStepToHistory(QString("👁️ %1: Currently visiting node %2").arg(algo).arg(current->value));
    
    traversalIndex++;
    update();
}