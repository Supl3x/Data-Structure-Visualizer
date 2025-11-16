#include "theorypage.h"
#include <QPainter>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QFont>
#include <QFontDatabase>
#include <QGraphicsDropShadowEffect>

TheoryPage::TheoryPage(const QString &dataStructureName, QWidget *parent)
    : QWidget(parent)
    , dsName(dataStructureName)
{
    setupUI();
    loadTheoryContent();
    setMinimumSize(900, 750);
}

TheoryPage::~TheoryPage()
{
}

void TheoryPage::setupUI()
{
    // Main layout with better spacing
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(30, 25, 30, 25);
    mainLayout->setSpacing(15);

    // Header section
    QHBoxLayout *headerLayout = new QHBoxLayout();

    // Back button with improved styling
    backButton = new BackButton(BackButton::BackToMenu, this);
    backButton->setFixedSize(150, 42);

    headerLayout->addWidget(backButton, 0, Qt::AlignLeft);
    headerLayout->addStretch();

    mainLayout->addLayout(headerLayout);

    // Title with better typography
    titleLabel = new QLabel(dsName, this);
    QFont titleFont("Segoe UI", 32, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #1a202c;
            background: transparent;
            padding: 10px 0px;
        }
    )");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // Content area - Scrollable with hidden scrollbar
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);  // Hide scrollbar but allow scrolling
    scrollArea->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    
    contentWidget = new QWidget();
    contentWidget->setStyleSheet("background: transparent;");
    
    // Create a vertical layout for content
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
    contentLayout->setSpacing(15);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setAlignment(Qt::AlignTop);
    
    scrollArea->setWidget(contentWidget);
    mainLayout->addWidget(scrollArea, 1); // Give it stretch

    // Try it yourself button with modern styling
    tryButton = new QPushButton("Try It Yourself →", this);
    tryButton->setFixedSize(240, 50);
    tryButton->setCursor(Qt::PointingHandCursor);

    QFont tryFont("Segoe UI", 14, QFont::Bold);
    tryButton->setFont(tryFont);
    tryButton->setStyleSheet(R"(
        QPushButton {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #667eea, stop:0.5 #764ba2, stop:1 #f093fb);
            color: white;
            border: none;
            border-radius: 25px;
            padding: 12px 24px;
        }
        QPushButton:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #5a6fd8, stop:0.5 #6a4190, stop:1 #de81e9);
            transform: translateY(-2px);
        }
        QPushButton:pressed {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #4e5bc6, stop:0.5 #5e377e, stop:1 #cc6fd7);
        }
    )");

    mainLayout->addWidget(tryButton, 0, Qt::AlignCenter);
    mainLayout->addSpacing(10);

    setLayout(mainLayout);

    // Connect signals
    connect(backButton, &BackButton::backRequested, this, &TheoryPage::backToMenu);
    connect(tryButton, &QPushButton::clicked, this, &TheoryPage::tryItYourself);
}

void TheoryPage::loadTheoryContent()
{
    QVBoxLayout *contentLayout = qobject_cast<QVBoxLayout*>(contentWidget->layout());
    if (!contentLayout) return;

    // Clear existing content
    QLayoutItem *item;
    while ((item = contentLayout->takeAt(0)) != nullptr) {
        delete item->widget();
        delete item;
    }

    // Add theory cards in vertical layout
    contentLayout->addWidget(createInfoCard("📖 Definition", getDefinition(), "📖"));
    contentLayout->addWidget(createInfoCard("⏱️ Time Complexity", getTimeComplexity(), "⏱️"));
    contentLayout->addWidget(createInfoCard("💡 Applications", getApplications(), "💡"));
    contentLayout->addWidget(createInfoCard("✅ Advantages", getAdvantages(), "✅"));
    contentLayout->addWidget(createInfoCard("⚠️ Disadvantages", getDisadvantages(), "⚠️"));
    
    // Add special sections for Hash Table
    if (dsName == "Hash Table") {
        contentLayout->addWidget(createInfoCard("🔑 Why Hashing?", getWhyHashing(), "🔑"));
        contentLayout->addWidget(createInfoCard("🔄 Rehashing", getRehashing(), "🔄"));
    }
    
    // Add stretch at the end
    contentLayout->addStretch();
}

