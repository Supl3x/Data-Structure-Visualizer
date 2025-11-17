#ifndef HASHMAPVISUALIZATION_H
#define HASHMAPVISUALIZATION_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QListWidget>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QGraphicsPathItem>
#include <QPainterPath>
#include <QTimer>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QScrollArea>
#include <QSplitter>
#include <QComboBox>
#include <QGroupBox>
#include <QTabWidget>
#include <QPainter>
#include <QLinearGradient>
#include <QFont>
#include <QFontDatabase>
#include <QDebug>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QGraphicsDropShadowEffect>
#include <QScrollBar>
#include <QSplitterHandle>
#include <memory>
#include "hashmap.h"
#include "backbutton.h"
#include "stylemanager.h"
#include "widgetmanager.h"

class HashMapVisualization : public QWidget
{
    Q_OBJECT

public:
    explicit HashMapVisualization(QWidget *parent = nullptr);
    ~HashMapVisualization();

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
    void onTypeChanged();
    void updateVisualization();
    void updateStepTrace();

private:
    void setupUI();
    void setupVisualizationArea();
    void setupRightPanel();
    void setupTypeSelection();
    void setupStatsAndControls();
    void setupStepTrace();
    void setupStepTraceTop();
    void drawBuckets();
    void animateOperation(const QString &operation);
    void animateSearchResult(const QString &key, bool found);
    void showAlgorithm(const QString &operation);
    void showStats();
    void zoomToFit();
    QVariant convertStringToVariant(const QString &str, HashMap::DataType type);

    // UI Components
    QSplitter *mainSplitter;
    QWidget *leftPanel;
    QWidget *rightPanel;

    // Left panel - visualization and controls
    QVBoxLayout *leftLayout;
    BackButton *backButton;
    QLabel *titleLabel;
    QGraphicsView *visualizationView;
    QGraphicsScene *scene;

    // Right panel - controls and step trace
    QVBoxLayout *rightLayout;
    // Type selection
    QGroupBox *typeGroup;
    QComboBox *keyTypeCombo;
    QComboBox *valueTypeCombo;
    // Controls
    QLineEdit *keyInput;
    QLineEdit *valueInput;
    QPushButton *insertButton;
    QPushButton *searchButton;
    QPushButton *deleteButton;
    QPushButton *clearButton;
    QPushButton *randomizeButton;
    // Stats (main area only)
    QLabel *sizeLabel;
    QLabel *bucketCountLabel;
    QLabel *loadFactorLabel;
    QLabel *loadFactorWarning;  // Warning label for load factor status
    QLabel *bucketNote;

    // Step trace with tabs
    QGroupBox *traceGroup;
    QTabWidget *traceTabWidget;
    QListWidget *stepsList;
    QListWidget *algorithmList;

    // Data and visualization
    HashMap *hashMap;
    QVector<QGraphicsRectItem*> bucketRects;
    QVector<QGraphicsTextItem*> bucketTexts;
    QVector<QVector<QGraphicsTextItem*>> chainTexts;

    // Animation
    QTimer *animationTimer;
    QGraphicsRectItem *highlightRect;
    
    // Track previous bucket count for zoom detection
    int previousBucketCount;

    // Constants
    static const int BUCKET_WIDTH;
    static const int BUCKET_HEIGHT;
    static const int BUCKET_SPACING;
    static const int CHAIN_ITEM_HEIGHT;
    static const int MAX_VISIBLE_BUCKETS;
};

#endif // HASHMAPVISUALIZATION_H
