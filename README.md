# Library Management System – Patron Borrowing & Hold Management

This project is a desktop **Library Management System** written in **C++ using Qt Widgets**.  
It models what happens when a real person walks into a public library and:

- looks through the library’s **physical collection** (books, magazines, movies, and video games),
- checks out items to take home,
- returns those items later, and
- joins a **waiting line (hold queue)** for popular items that are already checked out.

The current prototype focuses on the **patron side** of the system. It does **not** talk to a real database yet; instead, all users and items live in an **in‑memory data store** that is seeded with a small demo library every time the program starts.

---

## Table of Contents

1. Domain Overview  
2. Key Concepts  
3. Features  
4. Seed Data  
5. Architecture  
6. Project Structure  
7. Building and Running  
8. Example User Flow  
9. Design Decisions  
10. Possible Extensions

---

## Domain Overview

The system represents a **small public library** that lends out physical items:

- **Printed books**
  - 5 *fiction* books
  - 5 *non‑fiction* books with Dewey Decimal numbers
- **Magazines**
  - 3 issues with an issue number and publication date
- **Movies**
  - 3 titles (e.g., DVDs/Blu‑rays) with a genre and age rating
- **Video games**
  - 4 games with a genre and age rating

Each item is a single physical copy. For example, the movie “Inception” in the catalogue is *one* DVD that can only be in one of these states at a time:

1. **Available on the shelf** – nobody has it checked out.
2. **On loan to a patron** – someone has borrowed it and must return it by a due date.
3. **Unavailable with a hold queue** – someone has it on loan and other patrons are waiting in line for it.

The program also tracks **patrons** and staff roles:

- **Patrons** – can borrow items, return items, and join waiting lists (holds).
- **Librarian** – placeholder window for future catalogue/administration features.
- **Admin** – placeholder window for future policy and reporting features.

---

## Key Concepts

To make the behaviour clear even to someone who has not seen the code, here are the main words used in this README and in the program.

### Loan

A **loan** is a record that a **specific physical item** (for example, one copy of a book or one DVD) has been checked out by a single patron and must be returned by a **due date**.

- In the program, a loan exists when:
  - the item’s status is “not available”, and
  - the patron’s list of active loans contains that item’s ID.

The system enforces a **maximum of 3 active loans per patron**.

### Hold

A **hold** is a request from a patron asking the library to **reserve a specific item** for them when it becomes available.

- Holds only make sense if the item is **not currently available** (someone else has it).
- Each item has a **hold queue** – a first‑in‑first‑out list of **patron names** waiting for that item.
- A patron can put themselves on the queue once for any given item and can later cancel their hold to leave the queue.

### Patron Account

A **patron account** is the combination of:

- all items that the patron **currently has on loan**, and
- all items on which the patron **currently has a hold**.

The “My Active Loans” and “My Active Holds” sections in the UI together give a complete summary of the patron’s relationship with the library at that moment.

---

## Features

This section explains **what the system actually does**, in terms that match real library behaviour.

### 1. Browsing the Library Catalogue

The **catalogue** is the full list of all physical items the library owns:

- fiction books,  
- non‑fiction books,  
- magazines,  
- movies, and  
- video games.

In the GUI, the catalogue is displayed as a **table** where each row represents **one physical item**. For every item the patron can see:

- **Item ID** – an internal numeric identifier
- **Title** – book title, magazine title, movie or game title
- **Creator** – book author, magazine publisher, movie/game creator
- **Format** – one of:
  - Fiction Book
  - Non‑Fiction Book
  - Magazine
  - Movie
  - Video Game
- **Current status**, such as:
  - `Available`
  - `On loan until 2025‑03‑15`
  - `Not available (holds queued)`

When a patron clicks on a row, a details section shows **extra information that depends on the type of item**:

- For **non‑fiction books** – Dewey Decimal number  
- For **magazines** – issue number and publication date  
- For **movies** and **video games** – genre and age rating (for example, “Action, PG‑13”)

