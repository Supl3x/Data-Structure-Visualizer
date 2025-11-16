# 📚 Project Explanation: Advanced Data Structure Visualizer

## 🏗️ **PART 1: PROJECT ARCHITECTURE**

### **Overall Structure**

Your project is a **Qt-based desktop application** that visualizes data structures. Think of it like a **multi-page book** where each page shows different content.

```
┌─────────────────────────────────────────┐
│         QStackedWidget (Main Window)    │
│  ┌──────────┐  ┌──────────┐  ┌────────┐│
│  │ HomePage │→ │ MenuPage │→ │ Theory ││
│  └──────────┘  └──────────┘  └────────┘│
│                      ↓                   │
│         ┌─────────────────────┐         │
│         │ Visualizations:      │         │
│         │ - TreeInsertion      │         │
│         │ - RedBlackTree       │         │
│         │ - HashMapVisualization│        │
│         │ - GraphVisualization │         │
│         └─────────────────────┘         │
└─────────────────────────────────────────┘
```

### **Key Classes & Their Roles**

#### **1. Main Entry Point (`main.cpp`)**
- **Purpose**: Starts the application, manages navigation
- **Key Component**: `QStackedWidget` - like a deck of cards, shows one page at a time
- **Flow**: Home → Menu → Theory → Visualization

#### **2. UI Pages**

**`HomePage`** (homepage.h/cpp)
- Welcome screen with animations
- "Get Started" button → goes to MenuPage

**`MenuPage`** (menupage.h/cpp)
- Shows 4 cards: BST, Red-Black Tree, Hash Table, Graph
- User clicks a card → goes to TheoryPage

**`TheoryPage`** (theorypage.h/cpp)
- Shows educational content about the selected data structure
- "Try It Yourself" button → opens Visualization

#### **3. Visualization Classes**

**`TreeInsertion`** - Binary Search Tree visualization
**`RedBlackTree`** - Red-Black Tree visualization  
**`HashMapVisualization`** - Hash Table visualization
**`GraphVisualization`** - Graph visualization

Each visualization:
- Has its own UI (buttons, input fields)
- Draws the data structure visually
- Shows step-by-step operations
- Has a "Back" button to return to theory

#### **4. Helper Classes**

**`StyleManager`** - Manages consistent styling across the app
**`WidgetManager`** - Handles safe widget deletion (prevents crashes)
**`BackButton`** - Reusable back button component
**`UIUtils`** - Utility functions for creating UI elements

---

## 🗺️ **PART 2: HASHMAP IMPLEMENTATION (Beginner-Friendly)**

### **What is a HashMap?**

A **HashMap** (also called Hash Table) is like a **smart filing cabinet**:

- **Traditional Array**: To find "John's phone", you'd search through all entries (slow!)
- **HashMap**: Converts "John" → number → goes directly to that drawer (fast!)

**Real-world analogy**: Like a library where books are organized by a code (hash), so you can find any book instantly!

### **How HashMap Works: 3 Simple Steps**

```
1. HASH: Convert key → number
   "apple" → hash function → 12345

2. MODULO: Get bucket index
   12345 % 8 buckets = 1 (go to bucket 1)

3. STORE: Put key-value in that bucket
   Bucket 1: [("apple", "red")]
```

---

## 🔍 **DETAILED HASHMAP CODE EXPLANATION**

### **Class Structure (`hashmap.h`)**

```cpp
class HashMap {
    // PUBLIC: What users can do
    - insert(key, value)    // Add new entry
    - put(key, value)       // Add or update
    - get(key)              // Find value
    - erase(key)            // Delete entry
    - contains(key)         // Check if exists
    
    // PRIVATE: Internal workings
    - buckets_              // Array of linked lists
    - numElements_          // How many items stored
    - maxLoadFactor_        // When to resize (0.75 = 75% full)
};
```

### **Key Data Structures**

#### **1. Buckets Array**
```cpp
std::vector<std::forward_list<Node>> buckets_;
```

**Explanation**:
- `vector` = Array that can grow
- `forward_list` = Linked list (for collision handling)
- `Node` = Stores one key-value pair

