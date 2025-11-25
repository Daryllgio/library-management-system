#include "patronwindow.hpp"
#include "datastore.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QTableWidget>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QMessageBox>

PatronWindow::PatronWindow(const QString &patronName, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QString("HinLIBS — Patron: %1").arg(patronName));
    resize(900, 540);

    // Load the user from DataStore (we assume existence)
    m_patron = DataStore::instance().findUser(patronName).value();

    auto *root = new QVBoxLayout(this);

    // Top: Catalogue table
    m_table = new QTableWidget(this);
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels({"ID", "Title", "Author/Creator", "Format", "Availability"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    root->addWidget(m_table, 3);

    // Middle: selection details + borrow
    auto *mid = new QHBoxLayout();
    m_selectedLabel = new QLabel("No item selected.");
    m_borrowBtn = new QPushButton("Borrow Selected Item");
    m_borrowBtn->setEnabled(false);

    //hold button
    m_holdBtn = new QPushButton("Place Hold");
    m_holdBtn->setEnabled(false);

    mid->addWidget(m_selectedLabel, 1);
    mid->addWidget(m_borrowBtn, 0);
    mid->addWidget(m_holdBtn, 0); // add the hold button
    root->addLayout(mid);

    // Bottom: Active loans panel
    auto *loansBox = new QHBoxLayout();
    auto *loansLabel = new QLabel("My Active Loans (max 3):");
    m_loansList = new QListWidget();
    loansBox->addWidget(loansLabel);
    loansBox->addStretch();
    root->addLayout(loansBox);
    root->addWidget(m_loansList, 1);

    //holds panel
    auto *holdsBox = new QHBoxLayout();
    auto *holdsLabel = new QLabel("My Active Holds:");
    m_holdsList = new QListWidget();
    holdsBox->addWidget(holdsLabel);
    holdsBox->addStretch();
    root->addLayout(holdsBox);
    root->addWidget(m_holdsList, 1);

    //cancel hold
    auto *cancelRow = new QHBoxLayout();
    cancelRow->addStretch();
    m_cancelHoldBtn = new QPushButton("Cancel Selected Hold");
    m_cancelHoldBtn->setEnabled(false);
    cancelRow->addWidget(m_cancelHoldBtn);
    root->addLayout(cancelRow);


    //return controls row under the loans list
    auto *retRow = new QHBoxLayout();
    retRow->addStretch();
    m_returnBtn = new QPushButton("Return Selected Loan");
    m_returnBtn->setEnabled(false);
    retRow->addWidget(m_returnBtn);
    root->addLayout(retRow);

    // Wire signals
    connect(m_table->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &PatronWindow::onCatalogueSelectionChanged);
    connect(m_borrowBtn, &QPushButton::clicked, this, &PatronWindow::onBorrowClicked);

    //selection + action for returns
    connect(m_loansList, &QListWidget::itemSelectionChanged,
            this, &PatronWindow::onLoansSelectionChanged);
    connect(m_returnBtn, &QPushButton::clicked,
            this, &PatronWindow::onReturnClicked);

    //hold operations connections
    connect(m_holdBtn, &QPushButton::clicked, this, &PatronWindow::onHoldClicked);
    connect(m_cancelHoldBtn, &QPushButton::clicked, this, &PatronWindow::onCancelHoldClicked);
    connect(m_holdsList, &QListWidget::itemSelectionChanged,
            this, &PatronWindow::onHoldsSelectionChanged);

    // Initial population
    populateCatalogue();
    refreshLoansView();
}

void PatronWindow::populateCatalogue()
{
    const auto &items = DataStore::instance().items();
    m_table->setRowCount((int)items.size());
    int r = 0;
    for (const auto &it : items)
    {
        // ID
        auto *idItem = new QTableWidgetItem(QString::number(it.id));
        idItem->setFlags(idItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(r, 0, idItem);

        // Title
        auto *titleItem = new QTableWidgetItem(it.title);
        titleItem->setFlags(titleItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(r, 1, titleItem);

        // Creator
        auto *creatorItem = new QTableWidgetItem(it.creator);
        creatorItem->setFlags(creatorItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(r, 2, creatorItem);

        // Format
        auto *fmtItem = new QTableWidgetItem(formatToString(it.format));
        fmtItem->setFlags(fmtItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(r, 3, fmtItem);

        // Availability
        QString status = it.status.available ? "Available" : QString("Out (due %1)").arg(it.status.dueDate ? it.status.dueDate->toString("yyyy-MM-dd") : "—");
        auto *statusItem = new QTableWidgetItem(status);
        statusItem->setFlags(statusItem->flags() & ~Qt::ItemIsEditable);
        m_table->setItem(r, 4, statusItem);

        r++;
    }
    m_table->resizeRowsToContents();
}

//updating users GUI when loans are selected
void PatronWindow::onCatalogueSelectionChanged()
{
    auto selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty())
    {
        m_selectedLabel->setText("No item selected.");
        m_borrowBtn->setEnabled(false);
        return;
    }
    int row = selected.first().row();
    int itemId = m_table->item(row, 0)->text().toInt();
    const Item *it = DataStore::instance().findItemById(itemId);
    if (!it)
        return;

    // Show details: title, author/creator, format, availability
    QString detail = QString("Selected #%1 — \"%2\" by %3  |  %4  |  %5")
                         .arg(it->id)
                         .arg(it->title)
                         .arg(it->creator)
                         .arg(formatToString(it->format))
                         .arg(it->status.available ? "Available" : QString("Checked out (due %1)").arg(it->status.dueDate ? it->status.dueDate->toString("yyyy-MM-dd") : "—"));
    m_selectedLabel->setText(detail);

    // You can only borrow if it's available and you haven't hit the cap
    bool canBorrow = it->status.available && ((int)m_patron.activeLoans.size() < Rules::MaxActiveLoans);
    m_borrowBtn->setEnabled(canBorrow);

    //can place hold if item is unavailable
    bool canHold = !it->status.available;
    m_holdBtn->setEnabled(canHold);
}

//when user selescts items to borrow
void PatronWindow::onBorrowClicked()
{
    auto selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return;
    int row = selected.first().row();
    int itemId = m_table->item(row, 0)->text().toInt();

    auto err = DataStore::instance().borrowItem(m_patron, itemId);
    if (err)
    {
        QMessageBox::warning(this, "Borrow failed", *err);
    }
    else
    {
        QMessageBox::information(this, "Success", "Item checked out! Due in 14 days.");
    }

    // Refresh UI to reflect new state
    populateCatalogue();
    refreshLoansView();
    onCatalogueSelectionChanged();
}

//updating users loans on GUI
void PatronWindow::refreshLoansView()
{
    m_loansList->clear();
    for (int id : m_patron.activeLoans)
    {
        const Item *it = DataStore::instance().findItemById(id);
        if (!it)
            continue;
        QString due = it->status.dueDate ? it->status.dueDate->toString("yyyy-MM-dd") : "—";
        QString daysRemaining = "—";
               if (it->status.dueDate)
               {
                   int days = QDate::currentDate().daysTo(*it->status.dueDate);
                   daysRemaining = QString::number(days);
               }        auto *li = new QListWidgetItem(QString("#%1  %2  (due %3)").arg(it->id).arg(it->title).arg(due));
        li->setData(Qt::UserRole, it->id); // stash item id for returns
        m_loansList->addItem(li);
    }
    refreshHoldsView();

    // After repopulating, enable/disable Return based on selection presence
    m_returnBtn->setEnabled(m_loansList->currentItem() != nullptr);
}

//enabling loan seelection only if item is selected
void PatronWindow::onLoansSelectionChanged()
{
    m_returnBtn->setEnabled(m_loansList->currentItem() != nullptr);
}

//when user returns item
void PatronWindow::onReturnClicked()
{
    auto *cur = m_loansList->currentItem();
    if (!cur)
        return;

    int itemId = cur->data(Qt::UserRole).toInt();
    auto err = DataStore::instance().returnItem(m_patron, itemId);
    if (err)
    {
        QMessageBox::warning(this, "Return failed", *err);
    }
    else
    {
        QMessageBox::information(this, "Returned", "Item returned successfully.");
    }

    // Refresh to show updated availability and loan list
    populateCatalogue();
    refreshLoansView();
    onCatalogueSelectionChanged();
}

//when user selects an item for hold
void PatronWindow::onHoldClicked()
{
    auto selected = m_table->selectionModel()->selectedRows();
    if (selected.isEmpty())
        return;

    int row = selected.first().row();
    int itemId = m_table->item(row, 0)->text().toInt();

    auto err = DataStore::instance().placeHold(m_patron, itemId); // assumes you added placeHold logic
    if (err)
    {
        QMessageBox::warning(this, "Hold failed", *err);
    }
    else
    {
        QMessageBox::information(this, "Hold placed", "You have successfully placed a hold on this item.");
    }

    // Refresh UI
    populateCatalogue();
    refreshLoansView();
    onCatalogueSelectionChanged();
}


//updating holds on users GUI

void PatronWindow::refreshHoldsView()
{
    m_holdsList->clear();
    for (size_t i = 0; i < m_patron.holds.size(); ++i)
    {
        int id = m_patron.holds[i];
        const Item *it = DataStore::instance().findItemById(id);
        if (!it) continue;

        int position = i + 1; // queue position is just the index + 1
        auto *li = new QListWidgetItem(
            QString("#%1  %2  (position %3)").arg(it->id).arg(it->title).arg(position)
        );
        li->setData(Qt::UserRole, it->id);
        m_holdsList->addItem(li);
    }
}

//ennabligng hhold selection only if item is selected from  hold list
void PatronWindow::onHoldsSelectionChanged()
{
    m_cancelHoldBtn->setEnabled(m_holdsList->currentItem() != nullptr);
}

//user cancelling their hold
void PatronWindow::onCancelHoldClicked()
{
    auto *cur = m_holdsList->currentItem();
    if (!cur)
        return;

    int itemId = cur->data(Qt::UserRole).toInt();

    auto err = DataStore::instance().cancelHold(m_patron, itemId);
    if (err)
    {
        QMessageBox::warning(this, "Cancel Hold Failed", *err);
    }
    else
    {
        QMessageBox::information(this, "Hold Canceled", "You have successfully canceled your hold on this item.");
    }

    // Refresh UI
    populateCatalogue();
    refreshLoansView();
    onCatalogueSelectionChanged();
}