QWidget* TheoryPage::createInfoCard(const QString &title, const QString &content, const QString &icon)
{
    QFrame *card = new QFrame();
    card->setFrameStyle(QFrame::NoFrame);
    
    // Modern glassmorphism-inspired design
    card->setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 rgba(255, 255, 255, 0.9),
                stop:0.5 rgba(248, 250, 255, 0.95),
                stop:1 rgba(240, 245, 255, 0.9));
            border: 1px solid rgba(255, 255, 255, 0.3);
            border-radius: 16px;
            margin: 5px;
        }
    )");

    // Subtle shadow effect
    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect();
    shadowEffect->setBlurRadius(20);
    shadowEffect->setColor(QColor(0, 0, 0, 15));
    shadowEffect->setOffset(0, 8);
    card->setGraphicsEffect(shadowEffect);

    QVBoxLayout *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(20, 18, 20, 18);
    cardLayout->setSpacing(12);

    // Title with modern typography
    QLabel *titleLabel = new QLabel(title);
    QFont titleFont("Segoe UI", 16, QFont::Bold);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet(R"(
        QLabel {
            color: #2d3748;
            background: transparent;
            padding: 0px;
            margin-bottom: 8px;
        }
    )");
    cardLayout->addWidget(titleLabel);

    // Content with better readability
    QLabel *contentLabel = new QLabel(content);
    QFont contentFont("Segoe UI", 12);
    contentLabel->setFont(contentFont);
    contentLabel->setStyleSheet(R"(
        QLabel {
            color: #4a5568;
            background: transparent;
            line-height: 1.6;
            padding: 0px;
        }
    )");
    contentLabel->setWordWrap(true);
    contentLabel->setTextFormat(Qt::RichText);
    contentLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    cardLayout->addWidget(contentLabel);

    // Add some stretch to push content to top
    cardLayout->addStretch();

    card->setLayout(cardLayout);
    return card;
}

QString TheoryPage::getDefinition()
{
    if (dsName == "Binary Search Tree") {
        return "A <b>Binary Search Tree</b> (BST) is a hierarchical data structure in which each node has at most two children, "
               "referred to as the <b>left child</b> and <b>right child</b>. It starts with a root node and "
               "branches out into a tree-like structure.<br><br>"
               
               "<b>Key Property:</b> For any node, all values in the left subtree are less than the node's value, "
               "and all values in the right subtree are greater than the node's value. This property enables efficient searching.<br><br>"
               
               "<b>Structure:</b> Each node contains a value and pointers to its left and right children. "
               "Binary Search Trees are fundamental in computer science and form the basis for more complex data structures "
               "like AVL Trees, Red-Black Trees, and Heaps.<br><br>"
               
               "<b>Operations:</b> The BST supports insertion, deletion, and search operations. The efficiency of these operations "
               "depends on the tree's balance. A balanced BST provides O(log n) time complexity, while an unbalanced tree can degrade to O(n).";
    } else if (dsName == "Red-Black Tree") {
        return "A <b>Red-Black Tree</b> is a self-balancing Binary Search Tree where each node has an extra bit "
               "for denoting the color (red or black). These color bits ensure that the tree remains approximately "
               "balanced during insertions and deletions, guaranteeing O(log n) time complexity for operations.<br><br>"
               
               "<b>Properties:</b> A Red-Black Tree must satisfy five properties:<br>"
               "1. Every node is either red or black<br>"
               "2. The root is always black<br>"
               "3. All leaves (NIL nodes) are black<br>"
               "4. If a node is red, both its children are black<br>"
               "5. Every path from a node to its descendant leaves contains the same number of black nodes<br><br>"
               
               "<b>Self-Balancing:</b> When insertions or deletions violate these properties, the tree performs rotations "
               "and color changes to restore balance. This ensures the tree height remains logarithmic, maintaining efficient operations.";
    } else if (dsName == "Graph") {
        return "A <b>Graph</b> is a non-linear data structure consisting of vertices (nodes) and edges that connect "
               "pairs of vertices. Graphs can be directed or undirected, weighted or unweighted, and are used to "
               "represent networks, relationships, and connections between entities.<br><br>"
               
               "<b>Types of Graphs:</b><br>"
               "• <b>Directed Graph:</b> Edges have direction (A → B is different from B → A)<br>"
               "• <b>Undirected Graph:</b> Edges have no direction (A-B is the same as B-A)<br>"
               "• <b>Weighted Graph:</b> Edges have associated weights or costs<br>"
               "• <b>Unweighted Graph:</b> All edges are treated equally<br><br>"
               
               "<b>Representation:</b> Graphs can be represented using adjacency lists or adjacency matrices. "
               "Adjacency lists are space-efficient for sparse graphs, while adjacency matrices provide O(1) edge lookup "
               "but require O(V²) space where V is the number of vertices.";
    } else if (dsName == "Hash Table") {
        return "A <b>Hash Table</b> (Hash Map) is a data structure that implements an associative array, mapping "
               "keys to values. It uses a hash function to compute an index into an array of buckets, from which "
               "the desired value can be found, providing average-case O(1) time complexity for search operations.<br><br>"
               
               "<b>Core Components:</b><br>"
               "• <b>Hash Function:</b> Converts keys into bucket indices<br>"
               "• <b>Buckets:</b> Array of containers that store key-value pairs<br>"
               "• <b>Collision Handling:</b> Strategy to deal with multiple keys mapping to the same bucket<br><br>"
               
               "<b>Collision Resolution:</b> Common strategies include chaining (linked lists in buckets) and open addressing "
               "(probing for empty slots). This implementation uses open chaining, where collisions are stored as linked lists within buckets.";
    }
    return "Information not available.";
}

