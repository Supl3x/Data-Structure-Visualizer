#include "graphvisualization.h"
#include "uiutils.h"

// Dedicated canvas widget for drawing the graph
class GraphVisualization::GraphCanvas : public QWidget
{
public:
    GraphCanvas(GraphVisualization *parent) : QWidget(parent), graphViz(parent) {
        setMinimumSize(400, 300);
        setStyleSheet(R"(
            QWidget {
                background: white;
                border: 3px solid rgba(123, 79, 255, 0.8);
                border-radius: 16px;
            }
        )");
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // Clear background
        painter.fillRect(rect(), Qt::white);
        
        if (graphViz->nodes.isEmpty()) {
            // Draw placeholder text
            painter.setPen(QColor(123, 79, 255, 150));
            QFont font("Segoe UI", 16);
            painter.setFont(font);
            painter.drawText(rect(), Qt::AlignCenter, "Add vertices to see the graph");
            return;
        }
        
        // Layout nodes within this widget's bounds
        layoutNodesInCanvas();
        
        // Draw debug info
        painter.setPen(Qt::black);
        QFont debugFont("Arial", 10);
        painter.setFont(debugFont);
        painter.drawText(10, 20, QString("Nodes: %1").arg(graphViz->nodes.size()));
        if (!graphViz->nodes.isEmpty()) {
            painter.drawText(10, 40, QString("Node 0: (%1, %2)").arg(graphViz->nodes[0].pos.x()).arg(graphViz->nodes[0].pos.y()));
        }
        
        // Draw center dot
        QPointF center(width() / 2, height() / 2);
        painter.setPen(QPen(Qt::red, 2));
        painter.setBrush(Qt::red);
        painter.drawEllipse(center.x() - 5, center.y() - 5, 10, 10);
        
        // Draw graph
        drawGraph(painter);
    }

private:
    GraphVisualization *graphViz;
    
    void layoutNodesInCanvas() {
        if (graphViz->nodes.isEmpty()) return;
        
        QPointF center(width() / 2, height() / 2);
        int n = graphViz->nodes.size();
        
        if (n == 1) {
            graphViz->nodes[0].pos = center;
        } else {
            double radius = qMin(width(), height()) * 0.3;
            if (radius < 50) radius = 50;
            
            for (int i = 0; i < n; ++i) {
                double angle = (2 * M_PI * i) / n;
                graphViz->nodes[i].pos = QPointF(
                    center.x() + radius * qCos(angle),
                    center.y() + radius * qSin(angle)
                );
            }
        }
    }
    
    void drawGraph(QPainter &painter) {
        if (graphViz->nodes.isEmpty()) return;

        // Draw edges first
        for (auto it = graphViz->adjacency.constBegin(); it != graphViz->adjacency.constEnd(); ++it) {
            int u = it.key();
            for (int v : it.value()) {
                if (u < v) {
                    const GraphNode *nu = nullptr, *nv = nullptr;
                    for (const auto &n : graphViz->nodes) {
                        if (n.id == u) nu = &n;
                        if (n.id == v) nv = &n;
                    }
                    if (nu && nv) {
                        painter.setPen(QPen(QColor(75, 0, 130), 3));
                        painter.drawLine(nu->pos, nv->pos);
                    }
                }
            }
        }

        // Draw nodes on top
        for (const auto &n : graphViz->nodes) {
            drawNode(painter, n);
        }
    }
    
    void drawNode(QPainter &painter, const GraphNode &node) {
        const int NODE_RADIUS = 24;
        
        // Set colors similar to tree nodes
        if (node.highlighted) {
            painter.setPen(QPen(QColor(255, 140, 0), 4));  // Orange border for highlighted
            painter.setBrush(QColor(255, 165, 0));         // Orange fill
        } else if (node.visited) {
            painter.setPen(QPen(QColor(34, 139, 34), 4));  // Green border for visited
            painter.setBrush(QColor(50, 205, 50));         // Green fill
        } else {
            // Default tree-like colors (similar to binary tree)
            painter.setPen(QPen(QColor(123, 79, 255), 4));  // Purple border
            painter.setBrush(QColor(200, 180, 255));        // Light purple fill
        }
        
        // Draw circle
        QRectF nodeRect(node.pos.x() - NODE_RADIUS, node.pos.y() - NODE_RADIUS, NODE_RADIUS * 2, NODE_RADIUS * 2);
        painter.drawEllipse(nodeRect);
        
        // Draw text - use black text for better visibility on light backgrounds
        painter.setPen(Qt::black);
        QFont font("Segoe UI", 12, QFont::Bold);
        painter.setFont(font);
        painter.drawText(nodeRect, Qt::AlignCenter, QString::number(node.id));
    }
};

