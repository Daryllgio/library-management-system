# Library Management System – Patron Borrowing & Hold Management

A C++/Qt **Library Management System** prototype focused on the **patron side** of a real library:

- Browse the full catalogue  
- Borrow and return items (with loan caps and due dates)  
- Place and cancel holds (with FIFO hold queues)  
- View active loans and holds in a dedicated account view  

Everything runs on an **in-memory data store** (no database yet), designed as a clean **vertical slice** that can later be extended to full librarian/admin workflows and persistence.

---

## ⭐ Tech Stack

- **Language:** C++17  
- **GUI Framework:** Qt Widgets (QDialog, QTableWidget, QListWidget, QPushButton, QLabel, layouts)  
- **Data Structures:** `std::vector`, `std::queue`, `std::optional`  
- **Build System:** Qt `.pro` project (`D1.pro`)  
- **Platform:** Desktop (Linux/Ubuntu VM, macOS, Windows with Qt)

---

## 📦 Features Overview

### ✅ Implemented (Patron Role)

- **Catalogue browsing**
  - Full item list with:
    - ID, Title, Creator
    - Format (Fiction, Non-Fiction, Magazine, Movie, Video Game)
    - Availability / Due date
  - Format-specific details (Dewey numbers, issues, genres, ratings) shown in a details label.

- **Borrowing**
  - Borrow available items from the catalogue table.
  - Enforces **business rules**:
    - Max **3 active loans per patron**.
    - Due date set to **14 days** from checkout.
  - Automatically updates:
    - Catalogue availability
    - “My Active Loans” list

- **Returning**
  - Select an item from **“My Active Loans”**.
  - Return it to make it **Available** again.
  - UI refreshes to keep catalogue and loans list in sync.

- **Hold management**
  - **Place holds** on checked-out items:
    - Each item maintains a **FIFO hold queue** (`std::queue<QString>`).
    - Patron sees their **position in the queue** (e.g. `#2 in line`).
  - **Cancel holds**:
    - Remove your hold from the queue.
    - Remaining patrons shift forward in order automatically.
  - “My Active Holds” list shows all holds + positions.

- **Account view**
  - **Active Loans**:
    - Item ID, title, due date.
  - **Active Holds**:
    - Item ID, title, and queue position.

### 🧑‍💼 Other Roles (Scaffolded)

- **Librarian** and **Admin** UIs are stubbed out as simple placeholder dialogs:
  - Proves that the **role-based routing** from startup is working.
  - Ready to be extended for catalog management, policy settings, and reports in later iterations.

---

## 📚 Domain Model & Seed Data

All data is stored in an **in-memory `DataStore` singleton** that seeds initial data at startup.

### 👥 Users

The system seeds **7 users**:

| Name       | Role        |
|-----------|-------------|
| Alice     | Patron      |
| Bob       | Patron      |
| Carmen    | Patron      |
| Dev       | Patron      |
| Eve       | Patron      |
| Librarian | Librarian   |
| Admin     | Admin       |

> For this prototype, only **patron** behaviour is fully implemented; librarian/admin windows are placeholders.

### 📖 Catalogue

The catalogue contains **20 items** total:

- **5 fiction books**
- **5 non-fiction books** (each with a Dewey Decimal number)
- **3 magazines** (issue + publication date)
- **3 movies** (genre + rating)
- **4 video games** (genre + rating)

Each `Item` stores:

- `id` – unique integer
- `title`
- `creator` – author or content creator
- `format` – `FictionBook`, `NonFictionBook`, `Magazine`, `Movie`, or `VideoGame`
- `status` – availability, borrower, due date, hold queue
- Optional metadata:
  - Non-fiction: `dewey`
  - Magazines: `issue`, `pubDate` (`YYYY-MM`)
  - Movies/Games: `genre`, `rating` (e.g., `PG-13`, `E10+`)

### ⚖️ Business Rules

Defined centrally in `models.hpp`:

- `Rules::MaxActiveLoans = 3`
- `Rules::LoanDays = 14`

Enforced behaviour:

- A patron may have at most **3 active loans**.
- Borrowing is only allowed if:
  - The item is **available**, and  
  - The patron is **under the loan limit**.
- Holds:
  - Can only be placed on **unavailable** items.
  - Use a **first-in-first-out queue**.
  - Only the patron who placed the hold can cancel it.

---

## 🧠 Architecture

### 🧩 High-Level Design

- **Presentation layer:** Qt dialogs & widgets (`StartupDialog`, `PatronWindow`, role placeholders).
- **Domain/data layer:** `DataStore` singleton + `User`/`Item` models.
- **Separation of concerns:**  
  - UI delegates all state changes to `DataStore`.  
  - `DataStore` enforces the rules and updates `User`/`Item` records.

### 📈 Diagram (Flow)

