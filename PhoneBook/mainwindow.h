#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private:
    QTableWidget *table;
    QLineEdit *nameEdit;
    QLineEdit *firstNameEdit;
    QLineEdit *patronymicEdit;
    QLineEdit *phoneEdit;
    QLineEdit *searchEdit;
    QPushButton *addButton;
    QPushButton *editButton;
    QPushButton *deleteButton;
    QPushButton *clearButton;
    QPushButton *deleteAllButton;
    QLabel *countLabel;
    QLabel *emptyHintLabel;

    void loadData();
    int currentContactId() const;
    void updateContactSummary();

private slots:
    void addContact();
    void editContact();
    void fillInputsFromCurrentRow();
    void deleteContact();
    void clearInputFields();
    void clearContacts();
};

#endif
