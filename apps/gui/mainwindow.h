#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>

#include "macbethmodel.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow: public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();

  void openFile(const QString &filename);

private:
  void dropEvent(QDropEvent *event);
  void dragEnterEvent(QDragEnterEvent *ev);

private slots:
  void on_action_Open_triggered();
  void on_action_Save_areas_triggered();

  void on_sliderInnerMarginX_valueChanged(int value);
  void on_sliderInnerMarginY_valueChanged(int value);
  void on_exposureValue_valueChanged(double value);


private:
  Ui::MainWindow *ui;
  MacbethModel    _model;
  QProgressBar *  _statusBarProgress;
};
#endif   // MAINWINDOW_H
