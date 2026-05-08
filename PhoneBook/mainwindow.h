#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>

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
    QPushButton *addButton;
    QPushButton *deleteButton;
    QPushButton *clearButton;

    void loadData();

private slots:
    void addContact();
    void deleteContact();
    void clearContacts();
};

#endif
