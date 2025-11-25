#pragma once
#include "models.hpp"
#include <vector>
#include <optional>

// ---------------------------------------------
// DataStore: in-memory "database" for D1–D4
// ---------------------------------------------
// Seeds default items/users on startup and offers
// basic operations for the Patron workflow.
class DataStore
{
public:
    static DataStore &instance();

    const std::vector<User> &users() const { return m_users; }
    const std::vector<Item> &items() const { return m_items; }

    //Look up a user by exact name
    std::optional<User> findUser(QString name) const;

    //Replace the stored user record
    void upsertUser(const User &user);

    //Borrow an item for a patron
    std::optional<QString> borrowItem(User &patron, int itemId);

    //Return an item
    std::optional<QString> returnItem(User &patron, int itemId);

    // Utility: locate an item by id (mutable), or const
    Item *findItemById(int id);
    const Item *findItemById(int id) const;

    //Reset session state when leaving a user UI
    void clearCurrentUserState();

    //hold functions
    std::optional<QString> placeHold(User &patron, int itemId);
    std::optional<QString> cancelHold(User &patron, int itemId);
    int holdPosition(const User &patron, int itemId) const;

private:
    DataStore();
    void seedUsers();
    void seedItems();

    std::vector<User> m_users;
    std::vector<Item> m_items;
};