This behaves like a simplified “search results” page in a real online library catalogue.

---

### 2. Borrowing Items (Creating Loans)

Borrowing allows a patron to **take a physical item home** for a limited time.

Steps in the prototype:

1. The patron selects an item in the catalogue whose status is **Available**.  
2. They click the **“Borrow Selected Item”** button.  
3. The system checks two rules:
   - The patron currently has **fewer than 3 items on loan**.
   - The selected item is not already on loan to someone else.
4. If both conditions are satisfied:
   - The item’s status changes from `Available` to `On loan`.
   - A **due date** is set (exactly **14 days after the current date**).
   - The item’s ID is added to the patron’s **Active Loans** list, which appears in the UI as “My Active Loans”.

If the patron already has **3 items checked out**, the system prevents the new loan and shows a message explaining that the **maximum of three active loans** has been reached.

This models a typical rule in a real public library where each patron can only have a small number of physical items out at the same time.

---

### 3. Returning Items (Ending Loans)

Returning an item means that the patron **brings the physical item back** to the library and the system records that they no longer have it.

In this prototype:

1. The patron looks at the **“My Active Loans”** list.  
   This list includes, for each loan:
   - the item ID,
   - the title,
   - and the due date.
2. They select a loan in that list.
3. They click **“Return Selected Loan”**.  
4. The system:
   - removes the item from the patron’s list of active loans, and
   - changes the item’s status back to `Available` in the catalogue.

After this, the item appears in the catalogue as if it were back on the shelf and ready to be borrowed by any patron.

(A future extension could connect returns directly to the hold queue so that the next person waiting for the item is notified or has it automatically assigned.)

---

### 4. Placing Holds (Joining Waiting Lines)

When an item is **not available** because someone else has borrowed it, a patron can **place a hold** to get in line for it.

This is implemented exactly as a waiting line:

- Each item has a **hold queue** implemented with `std::queue<QString>`.
- The queue stores **patron names** in the order in which they asked for the item.
- The first name in the queue is the person who should get the item next.

To place a hold in the UI:

1. The patron chooses an item in the catalogue whose status indicates that it is **not available**.  
2. They click **“Place Hold on Selected Item”**.  
3. The system:
   - checks that this patron is **not already in the hold queue** for that item, and
   - if they are not, appends their name to the end of the queue.

The system then calculates the patron’s **current position** in the queue (1 for the first person, 2 for the second, etc.) and shows that position both in a message and in the **“My Active Holds”** list.

This behaviour matches real libraries where you can join the waiting list for a popular book or movie and see your place in line.

---

### 5. Cancelling Holds (Leaving Waiting Lines)

If a patron no longer wants to wait for an item, they can **cancel their hold**, which means their name is removed from that item’s waiting line.

Steps in the prototype:

1. The patron opens the **“My Active Holds”** list.  
   For each hold, this list shows:
   - the item ID,
   - the title, and
   - the patron’s current position in that item’s hold queue.
