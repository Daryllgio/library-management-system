#pragma once
#include <QString>
#include <QDate>
#include <vector>
#include <optional>
#include <queue>

enum class UserType { Patron, Librarian, Admin };

struct User {
    QString name;
    UserType type;
    // For patrons only: store active loans by item id
    std::vector<int> activeLoans;
    //to store holds for the user
    std::vector<int> holds;
};

enum class ItemFormat { FictionBook, NonFictionBook, Magazine, Movie, VideoGame };

inline QString formatToString(ItemFormat f) {
    switch (f) {
        case ItemFormat::FictionBook:    return "Fiction Book";
        case ItemFormat::NonFictionBook: return "Non-Fiction Book";
        case ItemFormat::Magazine:       return "Magazine";
        case ItemFormat::Movie:          return "Movie";
        case ItemFormat::VideoGame:      return "Video Game";
    }
    return "Unknown";
}

// Availability/state for items
struct ItemStatus {
    bool available = true;
    std::optional<QString> borrower; // name of patron
    std::optional<QDate>  dueDate;   // 14 days from checkout
};

// Single catalogue item (kept intentionally compact for D1)
struct Item {
    int id;
    QString title;
    QString creator;    // author / director / studio, etc.
    ItemFormat format;
    ItemStatus status;
    //to track holds on an item
    std::queue<QString> holdQueue;

    // Optional fields for formats that require them
    QString dewey;      // for non-fiction e.g. "123.45"
    QString issue;      // for magazines
    QString pubDate;    // for magazines (YYYY-MM)
    QString genre;      // for movies/games
    QString rating;     // for movies/games (e.g., "PG-13", "E10+")
};

// Business constraints
namespace Rules {
    constexpr int MaxActiveLoans = 3;     // patrons may borrow at most 3 items at a time
    constexpr int LoanDays       = 14;    // due date is 14 days from checkout
}