GraphVisualization::GraphVisualization(QWidget *parent)
    : QWidget(parent)
    , traversalType(TraversalType::None)
    , traversalIndex(0)
    , animTimer(new QTimer(this))
    , nextId(0)
{
    setupUI();
    setMinimumSize(900, 750);

    connect(animTimer, &QTimer::timeout, this, &GraphVisualization::onAnimationStep);
}

GraphVisualization::~GraphVisualization()
{
    // Stop any running animations
    if (animTimer) {
        animTimer->stop();
    }
    
    // Clear nodes and adjacency data
    nodes.clear();
    adjacency.clear();
    traversalOrder.clear();
}

void GraphVisualization::setupUI()
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
    
    // Initialize combo boxes after all UI components are created
    refreshCombos();
}

void GraphVisualization::setupVisualizationArea()
{
    leftPanel = new QWidget();
    leftPanel->setStyleSheet(R"(
        QWidget {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(248, 250, 255, 0.95),
                stop:1 rgba(245, 248, 255, 0.98));
        }
    )");

    leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(30, 20, 30, 20);
    leftLayout->setSpacing(20);

    // Header with back button and title
    QHBoxLayout *headerLayout = new QHBoxLayout();
    
    backButton = UIUtils::createBackButton(BackButton::BackToOperations);
    
    titleLabel = UIUtils::createTitleLabel("Graph Visualization");
    titleLabel->setStyleSheet(titleLabel->styleSheet() + "padding: 15px 0;");

    headerLayout->addWidget(backButton, 0, Qt::AlignLeft | Qt::AlignTop);
    headerLayout->addStretch();
    headerLayout->addWidget(titleLabel, 0, Qt::AlignCenter);
    headerLayout->addStretch();

    leftLayout->addLayout(headerLayout);

    // First row - Vertex operations
    QHBoxLayout *vertexLayout = new QHBoxLayout();
    vertexLayout->setSpacing(10);

    // Vertex input
    vertexInput = new QLineEdit();
    vertexInput->setPlaceholderText("Vertex ID");
    vertexInput->setFixedSize(100, 35);
    vertexInput->setStyleSheet(R"(
        QLineEdit {
            border: 2px solid rgba(123, 79, 255, 0.3);
            border-radius: 17px;
            padding: 8px 12px;
            font-size: 11px;
            background: white;
            color: #2d1b69;
        }
        QLineEdit:focus {
            border-color: rgba(123, 79, 255, 0.8);
        }
    )");

    addVertexButton = new QPushButton("Add Vertex");
    addVertexButton->setFixedSize(85, 35);
    addVertexButton->setStyleSheet(R"(
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
    )");

    deleteVertexButton = new QPushButton("Delete Vertex");
    deleteVertexButton->setFixedSize(95, 35);
    deleteVertexButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ff4757, stop:1 #ff6b7a);
            color: white;
            border: none;
            border-radius: 17px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ff3742, stop:1 #ff5865);
        }
    )");

    clearButton = new QPushButton("Clear");
    clearButton->setFixedSize(55, 35);
    clearButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ffa502, stop:1 #ff9500);
            color: white;
            border: none;
            border-radius: 17px;
            font-weight: bold;
            font-size: 10px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #ff9500, stop:1 #ff8500);
        }
    )");

    randomizeButton = new QPushButton("Random");
    randomizeButton->setFixedSize(75, 35);
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

    vertexLayout->addWidget(vertexInput);
    vertexLayout->addWidget(addVertexButton);
    vertexLayout->addWidget(deleteVertexButton);
    vertexLayout->addWidget(clearButton);
    vertexLayout->addWidget(randomizeButton);
    vertexLayout->addStretch();

    // Second row - Edge operations
    QHBoxLayout *edgeLayout = new QHBoxLayout();
    edgeLayout->setSpacing(10);

    edgeFromInput = new QLineEdit();
    edgeFromInput->setPlaceholderText("From");
    edgeFromInput->setFixedSize(70, 35);
    edgeFromInput->setStyleSheet(vertexInput->styleSheet());

    edgeToInput = new QLineEdit();
    edgeToInput->setPlaceholderText("To");
    edgeToInput->setFixedSize(70, 35);
    edgeToInput->setStyleSheet(vertexInput->styleSheet());

    addEdgeButton = new QPushButton("Add Edge");
    addEdgeButton->setFixedSize(75, 35);
    addEdgeButton->setStyleSheet(addVertexButton->styleSheet());

    deleteEdgeButton = new QPushButton("Delete Edge");
    deleteEdgeButton->setFixedSize(85, 35);
    deleteEdgeButton->setStyleSheet(deleteVertexButton->styleSheet());

    randomizeEdgeButton = new QPushButton("Random Edge");
    randomizeEdgeButton->setFixedSize(90, 35);
    randomizeEdgeButton->setStyleSheet(R"(
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

    edgeLayout->addWidget(edgeFromInput);
    edgeLayout->addWidget(new QLabel("→"));
    edgeLayout->addWidget(edgeToInput);
    edgeLayout->addWidget(addEdgeButton);
    edgeLayout->addWidget(deleteEdgeButton);
    edgeLayout->addWidget(randomizeEdgeButton);
    edgeLayout->addStretch();

    leftLayout->addLayout(vertexLayout);
    leftLayout->addLayout(edgeLayout);

    statusLabel = new QLabel("Graph is empty. Add a vertex to begin.");
    QFont statusFont("Segoe UI", 11);
    statusLabel->setFont(statusFont);
    statusLabel->setStyleSheet("color: #7b4fff; padding: 8px;");
    statusLabel->setAlignment(Qt::AlignCenter);
    leftLayout->addWidget(statusLabel);

    // Add the dedicated canvas widget
    canvas = new GraphCanvas(this);
    leftLayout->addWidget(canvas, 1); // Give it stretch factor 1 to expand

    // Connect signals
    connect(backButton, &BackButton::backRequested, this, &GraphVisualization::onBackClicked);
    connect(addVertexButton, &QPushButton::clicked, this, &GraphVisualization::onAddVertexClicked);
    connect(deleteVertexButton, &QPushButton::clicked, this, &GraphVisualization::onDeleteVertexClicked);
    connect(addEdgeButton, &QPushButton::clicked, this, &GraphVisualization::onAddEdgeClicked);
    connect(deleteEdgeButton, &QPushButton::clicked, this, &GraphVisualization::onDeleteEdgeClicked);
    connect(clearButton, &QPushButton::clicked, this, &GraphVisualization::onClearClicked);
    connect(randomizeButton, &QPushButton::clicked, this, &GraphVisualization::onRandomizeClicked);
    connect(randomizeEdgeButton, &QPushButton::clicked, this, &GraphVisualization::onRandomizeEdgeClicked);
    connect(vertexInput, &QLineEdit::returnPressed, this, &GraphVisualization::onAddVertexClicked);
}

