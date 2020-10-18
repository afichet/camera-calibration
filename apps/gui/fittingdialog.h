#ifndef FITTINGDIALOG_H
#define FITTINGDIALOG_H

#include <QDialog>
#include <QFutureWatcher>

#include "imagemodel.h"
#include "macbethreferencemodel.h"
#include "macbethmeasuredmodel.h"

namespace Ui
{
class FittingDialog;
}

class FittingDialog: public QDialog
{
  Q_OBJECT

  typedef struct
  {
    size_t       n_patches;
    const float *reference_patches;
    const float *measured_patches;
    const bool * selected_patches;
  } fit_params;

  typedef struct
  {
    std::vector<float> illuminantSPD;
    int                firstWavelength;
  } illuminant_data;


  typedef struct
  {
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
    int                firstWavelength;
  } cmf_data;

public:
  explicit FittingDialog(ImageModel *model, QWidget *parent = nullptr);
  ~FittingDialog();

  const std::array<float, 9> &getFitMatrix() const { return _fitMatrix; }

protected:
  void showEvent(QShowEvent *event);

  static void measure(float *pParameters, float *pMeasurements, int n_parameters, int n_measurements, void *pUserParam);
  void        fit();

protected slots:
  void initModels();

private slots:
  void on_applyMatrix_toggled(bool checked);

  void on_colorMatchingFunctions_currentIndexChanged(int index);

  void on_illuminant_currentIndexChanged(int index);

  void on_minThreshold_valueChanged(double arg1);

  void on_maxThreshold_valueChanged(double arg1);

private:
  Ui::FittingDialog *ui;

  MacbethMeasuredModel  _measured;
  MacbethReferenceModel _reference;

  ImageModel *_image;

  QFutureWatcher<void> *_processWatcher;

  std::array<float, 9> _fitMatrix;

  std::vector<illuminant_data> _userIlluminants;
  std::vector<cmf_data>        _userCMFs;
};

#endif   // FITTINGDIALOG_H
