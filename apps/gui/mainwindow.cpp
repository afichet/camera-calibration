#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>

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


void MainWindow::on_action_Open_triggered()
{
  QString filename = QFileDialog::getOpenFileName(this, tr("Open Image"), "", tr("Image (*.tiff *.tif *.exr)"));

  if (filename.size() != 0)
  {
    _model.openFile(filename);
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
