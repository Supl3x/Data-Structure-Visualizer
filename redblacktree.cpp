#include "redblacktree.h"
#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

RedBlackTree::RedBlackTree(QWidget *parent)
    : QWidget(parent)
    , isAnimating(false)
{
    // Initialize NIL node (sentinel)
    NIL = new RBNode(0);
    NIL->color = BLACK;
    NIL->left = NIL->right = NIL->parent = nullptr;
    root = NIL;

    animationTimer = new QTimer(this);

    setupUI();
    setMinimumSize(900, 750);
}

RedBlackTree::~RedBlackTree()
{
    clearTree(root);
    delete NIL;
}

void RedBlackTree::setupUI()
{
    // Main stacked widget to switch between views
    mainStack = new QStackedWidget(this);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(mainStack);

    // Create tree view
    treeViewWidget = new QWidget();
    QVBoxLayout *treeLayout = new QVBoxLayout(treeViewWidget);
    treeLayout->setContentsMargins(30, 30, 30, 30);
    treeLayout->setSpacing(15);

    // Header
    QHBoxLayout *headerLayout = new QHBoxLayout();

    backButton = new QPushButton("← Back to Operations", treeViewWidget);
    backButton->setFixedSize(160, 38);
    backButton->setCursor(Qt::PointingHandCursor);
    backButton->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(123, 79, 255, 0.1);
            color: #7b4fff;
            border: 2px solid #7b4fff;
            border-radius: 19px;
            padding: 8px 16px;
            font-size: 11px;
        }
        QPushButton:hover { background-color: rgba(123, 79, 255, 0.2); }
    )");

    headerLayout->addWidget(backButton);
    headerLayout->addStretch();
    treeLayout->addLayout(headerLayout);

    // Title
    titleLabel = new QLabel("Red-Black Tree Visualization", treeViewWidget);
    titleLabel->setStyleSheet("color: #2d1b69; font-size: 26px; font-weight: bold;");
    titleLabel->setAlignment(Qt::AlignCenter);
    treeLayout->addWidget(titleLabel);

    // Controls
    QHBoxLayout *controlLayout = new QHBoxLayout();
    controlLayout->setSpacing(10);

    inputField = new QLineEdit(treeViewWidget);
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

    insertButton = new QPushButton("Insert", treeViewWidget);
    deleteButton = new QPushButton("Delete", treeViewWidget);
    searchButton = new QPushButton("Search", treeViewWidget);
    clearButton = new QPushButton("Clear", treeViewWidget);
    viewAlgorithmButton = new QPushButton("View Algorithm", treeViewWidget);

    QList<QPushButton*> buttons = {insertButton, deleteButton, searchButton, clearButton};
    for (auto btn : buttons) {
        btn->setFixedSize(90, 40);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #7b4fff, stop:1 #9b6fff);
                color: white;
                border: none;
                border-radius: 20px;
                font-weight: bold;
                font-size: 12px;
            }
            QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6c3cff, stop:1 #8b5fff); }
        )");
    }

    viewAlgorithmButton->setFixedSize(130, 40);
    viewAlgorithmButton->setCursor(Qt::PointingHandCursor);
    viewAlgorithmButton->setStyleSheet(R"(
        QPushButton {
            background-color: #28a745;
            color: white;
            border: none;
            border-radius: 20px;
            font-weight: bold;
            font-size: 12px;
        }
        QPushButton:hover { background-color: #218838; }
    )");

    controlLayout->addStretch();
    controlLayout->addWidget(inputField);
    controlLayout->addWidget(insertButton);
    controlLayout->addWidget(deleteButton);
    controlLayout->addWidget(searchButton);
    controlLayout->addWidget(clearButton);
    controlLayout->addWidget(viewAlgorithmButton);
    controlLayout->addStretch();

    treeLayout->addLayout(controlLayout);

    // Status
    statusLabel = new QLabel("Tree is empty. Insert values to begin!", treeViewWidget);
    statusLabel->setStyleSheet("color: #7b4fff; font-size: 11px; padding: 5px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    treeLayout->addWidget(statusLabel);

    // Split view - Tree and History
    QHBoxLayout *contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(15);

    // Tree visualization area (70%)
    treeLayout->addLayout(contentLayout, 1);

    // History panel (30% width)
    QVBoxLayout *historyLayout = new QVBoxLayout();
    QLabel *historyTitle = new QLabel("Operation History");
    historyTitle->setStyleSheet("color: #2d1b69; font-weight: bold; font-size: 14px;");
    historyLayout->addWidget(historyTitle);

    historyList = new QListWidget();
    historyList->setStyleSheet(R"(
        QListWidget {
            background-color: white;
            border: 2px solid #d0c5e8;
            border-radius: 8px;
            padding: 5px;
            font-size: 10px;
            color: #2d1b69;
        }
        QListWidget::item {
            padding: 6px;
            border-bottom: 1px solid #f0f0f0;
            color: #2d1b69;
        }
        QListWidget::item:hover {
            background-color: #f5f0ff;
        }
        QListWidget::item:selected {
            background-color: #e8e0ff;
            color: #2d1b69;
        }
    )");
    historyLayout->addWidget(historyList);

    contentLayout->addStretch(7);
    contentLayout->addLayout(historyLayout, 3);

    mainStack->addWidget(treeViewWidget);

    // Setup algorithm view
    setupAlgorithmView();

    // Connect signals
    connect(backButton, &QPushButton::clicked, this, &RedBlackTree::onBackClicked);
    connect(insertButton, &QPushButton::clicked, this, &RedBlackTree::onInsertClicked);
    connect(deleteButton, &QPushButton::clicked, this, &RedBlackTree::onDeleteClicked);
    connect(searchButton, &QPushButton::clicked, this, &RedBlackTree::onSearchClicked);
    connect(clearButton, &QPushButton::clicked, this, &RedBlackTree::onClearClicked);
    connect(viewAlgorithmButton, &QPushButton::clicked, this, &RedBlackTree::onViewAlgorithmClicked);
    connect(inputField, &QLineEdit::returnPressed, this, &RedBlackTree::onInsertClicked);
}

void RedBlackTree::setupAlgorithmView()
{
    algorithmViewWidget = new QWidget();
    QVBoxLayout *algoLayout = new QVBoxLayout(algorithmViewWidget);
    algoLayout->setContentsMargins(30, 30, 30, 30);
    algoLayout->setSpacing(20);

    // Header
    QHBoxLayout *algoHeaderLayout = new QHBoxLayout();

    algorithmBackButton = new QPushButton("← Back to Tree", algorithmViewWidget);
    algorithmBackButton->setFixedSize(140, 38);
    algorithmBackButton->setCursor(Qt::PointingHandCursor);
    algorithmBackButton->setStyleSheet(R"(
        QPushButton {
            background-color: rgba(123, 79, 255, 0.1);
            color: #7b4fff;
            border: 2px solid #7b4fff;
            border-radius: 19px;
            padding: 8px 16px;
            font-size: 11px;
        }
        QPushButton:hover { background-color: rgba(123, 79, 255, 0.2); }
    )");

    algoHeaderLayout->addWidget(algorithmBackButton);
    algoHeaderLayout->addStretch();
    algoLayout->addLayout(algoHeaderLayout);

    // Title
    algorithmTitleLabel = new QLabel("Red-Black Tree Algorithms", algorithmViewWidget);
    algorithmTitleLabel->setStyleSheet("color: #2d1b69; font-size: 24px; font-weight: bold;");
    algorithmTitleLabel->setAlignment(Qt::AlignCenter);
    algoLayout->addWidget(algorithmTitleLabel);

    // Algorithm selection buttons
    QHBoxLayout *algoButtonLayout = new QHBoxLayout();

    insertAlgoButton = new QPushButton("Insertion Algorithm", algorithmViewWidget);
    deleteAlgoButton = new QPushButton("Deletion Algorithm", algorithmViewWidget);
    searchAlgoButton = new QPushButton("Search Algorithm", algorithmViewWidget);

    QList<QPushButton*> algoBtns = {insertAlgoButton, deleteAlgoButton, searchAlgoButton};
    for (auto btn : algoBtns) {
        btn->setFixedSize(180, 45);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(R"(
            QPushButton {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #7b4fff, stop:1 #9b6fff);
                color: white;
                border: none;
                border-radius: 22px;
                font-weight: bold;
                font-size: 13px;
            }
            QPushButton:hover { background: qlineargradient(x1:0, y1:0, x2:1, y2:1, stop:0 #6c3cff, stop:1 #8b5fff); }
        )");
    }

    algoButtonLayout->addStretch();
    algoButtonLayout->addWidget(insertAlgoButton);
    algoButtonLayout->addWidget(deleteAlgoButton);
    algoButtonLayout->addWidget(searchAlgoButton);
    algoButtonLayout->addStretch();

    algoLayout->addLayout(algoButtonLayout);

    // Algorithm display
    algorithmDisplay = new QTextEdit(algorithmViewWidget);
    algorithmDisplay->setReadOnly(true);
    algorithmDisplay->setStyleSheet(R"(
        QTextEdit {
            background-color: #f8f9fa;
            border: 2px solid #d0c5e8;
            border-radius: 12px;
            padding: 20px;
            font-family: 'Courier New', monospace;
            font-size: 12px;
            color: #2d1b69;
            line-height: 1.6;
        }
    )");

    algoLayout->addWidget(algorithmDisplay);

    mainStack->addWidget(algorithmViewWidget);

    // Connect algorithm buttons
    connect(algorithmBackButton, &QPushButton::clicked, this, &RedBlackTree::onAlgorithmBackClicked);
    connect(insertAlgoButton, &QPushButton::clicked, this, &RedBlackTree::showInsertionAlgorithm);
    connect(deleteAlgoButton, &QPushButton::clicked, this, &RedBlackTree::showDeletionAlgorithm);
    connect(searchAlgoButton, &QPushButton::clicked, this, &RedBlackTree::showSearchAlgorithm);
}

void RedBlackTree::onBackClicked()
{
    emit backToOperations();
}

void RedBlackTree::onViewAlgorithmClicked()
{
    mainStack->setCurrentWidget(algorithmViewWidget);
    showInsertionAlgorithm(); // Show insertion by default
}

void RedBlackTree::onAlgorithmBackClicked()
{
    mainStack->setCurrentWidget(treeViewWidget);
}

void RedBlackTree::showInsertionAlgorithm()
{
    algorithmDisplay->setHtml(R"(
        <h2 style="color: #7b4fff;">Red-Black Tree Insertion Algorithm</h2>
        <h3>Properties:</h3>
        <ul>
            <li>Every node is either RED or BLACK</li>
            <li>Root is always BLACK</li>
            <li>All NIL leaves are BLACK</li>
            <li>Red nodes have BLACK children (no two consecutive red nodes)</li>
            <li>All paths from root to NIL have same number of black nodes</li>
        </ul>

        <h3>Algorithm Steps:</h3>
        <pre>
<b>1. BST Insertion:</b>
   - Insert node as in regular BST
   - Color new node RED

<b>2. Fix Violations:</b>
   While (node != root AND parent is RED):

   <b>Case 1: Uncle is RED</b>
      - Recolor parent and uncle to BLACK
      - Recolor grandparent to RED
      - Move to grandparent

   <b>Case 2: Uncle is BLACK (Triangle)</b>
      - Rotate to convert to Case 3

   <b>Case 3: Uncle is BLACK (Line)</b>
      - Rotate grandparent
      - Recolor parent and grandparent

<b>3. Final Step:</b>
   - Color root BLACK
        </pre>

        <h3>Time Complexity: O(log n)</h3>
    )");
}

void RedBlackTree::showDeletionAlgorithm()
{
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
    )");
}

void RedBlackTree::showSearchAlgorithm()
{
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
    )");
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

    searchNode(value);
    inputField->clear();
}

void RedBlackTree::onClearClicked()
{
    clearTree(root);
    root = NIL;
    history.clear();
    historyList->clear();
    statusLabel->setText("Tree cleared!");
    addHistory("CLEAR", 0, "Entire tree cleared");
    update();
}

void RedBlackTree::insertNode(int value)
{
    // Check for duplicate
    if (findNode(root, value) != NIL) {
        statusLabel->setText(QString("Value %1 already exists!").arg(value));
        addHistory("INSERT", value, QString("Failed: Value %1 already exists").arg(value));
        return;
    }

    isAnimating = true;
    insertButton->setEnabled(false);
    deleteButton->setEnabled(false);
    searchButton->setEnabled(false);

    statusLabel->setText(QString("Inserting %1...").arg(value));

    RBNode *node = new RBNode(value);
    node->left = node->right = NIL;

    root = BSTInsert(root, node);

    QTimer::singleShot(500, this, [this, node, value]() {
        node->isHighlighted = true;
        statusLabel->setText("Fixing Red-Black properties...");
        update();

        QTimer::singleShot(800, this, [this, node, value]() {
            fixInsert(node);
            node->isHighlighted = false;

            statusLabel->setText(QString("Successfully inserted %1").arg(value));
            addHistory("INSERT", value, QString("Node %1 inserted and tree balanced").arg(value));

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
        return node;
    }

    if (node->value < root->value) {
        root->left = BSTInsert(root->left, node);
        root->left->parent = root;
    } else if (node->value > root->value) {
        root->right = BSTInsert(root->right, node);
        root->right->parent = root;
    }

    return root;
}

void RedBlackTree::fixInsert(RBNode* node)
{
    while (node->parent && node->parent->color == RED) {
        if (node->parent == node->parent->parent->left) {
            RBNode *uncle = node->parent->parent->right;

            if (uncle && uncle->color == RED) {
                // Case 1: Uncle is red
                node->parent->color = BLACK;
                uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->right) {
                    // Case 2: Triangle
                    node = node->parent;
                    rotateLeft(node);
                }
                // Case 3: Line
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                rotateRight(node->parent->parent);
            }
        } else {
            RBNode *uncle = node->parent->parent->left;

            if (uncle && uncle->color == RED) {
                node->parent->color = BLACK;
                uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            } else {
                if (node == node->parent->left) {
                    node = node->parent;
                    rotateRight(node);
                }
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                rotateLeft(node->parent->parent);
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

        node->isRotating = false;
        update();
    });
}

void RedBlackTree::rotateRight(RBNode* node)
{
    if (!node || node->left == NIL) return;

    statusLabel->setText("Performing right rotation...");
    node->isRotating = true;
    update();

    QTimer::singleShot(600, this, [this, node]() {
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

        node->isRotating = false;
        update();
    });
}

void RedBlackTree::deleteNode(int value)
{
    RBNode *node = findNode(root, value);
    if (node == NIL) {
        statusLabel->setText(QString("Value %1 not found!").arg(value));
        addHistory("DELETE", value, QString("Failed: Value %1 not found").arg(value));
        return;
    }

    isAnimating = true;
    insertButton->setEnabled(false);
    deleteButton->setEnabled(false);
    searchButton->setEnabled(false);

    // Highlight node to be deleted
    node->isHighlighted = true;
    statusLabel->setText(QString("Deleting %1...").arg(value));
    update();

    QTimer::singleShot(800, this, [this, node, value]() {
        root = deleteNodeHelper(root, value);

        statusLabel->setText(QString("Successfully deleted %1").arg(value));
        addHistory("DELETE", value, QString("Node %1 deleted and tree rebalanced").arg(value));

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
    } else if (value > node->value) {
        node->right = deleteNodeHelper(node->right, value);
    } else {
        // Node found - delete it
        if (node->left == NIL) {
            RBNode* temp = node->right;
            if (temp != NIL) temp->parent = node->parent;
            delete node;
            return temp;
        } else if (node->right == NIL) {
            RBNode* temp = node->left;
            if (temp != NIL) temp->parent = node->parent;
            delete node;
            return temp;
        }

        // Node with two children
        RBNode* successor = findMin(node->right);
        node->value = successor->value;
        node->right = deleteNodeHelper(node->right, successor->value);
    }

    return node;
}

void RedBlackTree::searchNode(int value)
{
    resetHighlights(root);
    RBNode *node = findNode(root, value);

    if (node != NIL) {
        node->isHighlighted = true;
        statusLabel->setText(QString("Found %1 in tree!").arg(value));
        addHistory("SEARCH", value, QString("Node %1 found at position").arg(value));
    } else {
        statusLabel->setText(QString("Value %1 not found!").arg(value));
        addHistory("SEARCH", value, QString("Value %1 not found in tree").arg(value));
    }
    update();
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

void RedBlackTree::addHistory(const QString &operation, int value, const QString &description)
{
    HistoryEntry entry;
    entry.operation = operation;
    entry.value = value;
    entry.description = description;
    entry.timestamp = getCurrentTime();

    history.append(entry);

    QString displayText = QString("[%1] %2: %3")
                              .arg(entry.timestamp)
                              .arg(entry.operation)
                              .arg(entry.description);

    historyList->addItem(displayText);
    historyList->scrollToBottom();
}

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
    resetHighlights(node->left);
    resetHighlights(node->right);
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
    if (mainStack->currentWidget() == treeViewWidget && root != NIL) {
        // Calculate canvas area (left 70% of content area)
        int canvasY = 280;
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

    if (node->isHighlighted) {
        painter.setPen(QPen(QColor(255, 215, 0), 4));
        painter.setBrush(node->color == RED ? QColor(255, 150, 150) : QColor(100, 100, 100));
    } else if (node->isRotating) {
        painter.setPen(QPen(QColor(0, 200, 0), 4));
        painter.setBrush(node->color == RED ? QColor(255, 100, 100) : QColor(80, 80, 80));
    } else {
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