QString TheoryPage::getTimeComplexity()
{
    if (dsName == "Binary Search Tree") {
        return "<b>Search:</b> O(n) in worst case (unbalanced), O(log n) for balanced trees<br>"
               "<b>Insertion:</b> O(n) in worst case, O(log n) for balanced trees<br>"
               "<b>Deletion:</b> O(n) in worst case, O(log n) for balanced trees<br>"
               "<b>Traversal:</b> O(n) for in-order, pre-order, post-order traversals<br>"
               "<b>Space Complexity:</b> O(n) for storing n nodes<br><br>"
               
               "<b>Best Case:</b> When the tree is perfectly balanced (height = log n), all operations are O(log n)<br>"
               "<b>Worst Case:</b> When the tree degenerates into a linked list (height = n), operations become O(n)<br>"
               "<b>Average Case:</b> O(log n) for random insertions";
    } else if (dsName == "Red-Black Tree") {
        return "<b>Search:</b> O(log n) - guaranteed due to self-balancing<br>"
               "<b>Insertion:</b> O(log n) - includes rebalancing operations<br>"
               "<b>Deletion:</b> O(log n) - includes rebalancing operations<br>"
               "<b>Rotation Operations:</b> O(1) - constant time rotations<br>"
               "<b>Space Complexity:</b> O(n) for storing n nodes<br><br>"
               
               "<b>Guaranteed Performance:</b> Unlike regular BSTs, Red-Black Trees guarantee O(log n) performance "
               "for all operations regardless of insertion order. The tree height is always at most 2*log(n+1).";
    } else if (dsName == "Graph") {
        return "<b>BFS (Breadth-First Search):</b> O(V + E) where V=vertices, E=edges<br>"
               "<b>DFS (Depth-First Search):</b> O(V + E) - visits each vertex and edge once<br>"
               "<b>Insertion:</b> O(1) for adding vertex/edge<br>"
               "<b>Deletion:</b> O(V + E) in worst case - may need to traverse all edges<br>"
               "<b>Shortest Path (Dijkstra):</b> O((V + E) log V) with priority queue<br>"
               "<b>Space Complexity:</b> O(V + E) for adjacency list representation<br><br>"
               
               "<b>Representation Impact:</b> Adjacency list is space-efficient O(V + E), while adjacency matrix requires O(V²) space "
               "but provides O(1) edge lookup.";
    } else if (dsName == "Hash Table") {
        return "<b>Search:</b> O(1) average case, O(n) worst case (all keys in one bucket)<br>"
               "<b>Insertion:</b> O(1) average case, O(n) worst case<br>"
               "<b>Deletion:</b> O(1) average case, O(n) worst case<br>"
               "<b>Rehashing:</b> O(n) - occurs when load factor exceeds threshold<br>"
               "<b>Space Complexity:</b> O(n) for storing n key-value pairs<br><br>"
               
               "<b>Average Case:</b> With a good hash function and proper load factor management, operations are O(1)<br>"
               "<b>Worst Case:</b> Poor hash function or high load factor can cause all keys to hash to the same bucket, "
               "degrading to O(n) performance";
    }
    return "Information not available.";
}

