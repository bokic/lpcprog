#include "qappmainwindow.h"
#include "ui_qappmainwindow.h"
#include "qhexloader.h"
#include "qlpcprog.h"

#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>


#define STATUSBAR_TIMEOUT 2000


QAppMainWindow::QAppMainWindow(QWidget *parent) :
    QMainWindow(parent),
    m_SerialPortTimer(0),
    ui(new Ui::QMainWindow)
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "BokiCSoft", "LPCProg");

    ui->setupUi(this);

    updateSerialPorts();
    if (ui->ports_comboBox->count() > 0)
    {
        ui->ports_comboBox->setCurrentIndex(ui->ports_comboBox->count() - 1);
    }

    if (!settings.value("Filename").isNull())
    {
        ui->file_lineEdit->setText(settings.value("Filename").toString());
    }

    m_SerialPortTimer = startTimer(500);
}

QAppMainWindow::~QAppMainWindow()
{
    killTimer(m_SerialPortTimer);
    m_SerialPortTimer = 0;

    delete ui;
}

void QAppMainWindow::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_SerialPortTimer)
    {
        updateSerialPorts();
    }
}

void QAppMainWindow::updateSerialPorts()
{
    QStringList ports = QLpcProg::detectSerialPorts();

    for(int c = 0; c < ports.count(); c++)
    {
        if (c < ui->ports_comboBox->count())
        {
            if (ui->ports_comboBox->itemText(c) != ports.at(c))
            {
                ui->ports_comboBox->setItemText(c, ports.at(c));
            }
        }
        else
        {
            ui->ports_comboBox->addItem(ports.at(c));
        }
    }
}