void GraphVisualization::setupRightPanel()
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
    setupTraversalControls();
}

void GraphVisualization::setupStepTrace()
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
    QLabel *traceTitle = new QLabel("🔗 Graph Operations & Algorithms");
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

void GraphVisualization::setupTraversalControls()
{
    // Traversal controls using StyleManager - consistent styling!
    traversalGroup = new QGroupBox("");
    StyleManager::instance().applyTraversalGroupStyle(traversalGroup);

    QVBoxLayout *traversalLayout = new QVBoxLayout(traversalGroup);
    traversalLayout->setContentsMargins(20, 15, 20, 15);
    traversalLayout->setSpacing(10);

    // Compact heading
    QLabel *traversalTitle = new QLabel("🚀 Graph Traversal");
    traversalTitle->setStyleSheet(R"(
        QLabel {
            font-weight: bold;
            font-size: 14px;
            color: white;
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 rgba(138, 43, 226, 0.9),
                stop:0.5 rgba(30, 144, 255, 0.9),
                stop:1 rgba(0, 191, 255, 0.9));
            border-radius: 12px;
            padding: 8px 20px;
            margin: 3px;
        }
    )");
    traversalTitle->setAlignment(Qt::AlignCenter);
    traversalLayout->addWidget(traversalTitle);

    // Simple controls - just 2 buttons (no dropdown needed)
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

    // Connect traversal signals
    connect(bfsButton, &QPushButton::clicked, this, &GraphVisualization::onStartBFS);
    connect(dfsButton, &QPushButton::clicked, this, &GraphVisualization::onStartDFS);
}

void GraphVisualization::addStepToHistory(const QString &step)
{
    stepHistory.append(step);
    updateStepTrace();
}

void GraphVisualization::addOperationSeparator()
{
    stepHistory.append("────────────────────");
    updateStepTrace();
}

