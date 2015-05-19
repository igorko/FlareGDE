#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

const int TAB_MAIN = 0;
const int TAB_ITEMS = 1;
const int TAB_POWERS = 2;
const int TAB_ENEMIES = 3;
const int TAB_NPCS = 4;
const int TAB_QUESTS = 5;

const int MENU_CREATURES = 1;
const int MENU_MENUS = 2;
const int MENU_STUFF = 3;
const int MENU_STORY = 5;

class EditorItemManager;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_addNewItem_clicked();

    void on_clearBtn_clicked();

    void on_itemClose_clicked();

    void on_actionClose_Mod_triggered();

    void on_pushBtn_clicked();

    void on_itemsList_itemClicked(QListWidgetItem *item);

    void on_actionAdd_Item_triggered();

    void on_actionSave_Mod_triggered();

    void on_actionQuit_triggered();

    void on_actionAbout_triggered();

    void on_actionOpen_Mod_triggered();

    void on_actionNew_Mod_triggered();

private:
    Ui::MainWindow *ui;

    QString modPath;
    QString modName;
    void disableAllTabsExceptIndex(int index);
    void setMenusEnabled(bool state);
    void ToDo();
    void CloseAll();
    bool newMod;
    EditorItemManager * items;

};

#endif // MAINWINDOW_H