QString TheoryPage::getApplications()
{
    if (dsName == "Binary Search Tree") {
        return "• <b>Expression Trees:</b> Used in compilers for parsing mathematical and logical expressions<br>"
               "• <b>Search Operations:</b> Efficient searching and sorting in databases<br>"
               "• <b>Huffman Coding Trees:</b> Data compression algorithms for file encoding<br>"
               "• <b>File System:</b> Directory structure representation in operating systems<br>"
               "• <b>Database Indexing:</b> B-trees (extension of BST) for efficient data retrieval<br>"
               "• <b>Priority Queues:</b> Heap data structure (specialized BST) for scheduling<br>"
               "• <b>Decision Trees:</b> Machine learning algorithms for classification<br>"
               "• <b>Game Trees:</b> Minimax algorithm in game AI (chess, tic-tac-toe)";
    } else if (dsName == "Red-Black Tree") {
        return "• <b>Associative Arrays:</b> Implementation of std::map and std::set in C++ STL<br>"
               "• <b>Java TreeMap:</b> Sorted map implementation in Java Collections Framework<br>"
               "• <b>Linux Kernel:</b> Completely Fair Scheduler (CFS) for process scheduling<br>"
               "• <b>Memory Management:</b> Virtual memory management in operating systems<br>"
               "• <b>Database Systems:</b> Index structures in MySQL and PostgreSQL<br>"
               "• <b>Interval Trees:</b> Efficient range queries in computational geometry<br>"
               "• <b>Event Scheduling:</b> Calendar and scheduling applications";
    } else if (dsName == "Graph") {
        return "• <b>Social Networks:</b> Representing connections between users (Facebook, LinkedIn)<br>"
               "• <b>Maps & Navigation:</b> GPS and route finding using Dijkstra's and A* algorithms<br>"
               "• <b>Computer Networks:</b> Network topology, routing protocols (OSPF, BGP)<br>"
               "• <b>Web Crawlers:</b> Link structure of websites for search engines<br>"
               "• <b>Recommendation Systems:</b> Product and content recommendations (Amazon, Netflix)<br>"
               "• <b>Dependency Resolution:</b> Package managers (npm, pip) resolving dependencies<br>"
               "• <b>Compiler Design:</b> Control flow graphs and data flow analysis<br>"
               "• <b>Game Development:</b> Pathfinding in video games (NPC movement)";
    } else if (dsName == "Hash Table") {
        return "• <b>Databases:</b> Fast data retrieval and indexing in database systems<br>"
               "• <b>Caching:</b> Quick access to frequently used data (Redis, Memcached)<br>"
               "• <b>Symbol Tables:</b> Compiler design and interpreters for variable lookup<br>"
               "• <b>Password Verification:</b> Storing hashed passwords securely (bcrypt, SHA-256)<br>"
               "• <b>Dictionaries:</b> Implementing key-value storage systems (Python dict, JavaScript Map)<br>"
               "• <b>Browser Caching:</b> URL to webpage mapping in web browsers<br>"
               "• <b>Distributed Systems:</b> Consistent hashing for load balancing";
    }
    return "Information not available.";
}

QString TheoryPage::getAdvantages()
{
    if (dsName == "Binary Search Tree") {
        return "• <b>Simple Structure:</b> Easy to understand and implement hierarchical data<br>"
               "• <b>Efficient Search:</b> O(log n) search time in balanced trees<br>"
               "• <b>Ordered Traversal:</b> In-order traversal provides sorted sequence<br>"
               "• <b>Flexible:</b> Supports dynamic insertion and deletion<br>"
               "• <b>Foundation:</b> Forms the basis for more advanced tree structures (AVL, Red-Black, B-trees)<br>"
               "• <b>Memory Efficient:</b> Only stores necessary pointers (left, right, parent)<br>"
               "• <b>Range Queries:</b> Efficiently finds all values within a range";
    } else if (dsName == "Red-Black Tree") {
        return "• <b>Guaranteed Performance:</b> O(log n) time complexity for all operations, regardless of insertion order<br>"
               "• <b>Self-Balancing:</b> Automatically maintains balance during insertions and deletions<br>"
               "• <b>Efficient Rotations:</b> Requires at most 2 rotations for insertion (better than AVL trees)<br>"
               "• <b>Production Ready:</b> Widely used in production systems (C++ STL, Java Collections)<br>"
               "• <b>Predictable:</b> Height is always at most 2*log(n+1), ensuring consistent performance<br>"
               "• <b>Sorted Order:</b> Maintains sorted order while providing fast access";
    } else if (dsName == "Graph") {
        return "• <b>Versatile:</b> Can represent complex relationships and networks from various domains<br>"
               "• <b>Flexible:</b> Supports directed, undirected, weighted, and unweighted graphs<br>"
               "• <b>Rich Algorithms:</b> Many powerful algorithms available (BFS, DFS, Dijkstra, MST, etc.)<br>"
               "• <b>Real-World Modeling:</b> Naturally models real-world scenarios (social networks, maps, networks)<br>"
               "• <b>Cyclic Support:</b> Can represent both cyclic and acyclic dependencies<br>"
               "• <b>Scalable:</b> Efficient representation allows handling large graphs";
    } else if (dsName == "Hash Table") {
        return "• <b>Ultra-Fast Lookup:</b> O(1) average-case lookup, insertion, and deletion<br>"
               "• <b>Efficient Memory:</b> Good memory usage with proper load factor management<br>"
               "• <b>Simple API:</b> Easy to implement and use (put, get, remove operations)<br>"
               "• <b>Key-Value Storage:</b> Ideal for associative arrays and dictionaries<br>"
               "• <b>Universal Support:</b> Widely supported in all major programming languages<br>"
               "• <b>Scalable:</b> Performance remains constant as data grows (in average case)";
    }
    return "Information not available.";
}

