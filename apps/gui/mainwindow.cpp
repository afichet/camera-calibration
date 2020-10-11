#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QUrl>
#include <QMimeData>
#include <QDragEnterEvent>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
  , _statusBarProgress(new QProgressBar(this))
{
  ui->setupUi(this);
  ui->graphicsView->setModel(&_model);

  QSizePolicy p = QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Ignored);
  _statusBarProgress->setMaximumWidth(150);
  _statusBarProgress->setSizePolicy(p);

  statusBar()->insertPermanentWidget(0, _statusBarProgress);

  on_sliderInnerMarginX_valueChanged(ui->sliderInnerMarginX->value());
  on_sliderInnerMarginY_valueChanged(ui->sliderInnerMarginY->value());
  connect(&_model, SIGNAL(exposureChanged(double)), ui->exposureValue, SLOT(setValue(double)));
  connect(&_model, SIGNAL(processProgress(int)), _statusBarProgress, SLOT(setValue(int)));
  connect(&_model, SIGNAL(loadingMessage(QString const &)), statusBar(), SLOT(showMessage(QString const &)));

  connect(&_model, SIGNAL(matrixActivationStateChanged(bool)), ui->activeMatrix, SLOT(setChecked(bool)));
  connect(ui->activeMatrix, SIGNAL(toggled(bool)), &_model, SLOT(setMatrixActive(bool)));

  connect(
      &_model,
      SIGNAL(matrixChanged(const std::array<float, 9> &)),
      this,
      SLOT(onMatrixChanged(const std::array<float, 9> &)));
}


MainWindow::~MainWindow()
{
  delete ui;
}


void MainWindow::openFile(const QString &filename)
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


void MainWindow::on_action_Save_areas_triggered()
{
  QString filename = QFileDialog::getSaveFileName(this, tr("Save patches"), "", tr("CSV (*.csv)"));

  if (filename.size() != 0)
  {
    _model.savePatches(filename);
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

void MainWindow::onMatrixChanged(const std::array<float, 9> &matrix)
{
  ui->m00->setText(QString::number(matrix[0]));
  ui->m01->setText(QString::number(matrix[1]));
  ui->m02->setText(QString::number(matrix[2]));
  ui->m10->setText(QString::number(matrix[3]));
  ui->m11->setText(QString::number(matrix[4]));
  ui->m12->setText(QString::number(matrix[5]));
  ui->m20->setText(QString::number(matrix[6]));
  ui->m21->setText(QString::number(matrix[7]));
  ui->m22->setText(QString::number(matrix[8]));
}

void MainWindow::on_m00_textChanged(const QString &arg1)
{
  std::array<float, 9> prevMatrix = _model.getCorrectionMatrix();
  prevMatrix[0]                   = arg1.toFloat();
  _model.setMatrix(prevMatrix);
}

void MainWindow::on_m01_textChanged(const QString &arg1)
{
  std::array<float, 9> prevMatrix = _model.getCorrectionMatrix();
  prevMatrix[1]                   = arg1.toFloat();
  _model.setMatrix(prevMatrix);
}

void MainWindow::on_m02_textChanged(const QString &arg1)
{
  std::array<float, 9> prevMatrix = _model.getCorrectionMatrix();
  prevMatrix[2]                   = arg1.toFloat();
  _model.setMatrix(prevMatrix);
}

void MainWindow::on_m10_textChanged(const QString &arg1)
{
  std::array<float, 9> prevMatrix = _model.getCorrectionMatrix();
  prevMatrix[3]                   = arg1.toFloat();
  _model.setMatrix(prevMatrix);
}

void MainWindow::on_m11_textChanged(const QString &arg1)
{
  std::array<float, 9> prevMatrix = _model.getCorrectionMatrix();
  prevMatrix[4]                   = arg1.toFloat();
  _model.setMatrix(prevMatrix);
}

void MainWindow::on_m12_textChanged(const QString &arg1)
{
  std::array<float, 9> prevMatrix = _model.getCorrectionMatrix();
  prevMatrix[5]                   = arg1.toFloat();
  _model.setMatrix(prevMatrix);
}

void MainWindow::on_m20_textChanged(const QString &arg1)
{
  std::array<float, 9> prevMatrix = _model.getCorrectionMatrix();
  prevMatrix[6]                   = arg1.toFloat();
  _model.setMatrix(prevMatrix);
}

void MainWindow::on_m21_textChanged(const QString &arg1)
{
  std::array<float, 9> prevMatrix = _model.getCorrectionMatrix();
  prevMatrix[7]                   = arg1.toFloat();
  _model.setMatrix(prevMatrix);
}

void MainWindow::on_m22_textChanged(const QString &arg1)
{
  std::array<float, 9> prevMatrix = _model.getCorrectionMatrix();
  prevMatrix[8]                   = arg1.toFloat();
  _model.setMatrix(prevMatrix);
}
