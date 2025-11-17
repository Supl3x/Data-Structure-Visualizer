#pragma once

#include <QString>
#include <QVector>
#include <QVariant>
#include <QHashFunctions>
#include <forward_list>
#include <optional>
#include <vector>

// Hash Map supporting multiple data types for keys and values.
// Instrumented with a human-readable step trace for visualization.
class HashMap {
public:
    enum DataType {
        STRING,
        INTEGER,
        DOUBLE,
        FLOAT,
        CHAR
    };

    explicit HashMap(int initialBucketCount = 16, float maxLoadFactor = 0.75f);

    // Set data types for key and value
    void setKeyType(DataType type) { keyType_ = type; }
    void setValueType(DataType type) { valueType_ = type; }
    DataType getKeyType() const { return keyType_; }
    DataType getValueType() const { return valueType_; }

    // Generic insert/put methods using QVariant
    bool insert(const QVariant &key, const QVariant &value);
    void put(const QVariant &key, const QVariant &value);
    std::optional<QVariant> get(const QVariant &key);
    bool erase(const QVariant &key);
    bool contains(const QVariant &key);

    void clear();

    int size() const;
    int bucketCount() const;
    float loadFactor() const;

    void rehash(int newBucketCount);
    void reserve(int expectedElements);

    // Visualization helpers
    const QVector<QString> &lastSteps() const;
    void clearSteps();
    void addStepToHistory(const QString &step);
    QVector<int> bucketSizes() const;
    QVector<QVector<QPair<QVariant, QVariant>>> getBucketContents() const;

    // Type conversion helpers
    static QString dataTypeToString(DataType type);
    static QString variantToDisplayString(const QVariant &var);
    // Hash function (public for visualization)
    int indexFor(const QVariant &key, int bucketCount) const;
    // Get computed hash value (for display)
    size_t getHashValue(const QVariant &key) const;

private:
    struct Node {
        QVariant key;
        QVariant value;
    };

    std::vector<std::forward_list<Node>> buckets_;
    int numElements_ = 0;
    float maxLoadFactor_ = 0.75f;
    QVector<QString> stepHistory_;  // Persistent history
    DataType keyType_ = STRING;
    DataType valueType_ = STRING;
    bool hasRehashed_ = false;  // Track if rehashing has occurred (only once)

    void addStep(const QString &text);
    bool emplaceOrAssign(const QVariant &key, const QVariant &value, bool assignIfExists);
    void maybeGrow();
    bool validateType(const QVariant &value, DataType expectedType) const;
};