QString TheoryPage::getDisadvantages()
{
    if (dsName == "Binary Search Tree") {
        return "• <b>Unbalanced Trees:</b> Can degenerate into linked lists, leading to O(n) operations<br>"
               "• <b>No Guarantees:</b> Performance depends on insertion order; worst case is O(n)<br>"
               "• <b>Memory Overhead:</b> Requires extra memory for storing pointers (left, right, parent)<br>"
               "• <b>Complex Deletion:</b> Deleting nodes with two children requires finding successor/predecessor<br>"
               "• <b>No Random Access:</b> Cannot access elements by index like arrays<br>"
               "• <b>Sorting Required:</b> Not efficient for unsorted or random data insertion";
    } else if (dsName == "Red-Black Tree") {
        return "• <b>Complexity:</b> More complex implementation than basic binary trees<br>"
               "• <b>Memory Overhead:</b> Requires extra storage for color information (1 bit per node)<br>"
               "• <b>Rebalancing Cost:</b> Rotations and color changes add overhead to insertions/deletions<br>"
               "• <b>Learning Curve:</b> Harder to understand and debug than simple BSTs<br>"
               "• <b>Search Performance:</b> Slightly slower than AVL trees for search-heavy workloads<br>"
               "• <b>No Random Access:</b> Cannot access elements by index";
    } else if (dsName == "Graph") {
        return "• <b>Memory Intensive:</b> Can consume significant memory for dense graphs (O(V²) for adjacency matrix)<br>"
               "• <b>Algorithm Complexity:</b> Many graph algorithms are complex to implement correctly<br>"
               "• <b>Variable Performance:</b> Time complexity depends on graph structure (sparse vs dense)<br>"
               "• <b>Cycle Detection:</b> Detecting cycles can be computationally expensive<br>"
               "• <b>Disconnected Components:</b> Requires careful handling of disconnected graph components<br>"
               "• <b>No Standard Structure:</b> Different representations needed for different use cases";
    } else if (dsName == "Hash Table") {
        return "• <b>Worst-Case Performance:</b> O(n) time complexity when all keys hash to same bucket<br>"
               "• <b>No Ordering:</b> Elements are not stored in any particular order<br>"
               "• <b>Hash Function Dependency:</b> Requires good hash function to avoid clustering and collisions<br>"
               "• <b>Rehashing Cost:</b> Resizing/rehashing can be expensive (O(n) operation)<br>"
               "• <b>Security Risk:</b> Vulnerable to hash collision attacks (DoS attacks)<br>"
               "• <b>Memory Overhead:</b> May waste memory if load factor is too low";
    }
    return "Information not available.";
}

