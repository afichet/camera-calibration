#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>

#include "imagemodel.h"

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
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void openFile(const QString& filename);

  private:
    void dropEvent(QDropEvent* event);
    void dragEnterEvent(QDragEnterEvent* ev);

  private slots:
    void on_action_Open_triggered();
    void on_actionExport_coordinates_triggered();
    void on_action_Save_areas_triggered();
    void on_actionSave_correction_matrix_triggered();

    void on_showMacbeth_toggled(bool checked);
    void on_sliderInnerMarginX_valueChanged(int value);
    void on_sliderInnerMarginY_valueChanged(int value);
    void on_exposureValue_valueChanged(double value);

    void onImageLoaded(int width, int height);
    void onMatrixLoaded(const std::array<float, 9>& matrix);

    void on_m00_textChanged(const QString& arg1);
    void on_m01_textChanged(const QString& arg1);
    void on_m02_textChanged(const QString& arg1);

    void on_m10_textChanged(const QString& arg1);
    void on_m11_textChanged(const QString& arg1);
    void on_m12_textChanged(const QString& arg1);

    void on_m20_textChanged(const QString& arg1);
    void on_m21_textChanged(const QString& arg1);
    void on_m22_textChanged(const QString& arg1);

    void on_buttonFit_clicked();

    void on_demosaicingMode_currentIndexChanged(const QString& arg1);

  private:
    Ui::MainWindow* ui;
    ImageModel      _model;
    QProgressBar*   _statusBarProgress;
};
#endif   // MAINWINDOW_H