```mermaid
flowchart TD
    A[StartupDialog<br/>Enter Name] --> B{Lookup User in DataStore}

    B -->|Patron| C[PatronWindow<br/>Catalogue + Loans + Holds]
    B -->|Librarian| D[LibrarianWindow<br/>Placeholder]
    B -->|Admin| E[AdminWindow<br/>Placeholder]

    C -->|Borrow / Return / Hold Ops| F[(DataStore)]
    F --> G[Users (in-memory)]
    F --> H[Items (in-memory)]
```

### 🗂 Key Components

#### `main.cpp`

- Creates `QApplication`.
- Instantiates and shows `StartupDialog`.

#### `StartupDialog` (`startupdialog.hpp/cpp`)

- Simple “enter your name” dialog (not real authentication).
- On **Enter**:
  - Looks up the user in `DataStore::users()`.
  - If found:
    - Routes to:
      - `PatronWindow` for patrons
      - `LibrarianWindow` for librarian
      - `AdminWindow` for admin
  - If not found:
    - Shows an error message.
- After a role window closes:
  - Calls `DataStore::clearCurrentUserState()`.
  - Clears the input and shows the startup dialog again.

#### `PatronWindow` (`patronwindow.hpp/cpp`)

- Main GUI for the patron role:
  - `QTableWidget` ➝ catalogue
  - `QListWidget` ➝ active loans
  - `QListWidget` ➝ active holds
  - `QPushButton` ➝ Borrow, Return, Place Hold, Cancel Hold
  - `QLabel` ➝ currently selected item & details
- Interacts with `DataStore` to:
  - Borrow/return items
  - Place/cancel holds
  - Query hold positions
- Handles UI logic:
  - Enabling/disabling buttons based on selection
  - Refreshing catalogue/loans/holds after every operation

#### `DataStore` (`datastore.hpp/cpp`)

- Implemented as a **Singleton**:

  ```cpp
  DataStore& DataStore::instance() {
      static DataStore ds;
      return ds;
  }
  ```

- Responsibilities:
  - `seedUsers()` and `seedItems()` on construction.
  - Store:
    - `std::vector<User> m_users;`
    - `std::vector<Item> m_items;`
  - Provide read access:
    - `const std::vector<User>& users() const;`
    - `const std::vector<Item>& items() const;`
  - Core operations:
    - `std::optional<User> findUser(QString name) const;`
    - `Item* findItemById(int id);`
    - `std::optional<QString> borrowItem(User& patron, int itemId);`
    - `std::optional<QString> returnItem(User& patron, int itemId);`
    - `std::optional<QString> placeHold(User& patron, int itemId);`
    - `std::optional<QString> cancelHold(User& patron, int itemId);`
    - `int holdPosition(const User& patron, int itemId) const;`
  - Ensures all business rules are consistently applied from one central place.

#### `models.hpp`

- Defines domain types:

  ```cpp
  enum class UserType { Patron, Librarian, Admin };

  struct User {
      QString name;
      UserType type;
      std::vector<int> activeLoans;
      std::vector<int> holds;
  };

  enum class ItemFormat {
      FictionBook,
      NonFictionBook,
      Magazine,
      Movie,
      VideoGame
  };

  struct ItemStatus {
      bool available;
      std::optional<QString> borrower;
      std::optional<QDate> dueDate;
      std::queue<QString> holdQueue;
  };

  struct Item {
      int id;
      QString title;
      QString creator;
      ItemFormat format;
      ItemStatus status;
      QString dewey;
      QString issue;
      QString pubDate;
      QString genre;
      QString rating;
  };

  namespace Rules {
      constexpr int MaxActiveLoans = 3;
      constexpr int LoanDays       = 14;
  }
  ```

#### `rolewindows.*`

- `LibrarianWindow` and `AdminWindow`:
  - Minimal dialogs:
    - Show the user’s name and role.
    - Display a message such as “Librarian/Admin interface coming in a future iteration.”

#### `patron.*`

- A small standalone `Patron` class (id, name, status) retained for future use and design evolution.
- Not heavily used in the current Qt workflow (the primary model is `User` in `models.hpp`).

---

## 🗂 Project Structure

```text
.
├── D1.pro                 # Qt project file
├── main.cpp               # Qt application entry point
├── startupdialog.hpp/cpp  # Startup dialog (name input + role routing)
├── patronwindow.hpp/cpp   # Main patron GUI (catalogue, loans, holds)
├── rolewindows.hpp/cpp    # Librarian/Admin placeholder dialogs
├── datastore.hpp/cpp      # Singleton in-memory data store + business logic
├── models.hpp             # Domain models (User, Item, Rules)
├── patron.h/.cpp          # Standalone Patron class (legacy/future use)
├── mainwindow.h/.cpp/.ui  # Qt Creator scaffold (not central to D1)
├── hinlibs_d1.pro*        # Additional Qt project file (legacy/alt config)
└── hinlibs_d1_en_CA.ts*   # Qt translation file (auto-generated)
```