void GraphVisualization::updateStepTrace()
{
    stepsList->clear();
    
    for (const QString &step : stepHistory) {
        QListWidgetItem *item = new QListWidgetItem(step);
        
        if (step == "────────────────────") {
            item->setTextAlignment(Qt::AlignCenter);
            item->setForeground(QColor("#7b4fff"));
            QFont separatorFont = item->font();
            separatorFont.setBold(true);
            item->setFont(separatorFont);
        } else if (step.contains("✅") || step.contains("🎯")) {
            item->setForeground(QColor("#28a745"));
        } else if (step.contains("❌") || step.contains("⚠️")) {
            item->setForeground(QColor("#dc3545"));
        } else if (step.contains("🔍") || step.contains("👀")) {
            item->setForeground(QColor("#007bff"));
        } else if (step.contains("➕") || step.contains("🆕")) {
            item->setForeground(QColor("#6f42c1"));
        } else if (step.contains("🗑️") || step.contains("❌")) {
            item->setForeground(QColor("#fd7e14"));
        } else if (step.contains("🧮") || step.contains("⚡")) {
            item->setForeground(QColor("#20c997"));
        } else if (step.contains("🔄") || step.contains("↗️") || step.contains("↙️")) {
            item->setForeground(QColor("#e83e8c"));
        } else {
            item->setForeground(QColor("#495057"));
        }
        
        stepsList->addItem(item);
    }
    
    stepsList->scrollToBottom();
}

