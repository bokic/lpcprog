#ifndef QAPPMAINWINDOW_H
#define QAPPMAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class QMainWindow;
}

class QAppMainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit QAppMainWindow(QWidget *parent = 0);
    ~QAppMainWindow();

protected:
    void timerEvent (QTimerEvent *event);
    
private slots:
    void on_chipID_pushButton_clicked();
    void on_firmwareVersion_pushButton_clicked();
    void on_fileBrowse_toolButton_clicked();
    void on_fileOperation_pushButton_clicked();
    void on_erase_pushButton_clicked();
    void on_read_pushButton_clicked();
    void on_decompile_pushButton_clicked();
    void on_file_lineEdit_textChanged(const QString &text);

private:
    void updateSerialPorts();
    void fileProgram(const QString &file);
    void fileVerify(const QString &file);
    void fileDecompile(const QString &file);

    int m_SerialPortTimer;

    Ui::QMainWindow *ui;
};

#endif // QAPPMAINWINDOW_H