**Visual**:
```
Buckets:  [0]  [1]  [2]  [3]  [4]  [5]  [6]  [7]
           │    │    │    │    │    │    │    │
           ▼    ▼    ▼    ▼    ▼    ▼    ▼    ▼
          empty apple banana empty empty empty empty
                  │
                  ▼
                 red
```

#### **2. Node Structure**
```cpp
struct Node {
    QVariant key;    // Can be string, int, double, etc.
    QVariant value;  // Can be any type
};
```

**Why QVariant?** - Allows storing different data types (string, int, etc.) in one structure.

---

### **Core Functions Explained**

#### **1. Hash Function (`indexFor`)**

```cpp
int HashMap::indexFor(const QVariant &key, int bucketCount) const {
    // Step 1: Convert key to hash number
    size_t hashValue = std::hash<std::string>{}(key.toString());
    
    // Step 2: Get bucket index (0 to bucketCount-1)
    return hashValue % bucketCount;
}
```

**Example**:
```
Key: "apple"
Hash: 123456789
Buckets: 8
Index: 123456789 % 8 = 5

Result: Store "apple" in bucket 5
```

**Why Modulo?** - Ensures index is always between 0 and (bucketCount - 1)

#### **2. Insert Operation (`insert` / `put`)**

```cpp
bool HashMap::insert(const QVariant &key, const QVariant &value) {
    // Step 1: Check if we need to resize (rehash)
    maybeGrow();
    
    // Step 2: Find the bucket
    int index = indexFor(key, bucketCount());
    
    // Step 3: Check if key already exists in that bucket
    // Step 4: If not, add new node to the chain
    return emplaceOrAssign(key, value, false);
}
```

**Step-by-Step Example**:
```
Insert("apple", "red"):

1. Hash "apple" → index = 5
2. Go to bucket 5
3. Check chain: empty? → Yes
4. Add node: ("apple", "red")
5. Done!
```

**Collision Handling** (when two keys go to same bucket):
```
Insert("banana", "yellow"):
1. Hash "banana" → index = 5 (same as apple!)
2. Go to bucket 5
3. Check chain: ("apple", "red") exists
4. Add to chain: ("apple", "red") → ("banana", "yellow")
5. Now bucket 5 has a chain of 2 items
```

#### **3. Search Operation (`get`)**

```cpp
std::optional<QVariant> HashMap::get(const QVariant &key) {
    // Step 1: Calculate bucket index
    int index = indexFor(key, bucketCount());
    
    // Step 2: Search through the chain in that bucket
    for (const auto &node : buckets_[index]) {
        if (node.key == key) {
            return node.value;  // Found!
        }
    }
    return std::nullopt;  // Not found
}
```

**Example**:
```
Get("apple"):

1. Hash "apple" → index = 5
2. Go to bucket 5
3. Check first node: key = "apple"? → Yes!
4. Return value: "red"
```

#### **4. Delete Operation (`erase`)**

```cpp
bool HashMap::erase(const QVariant &key) {
    int index = indexFor(key, bucketCount());
    
    // Traverse chain and remove matching node
    auto &chain = buckets_[index];
    // ... find and remove node from linked list
}
```

#### **5. Rehashing (`rehash`)**

**Why Rehash?** - When too many items in buckets, chains get long → slow!

```cpp
void HashMap::rehash(int newBucketCount) {
    // Step 1: Create new, larger bucket array
    std::vector<std::forward_list<Node>> newBuckets(newBucketCount);
    
    // Step 2: Move all existing items to new buckets
    for (each old bucket) {
        for (each node in bucket) {
            int newIndex = indexFor(node.key, newBucketCount);
            newBuckets[newIndex].push_front(node);
        }
    }
    
    // Step 3: Replace old buckets with new ones
    buckets_.swap(newBuckets);
}
```

**Example**:
```
Before: 8 buckets, 6 items (load factor = 0.75)
After:  16 buckets, 6 items (load factor = 0.375)

Why? More buckets = shorter chains = faster!
```

#### **6. Load Factor & Auto-Resize (`maybeGrow`)**

```cpp
void HashMap::maybeGrow() {
    float currentLoad = (numElements_ + 1) / bucketCount();
    
    if (currentLoad > maxLoadFactor_) {  // Usually 0.75 (75%)
        rehash(bucketCount() * 2);  // Double the buckets
    }
}
```

