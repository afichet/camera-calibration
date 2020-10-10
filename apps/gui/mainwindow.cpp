#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QUrl>
#include <QMimeData>
#include <QDragEnterEvent>

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ui->graphicsView->setModel(&_model);

  on_sliderInnerMarginX_valueChanged(ui->sliderInnerMarginX->value());
  on_sliderInnerMarginY_valueChanged(ui->sliderInnerMarginY->value());
}

MainWindow::~MainWindow()
{
  delete ui;
}

void MainWindow::openFile(const QString& filename)
{
  _model.openFile(filename);
}

void MainWindow::dropEvent(QDropEvent *ev)
{
    QList<QUrl> urls = ev->mimeData()->urls();

    if (!urls.empty())
    {
        QString fileName = urls[0].toString();
        QString startFileTypeString =
            #ifdef _WIN32
                "file:///";
            #else
                "file://";
            #endif

        if (fileName.startsWith(startFileTypeString))
        {
            fileName = fileName.remove(0, startFileTypeString.length());
            openFile(fileName);
        }
    }
}


void MainWindow::dragEnterEvent(QDragEnterEvent *ev)
{
    ev->acceptProposedAction();
}

void MainWindow::on_action_Open_triggered()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image (*.tiff *.tif *.exr)"));

  if (filename.size() != 0)
  {
    openFile(filename);
  }
}

void MainWindow::on_sliderInnerMarginX_valueChanged(int value)
{
  float p = (float)(value - ui->sliderInnerMarginX->minimum())
            / (float)(ui->sliderInnerMarginX->maximum() - ui->sliderInnerMarginX->minimum());
  _model.setInnerMarginX(p);
}

void MainWindow::on_sliderInnerMarginY_valueChanged(int value)
{
  float p = (float)(value - ui->sliderInnerMarginY->minimum())
            / (float)(ui->sliderInnerMarginY->maximum() - ui->sliderInnerMarginY->minimum());
  _model.setInnerMarginY(p);
}

void MainWindow::on_exposureValue_valueChanged(double value)
{
  _model.setExposure(value);
}


void MainWindow::on_action_Save_areas_triggered()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("Save patches"), "", tr("CSV (*.csv)"));

  if (filename.size() != 0)
  {
    _model.savePatches(filename);
  }
}
