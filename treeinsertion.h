#ifndef TREEINSERTION_H
#define TREEINSERTION_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QTimer>
#include <QVector>
#include <QGraphicsOpacityEffect>
#include <QSplitter>
#include <QGroupBox>
#include <QListWidget>
#include <QTabWidget>
#include <QLinearGradient>
#include <QFont>
#include <QFontDatabase>
#include <QMessageBox>
#include <QDebug>
#include <QRandomGenerator>
#include <cmath>
#include "uiutils.h"
#include "backbutton.h"
#include "stylemanager.h"
#include "widgetmanager.h"

// Tree Node structure
struct TreeNode {
    int value;
    TreeNode *left;
    TreeNode *right;
    int x, y;  // Position for drawing
    bool isHighlighted;
    bool isNewNode;
    bool isSearchHighlighted;
    bool isDeleteHighlighted;
    bool isTraversalHighlighted;
    bool isVisited;

    TreeNode(int val) : value(val), left(nullptr), right(nullptr),
        x(0), y(0), isHighlighted(false), isNewNode(false), 
        isSearchHighlighted(false), isDeleteHighlighted(false),
        isTraversalHighlighted(false), isVisited(false) {}
};

class TreeInsertion : public QWidget
{
    Q_OBJECT

public:
    explicit TreeInsertion(QWidget *parent = nullptr);
    ~TreeInsertion();

signals:
    void backToOperations();

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onBackClicked();
    void onInsertClicked();
    void onSearchClicked();
    void onDeleteClicked();
    void onClearClicked();
    void onRandomizeClicked();
    void onStartBFS();
    void onStartDFS();
    void onTraversalAnimationStep();

private:
    void setupUI();
    void setupVisualizationArea();
    void setupRightPanel();
    void setupStepTrace();
    void setupTraversalControls();
    void insertNode(int value);
    void animateInsertion(int value);
    void animateSearch(int value);
    void animateDelete(int value);
    TreeNode* searchNode(int value);
    TreeNode* deleteNode(TreeNode* node, int value);
    TreeNode* findMin(TreeNode* node);
    void calculateNodePositions(TreeNode *node, int x, int y, int horizontalSpacing);
    void calculateNodePositionsWithSpacing(TreeNode *node, int x, int y, int horizontalSpacing, int verticalSpacing);
    void drawTree(QPainter &painter, TreeNode *node);
    void drawNode(QPainter &painter, TreeNode *node);
    void drawEdge(QPainter &painter, int x1, int y1, int x2, int y2);
    void clearTree(TreeNode *node);
    int getTreeHeight(TreeNode *node);
    void resetHighlights(TreeNode *node);
    void resetTraversalHighlights(TreeNode *node);
    void animateTraversal(TreeNode *node, int value, int step);
    void animateSearchTraversal(TreeNode *node, int value, int step);
    void animateDeleteTraversal(TreeNode *node, int value, int step);
    void addStepToHistory(const QString &step);
    void addOperationSeparator();
    void updateStepTrace();
    void showAlgorithm(const QString &operation);
    
    // Traversal methods
    void performBFS();
    void performDFS();
    void performDFSRecursive(TreeNode* node);
    void setControlsEnabled(bool enabled);

    // UI Components - using UIUtils for reduced repetition
    QSplitter *mainSplitter;
    QWidget *leftPanel;
    QWidget *rightPanel;
    QVBoxLayout *leftLayout;
    QVBoxLayout *rightLayout;
    
    BackButton *backButton;
    QPushButton *insertButton;
    QPushButton *searchButton;
    QPushButton *deleteButton;
    QPushButton *clearButton;
    QPushButton *randomizeButton;
    QPushButton *bfsButton;
    QPushButton *dfsButton;
    QLineEdit *inputField;
    QLabel *titleLabel;
    QLabel *instructionLabel;
    QLabel *statusLabel;
    
    // Right panel components
    QGroupBox *traceGroup;
    QTabWidget *traceTabWidget;
    QListWidget *stepsList;
    QListWidget *algorithmList;
    
    // Traversal controls
    QGroupBox *traversalGroup;
    QListWidget *traversalResultList;

    // Tree data
    TreeNode *root;

    // Animation and operation tracking
    QTimer *animationTimer;
    QVector<TreeNode*> traversalPath;
    QVector<QString> stepHistory;
    int currentTraversalStep;
    bool isAnimating;
    QString currentOperation;
    
    // Traversal animation
    enum class TraversalType { None, BFS, DFS };
    TraversalType traversalType;
    QList<TreeNode*> traversalOrder;
    int traversalIndex;
    QTimer *traversalAnimTimer;

    // Drawing constants
    const int NODE_RADIUS = 25;
    const int LEVEL_HEIGHT = 80;
    const int CANVAS_TOP_MARGIN = 60;
};

#endif // TREEINSERTION_H
