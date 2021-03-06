#ifndef TODODIALOG_H
#define TODODIALOG_H

#include <QDialog>
#include <QListWidgetItem>
#include <QSplitter>

#include "libraries/simplecrypt/simplecrypt.h"
#include "entities/calendaritem.h"
#include "masterdialog.h"

namespace Ui {
class TodoDialog;
}

class TodoDialog : public MasterDialog
{
    Q_OBJECT

public:
    explicit TodoDialog(SimpleCrypt *crypto, QWidget *parent = 0);
    ~TodoDialog();

    void reloadTodoListItems();
    void reloadTodoList();
    void clearTodoList();
    void todoItemLoadingProgressBarIncrement();
    void todoItemLoadingProgressBarSetMaximum(int value);
    void todoItemLoadingProgressBarHide();
    void todoItemLoadingProgressBarHideIfOnMaximum();
private slots:
    void on_TodoDialog_finished(int result);
    void on_todoListSelector_currentIndexChanged(const QString &arg1);
    void on_todoList_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_prioritySlider_valueChanged(int value);
    void on_showCompletedItemsCheckBox_clicked();
    void on_saveButton_clicked();
    void on_todoItemLoadingProgressBar_valueChanged(int value);
    void on_newItemEdit_returnPressed();
    void on_removeButton_clicked();
    void on_todoList_itemChanged(QListWidgetItem *item);
    void on_reminderCheckBox_clicked();
    void on_reloadTodoListButton_clicked();
    void on_summaryEdit_returnPressed();
    void on_newItemEdit_textChanged(const QString &arg1);

private:
    Ui::TodoDialog *ui;
    QSplitter *mainSplitter;
    SimpleCrypt *crypto;
    CalendarItem currentCalendarItem;
    CalendarItem lastCreatedCalendarItem;
    int firstVisibleTodoListRow;
    void setupMainSplitter();
    void storeSettings();
    void loadTodoListData();
    int findTodoItemRowByUID(QString uid);
    void resetEditFrameControls();
    void setupUi();
    void updateCurrentCalendarItemWithFormData();
    void searchForSearchLineTextInNoteTextEdit();
    void searchInDescriptionTextEdit(QString &str);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

};

#endif // TODODIALOG_H