> `*` Files are part of the Qt project ecosystem but not core to the prototype logic.

---

## 🚀 Building & Running

### ✅ Prerequisites

- Qt 5/6 with Qt Widgets module  
- C++17-capable compiler (e.g. `g++`)  
- Qt Creator (recommended for easiest setup)

### 🧩 Option 1 – Qt Creator (Recommended)

1. Open **Qt Creator**.
2. Go to **File → Open File or Project…**.
3. Select `D1.pro`.
4. Configure the detected kit (use the course VM/default kit).
5. Click **Configure Project**.
6. Build & run using the green **Run** button.

The application will launch showing the **startup dialog**.

### 🧩 Option 2 – Command Line

From the project root:

```bash
qmake D1.pro        # or `qmake6` depending on your install
make
./D1                # or the generated binary name
```

> The executable name may differ depending on your kit and platform (e.g., `./D1`, `./LibraryManagementSystem`, or similar).

---

## 🧭 Usage Walkthrough

### 1️⃣ Startup

1. Launch the app.
2. In the **startup dialog**, type one of the seeded user names:
   - Patrons: `Alice`, `Bob`, `Carmen`, `Dev`, `Eve`
   - Librarian: `Librarian`
   - Admin: `Admin`
3. Click **Enter**.
4. A role-specific window opens:
   - Patrons → `PatronWindow`
   - Librarian/Admin → placeholder dialog

### 2️⃣ Patron Flow

Once in the **PatronWindow**:

#### 🔍 Browse Catalogue

- The main table lists all **20 items**.
- Columns: **ID, Title, Creator, Format, Availability**.
- When you click a row:
  - A label shows more detail (e.g., Dewey for non-fiction, issue/pubDate for magazines, genre/rating for media).

#### 📥 Borrow an Item

1. Click an item with **Availability = Available**.
2. Click **“Borrow Selected Item”**.
3. If rules are satisfied:
   - The item is checked out to you.
   - A **due date (today + 14 days)** is stored and shown.
   - The item moves into **“My Active Loans”**.
4. If you already have 3 loans:
   - An error dialog tells you why borrowing is blocked.

#### 📤 Return an Item

1. In **“My Active Loans”**, select the loan to return.
2. Click **“Return Selected Loan”**.
3. The item becomes **Available**.
4. The loans list updates immediately.

#### ⏳ Place a Hold

1. In the catalogue, choose an item that is **unavailable**.
2. Click **“Place Hold on Selected Item”**.
3. If you don’t already have a hold on it:
   - Your name is added to the item’s hold queue.
   - You see a message with your **queue position**.
   - The item appears in **“My Active Holds”**.

#### ❌ Cancel a Hold

1. In **“My Active Holds”**, select a hold.
2. Click **“Cancel Selected Hold”**.
3. You are removed from the hold queue, and remaining positions update.
4. The holds list refreshes.

---

## 🧪 Code Stats

- ~**950 lines of C++/Qt code** across **14 source/header files**  
- **20** seeded items and **7** seeded users  
- Centralized rule enforcement in `DataStore` and `Rules` namespace  

These numbers make it a strong, concrete project entry on a resume or portfolio.

---

## 🧩 Design Decisions

- **Singleton `DataStore`**  
  Provides a single source of truth for users and items; easy to swap out for a database-backed implementation later.

- **UI/Domain separation**  
  Qt windows only deal with **display and user interaction**. All logic for:
  - Loan caps  
  - Due dates  
  - Hold queues  
  lives inside `DataStore` and the models.

- **Explicit domain models**  
  `User`, `Item`, `ItemStatus`, and `Rules` make the business logic easier to reason about and test in isolation.

- **D1 scope by design**  
  No persistence or fines yet; this is intentionally an in-memory prototype that proves the interaction design and rule enforcement before scaling up.

---

## 🔮 Future Enhancements

Some natural next steps for this project:

- 💾 **Persistence layer**  
  Replace the in-memory store with an SQLite or PostgreSQL backend (or use Qt’s `QSqlDatabase`).

- 🧑‍💼 **Librarian features**
  - Add/edit/remove catalogue items.
  - Check for overdue items.
  - Override loan caps for special cases.

- 🛠 **Admin features**
  - Configure rules (loan caps, loan period, fine rates).
  - Generate summary reports (overdues, popular items, usage stats).

- 🧪 **Unit testing**
  - Add unit tests around `DataStore` operations (borrow/return/holds).
  - Validate business rules independent of the UI.

- 🎨 **UI improvements**
  - Better styling and icons.
  - Filter/search on the catalogue (by title, format, availability).
  - Accessibility improvements (keyboard shortcuts, focus hints).

---

## 📎 License

> This project was originally developed as part of a course assignment.  
