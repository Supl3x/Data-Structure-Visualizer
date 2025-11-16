#ifndef REDBLACKTREE_H
#define REDBLACKTREE_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QTimer>
#include <QVector>
#include <QTextEdit>
#include <QListWidget>
#include <QStackedWidget>
#include <QScrollArea>
#include <QSplitter>
#include <QGroupBox>
#include <QTabWidget>
#include <QFontDatabase>
#include <QLinearGradient>
#include <QFont>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QRandomGenerator>
#include <QGraphicsDropShadowEffect>
#include <algorithm>
#include "backbutton.h"
#include "stylemanager.h"
#include "widgetmanager.h"

enum Color { RED, BLACK };

struct RBNode {
    int value;
    Color color;
    RBNode *left;
    RBNode *right;
    RBNode *parent;
    int x, y;
    bool isHighlighted;
    bool isRotating;
    bool isSearchHighlighted;
    bool isDeleteHighlighted;
    bool isNewNode;
    bool isTraversalHighlighted;
    bool isVisited;

    RBNode(int val) : value(val), color(RED), left(nullptr), right(nullptr),
        parent(nullptr), x(0), y(0), isHighlighted(false),
        isRotating(false), isSearchHighlighted(false), 
        isDeleteHighlighted(false), isNewNode(false), 
        isTraversalHighlighted(false), isVisited(false) {}
};

// HistoryEntry removed - now using step tracking system

class RedBlackTree : public QWidget
{
    Q_OBJECT

public:
    explicit RedBlackTree(QWidget *parent = nullptr);
    ~RedBlackTree();

signals:
    void backToOperations();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onBackClicked();
    void onInsertClicked();
    void onDeleteClicked();
    void onSearchClicked();
    void onClearClicked();
    void onRandomizeClicked();
    void onViewAlgorithmClicked();
    void onAlgorithmBackClicked();
    void showInsertionAlgorithm();
    void showDeletionAlgorithm();
    void showSearchAlgorithm();
    void onStartBFS();
    void onStartDFS();
    void onTraversalAnimationStep();

private:
    void setupUI();
    void setupVisualizationArea();
    void setupRightPanel();
    void setupStepTrace();
    void setupAlgorithmView();
    void setupTraversalControls();

    // RB Tree operations
    void insertNode(int value);
    void deleteNode(int value);
    void searchNode(int value);
    RBNode* BSTInsert(RBNode* root, RBNode* node);
    void fixInsert(RBNode* node);
    void rotateLeft(RBNode* node);
    void rotateRight(RBNode* node);
    void rotateLeftSync(RBNode* node);  // Synchronous rotation for fixInsert
    void rotateRightSync(RBNode* node); // Synchronous rotation for fixInsert
    RBNode* findNode(RBNode* node, int value);
    void fixDelete(RBNode* node);
    RBNode* findMin(RBNode* node);
    void transplant(RBNode* u, RBNode* v);
    RBNode* deleteNodeHelper(RBNode* node, int value);

    // Animation
    void animateRotation(RBNode* node, const QString& direction);
    void highlightNode(RBNode* node, int delay);

    // Drawing
    void calculateNodePositions(RBNode *node, int x, int y, int horizontalSpacing);
    void drawTree(QPainter &painter, RBNode *node);
    void drawNode(QPainter &painter, RBNode *node);
    void drawEdge(QPainter &painter, int x1, int y1, int x2, int y2, Color color);
    void clearTree(RBNode *node);
    void resetHighlights(RBNode *node);
    void resetTraversalHighlights(RBNode *node);
    
    // Traversal methods
    void performBFS();
    void performDFS();
    void performDFSRecursive(RBNode* node);
    void setControlsEnabled(bool enabled);

    // History and step tracking
    // addHistory removed - now using addStepToHistory
    void addStepToHistory(const QString &step);
    void addOperationSeparator();
    void updateStepTrace();
    void showAlgorithm(const QString &operation);
    QString getCurrentTime();

    // UI Components - Main View
    QSplitter *mainSplitter;
    QWidget *leftPanel;
    QWidget *rightPanel;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    
    // Main UI components
    BackButton *backButton;
    QPushButton *insertButton;
    QPushButton *deleteButton;
    QPushButton *searchButton;
    QPushButton *clearButton;
    QPushButton *randomizeButton;
    QPushButton *bfsButton;
    QPushButton *dfsButton;

    QLineEdit *inputField;
    QLabel *titleLabel;
    QLabel *statusLabel;
    
    // Right panel components
    QGroupBox *traceGroup;
    QTabWidget *traceTabWidget;
    QListWidget *stepsList;
    QListWidget *algorithmList;
    
    // Traversal controls
    QGroupBox *traversalGroup;
    QListWidget *traversalResultList;

    // Algorithm functionality now integrated in right panel

    // Tree data
    RBNode *root;
    RBNode *NIL;  // Sentinel node

    // History and step tracking
    // history vector removed - now using stepHistory QStringList
    QVector<QString> stepHistory;
    QString currentOperation;

    // Animation
    bool isAnimating;
    QTimer *animationTimer;
    
    // Traversal animation
    enum class TraversalType { None, BFS, DFS };
    TraversalType traversalType;
    QList<RBNode*> traversalOrder;
    int traversalIndex;
    QTimer *traversalAnimTimer;

    // Drawing constants
    const int NODE_RADIUS = 25;
    const int LEVEL_HEIGHT = 80;
};

#endif // REDBLACKTREE_H