void GraphVisualization::showAlgorithm(const QString &operation)
{
    currentOperation = operation;
    algorithmList->clear();
    
    if (operation == "Add Vertex") {
        QStringList lines = {
            "📊 Add Vertex Algorithm",
            "",
            "⏰ Time Complexity: O(1)",
            "💾 Space Complexity: O(1)",
            "",
            "🔄 Steps:",
            "1. Check if vertex already exists",
            "2. If not exists, create new vertex",
            "3. Add vertex to adjacency list",
            "4. Update vertex counter",
            "5. Refresh display and combos"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("📊")) {
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
            } else if (line.contains(". ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }
    } else if (operation == "Add Edge") {
        QStringList lines = {
            "🔗 Add Edge Algorithm",
            "",
            "⏰ Time Complexity: O(1)",
            "💾 Space Complexity: O(1)",
            "",
            "🔄 Steps:",
            "1. Validate source and destination vertices",
            "2. Check if edge already exists",
            "3. Add edge to adjacency list",
            "4. For undirected graph, add reverse edge",
            "5. Update edge counter",
            "6. Refresh display"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("🔗")) {
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
            } else if (line.contains(". ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }
    } else if (operation == "Delete Vertex") {
        QStringList lines = {
            "🗑️ Delete Vertex Algorithm",
            "",
            "⏰ Time Complexity: O(V + E)",
            "💾 Space Complexity: O(1)",
            "",
            "🔄 Steps:",
            "1. Check if vertex exists in graph",
            "2. Remove all edges connected to vertex",
            "3. Remove vertex from adjacency list",
            "4. Remove vertex from nodes list",
            "5. Update layout and refresh display"
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
            } else if (line.contains(". ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }
    } else if (operation == "Delete Edge") {
        QStringList lines = {
            "✂️ Delete Edge Algorithm",
            "",
            "⏰ Time Complexity: O(1)",
            "💾 Space Complexity: O(1)",
            "",
            "🔄 Steps:",
            "1. Validate source and destination vertices",
            "2. Check if edge exists in adjacency list",
            "3. Remove edge from source vertex",
            "4. For undirected graph, remove reverse edge",
            "5. Update display"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("✂️")) {
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
            } else if (line.contains(". ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }
    } else if (operation == "BFS") {
        QStringList lines = {
            "🌊 Breadth-First Search (BFS)",
            "",
            "⏰ Time Complexity: O(V + E)",
            "💾 Space Complexity: O(V)",
            "",
            "🔄 Algorithm:",
            "1. Initialize queue with start vertex",
            "2. Mark start vertex as visited",
            "3. While queue is not empty:",
            "   a. Dequeue vertex from front",
            "   b. Process current vertex",
            "   c. For each unvisited neighbor:",
            "      - Mark as visited",
            "      - Enqueue neighbor",
            "4. Continue until queue is empty"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("🌊")) {
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
            } else if (line.contains(". ") || line.contains("a. ") || line.contains("b. ") || line.contains("c. ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }
    } else if (operation == "DFS") {
        QStringList lines = {
            "🏔️ Depth-First Search (DFS)",
            "",
            "⏰ Time Complexity: O(V + E)",
            "💾 Space Complexity: O(V)",
            "",
            "🔄 Algorithm:",
            "1. Initialize stack with start vertex",
            "2. Mark start vertex as visited",
            "3. While stack is not empty:",
            "   a. Pop vertex from top",
            "   b. Process current vertex",
            "   c. For each unvisited neighbor:",
            "      - Mark as visited",
            "      - Push neighbor to stack",
            "4. Continue until stack is empty"
        };
        
        for (const QString &line : lines) {
            QListWidgetItem *item = new QListWidgetItem(line);
            
            if (line.startsWith("🏔️")) {
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
            } else if (line.contains(". ") || line.contains("a. ") || line.contains("b. ") || line.contains("c. ")) {
                item->setForeground(QColor("#495057"));
            } else {
                item->setForeground(QColor("#6c757d"));
            }
            
            algorithmList->addItem(item);
        }
    }
}


void GraphVisualization::refreshCombos()
{
    // No longer needed since we use input fields instead of combo boxes
}

void GraphVisualization::setControlsEnabled(bool enabled)
{
    // Back button should NEVER be disabled - user must always be able to go back!
    if (backButton) backButton->setEnabled(true);
    
    if (addVertexButton) addVertexButton->setEnabled(enabled);
    if (deleteVertexButton) deleteVertexButton->setEnabled(enabled);
    if (addEdgeButton) addEdgeButton->setEnabled(enabled);
    if (deleteEdgeButton) deleteEdgeButton->setEnabled(enabled);
    if (clearButton) clearButton->setEnabled(enabled);
    if (bfsButton) bfsButton->setEnabled(enabled);
    if (dfsButton) dfsButton->setEnabled(enabled);
    if (vertexInput) vertexInput->setEnabled(enabled);
    if (edgeFromInput) edgeFromInput->setEnabled(enabled);
    if (edgeToInput) edgeToInput->setEnabled(enabled);
}

void GraphVisualization::onBackClicked()
{
    if (traversalType != TraversalType::None) {
        animTimer->stop();
        traversalType = TraversalType::None;
    }
    emit backToOperations();
}

void GraphVisualization::onAddVertexClicked()
{
    showAlgorithm("Add Vertex");
    
    int id = -1;
    QString text = vertexInput->text().trimmed();
    if (!text.isEmpty()) {
        bool ok;
        id = text.toInt(&ok);
        if (!ok) {
            addStepToHistory("❌ Invalid input: Please enter a valid integer for vertex ID");
            QMessageBox::warning(this, "Invalid Input", "Please enter a valid integer for vertex ID.");
            return;
        }
        addStepToHistory(QString("🔍 Checking if vertex %1 already exists...").arg(id));
        // Check if ID already exists
        for (const auto &n : nodes) {
            if (n.id == id) {
                addStepToHistory(QString("❌ Vertex %1 already exists in the graph").arg(id));
                QMessageBox::warning(this, "Duplicate ID", QString("Vertex %1 already exists.").arg(id));
                return;
            }
        }
        addStepToHistory(QString("✅ Vertex %1 is unique, proceeding with addition").arg(id));
    } else {
        // Auto-generate ID
        id = nextId++;
        addStepToHistory(QString("🆕 Auto-generating vertex ID: %1").arg(id));
    }
    
    addStepToHistory(QString("➕ Creating new vertex %1 in adjacency list").arg(id));
    nodes.append(GraphNode(id, QPointF(0, 0)));
    adjacency.insert(id, QSet<int>());
    if (id >= nextId) nextId = id + 1;
    
    addStepToHistory("🔄 Recalculating graph layout...");
    refreshCombos();
    vertexInput->clear();
    statusLabel->setText(QString("Vertex %1 added.").arg(id));
    addStepToHistory(QString("✅ Vertex %1 successfully added to graph").arg(id));
    addOperationSeparator();
    
    // Update the canvas
    if (canvas) {
        canvas->update();
    }
}

void GraphVisualization::onDeleteVertexClicked()
{
    showAlgorithm("Delete Vertex");
    
    QString text = vertexInput->text().trimmed();
    if (text.isEmpty()) {
        addStepToHistory("❌ Invalid input: Please enter a vertex ID to delete");
        QMessageBox::warning(this, "Invalid Input", "Please enter a vertex ID to delete.");
        return;
    }
    
    bool ok;
    int id = text.toInt(&ok);
    if (!ok) {
        addStepToHistory("❌ Invalid input: Please enter a valid integer for vertex ID");
        QMessageBox::warning(this, "Invalid Input", "Please enter a valid integer for vertex ID.");
        return;
    }
    
    addStepToHistory(QString("🔍 Searching for vertex %1 in the graph...").arg(id));
    
    // Check if vertex exists
    bool found = false;
    for (const auto &n : nodes) {
        if (n.id == id) {
            found = true;
            break;
        }
    }
    
    if (!found) {
        addStepToHistory(QString("❌ Vertex %1 not found in the graph").arg(id));
        QMessageBox::warning(this, "Vertex Not Found", QString("Vertex %1 does not exist.").arg(id));
        return;
    }
    
    addStepToHistory(QString("✅ Vertex %1 found, proceeding with deletion").arg(id));
    addStepToHistory(QString("🔗 Removing all edges connected to vertex %1").arg(id));
    
    if (removeVertexInternal(id)) {
        addStepToHistory("🔄 Recalculating graph layout...");
        vertexInput->clear();
        statusLabel->setText(QString("Vertex %1 and all its edges removed.").arg(id));
        addStepToHistory(QString("✅ Vertex %1 and all its edges successfully removed").arg(id));
        addOperationSeparator();
        if (canvas) canvas->update();
    }
}

void GraphVisualization::onAddEdgeClicked()
{
    showAlgorithm("Add Edge");
    
    QString fromText = edgeFromInput->text().trimmed();
    QString toText = edgeToInput->text().trimmed();
    
    if (fromText.isEmpty() || toText.isEmpty()) {
        addStepToHistory("❌ Invalid input: Please enter both vertex IDs");
        QMessageBox::warning(this, "Invalid Input", "Please enter both vertex IDs.");
        return;
    }
    
    bool ok1 = false, ok2 = false;
    int u = fromText.toInt(&ok1);
    int v = toText.toInt(&ok2);
    
    if (!ok1 || !ok2) {
        addStepToHistory("❌ Invalid input: Please enter valid integers for vertex IDs");
        QMessageBox::warning(this, "Invalid Input", "Please enter valid integers for vertex IDs.");
        return;
    }
    
    if (u == v) {
        addStepToHistory("❌ Self-loops are not allowed in this graph");
        QMessageBox::warning(this, "Invalid", "Self-loops are not allowed.");
        return;
    }
    
    addStepToHistory(QString("🔍 Attempting to add edge between vertex %1 and vertex %2").arg(u).arg(v));
    
    if (addEdgeInternal(u, v)) {
        edgeFromInput->clear();
        edgeToInput->clear();
        statusLabel->setText(QString("Edge %1-%2 added.").arg(u).arg(v));
        addStepToHistory(QString("✅ Edge %1-%2 successfully added to graph").arg(u).arg(v));
        addOperationSeparator();
        if (canvas) canvas->update();
    } else {
        statusLabel->setText("Edge already exists or vertices missing.");
        addStepToHistory(QString("❌ Failed: Edge %1-%2 already exists or vertices missing").arg(u).arg(v));
        addOperationSeparator();
    }
}


void GraphVisualization::onDeleteEdgeClicked()
{
    showAlgorithm("Delete Edge");
    
    QString fromText = edgeFromInput->text().trimmed();
    QString toText = edgeToInput->text().trimmed();
    
    if (fromText.isEmpty() || toText.isEmpty()) {
        addStepToHistory("❌ Invalid input: Please enter both vertex IDs");
        QMessageBox::warning(this, "Invalid Input", "Please enter both vertex IDs.");
        return;
    }
    
    bool ok1 = false, ok2 = false;
    int u = fromText.toInt(&ok1);
    int v = toText.toInt(&ok2);
    
    if (!ok1 || !ok2) {
        addStepToHistory("❌ Invalid input: Please enter valid integers for vertex IDs");
        QMessageBox::warning(this, "Invalid Input", "Please enter valid integers for vertex IDs.");
        return;
    }
    
    addStepToHistory(QString("🔍 Attempting to remove edge between vertex %1 and vertex %2").arg(u).arg(v));
    
    if (removeEdgeInternal(u, v)) {
        edgeFromInput->clear();
        edgeToInput->clear();
        statusLabel->setText(QString("Edge %1-%2 removed.").arg(u).arg(v));
        addStepToHistory(QString("✅ Edge %1-%2 successfully removed from graph").arg(u).arg(v));
        addOperationSeparator();
        if (canvas) canvas->update();
    } else {
        statusLabel->setText("Edge does not exist or vertices missing.");
        addStepToHistory(QString("❌ Failed: Edge %1-%2 does not exist or vertices missing").arg(u).arg(v));
        addOperationSeparator();
    }
}

void GraphVisualization::onRemoveEdgeClicked()
{
    // This function is kept for compatibility but does nothing
    QMessageBox::information(this, "Info", "Remove edge functionality not available in simplified UI.");
}

void GraphVisualization::onClearClicked()
{
    nodes.clear();
    adjacency.clear();
    nextId = 0;
    traversalType = TraversalType::None;
    frontier.clear();
    traversalOrder.clear();
    traversalIndex = 0;
    animTimer->stop();
    stepHistory.clear();
    stepsList->clear();
    refreshCombos();
    statusLabel->setText("Graph cleared! Add a vertex to begin.");
    addStepToHistory("🗑️ Entire graph cleared");
    if (canvas) canvas->update();
}

void GraphVisualization::onRandomizeClicked()
{
    // Generate a single random vertex ID between 1 and 100
    int randomVertex = QRandomGenerator::global()->bounded(1, 101);
    
    // Set the input field and trigger add vertex
    vertexInput->setText(QString::number(randomVertex));
    onAddVertexClicked();
}

void GraphVisualization::onRandomizeEdgeClicked()
{
    // Check if graph has at least 2 vertices
    if (nodes.size() < 2) {
        QMessageBox::warning(this, "Insufficient Vertices", 
                             "Please add at least 2 vertices before creating random edges.");
        return;
    }
    
    // Pick two random existing vertices
    QVector<int> vertexIds;
    for (const auto &node : nodes) {
        vertexIds.append(node.id);
    }
    
    // Shuffle and pick two different vertices
    int fromIdx = QRandomGenerator::global()->bounded(vertexIds.size());
    int toIdx = QRandomGenerator::global()->bounded(vertexIds.size());
    
    // Ensure they're different
    while (toIdx == fromIdx && vertexIds.size() > 1) {
        toIdx = QRandomGenerator::global()->bounded(vertexIds.size());
    }
    
    int fromVertex = vertexIds[fromIdx];
    int toVertex = vertexIds[toIdx];
    
    // Set the input fields and trigger add edge
    edgeFromInput->setText(QString::number(fromVertex));
    edgeToInput->setText(QString::number(toVertex));
    onAddEdgeClicked();
}

void GraphVisualization::onStartBFS()
{
    showAlgorithm("BFS");
    
    if (nodes.isEmpty()) {
        addStepToHistory("❌ Graph is empty: Please add vertices first");
        QMessageBox::warning(this, "Empty Graph", "Please add vertices to the graph first.");
        return;
    }
    
    // Use the first vertex as starting point
    int s = nodes.first().id;
    
    addStepToHistory(QString("🌊 Starting BFS traversal from vertex %1").arg(s));
    resetHighlights();
    traversalType = TraversalType::BFS;
    traversalOrder.clear();
    frontier.clear();
    traversalIndex = 0;

    // BFS precompute order
    addStepToHistory("🔄 Initializing BFS queue and visited set");
    QSet<int> visited;
    QList<int> queue;
    queue.append(s);
    visited.insert(s);
    addStepToHistory(QString("➕ Added vertex %1 to queue and marked as visited").arg(s));
    
    while (!queue.isEmpty()) {
        int u = queue.front();
        queue.pop_front();
        traversalOrder.append(u);
        addStepToHistory(QString("👀 Processing vertex %1 (dequeued from front)").arg(u));
        
        for (int v : adjacency.value(u)) {
            if (!visited.contains(v)) {
                visited.insert(v);
                queue.append(v);
                addStepToHistory(QString("🆕 Found unvisited neighbor %1, added to queue").arg(v));
            }
        }
    }

    // Display traversal result
    QString result = "🌊 BFS Order: ";
    for (int i = 0; i < traversalOrder.size(); ++i) {
        result += QString::number(traversalOrder[i]);
        if (i < traversalOrder.size() - 1) result += " → ";
    }
    
    traversalResultList->clear();
    if (traversalOrder.isEmpty()) {
        traversalResultList->addItem("❌ No traversal result");
    } else {
        traversalResultList->addItem(result);
        // Also add to step history for debugging
        addStepToHistory(QString("📋 %1").arg(result));
    }

    setControlsEnabled(false);
    animTimer->start(700);
    statusLabel->setText("Running BFS...");
    addStepToHistory("✅ BFS traversal order computed, starting animation");
    addOperationSeparator();
}

void GraphVisualization::onStartDFS()
{
    showAlgorithm("DFS");
    
    if (nodes.isEmpty()) {
        addStepToHistory("❌ Graph is empty: Please add vertices first");
        QMessageBox::warning(this, "Empty Graph", "Please add vertices to the graph first.");
        return;
    }
    
    // Use the first vertex as starting point
    int s = nodes.first().id;
    
    addStepToHistory(QString("🏔️ Starting DFS traversal from vertex %1").arg(s));
    resetHighlights();
    traversalType = TraversalType::DFS;
    traversalOrder.clear();
    frontier.clear();
    traversalIndex = 0;

    // DFS precompute order
    addStepToHistory("🔄 Initializing DFS stack and visited set");
    QSet<int> visited;
    QList<int> stack;
    stack.append(s);
    addStepToHistory(QString("➕ Added vertex %1 to stack").arg(s));
    
    while (!stack.isEmpty()) {
        int u = stack.takeLast();
        if (visited.contains(u)) {
            addStepToHistory(QString("⚠️ Vertex %1 already visited, skipping").arg(u));
            continue;
        }
        visited.insert(u);
        traversalOrder.append(u);
        addStepToHistory(QString("👀 Processing vertex %1 (popped from stack)").arg(u));
        
        // push neighbors in reverse for a stable ordering
        QList<int> neigh = adjacency.value(u).values();
        std::sort(neigh.begin(), neigh.end(), std::greater<int>());
        for (int v : neigh) {
            if (!visited.contains(v)) {
                stack.append(v);
                addStepToHistory(QString("🆕 Found unvisited neighbor %1, pushed to stack").arg(v));
            }
        }
    }

    // Display traversal result
    QString result = "🏔️ DFS Order: ";
    for (int i = 0; i < traversalOrder.size(); ++i) {
        result += QString::number(traversalOrder[i]);
        if (i < traversalOrder.size() - 1) result += " → ";
    }
    
    traversalResultList->clear();
    if (traversalOrder.isEmpty()) {
        traversalResultList->addItem("❌ No traversal result");
    } else {
        traversalResultList->addItem(result);
        // Also add to step history for debugging
        addStepToHistory(QString("📋 %1").arg(result));
    }

    setControlsEnabled(false);
    animTimer->start(700);
    statusLabel->setText("Running DFS...");
    addStepToHistory("✅ DFS traversal order computed, starting animation");
    addOperationSeparator();
}

void GraphVisualization::onAnimationStep()
{
    if (traversalIndex > 0 && traversalIndex <= traversalOrder.size()) {
        int prev = traversalOrder[traversalIndex - 1];
        // Mark previous as visited (persist green), remove highlight
        for (auto &n : nodes) if (n.id == prev) { n.visited = true; n.highlighted = false; }
    }

    if (traversalIndex >= traversalOrder.size()) {
        QString algo = (traversalType == TraversalType::BFS) ? "BFS" : "DFS";
        animTimer->stop();
        traversalType = TraversalType::None;
        setControlsEnabled(true);
        statusLabel->setText("Traversal complete.");
        addStepToHistory(QString("🎯 %1 traversal completed. Visited %2 vertices").arg(algo).arg(traversalOrder.size()));
        update();
        return;
    }

    int u = traversalOrder[traversalIndex];
    for (auto &n : nodes) if (n.id == u) { n.highlighted = true; }
    statusLabel->setText(QString("Visiting %1").arg(u));
    traversalIndex++;
    update();
}

// layoutNodes moved to GraphCanvas class

void GraphVisualization::resetHighlights()
{
    for (auto &n : nodes) { n.highlighted = false; n.visited = false; }
}


bool GraphVisualization::addEdgeInternal(int u, int v)
{
    if (!adjacency.contains(u) || !adjacency.contains(v)) return false;
    if (adjacency[u].contains(v)) return false;
    adjacency[u].insert(v);
    adjacency[v].insert(u);
    return true;
}

bool GraphVisualization::removeVertexInternal(int u)
{
    if (!adjacency.contains(u)) return false;
    // remove edges
    for (int v : adjacency[u]) adjacency[v].remove(u);
    adjacency.remove(u);
    // remove node
    for (int i = 0; i < nodes.size(); ++i) {
        if (nodes[i].id == u) { nodes.removeAt(i); break; }
    }
    return true;
}

bool GraphVisualization::removeEdgeInternal(int u, int v)
{
    if (!adjacency.contains(u) || !adjacency.contains(v)) return false;
    bool existed = adjacency[u].remove(v) | adjacency[v].remove(u);
    return existed;
}

// paintEvent removed - now using dedicated GraphCanvas widget

void GraphVisualization::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (canvas) {
        canvas->update();
    }
}

// Drawing functions moved to GraphCanvas class


QString GraphVisualization::getCurrentTime()
{
    return QDateTime::currentDateTime().toString("HH:mm:ss");
}





