#pragma once
#include <QDialog>
#include "models.hpp"

class QTableWidget;
class QPushButton;
class QListWidget;
class QLabel;

class PatronWindow : public QDialog
{
    Q_OBJECT
public:
    explicit PatronWindow(const QString &patronName, QWidget *parent = nullptr);

private slots:
    void populateCatalogue();
    void onCatalogueSelectionChanged();
    void onBorrowClicked();
    void onLoansSelectionChanged();
    void onReturnClicked();
    void onHoldClicked();
    void onCancelHoldClicked();
    void onHoldsSelectionChanged();
private:
    // Working copy of the patron; we upsert after changes
    User m_patron;

    //UI Widgets
    QTableWidget *m_table;
    QPushButton *m_borrowBtn;
    QListWidget *m_loansList;
    QLabel *m_selectedLabel;
    QPushButton *m_returnBtn;
    QPushButton *m_holdBtn;
    QPushButton *m_cancelHoldBtn;
    QListWidget *m_holdsList;

    //to update loans and holds for user on  GUI
    void refreshLoansView();
    void refreshHoldsView();
};
