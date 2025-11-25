#include "datastore.hpp"
#include <algorithm>

DataStore &DataStore::instance()
{
    static DataStore ds;
    return ds;
}

DataStore::DataStore()
{
    seedUsers();
    seedItems();
}

void DataStore::seedUsers()
{
    // 5 patrons, 1 librarian, 1 admin — simple names so TAs can test quickly
    m_users.push_back(User{"Alice", UserType::Patron, {}, {}});
    m_users.push_back(User{"Bob", UserType::Patron, {}, {}});
    m_users.push_back(User{"Carmen", UserType::Patron, {}, {}});
    m_users.push_back(User{"Dev", UserType::Patron, {}, {}});
    m_users.push_back(User{"Eve", UserType::Patron, {}, {}});
    m_users.push_back(User{"Librarian", UserType::Librarian, {}, {}});
    m_users.push_back(User{"Admin", UserType::Admin, {}, {}});
}

void DataStore::seedItems()
{
    int id = 1;
    // 5 fiction
    m_items.push_back(Item{id++, "The Wind Road", "J. Harper", ItemFormat::FictionBook, {},{}, "", "", "", "", ""});
    m_items.push_back(Item{id++, "Night Harbor", "A. Singh", ItemFormat::FictionBook, {},{}, "", "", "", "", ""});
    m_items.push_back(Item{id++, "Echoes", "L. Chen", ItemFormat::FictionBook, {}, {},"", "", "", "", ""});
    m_items.push_back(Item{id++, "Summer Glass", "M. Ortega", ItemFormat::FictionBook, {}, {}, "", "", "", "", ""});
    m_items.push_back(Item{id++, "Hidden Leaves", "R. Patel", ItemFormat::FictionBook, {}, {},"", "", "", "", ""});

    // 5 non-fiction (with Dewey)
    m_items.push_back(Item{id++, "Quantum Basics", "S. Rao", ItemFormat::NonFictionBook, {}, {},"530.12", "", "", "", ""});
    m_items.push_back(Item{id++, "The Brain Map", "N. Ahmed", ItemFormat::NonFictionBook, {}, {},"612.82", "", "", "", ""});
    m_items.push_back(Item{id++, "Design Matters", "P. Nguyen", ItemFormat::NonFictionBook, {}, {},"745.4", "", "", "", ""});
    m_items.push_back(Item{id++, "Civic Algorithms", "K. Okafor", ItemFormat::NonFictionBook, {}, {},"303.38", "", "", "", ""});
    m_items.push_back(Item{id++, "Kitchen Chemistry", "D. Rossi", ItemFormat::NonFictionBook, {}, {},"540.1", "", "", "", ""});

    // 3 magazines (issue + pubDate)
    m_items.push_back(Item{id++, "Tech Monthly", "Editorial Board", ItemFormat::Magazine, {}, {},"", "Issue 142", "2025-10", "", ""});
    m_items.push_back(Item{id++, "Nature & You", "Editorial Board", ItemFormat::Magazine, {}, {},"", "Issue 88", "2025-09", "", ""});
    m_items.push_back(Item{id++, "Cinema Now", "Editorial Board", ItemFormat::Magazine, {}, {},"", "Issue 23", "2025-11", "", ""});

    // 3 movies (genre + rating)
    m_items.push_back(Item{id++, "Northern Lights", "K. Yamamoto", ItemFormat::Movie, {}, {},"", "", "", "Drama", "PG-13"});
    m_items.push_back(Item{id++, "Edge Protocol", "R. Coleman", ItemFormat::Movie, {}, {},"", "", "", "Sci-Fi", "PG-13"});
    m_items.push_back(Item{id++, "Riverfront", "M. Da Silva", ItemFormat::Movie, {}, {},"", "", "", "Documentary", "G"});

    // 4 video games (genre + rating)
    m_items.push_back(Item{id++, "Skyforge", "BlueFox Studio", ItemFormat::VideoGame, {}, {},"", "", "", "Adventure", "E10+"});
    m_items.push_back(Item{id++, "Circuit Clash", "ArcByte", ItemFormat::VideoGame, {}, {},"", "", "", "Action", "T"});
    m_items.push_back(Item{id++, "Farmstead 2049", "Sunseed", ItemFormat::VideoGame, {}, {},"", "", "", "Simulation", "E"});
    m_items.push_back(Item{id++, "Starlane", "Nova North", ItemFormat::VideoGame, {}, {},"", "", "", "Strategy", "E10+"});
}

std::optional<User> DataStore::findUser(QString name) const
{
    for (const auto &u : m_users)
    {
        if (u.name == name)
            return u;
    }
    return std::nullopt;
}