void QAppMainWindow::on_chipID_pushButton_clicked()
{
    QLpcProg prog;

    if (ui->ports_comboBox->currentIndex() == -1)
    {
        return;
    }

    ui->statusbar->showMessage(tr("Opening serial port."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.init(ui->ports_comboBox->currentText());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC initialization timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Set Crystal value."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setCrystalValue(ui->crystal_spinBox->value());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC set crystal value timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Disable echo."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setEcho(false);
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Read part ID."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    int partID = prog.readPartID();
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC getPartID timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t get PartID.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t get PartID(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    switch(partID)
    {
    case QLpcProg::LPC2141:
        ui->chipID_label->setText("LPC2141");
        break;
    case QLpcProg::LPC2142:
        ui->chipID_label->setText("LPC2142");
        break;
    case QLpcProg::LPC2144:
        ui->chipID_label->setText("LPC2144");
        break;
    case QLpcProg::LPC2146:
        ui->chipID_label->setText("LPC2146");
        break;
    case QLpcProg::LPC2148:
        ui->chipID_label->setText("LPC2148");
        break;
    default:
        ui->chipID_label->setText(QString("Unknown chip(%1)").arg(partID));
        break;
    }
}

void QAppMainWindow::on_firmwareVersion_pushButton_clicked()
{
    QLpcProg prog;

    if (ui->ports_comboBox->currentIndex() == -1)
    {
        return;
    }

    ui->statusbar->showMessage(tr("Opening serial port."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.init(ui->ports_comboBox->currentText());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC initialization timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Set Crystal value."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setCrystalValue(ui->crystal_spinBox->value());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC set crystal value timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Disable echo."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setEcho(false);
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Read Boot code version."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    QString bootCodeVersion = prog.readBootCodeVersion();

    if (bootCodeVersion.isEmpty())
    {
        ui->firmwareVersion_label->setText("");
    }
    else
    {
        ui->firmwareVersion_label->setText(QString("ver %1").arg(bootCodeVersion));
    }
}

void QAppMainWindow::on_fileBrowse_toolButton_clicked()
{
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, "BokiCSoft", "LPCProg");
    QFileDialog dlg(this);

    dlg.setNameFilters(QStringList() << "Intel Hex files (*.hex)" << "Any files (*.*)");

    if (!settings.value("Filename").isNull())
    {
        dlg.setDirectory(settings.value("Filename").toString());
    }

    if (dlg.exec() == QDialog::Accepted)
    {
        QString file = dlg.selectedFiles().at(0);

        QSettings settings(QSettings::IniFormat, QSettings::UserScope, "BokiCSoft", "LPCProg");

        settings.setValue("Filename", file);

        ui->file_lineEdit->setText(file);
    }
}

void QAppMainWindow::on_fileOperation_pushButton_clicked()
{
    const QString &file = ui->file_lineEdit->text();

    ui->fileOperation_pushButton->setEnabled(false);

    if (ui->fileProgram_radioButton->isChecked())
    {
        fileProgram(file);
    }
    else if (ui->fileVerify_radioButton->isChecked())
    {
        fileVerify(file);
    }
    else if (ui->fileDecompile_radioButton->isChecked())
    {
        fileDecompile(file);
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Unknown Error."));
    }

    ui->fileOperation_pushButton->setEnabled(true);
}

void QAppMainWindow::on_erase_pushButton_clicked()
{
    QLpcProg prog;

    if (ui->ports_comboBox->currentIndex() == -1)
    {
        return;
    }

    if (QMessageBox::question(this, tr("Question"), tr("Are you sure you want to erase the chip?")) != QMessageBox::Yes)
    {
        return;
    }

    ui->statusbar->showMessage(tr("Opening serial port."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.init(ui->ports_comboBox->currentText());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC initialization timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Set Crystal value."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setCrystalValue(ui->crystal_spinBox->value());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC set crystal value timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Disable echo."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setEcho(false);
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Chip erase."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.chipErase();
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC chip erase timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC chip erase failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC chip erase failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->clearMessage();
}

void QAppMainWindow::on_blankCheck_pushButton_clicked()
{
    QLpcProg prog;

    if (ui->ports_comboBox->currentIndex() == -1)
    {
        return;
    }

    ui->statusbar->showMessage(tr("Opening serial port."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.init(ui->ports_comboBox->currentText());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC initialization timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Set Crystal value."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setCrystalValue(ui->crystal_spinBox->value());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC set crystal value timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Disable echo."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setEcho(false);
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Blank Check."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    bool is_blank = prog.chipBlankCheck();
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC chip erase timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC chip erase failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC chip erase failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->clearMessage();

    if (is_blank)
    {
        QMessageBox::warning(this, tr("Info"), tr("LPC chip <b>IS</b> blank."));
    }
    else
    {
        QMessageBox::warning(this, tr("Warning"), tr("LPC chip <b>IS NOT</b> blank."));
    }
}

void QAppMainWindow::on_read_pushButton_clicked()
{

}

void QAppMainWindow::on_decompile_pushButton_clicked()
{

}

void QAppMainWindow::on_file_lineEdit_textChanged(const QString &text)
{
    if (text.isEmpty())
    {
        ui->fileOperation_pushButton->setEnabled(false);
    }
    else
    {
        ui->fileOperation_pushButton->setEnabled(true);
    }
}

void QAppMainWindow::fileProgram(const QString &file)
{
    if (ui->ports_comboBox->currentIndex() == -1)
    {
        return;
    }

    QHexLoader loader;

    if (loader.load(file) == false)
    {
        QMessageBox::critical(this, tr("Error"), tr("Error loading hex file."));

        return;
    }

    QByteArray data = loader.data();

    if (data.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Error loading hex file."));

        return;
    }

    if (QMessageBox::question(this, tr("Question"), tr("Are you sure you want to reprogram the chip?")) != QMessageBox::Yes)
    {
        return;
    }

    QLpcProg prog;

    ui->statusbar->showMessage(tr("Opening serial port."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.init(ui->ports_comboBox->currentText());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC initialization timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Set Crystal value."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setCrystalValue(ui->crystal_spinBox->value());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC set crystal value timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Disable echo."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setEcho(false);
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    /*prog.setBaudRate(9600);
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC set BaudRate timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC set BaudRate failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC set BaudRate failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }*/

    ui->statusbar->showMessage(tr("Chip erase."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.chipErase();
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC chip erase timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC chip erase failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC chip erase failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    /*prog.unlock();
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC unlock timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC unlock failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC unlock failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }*/

    // patch the firmware.
    prog.patchFirmware(data);

    // program 1024byte blocks.
    int chunks = data.length() / 1024;
    if (data.length() % 1024) chunks++;

    for(int c = chunks - 1; c >= 0; c--)
    {
        ui->statusbar->showMessage(tr("Programming (%1% complete).").arg(((chunks - c - 1) * 100) / chunks), STATUSBAR_TIMEOUT);
        QApplication::processEvents();

        QByteArray chunk = data.mid(c * 1024, 1024);
        prog.chipProgram(chunk, c * 1024);

        if (prog.getStatus() != QLpcProg::StatusNoError)
        {
            QMessageBox::critical(this, tr("Error"), tr("Programming failed."));

            return;
        }
    }

    prog.deinit();

    ui->statusbar->clearMessage();
    QMessageBox::information(this, tr("Info"), tr("Chip firmware is programmed successfully."));
}

void QAppMainWindow::fileVerify(const QString &file)
{
    if (ui->ports_comboBox->currentIndex() == -1)
    {
        return;
    }

    // Read file
    QHexLoader loader;

    if (loader.load(file) == false)
    {
        QMessageBox::critical(this, tr("Error"), tr("Error loading hex file."));

        return;
    }

    QByteArray data = loader.data();

    if (data.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Error loading hex file."));

        return;
    }

    // chip erase
    QLpcProg prog;

    ui->statusbar->showMessage(tr("Opening serial port."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.init(ui->ports_comboBox->currentText());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC initialization timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t be initialized(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Set Crystal value."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setCrystalValue(ui->crystal_spinBox->value());
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC set crystal value timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC could\'t set crystal value(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    ui->statusbar->showMessage(tr("Disable echo."), STATUSBAR_TIMEOUT);
    QApplication::processEvents();

    prog.setEcho(false);
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC disable echo failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    /*prog.setBaudRate(9600);
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC set BaudRate timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC set BaudRate failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC set BaudRate failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }*/

    prog.unlock();
    switch (prog.getStatus())
    {
    case QLpcProg::StatusNoError:
        break;
    case QLpcProg::StatusTimeOut:
        QMessageBox::critical(this, tr("Error"), tr("LPC unlock timeout."));
        return;
    case QLpcProg::StatusError:
        QMessageBox::critical(this, tr("Error"), tr("LPC unlock failed.\n Error string: %1.").arg(prog.getStatusText()));
        return;
    default:
        QMessageBox::critical(this, tr("Error"), tr("LPC unlock failed(unknown error type).\n Error string: %1.").arg(prog.getStatusText()));
        return;
    }

    // patch the firmware.
    prog.patchFirmware(data);

    // verify 1024byte blocks.
    int chunks = data.length() / 1024;
    if (data.length() % 1024) chunks++;

    for(int c = 0; c < chunks; c++)
    {
        ui->statusbar->showMessage(tr("Verify (%1% complete).").arg(((chunks - c - 1) * 100) / chunks), STATUSBAR_TIMEOUT);
        QApplication::processEvents();

        if (c != 0) // Do not verify first 32 bytes.
        {
            QByteArray chunk = data.mid(c * 1024, 1024);
            prog.chipVerify(chunk, c * 1024);
        }
        else
        {
            QByteArray chunk = data.mid(64, 1024 - 64);
            prog.chipVerify(chunk, 64);
        }

        if (prog.getStatus() != QLpcProg::StatusNoError)
        {
            QMessageBox::critical(this, tr("Error"), tr("Verify failed."));

            return;
        }
    }

    prog.deinit();

    ui->statusbar->clearMessage();
    QMessageBox::information(this, tr("Info"), tr("Chip firmware is successfully verified with file."));
}

void QAppMainWindow::fileDecompile(const QString &file)
{
    Q_UNUSED(file);
    // Read file

    // decompile.
}