**Load Factor Formula**:
```
Load Factor = Number of Items / Number of Buckets

Example:
- 6 items, 8 buckets → 6/8 = 0.75 (75% full)
- If adding 1 more → 7/8 = 0.875 (87.5%) → TOO HIGH!
- Solution: Rehash to 16 buckets → 7/16 = 0.4375 (43.75%) ✓
```

---

## 🎨 **VISUALIZATION LAYER (`hashmapvisualization.h/cpp`)**

### **Purpose**
Takes the HashMap class and makes it **visual and interactive**.

### **Key Components**

1. **UI Elements**:
   - Input fields (key, value)
   - Buttons (Insert, Search, Delete, Clear)
   - Visualization area (draws buckets)

2. **Visualization**:
   - Draws each bucket as a rectangle
   - Shows key-value pairs inside buckets
   - Highlights buckets during operations
   - Shows load factor warnings

3. **Step Trace**:
   - Records every operation step
   - Shows in right panel for learning

### **How It Connects**

```
User clicks "Insert" button
    ↓
HashMapVisualization::onInsertClicked()
    ↓
hashMap->put(key, value)  // Calls HashMap class
    ↓
HashMap does the work, records steps
    ↓
updateVisualization()  // Redraws buckets
    ↓
User sees visual update!
```

---

## 📊 **COMPLETE EXAMPLE: Inserting "apple" → "red"**

### **Step-by-Step Flow**

```
1. USER ACTION:
   User types "apple" in key field, "red" in value field
   Clicks "Insert" button

2. UI LAYER (HashMapVisualization):
   onInsertClicked() called
   Gets text from input fields
   Converts to QVariant

3. HASHMAP LAYER:
   put("apple", "red") called
   
   a) maybeGrow() checks load factor
   b) indexFor("apple", 8) calculates:
      - Hash("apple") = 123456789
      - 123456789 % 8 = 5
      - Returns index = 5
   
   c) emplaceOrAssign():
      - Goes to buckets_[5]
      - Checks chain: empty
      - Adds node: ("apple", "red")
      - Increments numElements_
      - Records steps in stepHistory_

4. VISUALIZATION UPDATE:
   updateVisualization() called
   drawBuckets() redraws all buckets
   Bucket 5 now shows "apple→red"
   showStats() updates load factor display

5. USER SEES:
   - Bucket 5 highlighted
   - "apple→red" appears in bucket
   - Load factor updated
   - Steps shown in right panel
```

---

## 🎯 **KEY CONCEPTS SUMMARY**

### **1. Hashing**
- Converts key → number → bucket index
- Should distribute evenly (avoid clustering)

### **2. Collision Handling**
- When two keys hash to same bucket
- Solution: Linked list (chain) in each bucket

### **3. Load Factor**
- Ratio: items / buckets
- High load factor = long chains = slow
- Solution: Rehash (double buckets) when > 0.75

### **4. Time Complexity**
- **Best Case**: O(1) - Direct access, no collisions
- **Average Case**: O(1) - With good hash function
- **Worst Case**: O(n) - All items in one bucket (rare!)

---

## 💡 **LEARNING TIPS**

1. **Start Simple**: Understand hash function first
2. **Visualize**: Draw buckets on paper when learning
3. **Trace Code**: Follow one insert operation line by line
4. **Experiment**: Try different keys, see where they go
5. **Watch Load Factor**: See when rehashing happens

---

## 🔗 **RELATIONSHIP BETWEEN CLASSES**

```
main.cpp
  └─> Creates QStackedWidget
      └─> HomePage
          └─> MenuPage
              └─> TheoryPage
                  └─> HashMapVisualization
                      ├─> Uses HashMap class (hashmap.h/cpp)
                      ├─> Uses StyleManager (for UI styling)
                      └─> Uses WidgetManager (for safe deletion)
```

**Data Flow**:
```
User Input → HashMapVisualization → HashMap → stepHistory_ → Visualization Update
```

---

This is a well-structured project! The separation between:
- **Data Structure Logic** (HashMap class)
- **Visualization** (HashMapVisualization class)
- **UI Navigation** (main.cpp)

Makes it easy to understand and maintain! 🎉