2. They select the hold they want to cancel.
3. They click **“Cancel Selected Hold”**.  
4. The system:
   - removes the patron’s name from the hold queue for that item, and
   - recomputes queue positions for everyone remaining in line (so someone who was #3 moves to #2, etc.).

After the cancellation, the item disappears from the patron’s holds list and they will no longer be considered when the item is returned.

---

### 6. Viewing the Patron Account

The prototype provides a simple **account view** for each patron that brings everything together:

- **Active Loans section**
  - Every physical item the patron currently has checked out.
  - For each loan:
    - item ID,
    - title,
    - format (book, magazine, movie, video game),
    - due date.
- **Active Holds section**
  - Every item for which the patron is currently waiting.
  - For each hold:
    - item ID,
    - title,
    - the patron’s position in the hold queue (for example, “#1 of 3” or “#2 of 5”).

This gives the same kind of overview that online library portals usually provide under “My Account”.

---

## Seed Data

All data is stored in a single **`DataStore`** object that lives in memory for the entire run of the program.

### Users

The system starts with 7 predefined users:

| Name       | Role      | Description                                     |
|-----------|-----------|-------------------------------------------------|
| Alice     | Patron    | Example library patron                          |
| Bob       | Patron    | Example library patron                          |
| Carmen    | Patron    | Example library patron                          |
| Dev       | Patron    | Example library patron                          |
| Eve       | Patron    | Example library patron                          |
| Librarian | Librarian | Staff account for future catalogue operations   |
| Admin     | Admin     | Staff account for future configuration/reporting|

For the D1 prototype, only the **patron** role has full behaviour; librarian and admin accounts open simple placeholder windows.

### Items

The library collection is seeded with 20 items. They are grouped as follows:

- 5 fiction books – ordinary novels or stories
- 5 non‑fiction books – each with a Dewey Decimal number stored as a string
- 3 magazines – each with an issue identifier and a year–month publication date
- 3 movies – with genre and age rating
- 4 video games – with genre and age rating

Each seeded item includes:

- a unique **integer ID**,
- a **title**,
- a **creator** (author, publisher, or studio),
- a **format** enum,
- an **ItemStatus** record indicating whether it is available,
- and optional metadata described in the earlier sections.

---

## Architecture

At a high level, the application is split into:

- **UI layer (Qt dialogs/windows)** – handles what the user sees and how they interact.
- **Domain / data layer (`DataStore` and models)** – stores users and items, enforces borrowing and hold rules.

### High‑Level Flow

```mermaid
flowchart TD
    A[Startup dialog] --> B{Look up user in DataStore}

    B -->|Patron| C[Patron window]
    B -->|Librarian| D[Librarian window (placeholder)]
    B -->|Admin| E[Admin window (placeholder)]

    C -->|Borrow / Return / Hold actions| F[(DataStore)]
    F --> G[Users in memory]
    F --> H[Items in memory]
```

If GitHub has trouble rendering the Mermaid diagram, the same idea in words is:

1. The program starts at a **Startup dialog** where the user types a name.  
2. The name is looked up in the **DataStore**.  
3. Depending on the user’s role, a **Patron window**, **Librarian window**, or **Admin window** is opened.  
4. The **Patron window** talks back to the **DataStore** to perform all borrow, return, and hold operations.

### Main Classes and Files

**`main.cpp`**

- Entry point of the program.
- Creates the `QApplication` object (required for all Qt GUI apps).
- Creates and displays the `StartupDialog`.

---

**`StartupDialog` (`startupdialog.hpp` / `startupdialog.cpp`)**

- A small Qt dialog that acts as the initial landing screen.
- Contains:
  - a text field where the user enters their name, and
  - a button to continue.
- On submit:
  - asks the `DataStore` to find a user with that name,
  - if found:
    - opens a `PatronWindow` if the user is a patron,
    - opens a `LibrarianWindow` or `AdminWindow` for staff roles,
  - if not found:
    - shows an error message and keeps the dialog open.
- After a role window closes, the dialog:
  - clears any transient state that was tracked for the current user, and
  - returns to the name entry screen.

---

**`PatronWindow` (`patronwindow.hpp` / `patronwindow.cpp`)**

- The main working screen for a patron.
- Layout includes:
  - a **catalogue table** that lists all items in the library,
  - a **“My Active Loans”** list,
  - a **“My Active Holds”** list,
  - action buttons:
    - “Borrow Selected Item”,
    - “Return Selected Loan”,
    - “Place Hold on Selected Item”,
    - “Cancel Selected Hold”,
  - a details area for the currently selected item.
- When the user clicks any of the buttons, `PatronWindow`:
  - figures out which item or loan is selected,
  - calls the corresponding method on `DataStore`,
  - refreshes the catalogue, loan list, and hold list to show the new state.

This window does **not** contain the business rules itself. It delegates the rules to `DataStore`.

---

**`DataStore` (`datastore.hpp` / `datastore.cpp`)**

- Acts as an in‑memory “mini database” for the whole application.
- Implemented as a **singleton**, so there is exactly one instance:

  ```cpp
  DataStore& DataStore::instance() {
      static DataStore instance;
      return instance;
  }
  ```

- On construction:
  - calls internal helper functions to seed all **users** and **items**.
- Stores:
  - `std::vector<User> m_users;`
  - `std::vector<Item> m_items;`
- Exposes operations such as:
  - `findUser(const QString& name)` – get a `User` by name.
  - `borrowItem(User& patron, int itemId)` – enforce rules and create a loan.
  - `returnItem(User& patron, int itemId)` – end a loan.
  - `placeHold(User& patron, int itemId)` – join the item’s hold queue.
  - `cancelHold(User& patron, int itemId)` – leave the queue.
  - `holdPosition(const User& patron, int itemId)` – compute the patron’s position in the queue.
- All **business rules** (loan limits, 14‑day loan period, no duplicate holds) are enforced here so that they apply consistently regardless of how the UI is structured.

---

**`models.hpp`**

Defines the core data types used throughout the program:

- `enum class UserType { Patron, Librarian, Admin };`
- `struct User` – name, type, and lists of active loan and hold item IDs.
- `enum class ItemFormat { FictionBook, NonFictionBook, Magazine, Movie, VideoGame };`
- `struct ItemStatus` – whether the item is available, who has borrowed it, the due date, and the hold queue.
- `struct Item` – an item in the catalogue with ID, title, creator, format, status, and optional metadata fields.
- `namespace Rules` – constants:
  - `MaxActiveLoans` (3) and
  - `LoanDays` (14).

---

**`rolewindows.hpp` / `rolewindows.cpp`**

- Contains simple Qt dialogs for **Librarian** and **Admin** roles.
- At the moment, each window:
  - displays the user’s name and role,
  - shows a message that full functionality will come in a later version.
- These windows are included to show that the system is already structured around multiple roles, even if only the patron role is fully implemented.

---

**`patron.hpp` / `patron.cpp`**

- A small `Patron` class representing a library patron as a standalone type.
- Retained for compatibility and to support future refactoring; the main logic currently uses the more general `User` structure from `models.hpp`.

---

**`mainwindow.*`, `hinlibs_d1.pro`, `hinlibs_d1_en_CA.ts`**

- Qt Creator–generated files and alternative/legacy project configuration.
- Kept in the repository so the project can be opened exactly as it was worked on, but they are not central to the behaviour described above.

---

## Project Structure

A simplified view of the repository layout:

```text
.
├── D1.pro                 # Primary Qt project file
├── main.cpp               # Program entry point
├── startupdialog.hpp/cpp  # Startup dialog (user name + role routing)
├── patronwindow.hpp/cpp   # Main patron UI (catalogue, loans, holds)
├── rolewindows.hpp/cpp    # Librarian/Admin placeholder UIs
├── datastore.hpp/cpp      # Singleton in-memory data store and business logic
├── models.hpp             # Core domain models and rules
├── patron.h/.cpp          # Patron class (legacy / future use)
├── mainwindow.h/.cpp/.ui  # Qt Creator scaffold (not central to D1 logic)
├── hinlibs_d1.pro         # Additional Qt project file
└── hinlibs_d1_en_CA.ts    # Qt translation file
```

---

## Building and Running

### Prerequisites

You will need:

- A working **Qt** installation with the **Qt Widgets** module (Qt 5 or Qt 6),
- A C++17‑capable compiler (for example, `g++`),
- Optionally, **Qt Creator** for an IDE experience.

### Option 1 – Using Qt Creator (recommended)

1. Open **Qt Creator**.
2. Choose **File → Open File or Project…**.
3. Select `D1.pro`.
4. Accept or configure the suggested kit (compiler + Qt version).
5. Click **Configure Project**.
6. Press the **Run** button (green triangle).

You should see the **Startup dialog** where you can type a user name such as `Alice`.

### Option 2 – Command Line

From the project root:

```bash
qmake D1.pro        # or `qmake6` depending on your installation
make
./D1                # or the name of the generated binary on your platform
```

If the executable name differs (for example, `./LibraryManagementSystem`), use that name instead of `./D1`.

---

## Example User Flow

This section walks through a complete example using one of the seeded patrons.

1. **Start the program.**  
   The Startup dialog appears.

2. **Log in as a patron.**  
   Type `Alice` in the name field and press the button.  
   The system finds the user `Alice` (a patron) and opens the **Patron window**.

3. **Browse the catalogue.**  
   In the catalogue table, scroll through the 20 items.  
   Click on a few items and observe how the details section changes to show Dewey numbers, magazine issues, or movie/game ratings depending on the format.

4. **Borrow an item.**  
   Find an item with status **Available** (for example, a fiction book).  
   Select the row and click **“Borrow Selected Item”**.  
   The item should:
   - move into the **“My Active Loans”** list, and
   - show a due date 14 days in the future.

5. **Try the loan limit.**  
   Borrow two more available items (for a total of three).  
   When you attempt to borrow a **fourth** item, the system should prevent the action and show a message explaining that you have reached the loan limit of 3 items.

6. **Place a hold on an unavailable item.**  
   Pick an item whose status indicates that it is **on loan**.  
   Select it and click **“Place Hold on Selected Item”**.  
   The system should:
   - add your name to that item’s hold queue in the `DataStore`, and
   - show the item and your queue position in the **“My Active Holds”** list.

7. **Cancel a hold.**  
   In the **“My Active Holds”** list, select the item you just queued for.  
   Click **“Cancel Selected Hold”**.  
   The entry should disappear from the list, and your name should be removed from that item’s hold queue in the data model.

8. **Return an item.**  
   In **“My Active Loans”**, select one of your loans and click **“Return Selected Loan”**.  
   The item should disappear from the loans list and become **Available** again in the catalogue.

This walkthrough demonstrates how all of the main features—browsing, borrowing, returning, and managing holds—fit together in practice.

---

## Design Decisions

- **In‑memory `DataStore` instead of a real database**  
  For a first deliverable, an in‑memory store keeps the focus on modelling library behaviour without adding database configuration. The design makes it straightforward to replace `DataStore` with a persistence layer later.

- **Separation between UI and rules**  
  The Qt windows (dialogs) do not directly enforce borrowing limits or queue behaviour. They simply gather user input and pass it to `DataStore`. This keeps the rules in one place and makes the system easier to test and extend.

- **Explicit modelling of library concepts**  
  Types such as `User`, `Item`, `ItemStatus`, and `Rules` match real‑world library ideas like patrons, catalogue items, loans, holds, and lending policies. This makes it easier for someone new to the code to connect lines of C++ back to the domain.

- **Multi‑role routing from the start**  
  Even though only the patron role is fully implemented, the Startup dialog already routes different users to different windows (Patron, Librarian, Admin). This prevents a big architectural change later when librarian/admin features are added.

---

## Possible Extensions

Some natural next steps for this project:

- Replace the in‑memory `DataStore` with a real database (for example, SQLite using Qt’s SQL module).
- Implement **Librarian** features such as:
  - adding, editing, and removing items from the catalogue,
  - viewing and managing overdue loans.
- Implement **Admin** features such as:
  - changing lending rules (loan limits, loan length),
  - generating statistics (most borrowed titles, total active loans).
- Add **unit tests** around `DataStore` operations to verify that borrowing and hold rules are enforced correctly.
- Improve the UI with search and filtering options so that patrons can quickly find items by title, creator, or format.

This README is intended to be detailed enough that someone who has never seen the code can still understand **what the system models**, **how it behaves**, and **which parts of the code implement each responsibility**.