QString TheoryPage::getWhyHashing()
{
    if (dsName == "Hash Table") {
        return "<b>The Problem:</b><br>"
               "Traditional data structures like arrays require O(n) time to search for an element. "
               "Even sorted arrays require O(log n) time for binary search. For large datasets, this becomes inefficient.<br><br>"
               
               "<b>Why Hashing?</b><br>"
               "• <b>Speed:</b> Hash tables provide O(1) average-case lookup time, making them ideal for fast data retrieval<br>"
               "• <b>Efficiency:</b> Direct access to data using computed indices eliminates the need for sequential or binary search<br>"
               "• <b>Scalability:</b> Performance remains constant regardless of data size (in average case)<br>"
               "• <b>Key-Value Mapping:</b> Perfect for associative arrays where keys map to values<br>"
               "• <b>Real-World Applications:</b> Used in databases, caches, symbol tables, and dictionaries<br><br>"
               
               "<b>How Hashing Works:</b><br>"
               "1. A hash function converts a key into an integer (hash value)<br>"
               "2. The hash value is mapped to a bucket index using modulo operation<br>"
               "3. The key-value pair is stored in the corresponding bucket<br>"
               "4. When searching, the same hash function quickly locates the bucket<br><br>"
               
               "<b>Example:</b><br>"
               "Key: \"apple\" → Hash function → Hash value: 123456789<br>"
               "Bucket index = 123456789 % 8 = 5<br>"
               "Store (\"apple\", \"red\") in bucket 5<br>"
               "Search \"apple\" → Compute hash → Go directly to bucket 5 → Found!<br><br>"
               
               "<b>Trade-offs:</b> While hashing provides excellent average-case performance, it requires: "
               "good hash functions, collision handling strategies, and periodic rehashing to maintain efficiency.";
    }
    return "";
}

QString TheoryPage::getRehashing()
{
    if (dsName == "Hash Table") {
        return "<b>What is Rehashing?</b><br>"
               "Rehashing is the process of increasing the number of buckets in a hash table when the load factor "
               "exceeds a certain threshold (typically 0.75). This helps maintain O(1) average-case performance.<br><br>"
               
               "<b>Why Rehash?</b><br>"
               "• <b>Performance:</b> As the load factor increases, collisions become more frequent, degrading performance<br>"
               "• <b>Load Factor:</b> Load factor = (number of elements) / (number of buckets)<br>"
               "• <b>Threshold:</b> When load factor > 0.75, the table is considered too full<br>"
               "• <b>Solution:</b> Double the bucket count and redistribute all elements<br><br>"
               
               "<b>How Rehashing Works:</b><br>"
               "1. Calculate new bucket count (typically 2x the current count)<br>"
               "2. Create a new array of buckets with the new size<br>"
               "3. For each existing key-value pair:<br>"
               "   • Recalculate hash using the new bucket count<br>"
               "   • Insert into the appropriate new bucket<br>"
               "4. Replace old buckets with new buckets<br><br>"
               
               "<b>Example:</b><br>"
               "Initial: 8 buckets, 6 elements → Load factor = 0.75<br>"
               "After insert: Load factor > 0.75 → Trigger rehash<br>"
               "Rehash to: 16 buckets → Load factor = 0.375 (much better!)<br><br>"
               
               "<b>Time Complexity:</b> O(n) where n is the number of elements<br>"
               "<b>Space Complexity:</b> O(n) for temporary storage during rehashing";
    }
    return "";
}

void TheoryPage::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Beautiful modern gradient background
    QLinearGradient gradient(0, 0, width(), height());
    gradient.setColorAt(0.0, QColor(255, 255, 255));      // Pure white at top
    gradient.setColorAt(0.3, QColor(248, 250, 252));      // Very light blue-gray
    gradient.setColorAt(0.7, QColor(241, 245, 249));      // Light blue-gray
    gradient.setColorAt(1.0, QColor(237, 242, 247));      // Slightly darker blue-gray

    painter.fillRect(rect(), gradient);

    // Add subtle geometric patterns for visual interest
    painter.setOpacity(0.02);
    QPen patternPen(QColor(99, 102, 241), 1);
    painter.setPen(patternPen);
    
    // Draw subtle diagonal grid pattern
    int spacing = 60;
    for (int x = 0; x < width(); x += spacing) {
        painter.drawLine(x, 0, x, height());
    }
    for (int y = 0; y < height(); y += spacing) {
        painter.drawLine(0, y, width(), y);
    }
    
    painter.setOpacity(1.0);

    // Add floating accent elements
    painter.setOpacity(0.08);
    QBrush accentBrush(QColor(139, 92, 246));
    painter.setBrush(accentBrush);
    painter.setPen(Qt::NoPen);
    
    // Top-left accent circle
    painter.drawEllipse(width() * 0.1, height() * 0.1, 120, 120);
    
    // Bottom-right accent circle
    painter.drawEllipse(width() * 0.85, height() * 0.8, 100, 100);
    
    // Middle accent element
    painter.drawEllipse(width() * 0.7, height() * 0.2, 80, 80);
    
    painter.setOpacity(1.0);
}