void DataStore::upsertUser(const User &user)
{
    for (auto &u : m_users)
    {
        if (u.name == user.name)
        {
            u = user;
            return;
        }
    }
    m_users.push_back(user);
}

Item *DataStore::findItemById(int id)
{
    for (auto &it : m_items)
        if (it.id == id)
            return &it;
    return nullptr;
}

const Item *DataStore::findItemById(int id) const
{
    for (const auto &it : m_items)
        if (it.id == id)
            return &it;
    return nullptr;
}

std::optional<QString> DataStore::borrowItem(User &patron, int itemId)
{
    // Check patron loan cap
    if ((int)patron.activeLoans.size() >= Rules::MaxActiveLoans)
    {
        return QString("Borrowing blocked: you already have %1 active loans.").arg(Rules::MaxActiveLoans);
    }
    Item *it = findItemById(itemId);
    if (!it)
        return QString("Internal error: item not found.");
    if (!it->status.available)
    {
        return QString("Item '%1' is not available to borrow.").arg(it->title);
    }

    // All good: perform checkout
    it->status.available = false;
    it->status.borrower = patron.name;
    it->status.dueDate = QDate::currentDate().addDays(Rules::LoanDays);
    patron.activeLoans.push_back(itemId);

    // Persist patron changes to store
    upsertUser(patron);
    return std::nullopt; // success
}

//to return item
std::optional<QString> DataStore::returnItem(User &patron, int itemId)
{
    Item *it = findItemById(itemId);
    if (!it)
        return QString("Internal error: item not found.");

    // Must currently be checked out
    if (it->status.available)
    {
        return QString("Item '%1' is already available.").arg(it->title);
    }

    // Defensive: ensure the returning patron is the borrower
    if (!it->status.borrower || *it->status.borrower != patron.name)
    {
        return QString("Item '%1' is not checked out by you.").arg(it->title);
    }

    // Remove from patron's active loans
    auto &loans = patron.activeLoans;
    auto newEnd = std::remove(loans.begin(), loans.end(), itemId);
    if (newEnd == loans.end())
    {
        return QString("Internal error: loan record not found for '%1'.").arg(it->title);
    }
    loans.erase(newEnd, loans.end());

    // Reset item status to Available
    it->status.available = true;
    it->status.borrower.reset();
    it->status.dueDate.reset();

    // Persist patron updates
    upsertUser(patron);

    return std::nullopt; // success
}


void DataStore::clearCurrentUserState()
{
    // Prototype-level "session" clear: nothing permanent to wipe.
    // Each window clears its own UI state and drops User references on close.
}

//user places hold
std::optional<QString> DataStore::placeHold(User &patron, int itemId) {
    Item *it = findItemById(itemId);
    if (!it) return "Internal error: item not found.";

    // Already on loan to patron?
    if (std::find(patron.activeLoans.begin(), patron.activeLoans.end(), itemId) != patron.activeLoans.end())
        return QString("You already have '%1' checked out.").arg(it->title);

    // Already has a hold
    if (std::find(patron.holds.begin(), patron.holds.end(), itemId) != patron.holds.end())
        return QString("You already placed a hold on '%1'.").arg(it->title);

    // Place the hold
    it->holdQueue.push(patron.name);
    patron.holds.push_back(itemId);
    upsertUser(patron);
    return QString("Hold placed successfully. You are #%1 in queue.")
                .arg(it->holdQueue.size());
}

//user cancels hold
std::optional<QString> DataStore::cancelHold(User &patron, int itemId) {
    Item *it = findItemById(itemId);
    if (!it) return "Internal error: item not found.";

    std::queue<QString> newQueue;
    bool removed = false;

    while (!it->holdQueue.empty()) {
        auto front = it->holdQueue.front();
        it->holdQueue.pop();
        if (front != patron.name)
            newQueue.push(front);
        else
            removed = true;
    }

    it->holdQueue = newQueue;

    if (removed) {
        patron.holds.erase(
            std::remove(patron.holds.begin(), patron.holds.end(), itemId),
            patron.holds.end());
        upsertUser(patron);
        return QString("Hold on '%1' cancelled.").arg(it->title);
    } else {
        return QString("You have no hold on '%1'.").arg(it->title);
    }
}

//calcualting hold position of user on item
int DataStore::holdPosition(const User &patron, int itemId) const {
    const Item *it = findItemById(itemId);
    if (!it) return -1;

    std::queue<QString> q = it->holdQueue;
    int pos = 1;
    while (!q.empty()) {
        if (q.front() == patron.name)
            return pos;
        q.pop();
        ++pos;
    }
    return -1;
}
